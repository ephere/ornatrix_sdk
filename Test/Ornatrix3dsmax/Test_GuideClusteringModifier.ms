struct Test_GuideClusteringModifier
(
	function Test_GuideClusteringTwisting =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on

		max modify mode

		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 2
		
		local hairFromGuidesModifierInstance = HairFromGuidesModifier interpolation:3
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		local tipsAfterHair = TestUtilities.GetTipPositions distributionObject
		
		local guideClusteringModifierInstance = GuideClusteringModifier()
		modPanel.addModToSelection guideClusteringModifierInstance
		
		local tipsAfterClustering = TestUtilities.GetTipPositions distributionObject
		EAssert.ArePointsNotAllNearEqual tipsAfterHair tipsAfterClustering
		
		guideClusteringModifierInstance.twistAmt = 0.5
		
		local tipsAfterTwist = TestUtilities.GetTipPositions distributionObject
		EAssert.ArePointsNotAllNearEqual tipsAfterClustering tipsAfterTwist
			
		-- Cleanup
		delete distributionObject
	)
)

EUnit.AddTestClass Test_GuideClusteringModifier
ok
