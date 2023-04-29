struct Test_GroundStrandsModifier
(
	function TestCleanup =
	(
		delete $objects
		units.SystemType = #Centimeters
	),
	
	-- Test for defect 2266
	function Test_DeformingMeshCausesStrandRotation =
	(
		-- Create a plane with 1x1 segments so we have 4 vertices all together
		local distributionObject = Plane lengthsegs:1 widthsegs:4
		
		-- And make a copy of it for later on
		local distributionObject2 = copy distributionObject

		max modify mode

		-- Setup guides with surface comb
		select distributionObject

		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 2
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		-- Collapse the whole stack to get baked guides
		maxOps.CollapseNode distributionObject on
		
		-- Add ground strands modifier on top of the baked guides, set mesh copy as distribution mesh, and ground
		local groundStrandsModifierInstance = Ox_Ground_Strands()
		modPanel.addModToSelection groundStrandsModifierInstance
		groundStrandsModifierInstance.surface = distributionObject2
		groundStrandsModifierInstance.detachRoots = off
		
		-- Capture the vector going from tip to root of the third strand
		local guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		myGuides = ( guidesCopy.GetGuides() ).AsHair()
		guide3DirectionBeforeBend = (( myGuides.GetPointT 2 1 ) - ( myGuides.GetPointT 2 0 ))
		delete guidesCopy
		
		-- Add a bend modifier to the distribution mesh and beform it
		select distributionObject2
		local bendInstance = Bend BendAxis:0 BendAngle:100
		modPanel.addModToSelection bendInstance
		
		-- TODO: Without this test fails below (ground strands is not evaluated after bend modifier addition)
		bendInstance.BendAngle = 101
		local dummyMesh = distributionObject2.mesh
		
		-- Get the new direction of the third guide. If deformed properly, it should be different from previous direction
		guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		myGuides = ( guidesCopy.GetGuides() ).AsHair()
		guide3DirectionAfterBend = (( myGuides.GetPointT 2 1 ) - ( myGuides.GetPointT 2 0 ))
		delete guidesCopy
		
		EAssert.AreNotNearEqual 0.0f ( length( guide3DirectionAfterBend - guide3DirectionBeforeBend ) )
	),
	
	-- Test for defect 2286, #66
	function Test_FirstStrandIsGroundedCorrectly =
	(
		-- Create a plane with 1x1 segments so we have 4 vertices all together
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		
		-- And make a copy of it for later on
		local distributionObject2 = copy distributionObject
		max modify mode

		-- Setup guides with surface comb
		select distributionObject

		-- Add guides from surface, strand curling, and ground strands
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 10
		modPanel.addModToSelection guidesFromSurfaceModifier
		local strandCurlingModifierInstance = Ox_Strand_Curling()
		strandCurlingModifierInstance.magn = 10
		modPanel.addModToSelection strandCurlingModifierInstance
		local groundStrandsModifierInstance = Ox_Ground_Strands()
		modPanel.addModToSelection groundStrandsModifierInstance
		groundStrandsModifierInstance.surface = distributionObject2
		
		-- Capture the world-space tip position of first strand before grounding
		local guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		myGuides = ( guidesCopy.GetGuides() ).AsHair()
		tipPositionBeforeGrounding = (myGuides.GetPointT 0 9) * distributionObject.transform
		delete guidesCopy
		
		-- Ground strands and capture tip position after grounding
		groundStrandsModifierInstance.detachRoots = off
		guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		myGuides = ( guidesCopy.GetGuides() ).AsHair()
		tipPositionAfterGrounding = (myGuides.GetPointT 0 9) * distributionObject.transform
		delete guidesCopy
		
		EAssert.ArePointsNearEqual tipPositionBeforeGrounding tipPositionAfterGrounding
	),
	
	-- Tests that the rotations of strands do not "flip" when the distribution mesh deforms
	function Test_GroundedStrandsRotateProperlyWhenMeshDeforms =
	(
		OxResetClassParameters (Ox_Ground_Strands() )
		OxResetClassParameters (Ox_Guides_from_Shape() )
		
		-- Create a plane with 1x1 segments so we have 4 vertices all together
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		
		-- Apply edit poly modifier at first so grounding information will be valid
		select distributionObject
		max modify mode
		local editPolyInstance = Edit_Poly()
		modPanel.addModToSelection editPolyInstance
		
		-- Create a bent spline with two vertices
		local guideObject = splineShape()
		addnewSpline guideObject
		addKnot guideObject 1 #corner #curve [0,0,0]
		addKnot guideObject 1 #bezier #curve [-5,-5,10] [-5,-5,1] [5,5,1]
		updateShape guideObject
		
		select guideObject
		max modify mode

		-- Apply guides from shape and ground strands to the spline
		local guidesFromShapeModifier = Ox_Guides_from_Shape()
		modPanel.addModToSelection guidesFromShapeModifier
		
		local groundStrandsModifierInstance = Ox_Ground_Strands surface:distributionObject topoOrient:on
		modPanel.addModToSelection groundStrandsModifierInstance
		groundStrandsModifierInstance.detachRoots = off
		
		-- Add edit poly onto the distribution object and rotate its vertices
		select distributionObject
		max modify mode
		subobjectLevel = 1
		editPolyInstance.SetSelection #Vertex #{}
		editPolyInstance.Select #Vertex #{1..4}
		actionMan.executeAction 0 "50002"  -- Tools: Select and Rotate
		editPolyInstance.SetOperation #Transform
		editPolyInstance.RotateSelection  (quat 0 -0.709985 0 0.704216)
		undo on
		(
			editPolyInstance.Commit()
		)
		
		-- Bake the guides and check the tip point's position, it should have been rotated properly
		local guideVertices = OxGetVertices guideObject true
		
		EAssert.ArePointsNearEqual [-5,-5,10] guideVertices[10]
	),
	
	function Test_ReloadSceneWithHairFromGuides =
	(
		local distributionObject2 = Plane lengthsegs:1 widthsegs:1 length:20 width:20
		local distributionObject = Plane lengthsegs:1 widthsegs:1 length:10 width:10 isSelected:on
		
		max modify mode

		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:2)
		
		local groundStrandsModifierInstance = Ox_Ground_Strands surface:distributionObject2 topoOrient:on
		modPanel.addModToSelection groundStrandsModifierInstance
		groundStrandsModifierInstance.detachRoots = off
		
		local hairFromGuidesModifierInstance = Ox_Hair_From_Guides method:4 numSegs:2
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		local strandCount = OxGetStrandCount distributionObject
		EAssert.AreEqual 4 strandCount
		
		local hairObjectName = distributionObject.Name
		
		-- Save and reload the scene
		holdMaxFile()
		fetchMaxFile quiet:true
		
		distributionObject = GetNodeByName hairObjectName
		EAssert.AreEqual strandCount (OxGetStrandCount distributionObject)
	),
	
	function Test_ReloadScene =
	(
		local distributionObject2 = Plane lengthsegs:1 widthsegs:1 length:20 width:20
		local distributionObject = Plane lengthsegs:1 widthsegs:1 length:10 width:10 isSelected:on
		
		max modify mode

		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:2)
		
		local groundStrandsModifierInstance = Ox_Ground_Strands surface:distributionObject2 topoOrient:on
		modPanel.addModToSelection groundStrandsModifierInstance
		groundStrandsModifierInstance.detachRoots = off
		
		local strandCount = OxGetStrandCount distributionObject
		EAssert.AreEqual 4 strandCount
		
		local hairObjectName = distributionObject.Name
		local verticesBeforeReloading = OxGetVertices distributionObject true
		
		-- Save and reload the scene
		holdMaxFile()
		fetchMaxFile quiet:true
		
		distributionObject = GetNodeByName hairObjectName
		EAssert.AreEqual strandCount (OxGetStrandCount distributionObject)
		local verticesAfterReloading = OxGetVertices distributionObject true
		
		EAssert.ArePointsAllNearEqual verticesBeforeReloading verticesAfterReloading
	),
	
	function Test_ReduceIncomingStrandPointCount =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		local distributionObject2 = copy distributionObject
		max modify mode
		select distributionObject
		
		local guidesFromSurfaceInstance = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:5
		
		local groundStrandsModifierInstance = Ox_Ground_Strands surface:distributionObject2 topoOrient:on
		modPanel.addModToSelection groundStrandsModifierInstance
		groundStrandsModifierInstance.detachRoots = off
		
		-- Reducing the number of incoming points should not have any errors generated
		guidesFromSurfaceInstance.numSegs = 2
	),
	
	function Test_ReloadSceneWithDeletedGroundStrandsDistributionMesh =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		local spline = line()
		addNewSpline spline
		addKnot spline 1 #corner #line [0,0,0]
		addKnot spline 1 #corner #line [0,0,10]
		updateShape spline
		
		local splineName = spline.Name
		local distributionObjectName= distributionObject.Name
		
		max modify mode
		select spline
		
		local guidesFromShapeInstance = Ox_Guides_From_Shape pointCount:2
		modPanel.addModToSelection guidesFromShapeInstance
		
		local groundStrandsModifierInstance = Ox_Ground_Strands surface:distributionObject topoOrient:on
		modPanel.addModToSelection groundStrandsModifierInstance
		groundStrandsModifierInstance.detachRoots = off
		
		delete distributionObject
		
		-- Save and reload the scene
		holdMaxFile()
		fetchMaxFile quiet:true
		
		-- Turning the ground strands modifier on/off should not cause a crash
		spline = GetNodeByName splineName
		spline.modifiers[#Ox_Ground_Strands].enabled = false
		spline.modifiers[#Ox_Ground_Strands].enabled = true
	),
	
	function Test_IncreaseInputGuideCount =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		local groundObject = Plane lengthsegs:1 widthsegs:1
		max modify mode
		select distributionObject
		
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection method: 0 guideCount: 10
		guidesFromSurfaceModifier.count = 10
		local groundStrandsModifierInstance = Ox_Ground_Strands surface:groundObject
		modPanel.addModToSelection groundStrandsModifierInstance
		groundStrandsModifierInstance.detachRoots = off
		
		guidesFromSurfaceModifier.count = 50
	),
	
	function Test_DegenerateFaces =
	(
		local degenerateMesh = plane lengthsegs:1 widthsegs:1 mapcoords:on
		convertToMesh degenerateMesh
		--meshop.deleteFaces degenerateMesh #{1,2}
		setNumFaces degenerateMesh 1
		setFace degenerateMesh 1 1 2 1
		update degenerateMesh
		
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on
		max modify mode
		select distributionObject
		
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		guidesFromSurfaceModifier.surfaceTangentComputeMethod = 1
		local groundStrandsModifierInstance = Ox_Ground_Strands surface:degenerateMesh
		modPanel.addModToSelection groundStrandsModifierInstance
		groundStrandsModifierInstance.detachRoots = off
		
		-- If there are no assertions we have succeeded
	),
	
	function Test_DecreaseInputGuideCount =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		local groundObject = Plane lengthsegs:1 widthsegs:1
		max modify mode
		select distributionObject
		
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection method: 0 guideCount: 50
		local groundStrandsModifierInstance = Ox_Ground_Strands surface:groundObject
		modPanel.addModToSelection groundStrandsModifierInstance
		groundStrandsModifierInstance.detachRoots = off
		
		local surfaceCombModifierInstance = Ox_Surface_Comb()
		modPanel.addModToSelection surfaceCombModifierInstance
		
		-- Reducing guide count should not trigger exceptions or errors
		guidesFromSurfaceModifier.count = 10
	),
	
	function Test_GroundByReattaching =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		local groundObject = Plane lengthsegs:1 widthsegs:1
		max modify mode
		select distributionObject
		
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection method: 0 guideCount: 50
		local groundStrandsModifierInstance = Ox_Ground_Strands surface:groundObject
		modPanel.addModToSelection groundStrandsModifierInstance
		
		groundStrandsModifierInstance.detachRoots = off
		local verticesBeforeSurfaceChange = OxGetVertices distributionObject true
		
		groundObject.width = groundObject.width * 2
		local verticesAfterSurfaceChange = OxGetVertices distributionObject true
		
		EAssert.ArePointsAllNotNearEqual verticesBeforeSurfaceChange verticesAfterSurfaceChange
	),
	
	function Test_StrandGroupsArePreserved =
	(
		local groundObject = Plane lengthsegs:1 widthsegs:1
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		
		-- Add guides from surface and set a non-default strand group manually
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection method:4
		modPanel.addModToSelection (Ox_Generate_Guide_Data newChannelName:"Groups" minimumTargetValue:2 generationMethod:0)
		
		-- Add ground strands modifier, render settings, and mesh from strands which will show the outcome of using strand groups
		local groundStrandsModifierInstance = Ox_Ground_Strands surface:groundObject
		modPanel.addModToSelection groundStrandsModifierInstance
		modPanel.addModToSelection (Ox_Render_Settings groupID:2 radius:10)
		modPanel.addModToSelection (Ox_Mesh_From_Strands type:1)
		
		local verticesBeforeGrounding = TestUtilities.GetVertexArray distributionObject
		
		-- Ground the strands. The resulting vertices should be identical if the strand groups were preserved.
		groundStrandsModifierInstance.detachRoots = off
		local verticesAfterGrounding = TestUtilities.GetVertexArray distributionObject
		
		EAssert.ArePointsAllNearEqual verticesBeforeGrounding verticesAfterGrounding
	),
	
	function Test_UndoRedoGrounding =
	(
		theHold.Accept "zero"
		theHold.Begin()
		-- Make surface object have different topology, slight rotation and offset
		local groundObject = Plane lengthsegs:2 widthsegs:2 pos:[1,-0.5,-5] rot:(angleaxis -4 [0,1,0])
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection method:4
		local groundStrandsModifierInstance = Ox_Ground_Strands surface:groundObject
		modPanel.addModToSelection groundStrandsModifierInstance
		theHold.Accept "Created"
		
		local verticesBeforeGrounding = OxGetVertices distributionObject true
		for i = 1 to (verticesBeforeGrounding.count) do
		(
			verticesBeforeGrounding[i] = ( verticesBeforeGrounding[i] * (distributionObject.transform) )
		)
		
		-- Ground the strands
		theHold.Begin()
		groundStrandsModifierInstance.detachRoots = off
		theHold.Accept "Grounded"
		
		local verticesAfterGrounding = OxGetVertices distributionObject true
		for i = 1 to (verticesAfterGrounding.count) do ( verticesAfterGrounding[i] = ( verticesAfterGrounding[i] * (distributionObject.transform) ) )
		
		-- Vertices should've been changed after grounding
		EAssert.ArePointsNotAllNearEqual verticesBeforeGrounding verticesAfterGrounding
		
		-- Undo grounding, make sure vertices were restored
		max undo
		local verticesAfterUndo = OxGetVertices distributionObject true
		for i = 1 to (verticesAfterUndo.count) do
		(
			verticesAfterUndo[i] = ( verticesAfterUndo[i] * (distributionObject.transform) ) 
		)
		
		EAssert.ArePointsAllNearEqual verticesBeforeGrounding verticesAfterUndo
		
		max redo
		local verticesAfterRedo = OxGetVertices distributionObject true
		for i = 1 to (verticesAfterRedo.count) do verticesAfterRedo[i] = verticesAfterRedo[i] * (distributionObject.transform)
		EAssert.ArePointsAllNearEqual verticesAfterGrounding verticesAfterRedo
	),
	
	function Test_GroundAndReloadScene =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		
		max modify mode
		modPanel.addModToSelection (Noisemodifier())
		
		local firstPoint = [2,2,0]
		local secondPoint = [2,5,10]
		
		local spline = line isSelected:on
		local splineName = spline.name
		addNewSpline spline
		addKnot spline 1 #corner #line firstPoint
		addKnot spline 1 #corner #line secondPoint
		updateShape spline
		
		-- Add guides from shape and ground strands modifier
		modPanel.addModToSelection (Ox_Guides_From_Shape NumSegs:2)
		local groundStrands = Ox_Ground_Strands surface:distributionObject
		modPanel.addModToSelection groundStrands
		
		groundStrands.detachRoots = off
		
		-- Evaluate so that ground modifier is updated		
		local verticesAfterGrounding1 = OxGetVertices spline true
		EAssert.ArePointsNearEqual secondPoint verticesAfterGrounding1[2]
		
		-- Move the distribution object's vertices
		distributionObject.modifiers[#Noise].strength = [1000,1000,1000]
		
		local verticesAfterGrounding2 = OxGetVertices spline true
		
		-- Save and reload the scene
		holdMaxFile()
		fetchMaxFile quiet:true
		
		spline = GetNodeByName splineName
		local verticesAfterReloading = OxGetVertices spline true
		
		EAssert.ArePointsAllNearEqual verticesAfterGrounding2 verticesAfterReloading
	),
	
	function Test_LoadV6GroundStrands =
	(
		EUnit.SkipBeforeMaxVersion 2020

		units.SystemType = #Inches
		local testSceneFilePath = pathConfig.convertPathToAbsolute (getSourceFileName() + "..\\..\\..\\..\\..\\..\\..\\..\\" ) + "External\\TestData\\OxMax6-Max2020-GroundStrands.max"
		loadMaxFile testSceneFilePath useFileUnit:true quiet:true
	
		local actualVertices = OxGetVertices $FurBall true
		local expectedVertices = #([-2.95276,2.95276,0], [-2.95276,2.95276,0.393701], [-2.95276,-2.95276,0], [-2.95276,-2.95276,0.393701], [2.95276,2.95276,0], [2.95276,2.95276,0.393701], [2.95276,-2.95276,0], [2.95276,-2.95276,0.393701])
		EAssert.ArePointsAllNearEqual expectedVertices actualVertices
		
		-- Cleanup
		resetMaxFile #noPrompt
	),

	function Test_GroundBakedHair =
	(
		local bakedHair = TestUtilities.CreateBakedHairOnPlane()
		local sphereForGrounding = Sphere radius:10 segs:4 mapcoords:on isSelected:on
		local initialVertices = OxGetVertices bakedHair false

		max modify mode

		local groundStrandsModifierInstance = Ox_Ground_Strands()
		select bakedHair
		modPanel.addModToSelection groundStrandsModifierInstance
		groundStrandsModifierInstance.surface = sphereForGrounding
		groundStrandsModifierInstance.detachRoots = off

		local verticesAfterGrounding = OxGetVertices bakedHair false
		
		EAssert.ArePointsNotAllNearEqual initialVertices verticesAfterGrounding
	)
)

EUnit.AddTestClass Test_GroundStrandsModifier
ok
