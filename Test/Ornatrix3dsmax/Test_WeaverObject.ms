struct Test_WeaverObject
(
	function TestCleanup =
	(
		delete $objects
	),
	
	function Test_TransferCurveToTargetMesh =
	(
		OxResetClassParameters (Sphere())
		OxResetClassParameters (WeaverObject())
		OxResetClassParameters (line())
		delete $objects
			
		local sphereRadius = 10
		local sphereInstance = Sphere radius:sphereRadius mapcoords:on
		local weaver = WeaverObject targetGeometry:sphereInstance uvSpaceScale:1000 curveSampleCount:10
		
		local spline1 = line()
		addNewSpline spline1
		addKnot spline1 1 #corner #line [0,5,0]
		addKnot spline1 1 #corner #line [10,5,0]
		updateShape spline1
		
		append weaver.sourceCurves spline1
		
		-- There should be one strand which loops around the circumference of the sphere
		EAssert.AreEqual 1 (OxGetStrandCount weaver)
		
		clearSelection()
		select sphereInstance
		
		local strandVertices = OxGetVertices weaver true
		
		-- We can test the curve by making sure all points are equidistant from the center
		local previousPoint = strandVertices[1]
		for i = 2 to (strandVertices.count) do
		(
			EAssert.ArePointsNotNearEqual previousPoint strandVertices[i]
			EAssert.AreNearEqual sphereRadius (distance strandVertices[i] [0,0,0]) Epsilon:0.5
			
			previousPoint = strandVertices[i]
		)
	),
	
	function Test_MultiplePatternsOnTargetMesh =
	(
		local targetMesh = Plane pos:[20,0,0] length:20 width:20 isSelected:on lengthsegs:1 widthsegs:1
		local weaver = WeaverObject targetGeometry:targetMesh
		local pattern1 = WeavePatternObject referenceShapeWidth:10 referenceShapeHeight:10
		
		local spline1 = line()
		addNewSpline spline1
		addKnot spline1 1 #corner #line [-5,0,0]
		addKnot spline1 1 #corner #line [5,0,0]
		updateShape spline1
		
		local spline2 = line()
		addNewSpline spline2
		addKnot spline2 1 #corner #line [0,5,0]
		addKnot spline2 1 #corner #line [0,-5,0]
		updateShape spline2
		
		append pattern1.sourceCurves spline1
		append pattern1.sourceCurves spline2
		append weaver.sourceCurves pattern1
		
		-- So far we should have 2 guides from the single pattern generated on the target mesh
		EAssert.AreEqual 2 (OxGetStrandCount weaver)
		
		-- Increase number of patterns per edge to 2, so we should have 4 overall patterns
		weaver.selectedEdgesDivisionMethod = 2
		weaver.selectedEdgesDivisionCount = 2
		EAssert.AreEqual 4 (OxGetStrandCount weaver)
		
		-- The resulting vertices of the pattern should encompass the plane
		local strandVertices = OxGetVertices weaver true
		local boundingBox = TestUtilities.GetPointsBoundingBox strandVertices
		EAssert.ArePointsNearEqual [10,-10,0] boundingBox[1]
		EAssert.ArePointsNearEqual [30,10,0] boundingBox[2]
	),
	
	function Test_ReferenceMeshWithLengthDependentPatterns =
	(
		local targetMesh = Sphere radius:10 mapcoords:on
		local referenceMesh = Plane pos:[15,15,0] length:20 width:20 isSelected:on lengthsegs:1 widthsegs:1
		local weaver = WeaverObject targetGeometry:targetMesh referenceGeometry:referenceMesh uvSpaceScale:3000 selectedEdgesDivisionMethod:3 selectedEdgesDivisionLength:10
		local pattern1 = WeavePatternObject referenceShapeWidth:10 referenceShapeHeight:10
		
		local spline1 = line()
		addNewSpline spline1
		addKnot spline1 1 #corner #line [-5,0,0]
		addKnot spline1 1 #corner #line [5,0,0]
		updateShape spline1
		
		local spline2 = line()
		addNewSpline spline2
		addKnot spline2 1 #corner #line [0,5,0]
		addKnot spline2 1 #corner #line [0,-5,0]
		updateShape spline2
		
		append pattern1.sourceCurves spline1
		append pattern1.sourceCurves spline2
		append weaver.sourceCurves pattern1
		
		-- We should have 3 guides initially
		EAssert.AreEqual 3 (OxGetStrandCount weaver)
		
		-- Increasing the sphere sould increase the number of patterns and thus guides because they depend on edge length
		targetMesh.radius = 20
		EAssert.IsGreaterThan 3 (OxGetStrandCount weaver)
	),
	
	function CreateWeaverWithSinglePatternStraightGuide patternSampleCount:10 =
	(
		local targetMesh = Plane length:20 width:20 isSelected:on lengthsegs:1 widthsegs:1
		local weaver = WeaverObject targetGeometry:targetMesh
		local pattern1 = WeavePatternObject referenceShapeWidth:10 referenceShapeHeight:10 curveSampleCount:patternSampleCount
		
		local spline1 = line()
		addNewSpline spline1
		addKnot spline1 1 #corner #line [-5,0,0]
		addKnot spline1 1 #corner #line [5,0,0]
		updateShape spline1
		
		append pattern1.sourceCurves spline1
		append weaver.sourceCurves pattern1
		
		EAssert.AreEqual 1 (OxGetStrandCount weaver)
		
		weaver
	),
	
	function Test_GlobalPatternRotation =
	(
		local weaver = CreateWeaverWithSinglePatternStraightGuide()
		
		-- With default pattern rotation points should be going West-East
		weaver.selectedFacePatternRotation = 0
		local strandVertices = OxGetVertices weaver true
		EAssert.ArePointsNearEqual [-10,0,0] strandVertices[1] useDistance:true epsilon:0.001
		EAssert.ArePointsNearEqual [10,0,0] strandVertices[strandVertices.Count] useDistance:true epsilon:0.001
		
		-- With rotated pattern points should be going North-South
		weaver.selectedFacePatternRotation = 1
		strandVertices = OxGetVertices weaver true
		EAssert.ArePointsNearEqual [0,10,0] strandVertices[1] useDistance:true epsilon:0.001
		EAssert.ArePointsNearEqual [0,-10,0] strandVertices[strandVertices.Count] useDistance:true epsilon:0.001
	),
	
	-- Tests that all resulting weaver points are generated in separate locations to avoid various artifacts associated with two consecutive points being in the same place
	function Test_NoTwoPointsAreInSamePlaceForMultiplePatterns =
	(
		local weaver = CreateWeaverWithSinglePatternStraightGuide patternSampleCount:10
		
		-- Use two patterns per edge so we generate multiple straight curves joined together
		weaver.selectedEdgesDivisionCount = 2
		
		-- No two points in resulting guides should lie in the same place (overlap)
		local strandVertices = OxGetVertices weaver true
		for i = 1 to strandVertices.count do
		(
			for j = 1 to strandVertices.count do
			(
				if i != j then EAssert.ArePointsNotNearEqual strandVertices[i] strandVertices[j] useDistance:true epsilon:0.001
			)
		)
	),
	
	function SetupWeaverWithTwoDisjointedReferenceFaces =
	(
		-- Create a box without a top
		local boxMesh = Box lengthsegs:1 widthsegs:1 heightsegs:1 length:10 width:10 height:10 mapcoords:on isSelected:on
		max modify mode
		local editPoly = Edit_Poly ()
		modPanel.addModToSelection editPoly
		subobjectLevel = 4
		editPoly.SetSelection #Face #{}
		editPoly.Select #Face #{2}
		editPoly.ButtonOp #DeleteFace
		subobjectLevel = 0
		
		-- Unwrap the box such that the sides are spread over one long element and the bottom is a separate element
		local unwrapUvw = Unwrap_UVW ()
		modPanel.addModToSelection unwrapUvw
		subobjectLevel = 3
		unwrapUvw.unwrap6.selectFacesByNode #{1} boxMesh
		unwrapUvw.unwrap2.MoveSelected [0,-1.5,0]
		unwrapUvw.unwrap6.selectFacesByNode #{2} boxMesh
		unwrapUvw.unwrap2.MoveSelected [2,0,0]
		unwrapUvw.unwrap6.selectFacesByNode #{5} boxMesh
		unwrapUvw.unwrap2.MoveSelected [1,0,0]
		unwrapUvw.unwrap6.selectFacesByNode #{3} boxMesh
		unwrapUvw.unwrap2.MoveSelected [-1,0,0]
		unwrapUvw.unwrap6.selectFacesByNode #{1..5} boxMesh
		unwrapUvw.unwrap2.ScaleSelectedCenter 0.25 0
		unwrapUvw.unwrap2.MoveSelected [-0.5 + (1.5 / 20),0.5,0]
		
		-- For some reason we need to add Turbosmooth to make guides appear as expected
		local turbosmoothMod = TurboSmooth iterations:1
		modPanel.addModToSelection turbosmoothMod
		
		-- Create weaver with a criss-cross pattern
		local Epsilon = 0.001
		local referenceMesh = Plane length:(2.5 - Epsilon) width:(10 - Epsilon) pos:[5,7.75,0] lengthsegs:1 widthsegs:1 isSelected:on
		local bottomReferenceMesh = Plane length:(2.5 - Epsilon) width:(2.5 - Epsilon) lengthsegs:1 widthsegs:1 pos:[3.75,4,0]
		local referenceMeshEditPoly = Edit_Poly ()
		modPanel.addModToSelection referenceMeshEditPoly
		referenceMeshEditPoly.SetOperation #Attach
		referenceMeshEditPoly.Attach bottomReferenceMesh editPolyNode:referenceMesh
		
		local weaver = WeaverObject targetGeometry:boxMesh referenceGeometry:referenceMesh uvSpaceScale:1000
		local pattern1 = WeavePatternObject referenceShapeWidth:10 referenceShapeHeight:10 curveSampleCount:10
		
		local spline1 = line()
		addNewSpline spline1
		addKnot spline1 1 #corner #line [-5,0,0]
		addKnot spline1 1 #corner #line [5,0,0]
		updateShape spline1
		
		local spline2 = line()
		addNewSpline spline2
		addKnot spline2 1 #corner #line [0,5,0]
		addKnot spline2 1 #corner #line [0,-5,0]
		updateShape spline2
		
		append pattern1.sourceCurves spline1
		append pattern1.sourceCurves spline2
		append weaver.sourceCurves pattern1
		
		#(weaver, turbosmoothMod, pattern1, spline1, spline2)
	),
	
	function Test_PatternsConnectedAcrossCloseWorldSpaceEdges =
	(
		local fixture = SetupWeaverWithTwoDisjointedReferenceFaces()
		local weaver = fixture[1]
		local turbosmoothMod = fixture[2]
		local pattern1 = fixture[3]
		local spline1 = fixture[4]
		
		-- Select the top and bottom edges of the sides and set there to be 4 patterns running along them
		select weaver
		subobjectLevel = 2
		weaver.SetEdgeSelection #{2, 4}
		weaver.selectedEdgesDivisionMethod = 2
		weaver.selectedEdgesDivisionCount = 4
		
		-- If all is well we should have 3 guides generated overall, one across the side segments, and two criss-crossing at the bottom and going up towards the top
		EAssert.AreEqual 3 (OxGetStrandCount weaver)
		
		-- Test that results are same without turbosmooth
		turbosmoothMod.enabled = off
		weaver.boundaryEdgesSampleCount = 30
		EAssert.AreEqual 3 (OxGetStrandCount weaver)
		
		deleteItem pattern1.sourceCurves 1
		EAssert.AreEqual 3 (OxGetStrandCount weaver)
		append pattern1.sourceCurves spline1
		EAssert.AreEqual 3 (OxGetStrandCount weaver)
		delete spline1
	),
	
	function Test_PatternsConnectedAcrossCloseWorldSpaceEdges2 =
	(
		local fixture = SetupWeaverWithTwoDisjointedReferenceFaces()
		local weaver = fixture[1]
		
		-- Set the number of patterns in the botom face to 4 (2 along each side)
		-- Set the number of patterns in the side faces to 8 horzontally, so they line up with the 4 patterns at the bottom
		select weaver
		subobjectLevel = 2
		weaver.SetEdgeSelection #{5, 6, 7, 8}
		weaver.selectedEdgesDivisionMethod = 2
		weaver.selectedEdgesDivisionCount = 2
		weaver.SetEdgeSelection #{2, 4}
		weaver.selectedEdgesDivisionMethod = 2
		weaver.selectedEdgesDivisionCount = 8
		
		-- If patterns line up we should end up with 5 guides overall
		EAssert.AreEqual 5 (OxGetStrandCount weaver)
	),
	
	function Test_PatternsConnectedAcrossCloseWorldSpaceEdgesWithSingleEdgeSpecification =
	(
		local fixture = SetupWeaverWithTwoDisjointedReferenceFaces()
		local weaver = fixture[1]
		
		-- Same as Test_PatternsConnectedAcrossCloseWorldSpaceEdges2 but this time only set one edge, the rest should be automatic
		select weaver
		subobjectLevel = 2
		weaver.SetEdgeSelection #{4}
		weaver.selectedEdgesDivisionMethod = 2
		weaver.selectedEdgesDivisionCount = 8
		
		-- If patterns line up we should end up with 5 guides overall
		EAssert.AreEqual 5 (OxGetStrandCount weaver)
	),
	
	function Test_PatternsConnectedAcrossCloseWorldSpaceEdgesWithSingleEdgeSpecification2 =
	(
		local fixture = SetupWeaverWithTwoDisjointedReferenceFaces()
		local weaver = fixture[1]
		
		-- Same as Test_PatternsConnectedAcrossCloseWorldSpaceEdges2 but this time only set one edge, the rest should be automatic
		select weaver
		subobjectLevel = 2
		weaver.SetEdgeSelection #{1}
		weaver.selectedEdgesDivisionMethod = 2
		weaver.selectedEdgesDivisionCount = 2
		weaver.SetEdgeSelection #{4}
		weaver.selectedEdgesDivisionMethod = 2
		weaver.selectedEdgesDivisionCount = 4
		
		-- If patterns line up we should end up with 5 guides overall
		EAssert.AreEqual 4 (OxGetStrandCount weaver)
	)
)

EUnit.AddTestClass Test_WeaverObject
ok
