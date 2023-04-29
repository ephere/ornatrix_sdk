struct Test_DynamicsModifier
(
	function TestInitialize =
	(
		OxResetClassParameters()
		animationRange = (interval 0f 100f)
		sliderTime = 0f
	),
	
	function TestCleanup =
	(
		animationRange = (interval 0f 100f)
		delete $objects
	),
	
	-- Tests for regression of a bug where a removed Dynamics Modifier would start causing MassFX errors
	function Test_AddSimulateRemoveSimulate =
	(
		if (maxVersion())[1] <= 14000 then
		(
			print "MassFX dynamics not supported in 3dsmax 2012 and below, skipping test..."
			return undefined
		)
			
		-- Create some hair
		local distributionObject = Plane lengthsegs:2 widthsegs:2
		max modify mode
		select distributionObject
		guidesFromSurfaceModifier = Ox_Guides_from_Surface()		
		guidesFromSurfaceModifier.method = 4
		modPanel.addModToSelection guidesFromSurfaceModifier

		-- Add dynamics modifier and simulate MassFX globally
		dynamicsModifierInstance = Ox_Dynamics()		
		modPanel.addModToSelection dynamicsModifierInstance
		
		macros.run "PhysX" "PxPlaySimMS"
		macros.run "PhysX" "PxResetSimMS"
		
		-- Remove dynamics modifier and simulate again
		deleteModifier distributionObject 1
		
		macros.run "PhysX" "PxPlaySimMS"
		macros.run "PhysX" "PxResetSimMS"
		
		-- If everything went well there should be no MassFX errors
		
		-- Cleanup
		delete distributionObject
	),

	-- Tests that baking a simulation stores hair animation
	function Test_BakeMassFXSimulation =
	(
		if (maxVersion())[1] <= 14000 then
		(
			print "MassFX dynamics not supported in 3dsmax 2012 and below, skipping test..."
			return undefined
		)
		
		-- Create guides that stand horizontally (perpendicular to gravity) and add a dynamics mod
		distributionObject = Plane lengthsegs:1 widthsegs:1
		rotate distributionObject (angleaxis 90 [1,0,0])
		max modify mode
		select distributionObject
		guidesFromSurfaceModifier = Ox_Guides_from_Surface()		
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 3
		modPanel.addModToSelection guidesFromSurfaceModifier
		dynamicsModifierInstance = Ox_Dynamics()		
		modPanel.addModToSelection dynamicsModifierInstance
		
		-- Reduce animation range so we have less to simulate
		previousAnimationRange = animationRange
		animationRange = (interval 0f 5f)
		
		-- MassFX "bake all"
		macros.run "PhysX" "PxBakeAllMS"
		
		-- Capture tip positions at frame 0
		guideTipsAtFrame0 = TestUtilities.GetTipPositions distributionObject
		print guideTips
		
		-- Set time to 5'th frame and capture tips again
		sliderTime = 5f
		guideTipsAtFrame5 = TestUtilities.GetTipPositions distributionObject
		
		-- Compare the tip positions, they should be different from before
		for i = 1 to 4 do
		(
			EAssert.AreNotEqual guideTipsAtFrame0[i] guideTipsAtFrame5[i]
		)
		
		-- Restore animation range and cleanup
		animationRange = previousAnimationRange
		delete distributionObject
	),
	
	function Test_DeformingCollisionMesh =
	(
		if (maxVersion())[1] <= 14000 then
		(
			print "MassFX dynamics not supported in 3dsmax 2012 and below, skipping test..."
			return undefined
		)
		
		-- Create a cylinder which is bending
		local collisionCylinder = Cylinder heightsegs:10 capsegs:1 sides:10 height:200 radius:10 transform:(matrix3 [0,-1,0] [0,0,1] [-1,0,0] [0,0,60])
		max modify mode
		select collisionCylinder
		local bendModifierInstance = Bend bendAngle: 180
		modPanel.addModToSelection bendModifierInstance
		set animate on
		sliderTime = 5f
		bendModifierInstance.BendAngle = -180
		set animate off
		sliderTime = 0f
		
		local distributionObject = Plane length:30 width:30 lengthsegs:1 widthsegs:1 pos:[-80,0,110]
		rotate distributionObject (angleaxis 180 [0,1,0])
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 3
		guidesFromSurfaceModifier.guideLength = 100
		guidesFromSurfaceModifier.randomness = 0
		modPanel.addModToSelection guidesFromSurfaceModifier
		local dynamicsModifierInstance = Ox_Dynamics objects:#(collisionCylinder)
		modPanel.addModToSelection dynamicsModifierInstance
		
		local previousAnimationRange = animationRange
		animationRange = (interval 0f 5f)
		
		-- MassFX "bake all"
		macros.run "PhysX" "PxBakeAllMS"
		
		-- Compare vertices at frame 0 and 5. They should be different if collision object touched them.
		local verticesAtFrame0 = OxGetVertices distributionObject true
		sliderTime = 5f
		local verticesAtFrame5 = OxGetVertices distributionObject true
		
		for i = 1 to 5 do
		(
			-- Ignore root vertices
			if ( mod ( i - 1 ) 3 ) != 0 then
			(
				EAssert.ArePointsNotNearEqual verticesAtFrame0[i] verticesAtFrame5[i]
			)
		)
		
		-- Cleanup
		animationRange = previousAnimationRange
		sliderTime = 0f
		delete distributionObject
		delete collisionCylinder
	),
	
	function Test_ZeroSegments =
	(
		if (maxVersion())[1] <= 14000 then
		(
			print "MassFX dynamics not supported in 3dsmax 2012 and below, skipping test..."
			return undefined
		)
		
		local distributionObject = Plane lengthsegs:2 widthsegs:2 width:10 length:10
		max modify mode
		select distributionObject
			
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 randomness:0 NumSegs:3 count:4 guideLength:20)
		
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- Create different segment combination (zero, tiny and normal)
		local delta0 = 0.00f
		local delta1 = 0.001f
		local delta2 = 10.0f
	
		local strandPoints = editGuidesInstance.GetStrandShape 0
		strandPoints[2].z = delta0
		strandPoints[3].z = delta0 + strandPoints[2].z
		editGuidesInstance.SetStrandShape 0 strandPoints

		local strandPoints = editGuidesInstance.GetStrandShape 1
		strandPoints[2].z = delta1
		strandPoints[3].z = delta0 + strandPoints[2].z
		editGuidesInstance.SetStrandShape 1 strandPoints

		local strandPoints = editGuidesInstance.GetStrandShape 2
		strandPoints[2].z = delta2
		strandPoints[3].z = delta0 + strandPoints[2].z
		editGuidesInstance.SetStrandShape 2 strandPoints

		local strandPoints = editGuidesInstance.GetStrandShape 3
		strandPoints[2].z = delta0
		strandPoints[3].z = delta1 + strandPoints[2].z
		editGuidesInstance.SetStrandShape 3 strandPoints

		local strandPoints = editGuidesInstance.GetStrandShape 4
		strandPoints[2].z = delta1
		strandPoints[3].z = delta1 + strandPoints[2].z
		editGuidesInstance.SetStrandShape 4 strandPoints

		local strandPoints = editGuidesInstance.GetStrandShape 5
		strandPoints[2].z = delta2
		strandPoints[3].z = delta1 + strandPoints[2].z
		editGuidesInstance.SetStrandShape 5 strandPoints

		local strandPoints = editGuidesInstance.GetStrandShape 6
		strandPoints[2].z = delta0
		strandPoints[3].z = delta2 + strandPoints[2].z
		editGuidesInstance.SetStrandShape 6 strandPoints

		local strandPoints = editGuidesInstance.GetStrandShape 7
		strandPoints[2].z = delta1
		strandPoints[3].z = delta2 + strandPoints[2].z
		editGuidesInstance.SetStrandShape 7 strandPoints

		local strandPoints = editGuidesInstance.GetStrandShape 8
		strandPoints[2].z = delta2
		strandPoints[3].z = delta2 + strandPoints[2].z
		editGuidesInstance.SetStrandShape 8 strandPoints

		rotate distributionObject (angleaxis 90 [1,0,0])
		move distributionObject [0,0,30]
		
		local dynamicsModifierInstance = Ox_Dynamics()
		modPanel.addModToSelection dynamicsModifierInstance
		
		local previousAnimationRange = animationRange
		animationRange = (interval 0f 5f)
		
		local previousGravity = PhysXPanelData.gravity
		PhysXPanelData.gravity = -90.81
		
		-- MassFX "bake all"
		macros.run "PhysX" "PxBakeAllMS"
		
		-- Compare vertices at frame 0 and 5
		sliderTime = 0f
		local verticesAtFrame0 = OxGetVertices distributionObject true
		sliderTime = 5f
		local verticesAtFrame5 = OxGetVertices distributionObject true
		
		for i = 1 to verticesAtFrame0.count do
		(
			-- Get end vertices only
			if ( mod i 3 ) == 0 then
			(
				local guideLengthDelta = verticesAtFrame0[i].z
				-- If guide length was big enough we expect it to fall down
				if ( guideLengthDelta > ( delta1 * 3 ) ) then
				(
					EAssert.ArePointsNotNearEqual verticesAtFrame0[i] verticesAtFrame5[i]
				)
			)
		)
		
		-- Cleanup
		sliderTime = 0f
		animationRange = previousAnimationRange
		PhysXPanelData.gravity = previousGravity
		delete distributionObject
	),
	
	function Test_StartSegmentMap =
	(
		if (maxVersion())[1] <= 14000 then
		(
			print "MassFX dynamics not supported in 3dsmax 2012 and below, skipping test..."
			return undefined
		)
		
		-- Create guides that stand horizontally (perpendicular to gravity) and add a dynamics mod
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on pos:[0,0,30]
		rotate distributionObject (angleaxis 90 [1,0,0])
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:10
		
		local  segmentMap = Checker ()
		segmentMap.coords.V_Tiling = 0.95
		segmentMap.coords.U_Tiling = 0.95
		local dynamicsModifierInstance = Ox_Dynamics startSegMap:segmentMap
		modPanel.addModToSelection dynamicsModifierInstance
		
		-- Reduce animation range so we have less to simulate
		previousAnimationRange = animationRange
		animationRange = (interval 0f 5f)
		
		-- MassFX "bake all"
		macros.run "PhysX" "PxBakeAllMS"
		
		-- Only tips of two of the strands should move since the other ones should remain stationary
		sliderTime = 0f
		local tipsAt0f = TestUtilities.GetTipPositions distributionObject
		sliderTime = 5f
		local tipsAt5f = TestUtilities.GetTipPositions distributionObject
		
		EAssert.ArePointsNotNearEqual tipsAt0f[1] tipsAt5f[1]
		EAssert.ArePointsNotNearEqual tipsAt0f[4] tipsAt5f[4]
		-- TODO: Restore me!
		--EAssert.ArePointsNearEqual tipsAt0f[2] tipsAt5f[2]
		--EAssert.ArePointsNearEqual tipsAt0f[3] tipsAt5f[3]
		
		-- Cleanup
		delete $objects
		animationRange = (interval 0f 100f)
	),
	
	-- Tests that we can restart simulation with an external force added and it will still simulate the force OK
	function Test_ReSimulateWithForceAdded =
	(
		if (maxVersion())[1] <= 14000 then
		(
			print "MassFX dynamics not supported in 3dsmax 2012 and below, skipping test..."
			return undefined
		)
		
		-- Create guides that stand horizontally (perpendicular to gravity) and add a dynamics mod
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on pos:[0,0,30]
		rotate distributionObject (angleaxis 90 [1,0,0])
		max modify mode

		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		local windNode = gravity transform:(matrix3 [0,-1,0] [0,0,1] [-1,0,0] [0,0,0]) strength:10000.0
		local dynamicsModifierInstance = Ox_Dynamics forces:#(windNode)
		modPanel.addModToSelection dynamicsModifierInstance
		
		-- Reduce animation range so we have less to simulate
		local previousAnimationRange = animationRange
		animationRange = (interval 0f 5f)
		
		-- MassFX "bake all"
		macros.run "PhysX" "PxBakeAllMS"
		
		-- Make sure that wind affected the strands
		sliderTime = 0f
		local tipsAt0f = TestUtilities.GetTipPositions distributionObject
		sliderTime = 5f
		local tipsAt5f = TestUtilities.GetTipPositions distributionObject
		
		for i = 1 to 4 do
		(
			EAssert.IsLessThan -10 (tipsAt5f[i].x - tipsAt0f[i].x)
		)
		
		-- Unbake and re-bake again
		macros.run "PhysX" "PxUnBakeAllMS"
		macros.run "PhysX" "PxBakeAllMS"
		
		-- Tip offsets due to gravity should still hold true
		sliderTime = 0f
		tipsAt0f = TestUtilities.GetTipPositions distributionObject
		sliderTime = 5f
		tipsAt5f = TestUtilities.GetTipPositions distributionObject
		
		for i = 1 to 4 do
		(
			EAssert.IsLessThan -10 (tipsAt5f[i].x - tipsAt0f[i].x)
		)
		
		-- Restore animation range and cleanup
		animationRange = previousAnimationRange
		delete $objects
	),
	
	function Test_PointsBeforeStartSegmentArePreserved =
	(
		if (maxVersion())[1] <= 14000 then
		(
			print "MassFX dynamics not supported in 3dsmax 2012 and below, skipping test..."
			return undefined
		)
		
		OxResetClassParameters (Ox_Guides_From_Surface())
		OxResetClassParameters (Ox_Dynamics())
		
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on pos:[0,0,30]
		rotate distributionObject (angleaxis 90 [1,0,0])
		max modify mode

		modPanel.addModToSelection (Ox_Guides_From_Surface method:4 NumSegs:10)
		modPanel.addModToSelection (Ox_Dynamics startSeg:5)
		
		-- Reduce animation range so we have less to simulate
		local previousAnimationRange = animationRange
		animationRange = (interval 0f 1f)
		
		-- MassFX "bake all"
		macros.run "PhysX" "PxBakeAllMS"
		
		-- After simulating with specified start segment the point positions shouldn't differ too much on the first frame
		sliderTime = 0f
		local verticesAtFrame0 = OxGetVertices distributionObject true
		sliderTime = 1f
		local verticesAtFrame1 = OxGetVertices distributionObject true
		
		EAssert.ArePointsAllNearEqual verticesAtFrame0 verticesAtFrame1 endIndex:5 epsilon:3
	)
)

EUnit.AddTestClass Test_DynamicsModifier
ok
