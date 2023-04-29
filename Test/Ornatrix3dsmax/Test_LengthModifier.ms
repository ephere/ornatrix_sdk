struct Test_LengthModifier
(
	function Test_LoadWithLengthMapAndNoInputStrands =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode
		select distributionObject
		
		-- Add guides from surface modifier with a black/default distribution channel which should result in no guides being generated
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:2 distrChan:65536)
		EAssert.AreEqual 0 (OxGetStrandCount distributionObject)
		
		-- Add a length modifier with a length map
		local lengthModifierInstance = Ox_Strand_Length lengthMap:(Checker())
		modPanel.addModToSelection lengthModifierInstance
		
		-- If nothing crashed we are good

		-- Cleanup
		delete distributionObject
	),
	
	function Test_GradientRampAsLengthMap =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		
		-- Add guides from surface modifier with a black/default distribution channel which should result in no guides being generated
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		-- Add a length modifier with a length map
		local lengthModifierInstance = Ox_Strand_Length lengthMap:(Gradient_Ramp())
		modPanel.addModToSelection lengthModifierInstance
		
		-- If nothing crashed we are good

		-- Cleanup
		delete distributionObject
	),
	
	function Test_LengthWithMapChannel =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		
		-- Add guides from surface modifier with a black/default distribution channel which should result in no guides being generated
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		-- Add a length modifier with a length map
		local lengthModifierInstance = Ox_Strand_Length lengthChan:65536
		modPanel.addModToSelection lengthModifierInstance
		
		-- If nothing crashed we are good

		-- Cleanup
		delete $objects
	)
)

EUnit.AddTestClass Test_LengthModifier
ok
