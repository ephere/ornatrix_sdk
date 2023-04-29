struct Test_GenerateGuideDataModifier
(
	function TestCleanup =
	(
		delete $objects
	),
	
	function Test_GenerateGuideData =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (GenerateGuideDataModifier())
		OxResetClassParameters (Ox_Strand_Frizz())
		OxResetClassParameters (Ox_Guides_From_Surface())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on

		max modify mode

		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		local generateGuideDataModifierInstance = GenerateGuideDataModifier createNewChannel:on newChannelName:"TestChannel"
		modPanel.addModToSelection generateGuideDataModifierInstance
		
		local frizzModifierInstance = Ox_Strand_Frizz amountChan:2
		modPanel.addModToSelection frizzModifierInstance
		
		local vertices1 = OxGetVertices distributionObject true
		generateGuideDataModifierInstance.minimumTargetValue = 5
		local vertices2 = OxGetVertices distributionObject true
		
		EAssert.ArePointsNotAllNearEqual vertices1 vertices2
	),
	
	function Test_RememberCurrentValues =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		guidesFromSurfaceModifier.randomness = 100
		
		-- Add generate guide data modifier with strand length as generation method
		local generateGuideDataModifierInstance = GenerateGuideDataModifier createNewChannel:on newChannelName:"TestChannel" generationMethod:3
		modPanel.addModToSelection generateGuideDataModifierInstance
		
		-- Add render settings modifier which uses the generated guide data channel to set hair widths
		modPanel.addModToSelection (Ox_Render_Settings thicknessChan:2)
		
		-- Convert to mesh so we can compare thickness
		modPanel.addModToSelection (Ox_Mesh_From_Strands())
		
		local verticesBeforeLengthChanges = TestUtilities.GetVertexArray distributionObject
		
		-- Store values, and evaluate once just for caching
		generateGuideDataModifierInstance.isStoringValues = on
		TestUtilities.GetVertexArray distributionObject
		
		guidesFromSurfaceModifier.randomness = 0
		local verticesAfterLengthChanges = TestUtilities.GetVertexArray distributionObject
		
		-- If the values were indeed cached we should not have any change in thickness of strands
		for i = 0 to ((verticesBeforeLengthChanges.count) / 2 - 1) do
		(
			local previousWidth = distance verticesBeforeLengthChanges[i * 2 + 1] verticesBeforeLengthChanges[i * 2 + 2]
			local currentWidth = distance verticesAfterLengthChanges[i * 2 + 1] verticesAfterLengthChanges[i * 2 + 2]
			
			EAssert.AreEqual previousWidth currentWidth
		)
	),
	
	function Test_GenerateThreeStrandDataChannels =
	(
		OxResetClassParameters ()
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		guidesFromSurfaceModifier.randomness = 100
		
		modPanel.addModToSelection (GenerateGuideDataModifier createNewChannel:on newChannelName:"TestChannel" generationMethod:3 sampleValueCount:3)
		
		-- One selection channel and 3 newly added ones
		EAssert.AreEqual 4 (OxGetStrandChannelCount distributionObject true)
	),
	
	-- Tests that there is no error/crash when we set output as texture coordinates and input hairs don't have any strands
	function Test_TargetTextureCoordsChannelWith0Strands =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection method:2 count:0
		modPanel.addModToSelection (GenerateGuideDataModifier targetData:5 generationMethod:3 sampleValueCount:3)
		
		-- No crash or assertions should happen here
	)
)

EUnit.AddTestClass Test_GenerateGuideDataModifier
ok
