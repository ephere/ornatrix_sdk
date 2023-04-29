struct Test_MoovPhysicsModifier
(
	function TestCleanup =
	(
		delete $objects
	),

	function Test_LoadScriptAndSimulateWithGravity =
	(
		OxResetClassParameters (MoovPhysicsModifier())
		max modify mode
		local distributionObject = Sphere radius:25 segs:6 mapcoords:on isSelected:on
			
		-- Rotate the sphere a bit so we don't have a hair sticking out exactly up (it'll cause problems with the gravity test)
		rotate distributionObject (angleaxis 35 [0,1,0])
			
		-- 10 segments so that the hair drops instead of remaining stiff and holding form
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:10

		-- Default installed HairSimulator.py should be loaded
		local moovPhysicsModifierInstance = MoovPhysicsModifier()
		modPanel.addModToSelection moovPhysicsModifierInstance
		
		--moovPhysicsModifierInstance.CollideWithBaseMesh = on
		moovPhysicsModifierInstance.CollideWithMeshes = off
		
		local collisionBox = Box width:100 length:100 height:100 mapcoords:on pos:[0,0,-150]
		append moovPhysicsModifierInstance.collisionMeshes collisionBox
		local originalTipPositions = TestUtilities.GetTipPositionsInWorldCoordinates distributionObject

		-- Evolve the simulation by 10 frames
		for i = 1 to 10 do
		(
			sliderTime += 1f
		)
		
		-- Make sure that the gravity brought hair tips down
		local tipPositionsAtFrame10 = TestUtilities.GetTipPositionsInWorldCoordinates distributionObject
		for i = 1 to (tipPositionsAtFrame10.count) do
		(
			EAssert.IsGreaterThan 0.5 (originalTipPositions[i].z - tipPositionsAtFrame10[i].z)
		)
	),

	function Test_ParametersValuesLoadFromSavedScene =
	(
		OxResetClassParameters (MoovPhysicsModifier())

		local sceneFilePath = TestUtilities.GetTestFilePath "Ornatrix\Scripts\TestFiles\MoovLoadTest2017.max"

		local nodeName = "FurBall"

		loadMaxFile sceneFilePath quiet:true

		local distributionObject = GetNodeByName nodeName
		local moovPhysicsModifierInstance = distributionObject.modifiers[MoovPhysicsModifier]
		EAssert.AreEqual moovPhysicsModifierInstance.SubstepCount 2
	),

	function Test_ImportFromNonDefaultScript =
	(
		OxResetClassParameters (MoovPhysicsModifier())

		max modify mode
		local distributionObject = Sphere radius:25 segs:6 mapcoords:on isSelected:on
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:10
		local scriptName = TestUtilities.GetTestFilePath "..\Maya\Ornatrix\Test\MoovPhysicsNode_HairSimulatorImport.py"
		local moovPhysicsModifierInstance = MoovPhysicsModifier pythonFileName:scriptName
		modPanel.addModToSelection moovPhysicsModifierInstance
		-- Check any script parameter to ensure script was evaluated
		EAssert.AreEqual moovPhysicsModifierInstance.Gravity -981.f
	),

	function Test_ExternalCollisionMeshDeforming =
	(
		OxResetClassParameters (MoovPhysicsModifier())

		-- Create a cylinder which is bending
		local collisionCylinder = Cylinder heightsegs:10 capsegs:1 sides:10 height:200 radius:10 mapcoords:on transform:(matrix3 [0,-1,0] [0,0,1] [-1,0,0] [0,0,60])
		max modify mode
		select collisionCylinder
		local bendModifierInstance = Bend bendAngle: 180
		modPanel.addModToSelection bendModifierInstance
		set animate on
		sliderTime = 10f
		bendModifierInstance.BendAngle = -180
		set animate off
		sliderTime = 0f
		
		local distributionObject = Plane length:30 width:30 lengthsegs:1 widthsegs:1 mapcoords:on pos:[-80,0,110]
		rotate distributionObject (angleaxis 180 [0,1,0])
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 3
		guidesFromSurfaceModifier.guideLength = 100
		guidesFromSurfaceModifier.randomness = 0
		modPanel.addModToSelection guidesFromSurfaceModifier
		local moovPhysicsModifierInstance = MoovPhysicsModifier()
		modPanel.addModToSelection moovPhysicsModifierInstance
		
		--moovPhysicsModifierInstance.CollideWithBaseMesh = on
		moovPhysicsModifierInstance.CollideWithMeshes = on
		moovPhysicsModifierInstance.Gravity = 0f
		moovPhysicsModifierInstance.RootStiffness = 0f
		
		append moovPhysicsModifierInstance.collisionMeshes collisionCylinder
		local originalTipPositions = TestUtilities.GetTipPositions distributionObject

		-- Evolve the simulation by 10 frames
		for i = 1 to 10 do
		(
			sliderTime += 1f
		)
		
		-- Make sure that the collision moved hair tips
		local tipPositionsAtFrame10 = TestUtilities.GetTipPositions distributionObject
		for i = 1 to (tipPositionsAtFrame10.count) do
		(
			EAssert.ArePointsNotNearEqual originalTipPositions[i] tipPositionsAtFrame10[i] epsilon:1f
		)
	), 

	function Test_ExternalCollisionMeshTransforming =
	(
		OxResetClassParameters (MoovPhysicsModifier())

		-- Create a cylinder which is bending
		local collisionCylinder = Cylinder heightsegs:10 capsegs:1 sides:10 height:200 radius:10 mapcoords:on transform:(matrix3 [0,-1,0] [0,0,1] [-1,0,0] [0,-50,60])
		max modify mode
		select collisionCylinder
		set animate on
		sliderTime = 10f
		collisionCylinder.transform = matrix3 [0,-1,0] [0,0,1] [-1,0,0] [0,50,60]
		set animate off
		sliderTime = 0f
		
		local distributionObject = Plane length:30 width:30 lengthsegs:1 widthsegs:1 mapcoords:on pos:[-80,0,110]
		rotate distributionObject (angleaxis 180 [0,1,0])
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 3
		guidesFromSurfaceModifier.guideLength = 100
		guidesFromSurfaceModifier.randomness = 0
		modPanel.addModToSelection guidesFromSurfaceModifier
		local moovPhysicsModifierInstance = MoovPhysicsModifier()
		modPanel.addModToSelection moovPhysicsModifierInstance
		
		--moovPhysicsModifierInstance.CollideWithBaseMesh = on
		moovPhysicsModifierInstance.CollideWithMeshes = on
		moovPhysicsModifierInstance.Gravity = 0f
		moovPhysicsModifierInstance.RootStiffness = 0f
		
		append moovPhysicsModifierInstance.collisionMeshes collisionCylinder
		local originalTipPositions = TestUtilities.GetTipPositions distributionObject

		-- Evolve the simulation by 10 frames
		for i = 1 to 10 do
		(
			sliderTime += 1f
		)
		
		-- Make sure that the collision moved hair tips
		local tipPositionsAtFrame10 = TestUtilities.GetTipPositions distributionObject
		for i = 1 to (tipPositionsAtFrame10.count) do
		(
			EAssert.ArePointsNotNearEqual originalTipPositions[i] tipPositionsAtFrame10[i] epsilon:1f
		)
	),

	function Test_PythonInterpreterCrash =
	(
		-- Issue 3696 Max crashes on attempts to use a second Python interpreter
		python.Execute "pass"

		OxResetClassParameters (MoovPhysicsModifier())
		max modify mode
		local distributionObject = Sphere radius:25 segs:6 mapcoords:on isSelected:on
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:10
		local moovPhysicsModifierInstance = MoovPhysicsModifier()
		modPanel.addModToSelection moovPhysicsModifierInstance
		-- Check any script parameter to ensure script was evaluated
		EAssert.AreEqual moovPhysicsModifierInstance.Gravity -981.f
	),

	function Test_MultipleFramesTimeChange =
	(
		-- Test for correct behaviour when dragging the time slider
		OxResetClassParameters (MoovPhysicsModifier())
		max modify mode
		local distributionObject = Sphere radius:25 segs:6 mapcoords:on isSelected:on
			
		-- Rotate the sphere a bit so we don't have a hair sticking out exactly up (it'll cause problems with the gravity test)
		rotate distributionObject (angleaxis 35 [0,1,0])

		-- Animate the base mesh to test correct mesh update for large steps
		set animate on
		sliderTime = 0f
		distributionObject.pos.x = -30
		sliderTime = 10f
		distributionObject.pos.x = 30
		set animate off
		sliderTime = 0f

		-- 10 segments so that the hair drops instead of remaining stiff and holding form
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:10

		-- Default installed HairSimulator.py should be loaded
		local moovPhysicsModifierInstance = MoovPhysicsModifier()
		modPanel.addModToSelection moovPhysicsModifierInstance
		
		-- Evolve the simulation by 10 frames
		realTimePlayback = false
		startFrame = sliderTime
		for i = 1 to 10 do
		(
			sliderTime += 1f
		)
		endFrame = sliderTime
		local tipPositionsAtFrame10 = TestUtilities.GetTipPositionsInWorldCoordinates distributionObject

		sliderTime = startFrame
		sliderTime = endFrame
		local tipPositionsAtFrame10WithJump = TestUtilities.GetTipPositionsInWorldCoordinates distributionObject
		EAssert.ArePointsAllNearEqual tipPositionsAtFrame10 tipPositionsAtFrame10WithJump
	),

	function Test_DistributionMeshAnimation =
	(
		-- Test for correct distribution mesh transform during animation
		OxResetClassParameters (MoovPhysicsModifier())
		max modify mode
		local collisionObject = Sphere radius:50 segs:6 mapcoords:on isSelected:on
		local distributionObject = Sphere radius:50 segs:6 mapcoords:on isSelected:on
			
		-- Animate the base mesh and collision mesh
		set animate on
		sliderTime = 0f
		distributionObject.pos.x = -70
		collisionObject.pos.x = -70
		sliderTime = 10f
		distributionObject.pos.x = 70
		collisionObject.pos.x = 70
		set animate off
		sliderTime = 0f

		-- 10 segments so that the hair drops instead of remaining stiff and holding form
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:10

		-- Default installed HairSimulator.py should be loaded
		local moovPhysicsModifierInstance = MoovPhysicsModifier()
		modPanel.addModToSelection moovPhysicsModifierInstance
		append moovPhysicsModifierInstance.collisionMeshes collisionObject
		-- Increase gravity to get faster evolving sim
		moovPhysicsModifierInstance.Gravity = -5000
		moovPhysicsModifierInstance.CollideWithMeshes = on
		moovPhysicsModifierInstance.StretchingStiffness = 0.1
		moovPhysicsModifierInstance.BendingStiffness = 0.1
		
		-- Evolve the simulation by 10 frames
		realTimePlayback = false
		sliderTime = 0f
		for i = 1 to 10 do
		(
			sliderTime += 1f
		)
		local tipPositionsWithCollisionMesh = TestUtilities.GetTipPositionsInWorldCoordinates distributionObject

		moovPhysicsModifierInstance.CollideWithMeshes = off
		moovPhysicsModifierInstance.CollideWithBaseMesh = on
		sliderTime = 0f
		for i = 1 to 10 do
		(
			sliderTime += 1f
		)
		local tipPositionsWithBaseMesh = TestUtilities.GetTipPositionsInWorldCoordinates distributionObject
		EAssert.ArePointsAllNearEqual tipPositionsWithCollisionMesh tipPositionsWithBaseMesh
	),

	function Test_CopyFromOneObjectToAnother =
	(
		max modify mode
		
		local moovPhysicsModifierInstance = MoovPhysicsModifier()
		
		local distributionObject1 = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:2
		modPanel.addModToSelection moovPhysicsModifierInstance
		TestUtilities.GetTipPositionsInWorldCoordinates distributionObject1
		
		local distributionObject2 = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:2
		modPanel.addModToSelection (copy moovPhysicsModifierInstance)
		TestUtilities.GetTipPositionsInWorldCoordinates distributionObject2
	)
)

EUnit.AddTestClass Test_MoovPhysicsModifier
ok
