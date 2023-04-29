struct Test_FilterModifier
(
	function TestCleanup =
	(
		delete $objects
	),
	
	function Test_Filter =
	(
		EUnit.SkipBeforeMaxVersion 2017

		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on
		max modify mode

		select distributionObject

		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 2
		guidesFromSurfaceModifier.numSegs = 10
		guidesFromSurfaceModifier.randomness = 0
		guidesFromSurfaceModifier.count = 10
		modPanel.addModToSelection guidesFromSurfaceModifier

		-- Get verts before 
		local vertexPositionsBefore = OxGetVertices distributionObject true
		
		local filterInstance = Ox_Filter()
		modPanel.addModToSelection filterInstance

		-- And after modifier
		local vertexPositionsAfter = OxGetVertices distributionObject true

		EAssert.AreNotEqual vertexPositionsAfter.count vertexPositionsBefore.count
	)
)

EUnit.AddTestClass Test_FilterModifier
ok
