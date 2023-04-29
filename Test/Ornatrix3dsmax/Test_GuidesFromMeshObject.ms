struct Test_GuidesFromMeshObject
(
	function TestInitialize =
	(
		OxResetClassParameters ()
	),
	
	-- Tests that a distribution map can be assigned to control which proxy mesh is used for which strand in Mesh From Strands modifier
	function Test_UVDistribution =
	(
		-- Create a plane with 1x1 segments (so that there are are no vertices going exactly down the middle)
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		Guides distrSurf:distributionObject uvChannel:1 isSelected:on
		guidesNode = $

		max modify mode
		
		-- Set mode to UV-based generation
		guidesNode.method = 1
		
		-- Roots should be generated
		EAssert.IsTrue ( ( ( guidesNode.GetGuides() ).AsHair() ).NumRoots() > 0 )

		-- Cleanup
		delete guidesNode
		delete distributionObject
	),
	
	function Test_DisplacementMapAndOffset =
	(
		-- Create a plane with 1x1 segments (so that there are are no vertices going exactly down the middle)
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		Guides distrSurf:distributionObject isSelected:on
		guidesNode = $

		max modify mode
			
		-- Set default parameters
		guidesNode.method = 0
		guidesNode.count = 10
		guidesNode.guideLength = 10
		guidesNode.randomness = 0
		guidesNode.NumSegs = 2
		
		-- Without Displacement
		guidesNode.displacementMap = undefined
		guidesNode.displacementOffset = 0	
		guidesCopy = instance guidesNode
		convertTo guidesCopy GuidesObject
		myHair = ( guidesCopy.GetGuides() ).AsHair()
		ZeroDisplaceTip = 0
		ZeroDisplaceRoot = 0
		for i = 0 to ( myHair.NumRoots() - 1 ) do
		(
			hairRootPoint = myHair.GetPointT i 0
			hairTipPoint = myHair.GetPointT i 1
			ZeroDisplaceTip += hairTipPoint.z
			ZeroDisplaceRoot += hairRootPoint.z
		)
		
		EAssert.AreNearEqual 0 ( ZeroDisplaceRoot )
		
		--With Displacement (with middle grey map and double Guide's Length as offset, it should be at the top of each strands without displacement)
		DisplacementMap = Checker ()
		DisplacementMap.color1 = DisplacementMap.color2 = (color 255 255 255) / 2
		guidesNode.displacementMap = DisplacementMap
		guidesNode.displacementOffset = guidesNode.guideLength * 2
		guidesCopy2 = instance guidesNode
		convertTo guidesCopy2 GuidesObject
		myHair2 = ( guidesCopy2.GetGuides() ).AsHair()
		DisplaceRoot = 0
		for i = 0 to ( myHair2.NumRoots() - 1 ) do
		(
			hairRootPoint = myHair2.GetPointT i 0
			DisplaceRoot += hairRootPoint.z
		)
		
		EAssert.AreEqual ZeroDisplaceTip DisplaceRoot

		-- Cleanup
		delete guidesNode
		delete distributionObject
		delete guidesCopy
		delete guidesCopy2
	),
	
	function Test_DisplacementMapChannels =
	(
		-- Create a plane with 1x1 segments (so that there are are no vertices going exactly down the middle)
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on

		-- Create Mapping Channels
		for i = 1 to 2 do
		(
			uvwMapModifier = Uvwmap ()
			uvwMapModifier.mapChannel = i
			uvwMapModifier.length = distributionObject.length / i
			uvwMapModifier.width = distributionObject.width / i
			modPanel.addModToSelection uvwMapModifier
		)
		
		Guides distrSurf:distributionObject isSelected:on
		guidesNode = $

		max modify mode
			
		-- Set default parameters
		guidesNode.method = 2
		guidesNode.uvChannel = 1
		guidesNode.count = 100
		guidesNode.guideLength = 10
		guidesNode.randomness = 0
		guidesNode.NumSegs = 2
		
		local DisplacementMap = Checker ()
		DisplacementMap.color1 = black
		DisplacementMap.color2 = white
		guidesNode.displacementMap = DisplacementMap
		guidesNode.displacementOffset = guidesNode.guideLength
		
		--Bake 1-st Mapping Channel
		DisplacementMap.coords.mapChannel = 1
		guidesCopy1 = instance guidesNode
		convertTo guidesCopy1 GuidesObject
		myHair1 = ( guidesCopy1.GetGuides() ).AsHair()
		
		--Bake 2-nd Mapping Channel
		DisplacementMap.coords.mapChannel = 2
		guidesCopy2 = instance guidesNode
		convertTo guidesCopy2 GuidesObject
		myHair2 = ( guidesCopy2.GetGuides() ).AsHair()
		
		--Hairs Count should be the same
		EAssert.AreEqual ( myHair1.NumRoots() ) ( myHair2.NumRoots() )
		
		--Check for matching, if Mapping channel isn't affected, 100% points will be at the same position
		matchedPoints = 0
		for i = 0 to ( myHair1.NumRoots() - 1 ) do
		(
			RootPoint1 = myHair1.GetPointT i 0
			RootPoint2 = myHair2.GetPointT i 0
			if ( RootPoint1 == RootPoint2 ) then
			(
				matchedPoints += 1
			)
		)
		
		EAssert.AreNotEqual matchedPoints ( myHair1.NumRoots() )
		
		-- Cleanup
		delete guidesNode
		delete distributionObject
		delete guidesCopy1
		delete guidesCopy2
	),
	
	function Test_RememberButton =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		local guidesObjectInstance = Guides distrSurf:distributionObject method:0 count:10 displacementOffset:0 useSubSel:off useMatID:off uvChannel:1 isSelected:on
			
		-- Get original vertex positions
		local pointPositionsBeforeRemember = TestUtilities.GetPointPositions guidesObjectInstance
		
		-- Turn remember on
		guidesObjectInstance.rememberPositions = on
		local pointPositionsAfterRemember = TestUtilities.GetPointPositions guidesObjectInstance
		local strandCountDuringRemember = OxGetStrandCount guidesObjectInstance
		EAssert.IsGreaterThan 0 strandCountDuringRemember
		
		-- Positions shouldn't have changed
		EAssert.ArePointsAllNearEqual pointPositionsBeforeRemember pointPositionsAfterRemember
		
		-- Changing root count should have no effect on outcome
		guidesObjectInstance.count = 5
		local pointPositionsAfterRootCountChange = TestUtilities.GetPointPositions guidesObjectInstance
		EAssert.ArePointsAllNearEqual pointPositionsAfterRemember pointPositionsAfterRootCountChange
		
		-- Turning off remember should change the guides
		guidesObjectInstance.rememberPositions = off
		local pointPositionsAfterForget = TestUtilities.GetPointPositions guidesObjectInstance
		EAssert.AreNotEqual strandCountDuringRemember (OxGetStrandCount guidesObjectInstance)
		
		-- Cleanup
		delete guidesObjectInstance
		delete distributionObject
	)
)

EUnit.AddTestClass Test_GuidesFromMeshObject
ok
