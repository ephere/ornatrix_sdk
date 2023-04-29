struct Test_CurlingModifier
(
	function TestCleanup =
	(
		delete $objects
	),
	
	-- Makes sure that curling can be applied along the Y axis to prevent a regression where Y axis would not be applicable since it coincides with the "up vector" used in curling
	function Test_CurlingAlongYAxis =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Strand_Curling())
		OxResetClassParameters (HairFromMeshStripsObject())
			
		-- Create a hair strips object on a default plane without rotating the strands to make sure they are generated along the Y axis
		local stripObject1 = Plane lengthsegs:1 widthsegs:1
		local hairObjectInstance = HairFromMeshStripsObject()
		hairObjectInstance.distributionMethod = 1
		hairObjectInstance.hairsPerStrip = 1
		hairObjectInstance.viewPercentage = 100
		hairObjectInstance.pointsPerStrand = 5
		hairObjectInstance.AddStripObject stripObject1

		-- Evaluate hair to force strips modifier to calculate, this is needed for the next step
		local hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		delete hairCopy

		-- Make sure the strips are aligned on the Y axis even if other tests changed this
		--hairObjectInstance.rootBoundaryIndices[1] = 1
		hairObjectInstance.InvalidateMeshStrips()

		hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		local myHair = hairCopy.GetHair()
		EAssert.AreEqual 1 (myHair.NumRoots())
		local point2A = myHair.GetPoint 0 1
		delete hairCopy

		select hairObjectInstance
		max modify mode

		-- Add curling modifier and increase the magnitude to make sure we see some change
		local curlingModifierInstance = Ox_Strand_Curling()
		curlingModifierInstance.magn = 10
		modPanel.addModToSelection curlingModifierInstance

		-- Evaluate the hair again, the points should be significantly different
		hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		myHair = hairCopy.GetHair()
		local point2B = myHair.GetPoint 0 1
		delete hairCopy

		EAssert.IsTrue( Length( point2B - point2A ) > 1 )
	),
	
	function Test_CurlingOnStripsDoesntChangeWhenStripsAreMoved =
	(
		-- Create a hair strips object on a default plane without rotating the strands to make sure they are generated along the Y axis
		local stripObject1 = Plane lengthsegs:1 widthsegs:1
		local hairObjectInstance = HairFromMeshStripsObject hairsPerStrip:1 viewPercentage:100 pointsPerStrand:5 isSelected:on
		hairObjectInstance.AddStripObject stripObject1
		max modify mode

		-- Add curling modifier and increase the magnitude to make sure we see some change
		local curlingModifierInstance = Ox_Strand_Curling()
		curlingModifierInstance.magn = 10
		modPanel.addModToSelection curlingModifierInstance

		-- Evaluate the hair again, the points should be significantly different
		local verticesBeforeMoving = OxGetVertices hairObjectInstance true
		
		local offsetVector = [0,5,0]
		
		-- Move the strip object slightly
		move stripObject1 offsetVector
		
		-- For some reason in 3dsmax 2009 we need to move an extra time before results kick in
		if (maxVersion())[1] <= 11000 then
		move stripObject1 [0,0,0]
		
		-- Make sure that the strand curling didn't change
		local verticesAfterMoving = OxGetVertices hairObjectInstance true
		
		EAssert.ArePointsNearEqual ( verticesAfterMoving[1] - offsetVector ) verticesBeforeMoving[1]
	),
	
	function Test_ChangeValueAfterChangingGlobalSegmentOrientation =
	(
		OxResetClassParameters()
		
		local distributionObject = Sphere radius:10 segs:4 mapcoords:on isSelected:on
		max modify mode

		local guidesFromSurface = Ox_Guides_from_Surface method:4 numSegs:2 randomness:0
		modPanel.addModToSelection guidesFromSurface
		local hairFromGuides = Ox_Hair_From_Guides method:4
		modPanel.addModToSelection hairFromGuides
		local curlingModifierInstance = Ox_Strand_Curling form:0 magn:10 phase:0.4
		modPanel.addModToSelection curlingModifierInstance
		
		hairFromGuides.globalSegmentOrientations = on
		local verticesBeforeAdjustingCurling = OxGetVertices distributionObject true
		curlingModifierInstance.magn += 0.1
		local verticesAfterAdjustingCurling = OxGetVertices distributionObject true
		
		-- The change in hair vertices shouldn't be significant
		EAssert.ArePointsAllNearEqual verticesBeforeAdjustingCurling verticesAfterAdjustingCurling epsilon:1
	),
	
	function Test_MagnitudeMap =
	(
		OxResetClassParameters()
		local curledGuides = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		local pointsPerStrand = 5
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:pointsPerStrand randomness:0)
		local verticesBeforeCurling = OxGetStrandPointsSortedByStrandId curledGuides true
		
		local curlingMod = Ox_Strand_Curling form:0 magn:10 phase:1.5
		modPanel.addModToSelection curlingMod
			
		local verticesBeforeMap = OxGetStrandPointsSortedByStrandId curledGuides true
		curlingMod.magnitudeMap = Checker()
		curlingMod.magnitudeMap.coords.U_Tiling = 0.05
		curlingMod.magnitudeMap.coords.V_Tiling = 0.05
		local verticesAfterMap = OxGetStrandPointsSortedByStrandId curledGuides true
		
		-- The checker map should result in two strands not being changed and the other two being changed to default
		local epsilon = 0.1
		EAssert.ArePointsAllNearEqual verticesBeforeMap verticesAfterMap startIndex:1 endIndex:(pointsPerStrand) epsilon:epsilon
		EAssert.ArePointsAllNearEqual verticesBeforeCurling verticesAfterMap startIndex:(1 + pointsPerStrand) endIndex:(pointsPerStrand * 2) epsilon:epsilon
		EAssert.ArePointsAllNearEqual verticesBeforeCurling verticesAfterMap startIndex:(1 + pointsPerStrand * 2) endIndex:(pointsPerStrand * 3) epsilon:epsilon
		EAssert.ArePointsAllNearEqual verticesBeforeMap verticesAfterMap startIndex:(1 + pointsPerStrand * 3) endIndex:(pointsPerStrand * 4) epsilon:epsilon
	)
)

EUnit.AddTestClass Test_CurlingModifier
ok
