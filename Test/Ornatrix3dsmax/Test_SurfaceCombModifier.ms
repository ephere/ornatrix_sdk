struct Test_SurfaceCombModifier
(
	function TestCleanup =
	(
		delete $objects
	),
	
	-- Tests that sink weights have an effect on resulting strand orientation
	function Test_SinkWeights =
	(
		-- Create a plane with 1x1 segments so we have 4 vertices all together
		local distributionObject = Plane lengthsegs:1 widthsegs:1

		max modify mode

		-- Setup guides with surface comb
		select distributionObject

		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		guidesFromSurfaceModifier.method = 2
		guidesFromSurfaceModifier.count = 10
		guidesFromSurfaceModifier.numSegs = 2
		modPanel.addModToSelection guidesFromSurfaceModifier

		local surfaceCombInstance = Ox_Surface_Comb()
		modPanel.addModToSelection surfaceCombInstance
		surfaceCombInstance.algorithm = 0
			
		-- Create two sinks on either end of the distribution mesh, pointing away from one another
		surfaceCombInstance.CreateSinkAtCoordinate 2 [20,0,0] 0 [0,0]
		surfaceCombInstance.CreateSinkAtCoordinate 2 [-20,0,0] 0 [1,1]
		
		surfaceCombInstance.SetSinkStrength 0 10
		surfaceCombInstance.SetSinkStrength 1 5

		-- Record current tip positions
		local guide1Tip = OxGetStrandPoint distributionObject 4 1 false
		local guide2Tip = OxGetStrandPoint distributionObject 5 1 false
		local guide3Tip = OxGetStrandPoint distributionObject 6 1 false
		local guide4Tip = OxGetStrandPoint distributionObject 7 1 false
		
		-- Change the strengths of the sinks and compare the new tip positions. They should be different.
		surfaceCombInstance.SetSinkStrength 0 5
		surfaceCombInstance.SetSinkStrength 1 100
		
		EAssert.AreNotEqual guide1Tip ( OxGetStrandPoint distributionObject 4 1 false )
		EAssert.AreNotEqual guide2Tip ( OxGetStrandPoint distributionObject 5 1 false )
		EAssert.AreNotEqual guide3Tip ( OxGetStrandPoint distributionObject 6 1 false )
		EAssert.AreNotEqual guide4Tip ( OxGetStrandPoint distributionObject 7 1 false )
	),
	
	-- Tests for customer issue #256
	function Test_AddEditMeshAndSelectOnStackBottom =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		
		max modify mode

		-- Setup guides with surface comb
		select distributionObject

		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		modPanel.addModToSelection guidesFromSurfaceModifier

		local surfaceCombInstance = Ox_Surface_Comb()
		modPanel.addModToSelection surfaceCombInstance
		
		-- Add an edit mesh modifier on the bottom of the stack
		modPanel.setCurrentObject $.baseObject
		local editPolyModifierInstance = Edit_Poly()
		modPanel.addModToSelection editPolyModifierInstance

		-- Select some vertices
		subobjectLevel = 1
		editPolyModifierInstance.SetSelection #Vertex #{}
		editPolyModifierInstance.Select #Vertex #{1..4}

		-- If everything is OK no crash should happen and the test passes
	),
	
	-- Tests for a crash bug where a sink was initially placed on a mesh whose topology changed and the face of the sink no longer exists
	function Test_SinkPlacedOnInvalidFace =
	(
		OxResetClassParameters()
		
		-- Initially a 2x2 plane, we will change to 1x1 later
		local distributionObject = Plane lengthsegs:2 widthsegs:2 isSelected:on
		
		max modify mode

		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()

		local surfaceCombInstance = Ox_Surface_Comb()
		modPanel.addModToSelection surfaceCombInstance
		
		-- Create a sink on 3'rd face
		surfaceCombInstance.CreateSinkAtCoordinate 2 [20,0,0] 1 [0,1]
		
		-- Now change the plane segments to 1x1 so face #3 no longer exists
		distributionObject.lengthsegs = 1
		distributionObject.widthsegs = 1
		
		-- If no crash happens we are good
		EAssert.AreEqual 4 (OxGetStrandCount distributionObject)
	),
	
	function SetupSurfaceCombWithOneDirectionSink =
	(
		local distributionObject = Sphere radius:20 smooth:on segs:40 mapcoords:on isSelected:on
		max modify mode
		
		local guidesFromMeshModifierInstance = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:5

		-- Add surface comb with a directional sink
		local surfaceCombInstance = Ox_Surface_Comb surfChaos:0
		modPanel.addModToSelection surfaceCombInstance
		surfaceCombInstance.CreateSinkAtCoordinate 2 [20,0,0] 3 [0,1]
		
		#(distributionObject, surfaceCombInstance, guidesFromMeshModifierInstance)
	),
	
	function Test_DetachAndReattach =
	(
		local params = SetupSurfaceCombWithOneDirectionSink()
		local distributionObject = params[1]
		local surfaceCombInstance = params[2]
		local guidesFromMeshModifierInstance = params[3]
		surfaceCombInstance.algorithm = 0
		
		copy distributionObject
		
		local vertexPositionsBeforeDetach = OxGetVertices distributionObject true
		surfaceCombInstance.detachSinks = on
		guidesFromMeshModifierInstance.detachRoots = on
			
		-- Cause an evaluation
		OxGetVertices distributionObject true
		distributionObject.segs = 30
		guidesFromMeshModifierInstance.detachRoots = off
		surfaceCombInstance.detachSinks = off
		
		local vertexPositionsAfterReattach = OxGetVertices distributionObject true
		EAssert.ArePointsAllNearEqual vertexPositionsBeforeDetach vertexPositionsAfterReattach useDistance:true epsilon:1
	),
	
	function Test_CloneObjectWithSurfaceComb =
	(
		local params = SetupSurfaceCombWithOneDirectionSink()
		local distributionObject = params[1]
		local surfaceCombInstance = params[2]
		local guidesFromMeshModifierInstance = params[3]
		
		local copiedObject = copy distributionObject
		local originalVertices = OxGetVertices distributionObject true
		local copiedVertices = OxGetVertices copiedObject true
		EAssert.ArePointsAllNearEqual originalVertices copiedVertices useDistance:true
	),

	-- Tests that mirrored sink is created
	function Test_MirroredSinks =
	(
		-- Create a plane with 1x1 segments so we have 4 vertices all together
		local distributionObject = Plane lengthsegs:1 widthsegs:1

		max modify mode

		-- Setup guides with surface comb
		select distributionObject

		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		guidesFromSurfaceModifier.method = 2
		guidesFromSurfaceModifier.count = 10
		guidesFromSurfaceModifier.numSegs = 2
		modPanel.addModToSelection guidesFromSurfaceModifier

		local surfaceCombInstance = Ox_Surface_Comb()
		modPanel.addModToSelection surfaceCombInstance

		-- Create two sinks on either end of the distribution mesh, pointing away from one another
		-- The second sink has mirrored one.
		surfaceCombInstance.CreateSinkAtCoordinate 2 [20,0,0] 0 [0,0]
		surfaceCombInstance.UI_SinkMirrorEnable = true
		surfaceCombInstance.CreateSinkAtCoordinate 2 [-20,0,0] 0 [1,1]
		surfaceCombInstance.UI_SinkMirrorEnable = false

		EAssert.AreEqual 3 ( surfaceCombInstance.GetSinkCount() )
	),
	
	function Test_SinksSavedToGroom =
	(
		OxResetClassParameters ( Plane() )
		OxResetClassParameters ( Ox_Guides_from_Surface() )
		OxResetClassParameters ( Ox_Surface_Comb() )
		
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		local distributionObjectName = distributionObject.Name
		max modify mode

		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:3)

		local surfaceCombInstance = Ox_Surface_Comb()
		modPanel.addModToSelection surfaceCombInstance

		surfaceCombInstance.CreateSinkAtCoordinate 2 [20,0,0] 0 [0,0]
		surfaceCombInstance.CreateSinkAtCoordinate 2 [-20,0,0] 0 [1,1]
		local verticesOnOriginalObject = OxGetVertices distributionObject true
		
		local xmlString = OxGenerateGroomXML distributionObject
		delete distributionObject
		
		OxCreateGroom xmlString
		local loadedGroomObject = GetNodeByName distributionObjectName
		local verticesOnGroomObject = OxGetVertices loadedGroomObject true
		
		EAssert.ArePointsAllNearEqual verticesOnOriginalObject verticesOnGroomObject
	),
	
	-- Tests that sink data loaded with the scene is accordingly resized so if we save and reload the scene and create/delete a sink there are no problems
	function Test_DeleteCreatedSinkAfterSceneReload =
	(
		OxResetClassParameters ( Ox_Guides_from_Surface() )
		OxResetClassParameters ( Ox_Surface_Comb() )
		
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		local distributionObjectName = distributionObject.Name
		max modify mode

		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:3)

		local surfaceCombInstance = Ox_Surface_Comb()
		modPanel.addModToSelection surfaceCombInstance

		surfaceCombInstance.CreateSinkAtCoordinate 2 [20,0,0] 0 [1,1]
		
		-- Save and reload the scene
		holdMaxFile()
		fetchMaxFile quiet:true
		
		distributionObject = GetNodeByName distributionObjectName
		surfaceCombInstance = distributionObject.modifiers[1]
		
		max modify mode
		surfaceCombInstance.CreateSinkAtCoordinate 2 [10,0,0] 0 [1,0]
		surfaceCombInstance.DeleteSink 1
		
		-- No crash should happen at this point
	),
	
	-- Tests for a bug where sinks are deleted when a scene with disabled SC modifier is reloaded
	function Test_ReloadSceneTwiceWithDisabledSurfaceCombModifier =
	(
		OxResetClassParameters ( Ox_Guides_from_Surface() )
		OxResetClassParameters ( Ox_Surface_Comb() )
		
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		local distributionObjectName = distributionObject.Name
		max modify mode

		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:3)

		local surfaceCombInstance = Ox_Surface_Comb()
		modPanel.addModToSelection surfaceCombInstance
		surfaceCombInstance.CreateSinkAtCoordinate 2 [20,0,0] 0 [1,1]
		
		-- Disable surface comb
		surfaceCombInstance.enabled = off
		
		-- Save and reload the scene twice (it only happened when doing it twice)
		holdMaxFile()
		resetMaxFile #noPrompt
		fetchMaxFile quiet:true
		holdMaxFile()
		resetMaxFile #noPrompt
		fetchMaxFile quiet:true
		
		distributionObject = GetNodeByName distributionObjectName
		surfaceCombInstance = distributionObject.modifiers[#Ox_Surface_Comb]
		
		-- We should still have the sink present
		EAssert.AreEqual 1 (surfaceCombInstance.GetSinkCount())
	),
	
	function Test_MirroredSinksOnDeformingMeshAfterSceneReload =
	(
		local cylinderHair = TestUtilities.AddBendingCylinder()
		local cylinderHairName = cylinderHair.Name
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:3 guideLength:4)

		local surfaceCombInstance = Ox_Surface_Comb()
		modPanel.addModToSelection surfaceCombInstance
		surfaceCombInstance.CreateSinkAtCoordinate 2 [50,0,0] 100 [0.5,0.5]
		surfaceCombInstance.SetSinkMirrorType 0 1
		-- Evaluate hair to generate sink data
		OxGetVertices cylinderHair true
		
		-- Move animation range so that it starts at the frame where cylinder is bent
		animationRange = (interval 10f 11f)
		local verticesBeforeSave = OxGetVertices cylinderHair true
	),
	
	function Test_MirroredSinksSavedWithScene =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		local distributionObjectName = distributionObject.Name
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:3 guideLength:4)

		local surfaceCombInstance = Ox_Surface_Comb()
		modPanel.addModToSelection surfaceCombInstance
		surfaceCombInstance.CreateSinkAtCoordinate 2 [50,0,0] 0 [0.25, 0.25]
		surfaceCombInstance.SetSinkMirrorType 0 1
		-- Evaluate hair to generate sink data
		local verticesBeforeSave = OxGetVertices distributionObject true
		
		holdMaxFile()
		fetchMaxFile quiet:true
		distributionObject = GetNodeByName distributionObjectName
		local verticesAfterLoad = OxGetVertices distributionObject true
		
		EAssert.ArePointsAllNearEqual verticesBeforeSave verticesAfterLoad
	),
	
	function TestSceneVertices fileName expectedStrandIds expectedVertices objectName:"FurBall" =
	(
		resetMaxFile #noPrompt
		units.SystemType = #Centimeters
		local testSceneFilePath = pathConfig.convertPathToAbsolute (getSourceFileName() + "..\\..\\..\\..\\..\\..\\..\\..\\" ) + "External\\TestData\\" + fileName
		loadMaxFile testSceneFilePath useFileUnit:true quiet:true

		local sceneNode = getNodeByName objectName
		local actualStrandIds = OxGetStrandIds sceneNode
		sort actualStrandIds
		EAssert.AreArraysEqual expectedStrandIds actualStrandIds
		
		local actualVertices = OxGetStrandPointsSortedByStrandId sceneNode true
		EAssert.ArePointsAllNearEqual expectedVertices actualVertices
		
		-- Cleanup
		resetMaxFile #noPrompt
	),
	
	-- This test is disabled because the placement of roots in Ox V7 is different than V6 for an unknown reason (mesh triangulation is checked to be identical). In the future it may be a good idea to figure out why.
	function Test_LoadV6Sinks =
	(
		EUnit.SkipBeforeMaxVersion 2020

		TestSceneVertices "OxMax6-Max2020-SurfaceComb.max" #(0, 1, 2, 3) #([-50,-50,0], [-97.5011,-61.7885,-2.13933e-06], [50,-50,0], [8.14587,-45.3091,-1.84096e-06], [-50,50,0], [-8.1286,43.4425,-1.85257e-06], [50,50,0], [95.304,35.9095,-2.07387e-06])
	)
)

EUnit.AddTestClass Test_SurfaceCombModifier
ok
