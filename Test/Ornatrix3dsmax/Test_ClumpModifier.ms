struct Test_ClumpModifier
(
	function TestInitialize = 
	(
		OxResetClassParameters()
	),
	
	function TestCleanup =
	(
		delete $objects
	),
	
	function Test_BasicClumping =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:2 randomness:0)

		-- Get verts before 
		local vertexPositionsBefore = OxGetVertices distributionObject true
		
		local clumpInstance = Ox_Clump clumpCount:1
		modPanel.addModToSelection clumpInstance

		-- And after modifier
		local vertexPositionsAfter = OxGetVertices distributionObject true
		
		EAssert.ArePointsNotAllNearEqual vertexPositionsBefore vertexPositionsAfter
	),
	
	function SetMaximumClumpingCurve clump =
	(
		-- Make the clumping diagram maximum everywhere so all strands are perfectly clumped
		clump.amountCurveData = #( 103, 3, 0.00970874, 1, 3, 0.595146, 1, 3, 1, 1 )
	),
	
	function Test_GuideClumpingOnBakedHair =
	(
		local baseMesh = Plane lengthsegs:1 widthsegs:1
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:2 randomness:0)
		modPanel.addModToSelection (Ox_Hair_From_Guides method:4)
		modPanel.addModToSelection (Ox_Surface_Comb())
		
		maxOps.CollapseNode distributionObject off
 		distributionObject.distrObj = baseMesh
		distributionObject.displayFraction = 100
		distributionObject.detached = off
			
		local guideVertices = OxGetVertices distributionObject true
		
		modPanel.addModToSelection (Ox_Guides_From_Hair())
		modPanel.addModToSelection (Ox_Hair_From_Guides method:2 interpolation:1 denseCount:10 viewCountPercentage:100)
			
		local clump = Ox_Clump clumpCreateMethod:2 flyawayFraction:0
		modPanel.addModToSelection clump
		clump.CreateClumps()
		
		-- Make the clumping diagram maximum everywhere so all strands are perfectly clumped
		SetMaximumClumpingCurve clump
		
		local clumpedHairVertices = OxGetVertices distributionObject true
		
		-- Each hair needs to match one of the guides
		for i = 1 to ((clumpedHairVertices.count) / 2) do
		(
			local foundMatchingGuide = false
			for j = 1 to ((guideVertices.count) / 2) do
			(
				local firstPointDistance = distance clumpedHairVertices[i * 2 - 1] guideVertices[j * 2 - 1]
				local secondPointDistance = distance clumpedHairVertices[i * 2] guideVertices[j * 2]
				if (firstPointDistance < 0.001) and (secondPointDistance < 0.001) then
				(
					foundMatchingGuide = true
					break;
				)
			)
			
			EAssert.IsTrue foundMatchingGuide
		)
	),
	
	function Test_AddSingleClumpIndexChannel =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:2 randomness:0)
		EAssert.AreEqual 1 (OxGetStrandChannelCount distributionObject true)
		
		modPanel.addModToSelection (Ox_Clump clumpCount:1 addClumpIndexChannel:on)
		EAssert.AreEqual 2 (OxGetStrandChannelCount distributionObject true)
		
		modPanel.addModToSelection (Ox_Strand_Frizz ())
		EAssert.AreEqual 2 (OxGetStrandChannelCount distributionObject true)
	),
	
	function Test_SubClumpChangeInputHairCount =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:2 randomness:0)
		hairFromGuides = Ox_Hair_From_Guides method:0 denseCount:10 viewCountPercentage:100
		modPanel.addModToSelection hairFromGuides

		modPanel.addModToSelection (Ox_Clump clumpCount:1 flyawayFraction:0)
		modPanel.addModToSelection (Ox_Clump clumpCount:3 flyawayFraction:0)

		local uniqueTipPositionsBefore = TestUtilities.GetUniqueTipPositionCount distributionObject
		hairFromGuides.viewCountPercentage = 50
		local uniqueTipPositionsAfter = TestUtilities.GetUniqueTipPositionCount distributionObject
		
		-- Make sure that sub-clumped tips are more or less the same, despite dropping preview count
		EAssert.AreEqual uniqueTipPositionsBefore uniqueTipPositionsAfter
	),
	
	function Test_ClumpsDontChangeAfterRendering =
	(
		local cylinderHair = TestUtilities.AddBendingCylinder()
		
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:2 guideLength:10 randomness:0)
		local hairFromGuides = Ox_Hair_From_Guides method:7 denseCount:500 viewCountPercentage:50
		modPanel.addModToSelection hairFromGuides
		hairFromGuides.rememberPositions = on
		modPanel.addModToSelection (Ox_Clump clumpCount:10 flyawayFraction:0)
		
		sliderTime = 10f
		TestUtilities.RenderAndTestHairsForChanges cylinderHair
		
		sliderTime = 0f
		TestUtilities.RenderAndTestHairsForChanges cylinderHair
	),
	
	function Test_StrandGroups =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on

		max modify mode
		
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:2)
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		editGuidesInstance.useGuideGroups = on

		editGuidesInstance.SelectRoots #{1,2}
		editGuidesInstance.SetSelectedGuidesGroupIndex 2

		editGuidesInstance.SelectRoots #{3,4}
		editGuidesInstance.SetSelectedGuidesGroupIndex 3
		
		local clumpInstance = Ox_Clump clumpCount:1 flyawayFraction:0 useUvSpace:off
		modPanel.addModToSelection clumpInstance
		
		-- Applied to all guides, we should have one tip
		EAssert.AreEqual 1 (TestUtilities.GetUniqueTipPositionCount distributionObject)
		
		-- Applied to group 2 we should get first two strands to clump, with other two not being clumped
		clumpInstance.strandGroup = "2"
		EAssert.AreEqual 3 (TestUtilities.GetUniqueTipPositionCount distributionObject)
		
		clumpInstance.strandGroup = "5"
		EAssert.AreEqual 4 (TestUtilities.GetUniqueTipPositionCount distributionObject)
	),
	
	function Test_CopyModifier =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:10)
		local clumpInstance = Ox_Clump clumpCount:1 flyawayFraction:0 useUvSpace:off
		modPanel.addModToSelection clumpInstance
		
		-- Modify the diagrams
		SetMaximumClumpingCurve clumpInstance
		
		local verticesBeforeCopy = OxGetVertices distributionObject true
		local object2 = copy distributionObject
		local verticesAfterPaste = OxGetVertices object2 true
		
		EAssert.ArePointsAllNearEqual verticesBeforeCopy verticesAfterPaste
	),
	
	--  For now we need to ensure that adding clump modifier on top of hair without a distribution mesh won't crash 3dsmax (will be handled with a warning)
	function Test_ClumpWithoutDistributionMesh =
	(
		local hairObjectInstance = HairFromMeshStripsObject viewPercentage:100 isSelected:on distributionMethod:0 pointsPerStrand:2
		hairObjectInstance.AddStripObject (Plane lengthsegs:1 widthsegs:1)
		
		max modify mode
		local clumpInstance = Ox_Clump clumpCount:1 flyawayFraction:0 useUvSpace:off
		modPanel.addModToSelection clumpInstance
		
		-- We should only get a warning here, no assertions or crashes
	)
)

EUnit.AddTestClass Test_ClumpModifier
ok
