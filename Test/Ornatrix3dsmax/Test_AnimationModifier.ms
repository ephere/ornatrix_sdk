struct Test_AnimationModifier
(
	function TestInitialize =
	(
		OxResetClassParameters()
		animationRange = (interval 0f 100f)
		sliderTime = 0f
	),
	
	function TestCleanup =
	(
		delete $objects
	),
	
	-- When animation modifier is added to baked guides on top of a deforming mesh the resulting animation should be correct as captured in object coordinates
	function Test_AddAnimationToDeformingDistributionMesh =
	(
		-- Create a plane with 1x1 segments so we have 4 vertices all together and create a reference which we will use as guides
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		local guidesObjectInstance = reference distributionObject

		max modify mode

		-- Add and animate a bend modifier on the distribution mesh, change its axis to "Y" to make the plane deform, and animate the bend value
		-- between frames 0 and 5
		bendInstance = Bend()
		modPanel.addModToSelection bendInstance
		bendInstance.BendAxis = 1
		set animate on
		sliderTime = 5f
		bendInstance.BendAngle = 639.992
		sliderTime = 0f
		set animate off

		-- Setup guides
		select guidesObjectInstance
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 2
		modPanel.addModToSelection guidesFromSurfaceModifier

		-- Capture the tip locations of all 4 strands at frame 5
		sliderTime = 5f
		guidesCopy = instance guidesObjectInstance
		convertTo guidesCopy GuidesObject
		myGuides = ( guidesCopy.GetGuides() ).AsHair()
		guide1Tip = myGuides.GetPoint 0 1
		guide2Tip = myGuides.GetPoint 1 1
		guide3Tip = myGuides.GetPoint 2 1
		guide4Tip = myGuides.GetPoint 3 1
		delete guidesCopy

		-- Export ABC file containing current 5-frame animation
		resultFilePath = TestUtilities.GetTestFilePath "Test_AddAnimationToDeformingDistributionMesh.abc"
		OxAlembicExport resultFilePath true 0 5 false 2 false 1

		-- Collapse guides into a baked guides object and select distribution mesh for the result
		maxOps.CollapseNode guidesObjectInstance on

		-- Add strand animation modifier on top and load the saved ABC file
		modPanel.addModToSelection (Ox_Animation_Cache animFilename:resultFilePath endframe:5)

		-- We no longer support strand animation in OXH files
		-- Capture the tip locations of strands at frame 5 and compare them to the previous version, the positions should be the same
		guidesCopy = instance guidesObjectInstance
		convertTo guidesCopy GuidesObject
		myGuides = ( guidesCopy.GetGuides() ).AsHair()
		EAssert.ArePointsNearEqual guide1Tip ( myGuides.GetPoint 0 1 )
		EAssert.ArePointsNearEqual guide2Tip ( myGuides.GetPoint 1 1 )
		EAssert.ArePointsNearEqual guide3Tip ( myGuides.GetPoint 2 1 )
		EAssert.ArePointsNearEqual guide4Tip ( myGuides.GetPoint 3 1 )
		delete guidesCopy

		-- Cleanup
		deleteFile resultFilePath
	),
	
	function AddAnimatedBendToSelection bendAxis:1 =
	(
		-- Add and animate a bend modifier on the distribution mesh, change its axis to "Y" to make the plane deform, and animate the bend value
		-- between frames 0 and 5
		local bendInstance = Bend()
		modPanel.addModToSelection bendInstance
		bendInstance.BendAxis = bendAxis
		set animate on
		sliderTime = 5f
		bendInstance.BendAngle = 639.992
		sliderTime = 0f
		set animate off
			
		bendInstance
	),
	
	function CreateAlembicAnimationFile distributionObject filePath =
	(
		-- Create a plane with 1x1 segments so we have 4 vertices all together and create a reference which we will use as guides
		local guidesObjectInstance = reference distributionObject
		max modify mode

		AddAnimatedBendToSelection()

		-- Setup guides
		select guidesObjectInstance
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		OxAlembicExport filePath true 0 5 false 2 false 1
		
		guidesObjectInstance
	),
	
	function Test_AlembicAnimation =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		local alembicFilePath = TestUtilities.GetTestFilePath "Test_AlembicAnimation.abc"
		local guidesObjectInstance = CreateAlembicAnimationFile distributionObject alembicFilePath
		
		local verticesOverTimeOnOriginalGuides = TestUtilities.GetHairVerticesOverTime guidesObjectInstance 0 5
		
		-- Collapse guides into a baked guides object and select distribution mesh for the result
		maxOps.CollapseNode guidesObjectInstance on

		-- Add strand animation modifier on top and load the saved .abc file
		local strandAnimationModifierInstance = Ox_Animation_Cache()
		strandAnimationModifierInstance.animFilename = alembicFilePath
		strandAnimationModifierInstance.endframe = 5
		modPanel.addModToSelection strandAnimationModifierInstance
		
		local verticesOverTimeOnAnimatedGuides = TestUtilities.GetHairVerticesOverTime guidesObjectInstance 0 5
		
		-- Check that points were loaded properly
		for i = 1 to (verticesOverTimeOnOriginalGuides.count) do
		(
			EAssert.ArePointsAllNearEqual verticesOverTimeOnOriginalGuides[i] verticesOverTimeOnAnimatedGuides[i]
		)
		
		-- Cleanup
		deleteFile alembicFilePath
	),
	
	function Test_TimeOffset =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		local alembicFilePath = TestUtilities.GetTestFilePath "Test_TimeOffset.abc"
		local guidesObjectInstance = CreateAlembicAnimationFile distributionObject alembicFilePath
		
		-- Collapse guides into a baked guides object and select distribution mesh for the result
		maxOps.CollapseNode guidesObjectInstance on

		-- Add strand animation modifier on top and load the saved .abc file
		local strandAnimationModifierInstance = Ox_Animation_Cache animFilename:alembicFilePath
		modPanel.addModToSelection strandAnimationModifierInstance
		
		sliderTime = 2f
		local verticesAtFrame2WithoutTimeOffset = OxGetVertices guidesObjectInstance true
		strandAnimationModifierInstance.timeOffset = -2f
		sliderTime = 0f
		local verticesAtFrame0WithTimeOffset = OxGetVertices guidesObjectInstance true
		
		EAssert.ArePointsAllNearEqual verticesAtFrame2WithoutTimeOffset verticesAtFrame0WithTimeOffset
		
		-- Cleanup
		deleteFile alembicFilePath
	),
	
	function Test_AlembicAnimationWithChangedVertexCount =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		local alembicFilePath = TestUtilities.GetTestFilePath "Test_AlembicAnimationWithChangedVertexCount.abc"
		local guidesObjectInstance = CreateAlembicAnimationFile distributionObject alembicFilePath
		
		-- Change guide vertex count and collapse
		guidesObjectInstance.modifiers[#Ox_Guides_From_Surface].NumSegs = 20
		maxOps.CollapseNode guidesObjectInstance on

		-- Add strand animation modifier on top and load the saved .abc file
		local strandAnimationModifierInstance = Ox_Animation_Cache()
		strandAnimationModifierInstance.animFilename = alembicFilePath
		strandAnimationModifierInstance.endframe = 5
		modPanel.addModToSelection strandAnimationModifierInstance
		
		local verticesOverTimeOnAnimatedGuides = TestUtilities.GetHairVerticesOverTime guidesObjectInstance 0 5
		
		-- Make sure that all vertices are animated
		for i = 2 to (verticesOverTimeOnAnimatedGuides.count) do
		(
			EAssert.ArePointsAllNotNearEqual verticesOverTimeOnAnimatedGuides[i - 1] verticesOverTimeOnAnimatedGuides[i]
		)
		
		-- Cleanup
		deleteFile alembicFilePath
	),
	
	function Test_AlembicAnimationWithVaryingStrandPointCount =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		local alembicFilePath = TestUtilities.GetTestFilePath "Test_AlembicAnimationWithVaryingStrandPointCount.abc"
		
		local bendInstance = AddAnimatedBendToSelection()

		-- Setup guides and plane a new guide on them
		local guidesFromSurfaceInstance = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		guidesFromSurfaceInstance.NumSegs = 5
		editGuidesModifierInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesModifierInstance
		
		-- Since we set the number of segments to 5 before the newly planed guide will have 5 points
		editGuidesModifierInstance.CreateRootAtCoordinate 0 [0.4, 0.4]
		
		-- Setting number of segments to 2 will ensure that all guides except the planted one will have 2 segments, thus making point count in guides non-uniform
		guidesFromSurfaceInstance.NumSegs = 2
		
		OxAlembicExport alembicFilePath true 0 5 false 2 false 1
		
		local verticesOverTimeOnOriginalGuides = TestUtilities.GetHairVerticesOverTime distributionObject 0 5
		
		-- Collapse guides into a baked guides object and select distribution mesh for the result
		bendInstance.enabled = off
		
		-- Add strand animation modifier on top and load the saved .abc file
		local strandAnimationModifierInstance = Ox_Animation_Cache()
		strandAnimationModifierInstance.animFilename = alembicFilePath
		strandAnimationModifierInstance.endframe = 5
		modPanel.addModToSelection strandAnimationModifierInstance
		
		local verticesOverTimeOnAnimatedGuides = TestUtilities.GetHairVerticesOverTime distributionObject 0 5
		
		-- Check that points were loaded properly
		for i = 1 to (verticesOverTimeOnOriginalGuides.count) do
		(
			EAssert.ArePointsAllNearEqual verticesOverTimeOnOriginalGuides[i] verticesOverTimeOnAnimatedGuides[i]
		)
		
		-- Cleanup
		deleteFile alembicFilePath
	),
	
	function Test_Recording =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		local alembicFilePath = TestUtilities.GetTestFilePath "Test_Recording.abc"

		-- Delete the test file from any previous runs
		if (doesFileExist alembicFilePath) then deleteFile alembicFilePath
		
		max modify mode
		local bendInstance = AddAnimatedBendToSelection()
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		local verticesOverTimeOnOriginalGuides = TestUtilities.GetHairVerticesOverTime distributionObject 0 5
		
		-- Add strand animation modifier on top and load the saved .abc file
		local strandAnimationModifierInstance = Ox_Animation_Cache animFilename: alembicFilePath endframe: 5
		modPanel.addModToSelection strandAnimationModifierInstance
		
		strandAnimationModifierInstance.Record 0f 5f
		
		-- Turn off Bend modifier and check results again
		bendInstance.enabled = off
		local verticesOverTimeOnAnimatedGuides = TestUtilities.GetHairVerticesOverTime distributionObject 0 5
		
		-- Check that points were loaded properly
		for i = 1 to (verticesOverTimeOnOriginalGuides.count) do
		(
			EAssert.ArePointsAllNearEqual verticesOverTimeOnOriginalGuides[i] verticesOverTimeOnAnimatedGuides[i]
		)
		
		-- Cleanup
		deleteFile alembicFilePath
	),
	
	-- Compares points in two arrays and finds how many unique points in one array are present in the second array
	function GetEqualPointsCount points1 points2 epsilon:0.01 =
	(
		local result = 0
		for i = 1 to (points1.count) do
		(
			local wasMatchFound = false
			for j = 1 to (points2.count) do
			(
				if (distance points1[i] points2[j]) <= epsilon then
				(
					wasMatchFound = true
					break
				)
			)
			
			if not wasMatchFound then result += 1
		)
		
		result
	),
	
	function Test_AnimationWithDeformingMesh =
	(
		local cylinderHair = Cylinder radius:15 height:50 heightSegs:10 mapcoords:on isSelected:on
		max modify mode
		
		-- Animate the cylinder
		AddAnimatedBendToSelection bendAxis:2
		
		-- Copy the animated cylinder mesh
		local cylinderCopy = copy cylinderHair
			
		local guidesFromSurfaceInstance = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		guidesFromSurfaceInstance.NumSegs = 5
		modPanel.addModToSelection (Ox_Hair_from_Guides viewCountPercentage:10 denseCount:100)
		modPanel.addModToSelection (Ox_Strand_Gravity ())
			
		local alembicFilePath = TestUtilities.GetTestFilePath "Test_AnimationWithDeformingMesh.abc"
		OxAlembicExport alembicFilePath true 0 5 false 2 false 1
		
		-- No longer need cylinder hair			
		delete cylinderHair
				
		local loadedHair = BakedHairObject isSelected:on
		loadedHair.distrObj = cylinderCopy
		loadedHair.sourceFilePath1 = alembicFilePath
		loadedHair.detached = off

		-- Add strand animation modifier on top and load the saved ABC file
		modPanel.addModToSelection (Ox_Animation_Cache animFilename:alembicFilePath endframe:5)
		
		showEndResult = on
		
		sliderTime = 2f
		loadedHair.displayFraction = 99
		local verticesAt99 = OxGetVertices loadedHair true
		
		loadedHair.displayFraction = 100
		local verticesAt100 = OxGetVertices loadedHair true
		
		-- All points which were present at 99% viewport preview should still be present at 100% viewport preview, at same positions
		local vertexCountDifference = (verticesAt100.count) - (verticesAt99.count)
		EAssert.AreEqual vertexCountDifference (GetEqualPointsCount verticesAt100 verticesAt99)
			
		-- Cleanup
		deleteFile alembicFilePath
	),
	
	function Test_RecordNonAnimatedObject =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		local alembicFilePath = TestUtilities.GetTestFilePath "Test_Recording.abc"

		-- Delete the test file from any previous runs
		if (doesFileExist alembicFilePath) then deleteFile alembicFilePath
		
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		-- Add strand animation modifier on top and load the saved .abc file
		local strandAnimationModifierInstance = Ox_Animation_Cache animFilename: alembicFilePath
		modPanel.addModToSelection strandAnimationModifierInstance
		
		strandAnimationModifierInstance.Record 0f 2f
		
		-- Make sure a file was recorded
		EAssert.IsTrue (doesFileExist alembicFilePath)
		
		-- Cleanup
		deleteFile alembicFilePath
	),

	-- Triggers the crash described in issue #3742
	function Test_RecordAnimationWithVaryingStrandPointCount =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		local alembicFilePath = TestUtilities.GetTestFilePath "Test_RecordAnimationWithVaryingStrandPointCount.abc"

		-- Delete the test file from any previous runs
		if (doesFileExist alembicFilePath) then deleteFile alembicFilePath

		-- Setup guides and plane a new guide on them
		local guidesFromSurfaceInstance = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		guidesFromSurfaceInstance.NumSegs = 5
		editGuidesModifierInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesModifierInstance

		-- Since we set the number of segments to 5 before the newly planted guide will have 5 points
		editGuidesModifierInstance.CreateRootAtCoordinate 0 [0.4, 0.4]

		-- Setting all guides but the planted one to a larger segment count triggers the crash; smaller counts do not.
		guidesFromSurfaceInstance.NumSegs = 10

		-- Add strand animation modifier on top
		local strandAnimationModifierInstance = Ox_Animation_Cache animFilename: alembicFilePath
		modPanel.addModToSelection strandAnimationModifierInstance

		-- Trigger the crash
		strandAnimationModifierInstance.Record 0f 2f

		-- Make sure a file was recorded
		EAssert.IsTrue (doesFileExist alembicFilePath)

		-- Cleanup
		deleteFile alembicFilePath
	),
	
	function Test_AnimationFileSequence =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		
		modPanel.addModToSelection (Ox_Guides_from_surface method:4 randomness:0 NumSegs:2 guideLength:10)
		
		-- Add and animate strand length
		local lengthMod = Ox_Strand_Length()
		modPanel.addModToSelection lengthMod
		
		set animate on
		sliderTime = 4f
		lengthMod.length = 0.5
		sliderTime = 0f
		set animate off
			
		local originalVertexPositionsPerFrame = #()
		for i = 0 to 4 do
		(
			sliderTime = (i * 1f)
			append originalVertexPositionsPerFrame (OxGetVertices distributionObject true)
		)
			
		local alembicFilePaths = #()
			
		-- Export 5 files, one for each frame, with a sequenced file path
		for i =0 to 4 do
		(
			local alembicFilePath = TestUtilities.GetTestFilePath ("Test_AnimationFileSequence." + (i as string) + ".abc")
			local currentFrame = i
			local currentFrameTime = currentFrame * 1f
			sliderTime = currentFrameTime
			OxAlembicExport alembicFilePath true currentFrameTime currentFrameTime false 2 false 1
			append alembicFilePaths alembicFilePath
		)
		
		-- Remove length mod to remove animation
		deleteModifier distributionObject lengthMod

		-- Add strand animation modifier on top and load the saved ABC file
		modPanel.addModToSelection (Ox_Animation_Cache animFilename:(TestUtilities.GetTestFilePath "Test_AnimationFileSequence.###.abc"))
			
		-- Compare all frames, we should get near identical results
		for i = 0 to 4 do
		(
			sliderTime = (i * 1f) 
			EAssert.ArePointsAllNearEqual originalVertexPositionsPerFrame[i+1] (OxGetVertices distributionObject true)
		)
			
		-- Cleanup
		for i =1 to 5 do
		(
			deleteFile alembicFilePaths[i]
		)
	),
	
	function Test_StepFrameWithOverwriteInput =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		local alembicFilePath = TestUtilities.GetTestFilePath "Test_AlembicAnimation.abc"
		local guidesObjectInstance = CreateAlembicAnimationFile distributionObject alembicFilePath
				
		-- Add strand animation modifier on top and load the saved ABC file
		modPanel.addModToSelection (Ox_Animation_Cache animFilename:alembicFilePath endframe:5 overwriteInput:on)
		
		showEndResult = on
		
		sliderTime = 0f
		OxGetVertices distributionObject true
		sliderTime = 1f
		OxGetVertices distributionObject true
		
		-- If no crash happened we are good
			
		-- Cleanup
		deleteFile alembicFilePath
	),
	
	function Test_AlembicReferenceSavedWithSceneArchive =
	(
		-- COREInterface18
		EUnit.SkipBeforeMaxVersion 2020

		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		local alembicFileName = "Test_AlembicAnimation.abc"
		local alembicFilePath = TestUtilities.GetTestFilePath alembicFileName
		local guidesObjectInstance = CreateAlembicAnimationFile distributionObject alembicFilePath
				
		-- Add strand animation modifier on top and load the saved ABC file
		modPanel.addModToSelection (Ox_Animation_Cache animFilename:alembicFilePath endframe:5 overwriteInput:on)
		
		-- Archive scene
		local sceneArchiveFilePath = TestUtilities.GetTestFilePath "Test_AlembicAnimation.zip"
		local iface = TestUtilities.GetTopInterface()
		EAssert.IsTrue (iface.ArchiveSceneFile sceneArchiveFilePath 23000)
		
		local extractedArchiveDirectory = TestUtilities.GetTestFilePath "Test_AlembicAnimation"
		makeDir extractedArchiveDirectory
		
		-- Unzip the archive
		local zipUtil = (getDir #maxRoot) + "maxunzip.exe" --the unzipping-program
        cmd = "" as stringStream --set up a unzipping-command
        format "\"%\" %" zipUtil sceneArchiveFilePath to:cmd --the commandstring
        (hiddendosCommand cmd exitcode:&exitcode startpath:extractedArchiveDirectory) --EXECUTE THE UNZIPPING
		EAssert.AreEqual 0 exitcode
		
		-- Inspect the ZIP archive to ensure that the ABC file is there
		EAssert.AreEqual 1 ((getfiles alembicFilePath).count)
			
		-- Cleanup
		deleteFile alembicFilePath
		deleteFile sceneArchiveFilePath
		hiddendosCommand ("rmdir " + extractedArchiveDirectory)
	),
	
	function Test_FileParameterIsNotSticky =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on

		max modify mode

		-- Setup guides
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:2)
		
		local alembicFileName = "Test_FileParameterIsNotSticky.abc"
		modPanel.addModToSelection (Ox_Animation_Cache animFilename:alembicFileName)
		
		
		local distributionObject2 = Plane lengthsegs:1 widthsegs:1 isSelected:on
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:2)
		local animationModifier2 = Ox_Animation_Cache()
		modPanel.addModToSelection animationModifier2
		
		EAssert.AreEqual "" (animationModifier2.animFilename)
	)
)

EUnit.AddTestClass Test_AnimationModifier
ok
