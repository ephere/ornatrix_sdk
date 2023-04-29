struct Test_BraidGuidesObject
(
	function TestCleanup =
	(
		delete $objects
	),
	
	-- Just a basic test that braid guides object exists and produces output
	function Test_BraidGuidesObject =
	(
		OxResetClassParameters (BraidGuidesObject())
		delete $objects
			
		local braidGuides = BraidGuidesObject()
			
		-- A pigtail braid by default
		EAssert.AreEqual 3 (OxGetStrandCount braidGuides)
	)
)

EUnit.AddTestClass Test_BraidGuidesObject
ok
