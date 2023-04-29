struct Test_GuidesFromShapeObject
(
	function TestCleanup =
	(
		delete $objects
	),
	
	function Test_SplineToGuideTranslation =
	(
		local firstPoint = [0,0,0]
		local secondPoint = [-1,5,10]
		
		local spline1 = line()
		addNewSpline spline1
		addKnot spline1 1 #corner #line firstPoint
		addKnot spline1 1 #corner #line secondPoint
		updateShape spline1
		
		local spline2 = line()
		addNewSpline spline2
		addKnot spline2 1 #corner #line firstPoint
		addKnot spline2 1 #corner #line secondPoint
		updateShape spline2

		local guidesFromShapeObject = Guides_from_Shape()
		append (guidesFromShapeObject.shapes) spline1
		append (guidesFromShapeObject.shapes) spline2
		
		EAssert.AreEqual 2 (OxGetStrandCount guidesFromShapeObject)
	)
)

EUnit.AddTestClass Test_GuidesFromShapeObject
ok
