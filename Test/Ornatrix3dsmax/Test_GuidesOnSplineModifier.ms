struct Test_GuidesOnSplineModifier
(
	-- Test for defect 2084
	function Test_CloningCopiesLengthDiagramData =
	(
		local distributionObject = Circle radius:10

		max modify mode

		-- Add strand symmetry
		select distributionObject

		local guidesOnSplineMod = Ox_Guides_on_Spline()
		guidesOnSplineMod.count = 10
		
		-- Set the diagram data with 2 data points
		guidesOnSplineMod.lengthDiagData[1] = 0.4
		guidesOnSplineMod.lengthDiagData[2] = 0.0
		
		modPanel.addModToSelection guidesOnSplineMod

		-- Clone the distribution object (and the modifier along with it)
		maxOps.CloneNodes #(distributionObject)
		
		-- Get the newly cloned object
		clonedObject = $Circle002
		EAssert.AreNotEqual distributionObject clonedObject
		
		-- The cloned object should have the same diagram data
		clonedModifier = clonedObject.modifiers[1]
		EAssert.AreEqual 0.4 clonedModifier.lengthDiagData[1]
		EAssert.AreEqual 0.0 clonedModifier.lengthDiagData[2]
			
		-- Cleanup
		delete distributionObject
		delete clonedObject
	),
	
	function Test_AllGuidesAreFacingInSameDirection =
	(
		local distributionObject = Circle radius:10

		max modify mode

		-- Add strand symmetry
		select distributionObject

		local guidesOnSplineMod = Ox_Guides_on_Spline()
		guidesOnSplineMod.method = 0
		guidesOnSplineMod.lenRand = 0
		guidesOnSplineMod.NumPts = 2
		guidesOnSplineMod.count = 10
		
		modPanel.addModToSelection guidesOnSplineMod

		-- All tips should be at the same Z coordinate
		local guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		local myGuides = ( guidesCopy.GetGuides() ).AsHair()
		local guide1Tip = myGuides.GetPointT 0 1
		
		for i = 1 to (myGuides.NumRoots() - 1) do
		(
			local guideTip = myGuides.GetPointT i 1
			EAssert.AreEqual (guide1Tip.z) (guideTip.z)
		)
		
		delete guidesCopy
			
		-- Cleanup
		delete distributionObject
	)
)

EUnit.AddTestClass Test_GuidesOnSplineModifier
ok
