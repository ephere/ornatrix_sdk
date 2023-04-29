struct Test_AlembicExporter
(
	function TestCleanup =
	(
		delete $objects
	),
	
	-- Tests that a single selected hair is written to an Alembic abc file
	function Test_ExportSingleStaticHair =
	(
		-- Create some hair
		local distributionObject = Plane lengthsegs:2 widthsegs:2 isSelected:on
		max modify mode

		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		modPanel.addModToSelection guidesFromSurfaceModifier

		-- Export to an alembic file
		local resultFilePath = TestUtilities.GetTestFilePath "SingleStaticHair.abc"
		OxAlembicExport resultFilePath true 0 0 false 2 false 1

		-- Make sure the file was written without errors
		EAssert.IsTrue( ( getfiles resultFilePath ).count != 0 )

		-- NOTE: Using near equal instead of equal because the size of .abc file will depend on the length of Ornatrix version string.
		-- If version string has more digits we will get larger file size (e.g. "6.0.9" will be 2 bytes smaller than "6.0.10")
		EAssert.AreNearEqual 8847L ( getFileSize ( ( getfiles resultFilePath )[1] ) ) epsilon:2L
			
		-- Cleanup
		deleteFile resultFilePath
	),
	
	-- Tests that a single selected hair is written to an Alembic abc file
	function Test_ExportSingleAnimatedHair =
	(
		-- .abc file size is different
		EUnit.SkipBeforeMaxVersion 2021

		-- Create some hair
		local distributionObject = Plane lengthsegs:2 widthsegs:2
		max modify mode
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		
		-- Animate the length value to make the hair animated
		with animate on
		(
			at time 0 guidesFromSurfaceModifier.guideLength = 10
			at time 50 guidesFromSurfaceModifier.guideLength = 20
		)
		
		modPanel.addModToSelection guidesFromSurfaceModifier

		-- Export to an alembic file
		resultFilePath = TestUtilities.GetTestFilePath "SingleAnimatedHair.abc"
		OxAlembicExport resultFilePath true 0 100 false 2 false 1

		-- Make sure the file was written without errors
		EAssert.IsTrue( ( getfiles resultFilePath ).count != 0 )
		EAssert.AreNearEqual 92310L ( getFileSize ( ( getfiles resultFilePath )[1] ) ) epsilon:2L
			
		-- Cleanup
		deleteFile resultFilePath
	),

	function IsNearEqual value1 value2 epsilon =
	(
		abs( value2 - value1 ) <= epsilon
	),
	
	function Test_ExportMultipleStaticHair =
	(
		-- Create some hair
		local distributionObject1 = Plane lengthsegs:1 widthsegs:1
		local distributionObject2 = Plane lengthsegs:1 widthsegs:1
		
		max modify mode
		
		select distributionObject1
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		select distributionObject2
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()

		-- Export to an alembic file
		resultFilePath = TestUtilities.GetTestFilePath "MultipleStaticHair.abc"
		OxAlembicExport resultFilePath false 0 0 false 2 false 1

		-- Make sure the file was written without errors
		EAssert.IsTrue( ( getfiles resultFilePath ).count != 0 )
		local fileSize = getFileSize ( ( getfiles resultFilePath )[1] )
		
		-- TC client gives 11831L result while local build gives the other one
		EAssert.IsTrue ( (IsNearEqual fileSize 12641L 2L) or (IsNearEqual fileSize 11831L 2L) ) message: ("Actual size: " + (fileSize as string))
			
		-- Cleanup
		deleteFile resultFilePath
	),
	
	function Test_ExportRenderHairs =
	(
		-- Create some hair
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode
		select distributionObject
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local hairFromGuidesModifierInstance = HairFromGuidesModifier method:1 viewCountPercentage:50 denseCount:200
		modPanel.addModToSelection hairFromGuidesModifierInstance

		-- Export to an alembic file
		local nonRenderFilePath = TestUtilities.GetTestFilePath "nonRender.abc"
		local renderFilePath = TestUtilities.GetTestFilePath "render.abc"
		OxAlembicExport nonRenderFilePath true 0 0 false 2 false 1 true true
		OxAlembicExport renderFilePath true 0 0 true 2 false 1 true true

		-- 10 hairs are exported if non-rendered parameter is specified
		EAssert.AreNearEqual 13480L ( getFileSize ( ( getfiles nonRenderFilePath )[1] ) ) epsilon:2L
		
		-- 20 hairs are exported if rendered hair parameter is specified
		EAssert.AreNearEqual 21208L ( getFileSize ( ( getfiles renderFilePath )[1] ) ) epsilon:2L
			
		-- Cleanup
		deleteFile nonRenderFilePath
		deleteFile renderFilePath
	),
	
	function Test_TwoObjectsWithSameName =
	(
		-- Create some hair
		local firstObject = Plane lengthsegs:1 widthsegs:1
		max modify mode
		select firstObject
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		local secondObject = copy firstObject
		local thirdObject = copy firstObject
		secondObject.Name = firstObject.Name
		thirdObject.Name = firstObject.Name

		-- Export to an alembic file
		local alembicFilePath = TestUtilities.GetTestFilePath "TwoObjectsWithSameName.abc"
		OxAlembicExport alembicFilePath false 0 0 false 2 false 1
		
		-- If nothing crashed we are good
			
		-- Cleanup
		deleteFile alembicFilePath
	),
	
	-- Tests that a single selected hair is written to an Alembic abc file
	function Test_ExportAnimatedGuidesWithGroundStrands =
	(
		-- Create some hair
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		
		max modify mode
		local bendInstance = bend BendAxis:1
		select distributionObject
		modPanel.addModToSelection bendInstance
		
		local spline = line()
		addNewSpline spline
		addKnot spline 1 #corner #line [5,2,0]
		addKnot spline 1 #corner #line [5,0,5]
		updateShape spline
		
		select spline
		modPanel.addModToSelection (Ox_Guides_from_Shape numSegs:2)
		local groundStrands = Ox_Ground_Strands surface:distributionObject
		modPanel.addModToSelection groundStrands
		groundStrands.Ground()
		
		-- Animate the plane object in first two frames
		set animate on
		sliderTime = 0f
		--distributionObject.width = 10
		bendInstance.BendAngle = 0
		sliderTime = 1f
		--distributionObject.width = 20
		bendInstance.BendAngle = 100
		set animate off
		sliderTime = 0f

		-- Export to an alembic file without animation first
		local resultWithoutAnimationFilePath = TestUtilities.GetTestFilePath "AnimatedGuidesWithGroundStrands_noAnimation.abc"
		OxAlembicExport resultWithoutAnimationFilePath true 0 0 false 2 false 1
		
		local resultWithAnimationFilePath = TestUtilities.GetTestFilePath "AnimatedGuidesWithGroundStrands_withAnimation.abc"
		OxAlembicExport resultWithAnimationFilePath true 0 1 false 2 false 1

		local sizeWithoutAnimation = getFileSize ( ( getfiles resultWithoutAnimationFilePath )[1] )
		local sizeWithAnimation = getFileSize ( ( getfiles resultWithAnimationFilePath )[1] )
		
		-- We can test if animation was exported by comparing file sizes
		EAssert.IsTrue( sizeWithoutAnimation < sizeWithAnimation )
			
		-- Cleanup
		deleteFile resultWithoutAnimationFilePath
		deleteFile resultWithAnimationFilePath
	)
)

EUnit.AddTestClass Test_AlembicExporter
ok
