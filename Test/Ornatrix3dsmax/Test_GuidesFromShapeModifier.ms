struct Test_GuidesFromShapeModifier
(
	-- Tests that a distribution map can be assigned to control which proxy mesh is used for which strand in Mesh From Strands modifier
	function Test_SplineToGuideTranslation =
	(
		local firstPoint = [0,0,0]
		local secondPoint = [-1,5,10]
		
		local spline = line()
		addNewSpline spline
		addKnot spline 1 #corner #line firstPoint
		addKnot spline 1 #corner #line secondPoint
		updateShape spline

		local groomNodeName = "GroomNode"
		spline.Name = groomNodeName
		max modify mode
		select spline
		
		-- Add guides from shape modifier
		modPanel.addModToSelection (Ox_Guides_From_Shape())

		local guideVertices = OxGetVertices spline true
		EAssert.ArePointsNearEqual firstPoint guideVertices[1]
		EAssert.ArePointsNearEqual secondPoint guideVertices[guideVertices.Count]
			
		delete spline
	),
	
	function Test_GuidesFromShapeGroupInitializedTo0 =
	(
		-- Use text because it will produce many guides
		local shapeObject = text size:20 isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_From_Shape())
			
		for i = 0 to (OxGetStrandCount shapeObject) - 1 do
		(
			EAssert.AreEqual 0 (OxGetStrandGroupIndex shapeObject i)
		)
		
		delete $objects
	)
)

EUnit.AddTestClass Test_GuidesFromShapeModifier
ok
