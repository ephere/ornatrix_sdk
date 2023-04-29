struct Test_MultiplierModifier
(
	function Test_Multiplier =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 2
		guidesFromSurfaceModifier.numSegs = 10
		guidesFromSurfaceModifier.randomness = 0
		guidesFromSurfaceModifier.count = 10
		modPanel.addModToSelection guidesFromSurfaceModifier
		guidesFromSurfaceModifier.rememberRootPositions = true

		-- Get vertices without modifications
		local verticesWithoutMod = OxGetVertices distributionObject true
		
		-- Add multiplier modifier
		local multiplierInstance = Ox_Strand_Multiplier probabilityChannel:0 probability:100 copies:1
		modPanel.addModToSelection multiplierInstance
		
		-- Get default vertices
		local verticesDefaults = OxGetVertices distributionObject true
		
		-- Disable multiplier by setting 0 probability
		multiplierInstance.probability = 0
		local verticesZero = OxGetVertices distributionObject true
		
		-- Check strands counters if multiplier probability is 50% 
		multiplierInstance.probability = 50
		multiplierInstance.copies = 4
		-- We should get about 2x roots
		local vertices2X = OxGetVertices distributionObject true
		EAssert.IsTrue( verticesWithoutMod.count < vertices2X.count )
		EAssert.IsTrue( 3 * verticesWithoutMod.count > vertices2X.count )
		
		-- Get vertices after save and load to check that random seeds is preserved
		multiplierInstance.probability = 100
		multiplierInstance.copies = 1
		local systemUnits = units.SystemType
		holdMaxFile()
		resetMaxFile #noprompt
		units.SystemType = systemUnits
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
		
		-- Cleanup
		delete distributionObject
	),
	
	function Test_MultiplierWithStrandChannels =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection method:4 pointCount:2
		
		local multiplierInstance = Ox_Strand_Multiplier()
		modPanel.addModToSelection multiplierInstance
		
		-- Assign root selection channel, this should not cause a crash
		multiplierInstance.probabilityChannel = 1
		OxGetVertices distributionObject true
		
		delete $objects
	)
)

EUnit.AddTestClass Test_MultiplierModifier
ok
