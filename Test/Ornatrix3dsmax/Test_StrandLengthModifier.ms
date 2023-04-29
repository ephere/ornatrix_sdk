struct Test_StrandLengthModifier
(	
	--Test Length parameter
	function Test_Length =
	(
		distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode

		-- Setup guides on object
		select distributionObject
		guidesFromSurfaceModifierInstance = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifierInstance
		guidesFromSurfaceModifierInstance.method = 4
		guidesFromSurfaceModifierInstance.randomness = 0
		guidesFromSurfaceModifierInstance.guideLength = 10
		modPanel.addModToSelection guidesFromSurfaceModifierInstance	
		
		--Create Strand Length Modifier and add it to object
		strandLengthModifierInstance = Ox_Strand_Length()
		modPanel.addModToSelection strandLengthModifierInstance	
		
		--Test Length - min value
		strandLengthModifierInstance.value = 0
		strandLengthModifierInstance.minimumValue = 0
		strandLengthModifierInstance.randomize = 0.0
		guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		myHair = ( guidesCopy.GetGuides() ).AsHair()
		local bakedDistance = 0
		EAssert.AreEqual 4 (myHair.NumRoots())
		EAssert.AreEqual 10 (myHair.NumPts 0)
		for i = 0 to ((myHair.NumPts 0) - 2) do
		(
			pointA = myHair.GetPoint 0 i
			pointB = myHair.GetPoint 0 (i + 1)
			bakedDistance += distance pointA pointB
		)
		
		EAssert.AreNearEqual 0 bakedDistance
		delete guidesCopy
		
		--Test Length - max value
		strandLengthModifierInstance.value = 1
		strandLengthModifierInstance.randomize = 0.0
		guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		myHair = ( guidesCopy.GetGuides() ).AsHair()
		local templateDistance = 0
		EAssert.AreEqual 4 (myHair.NumRoots())
		EAssert.AreEqual 10 (myHair.NumPts 0)
		for i = 0 to ( ( myHair.NumPts 0 ) - 2 ) do
		(
			pointA = myHair.GetPoint 0 i
			pointB = myHair.GetPoint 0 (i + 1)
			templateDistance += distance pointA pointB
		)
		
		delete guidesCopy
		
		strandLengthModifierInstance.value = 999
		strandLengthModifierInstance.randomize = 0.0
		guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		myHair = ( guidesCopy.GetGuides() ).AsHair()
		bakedDistance = 0
		EAssert.AreEqual 4 (myHair.NumRoots())
		EAssert.AreEqual 10 (myHair.NumPts 0)
		for i = 0 to ( ( myHair.NumPts 0 ) - 2 ) do
		(
			pointA = myHair.GetPoint 0 i
			pointB = myHair.GetPoint 0 ( i + 1 )
			bakedDistance += distance pointA pointB
		)
		
		EAssert.AreNearEqual strandLengthModifierInstance.value ( bakedDistance / templateDistance ) 
		delete guidesCopy
		
		-- Cleanup
		delete distributionObject
	),
	
	--Test Randomize parameter
	function Test_Randomize =
	(
		distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode

		-- Setup guides on object
		select distributionObject
		guidesFromSurfaceModifierInstance = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifierInstance
		guidesFromSurfaceModifierInstance.method = 4
		guidesFromSurfaceModifierInstance.randomness = 0
		guidesFromSurfaceModifierInstance.guideLength = 10
		modPanel.addModToSelection guidesFromSurfaceModifierInstance	
		
		--Create Strand Length Modifier and add it to object
		strandLengthModifierInstance = Ox_Strand_Length()
		strandLengthModifierInstance.value = 1
		modPanel.addModToSelection strandLengthModifierInstance	
		
		--Test Randomize Off
		strandLengthModifierInstance.randomize = 0.0
		guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		myHair = ( guidesCopy.GetGuides() ).AsHair()
		EAssert.AreEqual 4 (myHair.NumRoots())
		EAssert.AreEqual 10 (myHair.NumPts 0)
		local summaryDistance1 = 0
		etalonDistance = 0
		for rootID = 0 to ( myHair.NumRoots() - 1) do
		(
			for pointID = 0 to ((myHair.NumPts rootID) - 2) do
			(
				pointA = myHair.GetPoint rootID pointID
				pointB = myHair.GetPoint rootID ( pointID + 1 )
				summaryDistance1 += distance pointA pointB
				if ( rootID == 0 ) then
				(
					etalonDistance += distance pointA pointB
				)
			)
		)
		
		EAssert.AreNearEqual 1 ( summaryDistance1/( 4 * etalonDistance ) )
		delete guidesCopy
		
		--Test Randomize 0.75
		strandLengthModifierInstance.randomize = 0.75	
		guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		myHair = ( guidesCopy.GetGuides() ).AsHair()
		EAssert.AreEqual 4 (myHair.NumRoots())
		EAssert.AreEqual 10 (myHair.NumPts 0)
		local summaryDistance2 = 0
		local lengths = #(0,0,0,0)
		for rootID = 0 to ( myHair.NumRoots() - 1) do
		(
			for pointID = 0 to ((myHair.NumPts rootID) - 2) do
			(
				pointA = myHair.GetPoint rootID pointID
				pointB = myHair.GetPoint rootID ( pointID + 1 )				
				lengths[rootID + 1] += distance pointA pointB
			)
			
			summaryDistance2 += lengths[rootID + 1]
		)
		
		-- Each strand must have a unique random length, so no two strands should have same lengths
		EAssert.AreNotNearEqual lengths[1] lengths[2]
		EAssert.AreNotNearEqual lengths[1] lengths[3]
		EAssert.AreNotNearEqual lengths[1] lengths[4]
		EAssert.AreNotNearEqual lengths[2] lengths[3]
		EAssert.AreNotNearEqual lengths[2] lengths[4]
		EAssert.AreNotNearEqual lengths[3] lengths[4]
		
		EAssert.AreNotNearEqual summaryDistance1 summaryDistance2
		delete guidesCopy
		
		-- Cleanup
		delete distributionObject
	),
	
	--Test Seed parameters
	function Test_Seed = 
	(
		distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode

		-- Setup guides on object
		select distributionObject
		guidesFromSurfaceModifierInstance = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifierInstance
		guidesFromSurfaceModifierInstance.method = 4
		guidesFromSurfaceModifierInstance.randomness = 0
		guidesFromSurfaceModifierInstance.guideLength = 10
		modPanel.addModToSelection guidesFromSurfaceModifierInstance	
		
		--Create Strand Length Modifier and add it to object
		strandLengthModifierInstance = Ox_Strand_Length()
		strandLengthModifierInstance.value = 1
		strandLengthModifierInstance.randomize = 0.5
		modPanel.addModToSelection strandLengthModifierInstance	
		
		--Random Seed
		strandLengthModifierInstance.seed = 10
		guidesCopy1 = copy distributionObject
		convertTo guidesCopy1 GuidesObject
		myHair1 = ( guidesCopy1.GetGuides() ).AsHair()
		EAssert.AreEqual 4 (myHair1.NumRoots())
		EAssert.AreEqual 10 (myHair1.NumPts 0)
		
		--Random Seed
		strandLengthModifierInstance.seed = 64529
		guidesCopy2 = copy distributionObject
		convertTo guidesCopy2 GuidesObject
		myHair2 = ( guidesCopy2.GetGuides() ).AsHair()
		EAssert.AreEqual 4 (myHair2.NumRoots())
		EAssert.AreEqual 10 (myHair2.NumPts 0)
		
		hairsWithSameLength = 0
		for rootID = 0 to ( myHair1.NumRoots() - 1) do
		(
			hairLength1 = 0
			hairLength2 = 0
			for pointID = 0 to ((myHair1.NumPts rootID) - 2) do
			(
				pointA1 = myHair1.GetPoint rootID pointID
				pointB1 = myHair1.GetPoint rootID ( pointID + 1 )
				hairLength1 += distance pointA1 pointB1
				
				pointA2 = myHair2.GetPoint rootID pointID
				pointB2 = myHair2.GetPoint rootID ( pointID + 1 )
				hairLength2 += distance pointA2 pointB2
			)
			
			if ( hairLength1 == hairLength2 ) then
			(
				hairsWithSameLength += 1
			)
		)
		--Compare Hairs with same length
		EAssert.AreNotEqual 4 hairsWithSameLength
		
		-- Cleanup
		delete guidesCopy1
		delete guidesCopy2
		delete distributionObject
	),
	
	--Test Length Map parameter
	function Test_Length_Map =
	(
		distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode

		-- Setup guides on object
		select distributionObject
		guidesFromSurfaceModifierInstance = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifierInstance
		guidesFromSurfaceModifierInstance.method = 4
		guidesFromSurfaceModifierInstance.randomness = 0
		guidesFromSurfaceModifierInstance.guideLength = 10
		modPanel.addModToSelection guidesFromSurfaceModifierInstance	
		
		--Create Strand Length Modifier and add it to object
		strandLengthModifierInstance = Ox_Strand_Length()
		strandLengthModifierInstance.value = 1
		strandLengthModifierInstance.randomize = 0.0
		modPanel.addModToSelection strandLengthModifierInstance	
		
		--Before Assign Map
		guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		myHair = ( guidesCopy.GetGuides() ).AsHair()
		EAssert.AreEqual 4 (myHair.NumRoots())
		EAssert.AreEqual 10 (myHair.NumPts 0)
		distanceBefore = 0
		for pointID = 0 to ((myHair.NumPts 0) - 2) do
		(
			pointA = myHair.GetPoint 0 pointID
			pointB = myHair.GetPoint 0 ( pointID + 1 )
			distanceBefore += distance pointA pointB
		)
		delete guidesCopy
		
		--After Assign Map
		lengthMap = Checker ()
		lengthMap.color1 = lengthMap.color2 = color 128 128 128
		strandLengthModifierInstance.lengthMap = lengthMap
		EAssert.IsTrue( strandLengthModifierInstance.lengthMap != undefined )
		guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		myHair = ( guidesCopy.GetGuides() ).AsHair()
		EAssert.AreEqual 4 (myHair.NumRoots())
		EAssert.AreEqual 10 (myHair.NumPts 0)
		distanceAfter = 0
		for pointID = 0 to ((myHair.NumPts 0) - 2) do
		(
			pointA = myHair.GetPoint 0 pointID
			pointB = myHair.GetPoint 0 ( pointID + 1 )
			distanceAfter += distance pointA pointB
		)
		delete guidesCopy
		--Compare Hairs Length Before and After
		EAssert.AreNearEqual 2 ( distanceBefore / distanceAfter + 0.00781 )
		
		--Remove Length Map
		strandLengthModifierInstance.lengthMap = undefined
		EAssert.IsTrue( strandLengthModifierInstance.lengthMap == undefined )
		
		-- Cleanup
		delete distributionObject
	),
	
	--Test Other Parameters: Length Channel and Map Channel
	function Test_Others =
	(		
		distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode

		-- Setup guides on object
		select distributionObject
		guidesFromSurfaceModifierInstance = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifierInstance
		guidesFromSurfaceModifierInstance.method = 4
		guidesFromSurfaceModifierInstance.randomness = 0
		guidesFromSurfaceModifierInstance.guideLength = 10
		modPanel.addModToSelection guidesFromSurfaceModifierInstance	
		
		--Create Strand Length Modifier and add it to object
		strandLengthModifierInstance = Ox_Strand_Length()
		strandLengthModifierInstance.value = 1
		modPanel.addModToSelection strandLengthModifierInstance	
		
		--Test Length Channel: 1 - Root(0): Selection 
		strandLengthModifierInstance.lengthChan = 1
		EAssert.AreEqual 1 strandLengthModifierInstance.lengthChan
		
		--Test Length Channel: 65536 - Mesh(0): Vertex Color
		strandLengthModifierInstance.lengthChan = 65536
		EAssert.AreEqual 65536 strandLengthModifierInstance.lengthChan
		
		--Test Length Channel: 131072 - Mesh(1): Mapping Channel
		strandLengthModifierInstance.lengthChan = 131072
		EAssert.AreEqual 131072 strandLengthModifierInstance.lengthChan
		
		--Test Length Channel: 0 - Disable
		strandLengthModifierInstance.lengthChan = 0
		EAssert.AreEqual 0 strandLengthModifierInstance.lengthChan
		
		-- Cleanup
		delete distributionObject		
	)
)

EUnit.AddTestClass Test_StrandLengthModifier
ok
