struct Test_GravityModifier
(
	function TestInitialize =
	(
		OxResetClassParameters()
	),
	
	function TestCleanup =
	(
		delete $objects
		units.SystemType = #Centimeters
	),
	
	-- Makes sure gravity parameters can be animated and update strands when time changes
	function Test_AnimatedParameters =
	(
		-- Create a plane with 1x1 segments so we have 4 vertices all together and create a reference which we will use as guides
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on

		max modify mode

		-- Add gravity modifier to guides
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 3
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		local strandGravityModifierInstance = Ox_Strand_Gravity strandGroupPattern:"0"
		modPanel.addModToSelection strandGravityModifierInstance
		strandGravityModifierInstance.gravDir = (matrix3 [1,0,0] [0,0,1] [0,-1,0] [0,0,0])
		
		-- Animate gravity
		set animate on
		sliderTime = 0f
		strandGravityModifierInstance.gravForce = 0
		sliderTime = 10f
		strandGravityModifierInstance.gravForce = 1
		set animate off
			
		local guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		local myGuides = ( guidesCopy.GetGuides() ).AsHair()
		guide1Tip = myGuides.GetPoint 0 2
		guide2Tip = myGuides.GetPoint 1 2
		guide3Tip = myGuides.GetPoint 2 2
		guide4Tip = myGuides.GetPoint 3 2
		delete guidesCopy
		
		sliderTime = 0f
		guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		myGuides = ( guidesCopy.GetGuides() ).AsHair()
		EAssert.ArePointsNotNearEqual guide1Tip ( myGuides.GetPoint 0 2 )
		EAssert.ArePointsNotNearEqual guide2Tip ( myGuides.GetPoint 1 2 )
		EAssert.ArePointsNotNearEqual guide3Tip ( myGuides.GetPoint 2 2 )
		EAssert.ArePointsNotNearEqual guide4Tip ( myGuides.GetPoint 3 2 )
		delete guidesCopy
	),
	
	-- Test gravity save and restore
	-- Test general hair direction with gravity defaults
	-- Check that at zero gravity hair has original shape
	function Test_LoadSaveAndDefaults =
	(
		units.SystemType = #Centimeters
		
		-- Create a box with zero width
		local distributionObject = Box length:10 width:0 height:20 mapcoords:on isSelected:on
		max modify mode

		-- Add gravity modifier to guides
		
		-- With small strands numSegs value gravity modifier has same issues, let's leave defaults
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection method:2 pointCount:10
		guidesFromSurfaceModifier.count = 30
		guidesFromSurfaceModifier.rememberPositions = on
		
		-- Get vertices without modifications
		local verticesWithoutMod = OxGetVertices distributionObject true
		
		-- Get vertices after applying gravity with default parameters
		local strandGravityModifierInstance = Ox_Strand_Gravity gravForce:1
		modPanel.addModToSelection strandGravityModifierInstance
		local verticesDefaults = OxGetVertices distributionObject true
		
		-- Get vertices with zero gravity
		local defaultGravity = strandGravityModifierInstance.gravForce
		strandGravityModifierInstance.gravForce = 0
		local verticesZero = OxGetVertices distributionObject true
		strandGravityModifierInstance.gravForce = defaultGravity
		
		-- Check general strand directions
		for i = 1 to guidesFromSurfaceModifier.count do
		(
			for j = 2 to guidesFromSurfaceModifier.numSegs do
			(
				local index = ( i - 1 ) * guidesFromSurfaceModifier.numSegs + j
				local delta = verticesDefaults[index] - verticesDefaults[index - 1]
				EAssert.IsTrue ( abs( delta.x ) > 0 )
				EAssert.IsTrue ( delta.z < 0 )
				EAssert.AreNearEqual 0 delta.y
			)
		)
		
		-- Get vertices after save and load
		holdMaxFile()
		resetMaxFile #noprompt
		
		units.SystemType = #Centimeters
		fetchMaxFile quiet:true		
		EAssert.AreEqual 1 objects.count
		distributionObject = objects[1]
		local verticesReload = OxGetVertices distributionObject true
		
		-- Simple position checks
		for i = 1 to verticesWithoutMod.count do
		(
			EAssert.ArePointsNearEqual verticesWithoutMod[i] verticesZero[i]
			EAssert.ArePointsNearEqual verticesDefaults[i] verticesReload[i]
			
			-- Ignore root vertices
			if ( mod ( i - 1 ) guidesFromSurfaceModifier.numSegs ) != 0 then
			(
				EAssert.ArePointsNotNearEqual verticesWithoutMod[i] verticesDefaults[i]
				EAssert.ArePointsNotNearEqual verticesZero[i] verticesReload[i]
			)
		)
	),
	
	function Test_EmptyInput =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on

		max modify mode

		-- Add gravity modifier to zero guides. It should not cause a crash.
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface distrMap: (Color_Correction ())
		modPanel.addModToSelection guidesFromSurfaceModifier
		modPanel.addModToSelection (Ox_Strand_Gravity groupID:0)
		
		EAssert.AreEqual 0 (OxGetStrandCount distributionObject)
	)
)

EUnit.AddTestClass Test_GravityModifier
ok
