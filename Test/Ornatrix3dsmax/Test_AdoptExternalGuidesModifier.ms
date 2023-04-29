struct Test_AdoptExternalGuidesModifier
(
	function TestCleanup =
	(
		delete $objects
	),
	
	function Test_AdoptExternalGuides =
	(
		OxResetClassParameters( Ox_Guides_from_Surface() )
		OxResetClassParameters( Ox_Hair_From_Guides() )
		OxResetClassParameters( Ox_Adopt_External_Guides() )
		
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		local guidesDistributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode
		
		-- Set up guides object
		select guidesDistributionObject
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:2 randomness:0)
		local editGuides = Ox_Edit_Guides()
		modPanel.addModToSelection editGuides

		-- Set up hair object
		select distributionObject
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:2 randomness:0)
		local hairFromGuidesInstance = Ox_Hair_From_Guides denseCount:10 viewCountPercentage:100
		modPanel.addModToSelection hairFromGuidesInstance
		modPanel.addModToSelection (Ox_Adopt_External_Guides controlObject:guidesDistributionObject)
			
		local hairVerticesBeforeGuideEditing = OxGetStrandPointsSortedByStrandId distributionObject true
			
		-- Modify the guides and make sure hair is modified with them
		modPanel.setCurrentObject editGuides
		subobjectLevel = 2
		editGuides.SelectRoots #{1..2}
		editGuides.MoveSelectedControlPoints [0, 10, 0]
			
		local hairVerticesAfterGuideEditing = OxGetStrandPointsSortedByStrandId distributionObject true
		EAssert.ArePointsNotAllNearEqual hairVerticesBeforeGuideEditing hairVerticesAfterGuideEditing
			
		-- Reload the scene and make sure that the changes persist on the hair
		local hairObjectName = distributionObject.Name
		holdMaxFile()
		fetchMaxFile quiet:true
		distributionObject = GetNodeByName hairObjectName

		local hairVerticesAfterReloading = OxGetStrandPointsSortedByStrandId distributionObject true
		EAssert.ArePointsAllNearEqual hairVerticesAfterGuideEditing hairVerticesAfterReloading
	)
)

EUnit.AddTestClass Test_AdoptExternalGuidesModifier
ok
