struct Test_FrizzModifier
(
	function TestCleanup =
	(
		delete $objects
	),
	
	function SetFrizzSingleStepRamp diagData diagInterpData value:1 =
	(
		diagData = #(
			-0.5, value,
			0, value,
			1.5, value )
		
		diagInterpData = #( 0, 0, 0, 0, 0, 0 )
	),
	
	function Test_Frizz =
	(
		EUnit.SkipBeforeMaxVersion 2017

		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on
		max modify mode

		select distributionObject

		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 2
		guidesFromSurfaceModifier.numSegs = 10
		guidesFromSurfaceModifier.randomness = 0
		guidesFromSurfaceModifier.count = 10
		modPanel.addModToSelection guidesFromSurfaceModifier

		-- Get verts before 
		local vertexPositionsBefore = OxGetVertices distributionObject true
		
		local frizzInstance = Ox_Strand_Frizz()
		SetFrizzSingleStepRamp (frizzInstance.diagData) (frizzInstance.diagInterpData)
		modPanel.addModToSelection frizzInstance
		--frizzInstance.amount = 10

		-- And after modifier
		local vertexPositionsAfter = OxGetVertices distributionObject true
		
		for i = 1 to vertexPositionsBefore.count  do
		(
			-- Ignore root vertices
			if ( mod ( i - 1 ) guidesFromSurfaceModifier.numSegs ) != 0 then
			(
				EAssert.ArePointsNotNearEqual vertexPositionsBefore[i] vertexPositionsAfter[i]
			)
		)
	),
	
	-- When hair from strips is used, frizz modifier should not change strand orientations as the underlying mesh is rotated
	function Test_FrizzOrientationOnChangingMeshPlane =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode
		local editPolyModifier = Edit_Poly()
		modPanel.addModToSelection editPolyModifier
		
		local hairObjectInstance = HairFromMeshStripsObject viewPercentage:100 isSelected:on distributionMethod:0
		hairObjectInstance.AddStripObject distributionObject
		
		local frizzInstance = Ox_Strand_Frizz()
		modPanel.addModToSelection frizzInstance
		
		local verticesBeforeRotation = OxGetVertices hairObjectInstance true

		select distributionObject
		subobjectLevel = 4
		editPolyModifier.SetSelection #Face #{1}
		editPolyModifier.SetOperation #Transform
		
		local rotationQuat = quat 0 0 -0.778164 0.628061
		editPolyModifier.RotateSelection rotationQuat
		editPolyModifier.Commit()
		
		local verticesAfterRotation = OxGetVertices hairObjectInstance true
		for i = 1 to verticesBeforeRotation.count  do
		(
			local rotatedBeforeVertex = verticesBeforeRotation[i] * rotationQuat
			EAssert.ArePointsNearEqual rotatedBeforeVertex verticesAfterRotation[i]
		)
		
		-- TODO:
		/*local rotation2 = quat -0.923085 0 0 0.384596
		editPolyModifier.RotateSelection rotation2
		editPolyModifier.Commit()
		
		local verticesAfterRotation2 = OxGetVertices hairObjectInstance true
		for i = 1 to verticesBeforeRotation.count  do
		(
			local rotatedBeforeVertex = verticesBeforeRotation[i] * rotationQuat * rotation2
			EAssert.ArePointsNearEqual rotatedBeforeVertex verticesAfterRotation2[i]
		)*/
	),
	
	function Test_FrizzSegmentOrientationChangesSmoothlyWithHairFromMeshStripsBendBeyond90Degrees =
	(
		local distributionObject = Plane length:50 width:25 lengthsegs:2 widthsegs:1 mapcoords:on transform:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [0,0,0]) isSelected:on
		max modify mode
		local editPolyModifier = Edit_Poly()
		modPanel.addModToSelection editPolyModifier
		
		local hairObjectInstance = HairFromMeshStripsObject viewPercentage:100 isSelected:on distributionMethod:0
		hairObjectInstance.AddStripObject distributionObject
		
		local frizzInstance = Ox_Strand_Frizz amount:50
		modPanel.addModToSelection frizzInstance
		
		select distributionObject
		subobjectLevel = 1
		editPolyModifier.Select #Vertex #{3..4}
		editPolyModifier.SetOperation #Transform
		editPolyModifier.MoveSelection [0,120,0] parent:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [0,0,0])
		editPolyModifier.Commit()
		
		editPolyModifier.SetSelection #Vertex #{}
		editPolyModifier.Select #Vertex #{1..2}
		editPolyModifier.MoveSelection [0,10,0] parent:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [0,0,0])
		editPolyModifier.Commit()
		
		local tipPositionsBeforeMove = TestUtilities.GetTipPositions hairObjectInstance
		editPolyModifier.SetSelection #Vertex #{}
		editPolyModifier.Select #Vertex #{1..2}
		editPolyModifier.MoveSelection [0,2,0] parent:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [0,0,0])
		editPolyModifier.Commit()
		local tipPositionsAfterMove = TestUtilities.GetTipPositions hairObjectInstance
		
		-- If the tips didn't jump they should not be further apart than the distance by which we moved the vertices
		EAssert.ArePointsAllNearEqual tipPositionsBeforeMove tipPositionsAfterMove useDistance:true epsilon:3
	),
	
	function Test_FrizzSegmentOrientationChangesSmoothlyWithHairFromMeshStripsBendInMiddle =
	(
		local distributionObject = Plane length:50 width:25 lengthsegs:4 widthsegs:1 mapcoords:on isSelected:on
		max modify mode
		local editPolyModifier = Edit_Poly()
		modPanel.addModToSelection editPolyModifier
		
		local hairObjectInstance = HairFromMeshStripsObject viewPercentage:100 isSelected:on distributionMethod:0
		hairObjectInstance.AddStripObject distributionObject
		
		local frizzInstance = Ox_Strand_Frizz amount:50
		modPanel.addModToSelection frizzInstance
		
		select distributionObject
		subobjectLevel = 1
		editPolyModifier.Select #Vertex #{3..4}
		editPolyModifier.SetOperation #Transform
		editPolyModifier.RotateSelection  (quat 0 0 -0.698858 0.71526) axis:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [0,-12.5,0])
		editPolyModifier.Commit()
		
		local tipPositionsBeforeMove = TestUtilities.GetTipPositions hairObjectInstance
		--editPolyModifier.SetSelection #Vertex #{}
		--editPolyModifier.Select #Vertex #{3..4}
		editPolyModifier.SetOperation #Transform
		editPolyModifier.RotateSelection  (quat 0 0 -0.0422949 0.999105) axis:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [0,-12.5,0])
		editPolyModifier.Commit()
		local tipPositionsAfterMove = TestUtilities.GetTipPositions hairObjectInstance
		
		-- If the tips didn't jump they should not be further apart than the distance by which we moved the vertices
		EAssert.ArePointsAllNearEqual tipPositionsBeforeMove tipPositionsAfterMove useDistance:true epsilon:1
	),
	
	function OxGetTransforms strandObject =
	(
		local result = #()
		for i = 1 to (OxGetStrandCount strandObject) do
		(
			append result (OxGetStrandTransform strandObject (i - 1))
		)
		
		result
	),
	
	function AssertTransformsAlmostDontChangeBetweenFrames strandObject firstFrame:1 lastFrame:10 =
	(
		-- Changes in strand points between frames shouldn't be significant
		sliderTime = 0f
		local previousFrameTransforms = OxGetTransforms strandObject
		for frame = firstFrame to lastFrame do
		(
			local currentFrameTransforms = OxGetTransforms strandObject
			EAssert.AreTransformsAllNearEqual previousFrameTransforms currentFrameTransforms epsilon:0.5
			previousFrameTransforms = currentFrameTransforms
			sliderTime += 1f
		)
	),
	
	function AssertPointsAlmostDontChangeBetweenFrames strandObject firstFrame:1 lastFrame:10 epsilon:1.1 =
	(
		local largestChange = 0
		
		-- Changes in strand points between frames shouldn't be significant
		sliderTime = 0f
		local previousFrameVertices = OxGetVertices strandObject true
		for frame = firstFrame to lastFrame do
		(
			local currentFrameVertices = OxGetVertices strandObject true
			local largestDifference = EAssert.ArePointsAllNearEqual previousFrameVertices currentFrameVertices useDistance:true epsilon:epsilon
			
			if largestDifference > largestChange then largestChange = largestDifference
			previousFrameVertices = currentFrameVertices
			sliderTime += 1f
		)
		
		largestChange
	),
	
	function Test_FrizzFlickeringWhenOnTopOfStrandClustering =
	(
		local lastFrame = 15
		-- Create an animated deforming box
		local distributionObject = Box length:30 width:140 height:8 lengthsegs:1 widthsegs:5 heightsegs:1 mapcoords:on isSelected:on
		max modify mode
		local bendModifier = Bend BendAxis:0
		set animate on
		sliderTime = lastFrame * 1f
		bendModifier.BendAngle = 14
		bendModifier.BendDir = 43
		set animate off
		modPanel.addModToSelection bendModifier
		
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:6
		modPanel.addModToSelection (Ox_Surface_Comb())
		modPanel.addModToSelection (Ox_Strand_Frizz amount:30)
		modPanel.addModToSelection (Ox_Hair_From_Guides method:4)
		modPanel.addModToSelection (Ox_Hair_Clustering genCount:10)
		modPanel.addModToSelection (Ox_Strand_Frizz amount:30)
		
		-- Changes in strand points between frames shouldn't be significant
		AssertPointsAlmostDontChangeBetweenFrames distributionObject firstFrame:1 lastFrame:lastFrame
	),
	
	function Test_FrizzFlickeringWithNearZeroLengthHairs =
	(
		local distributionObject = Sphere radius:30 mapcoords:on segs:10 mapcoords:on isSelected:on
		max modify mode
		local bendModifier = Bend BendAxis:0
		set animate on
		sliderTime = 15 * 1f
		bendModifier.BendAngle = 14
		bendModifier.BendDir = 43
		set animate off
		modPanel.addModToSelection bendModifier
		
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection method:2 pointCount:6 length:5
		modPanel.addModToSelection (Ox_Strand_Curling form:0 magn:7)
		modPanel.addModToSelection (Ox_Strand_Length randomize:1 value:0.01 minimumValue:0.01)
		modPanel.addModToSelection (Ox_Hair_From_Guides method:4)
		modPanel.addModToSelection (Ox_Strand_Frizz amount:30)
		modPanel.addModToSelection (Ox_Strand_Frizz amount:30)
			
		AssertTransformsAlmostDontChangeBetweenFrames distributionObject firstFrame:1 lastFrame:15
		AssertPointsAlmostDontChangeBetweenFrames distributionObject firstFrame:1 lastFrame:15
	),
	
	-- Yet another test for smooth generation of per-vertex transforms to avoid flickering of frizz operator when placed on top of animated hair
	function Test_FrizzFlickeringWithGravityAndCurling =
	(
		max modify mode
		local distributionObject = Sphere radius:30 mapcoords:on segs:10 mapcoords:on isSelected:on
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection method:2 pointCount:40 length:5 count:100
		modPanel.addModToSelection (Ox_Strand_Curling form:1 magn:7 phase:2 groupID:0)
		local gravityModifierInstance = Ox_Strand_Gravity gravForce:2
		modPanel.addModToSelection gravityModifierInstance
		modPanel.addModToSelection (Ox_Strand_Frizz amount:30)
		
		set animate on
		sliderTime = 0 * 1f
		gravityModifierInstance.gravForce = 2.5
		sliderTime = 30 * 1f
		gravityModifierInstance.gravForce = 3
		set animate off
			
		AssertPointsAlmostDontChangeBetweenFrames distributionObject firstFrame:1 lastFrame:30 epsilon:2
	),
	
	function Test_StackedFrizzFlickering =
	(
		EUnit.SkipBeforeMaxVersion 2017

		local distributionObject = Plane length:10 width:10 lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode
		local gfm = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection method:6 pointCount:100 guideLength:150
		-- TODO: This test only passes when surface tangent compute is set to (deprecated) "First Edge" method. Make it work with others too.
		gfm.surfaceTangentComputeMethod = 1
		modPanel.addModToSelection (Noisemodifier scale:20 strength:[10,10,10] animate:on)
		
		-- Increase this number to icrease the sensitivity of frizz to initial strand movements
		local frizzModifierCount = 3
		
		for i = 1 to frizzModifierCount do
		(
			local frizz1 = Ox_Strand_Frizz amount:20 scale:30 outPercent:0
			SetFrizzSingleStepRamp (frizz1.diagData) (frizz1.diagInterpData)
			modPanel.addModToSelection frizz1
		)
		
		--  We expect shape of strands to change but smoothly
		AssertPointsAlmostDontChangeBetweenFrames distributionObject firstFrame:1 lastFrame:9 epsilon:2
	)
)

EUnit.AddTestClass Test_FrizzModifier
ok
