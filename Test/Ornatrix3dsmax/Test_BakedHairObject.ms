struct Test_BakedHairObject
(
	function TestInitialize =
	(
		OxResetClassParameters()
	),
	
	function TestCleanup =
	(
		delete $objects
	),
	
	function Test_ViewPercentage =
	(
		local distributionObject = TestUtilities.CreateBakedHairOnPlane()
		
		distributionObject.displayFraction = 10
		EAssert.AreNearEqual 10 (OxGetStrandCount distributionObject) epsilon:7
		
		distributionObject.displayFraction = 20
		EAssert.AreNearEqual 20 (OxGetStrandCount distributionObject) epsilon:3
	),
	
	function Test_RenderSettingsModifier =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Edit_Guides())
		OxResetClassParameters (Ox_Hair_from_Guides())
		OxResetClassParameters (Ox_Render_Settings())

		local distributionObject = Plane lengthsegs:3 widthsegs:3 width:40 length:40 isSelected:on
		local distributionObjectName = distributionObject.name
		local plane2 = maxOps.CloneNodes #(distributionObject)

		max modify mode
		
		-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		modPanel.addModToSelection guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.NumSegs = 2

		-- Edit guides modifier which sets the first two strands to use group #2
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		editGuidesInstance.CreateChannel true "Groups"

		editGuidesInstance.SelectRoots #{1,2,3}
		editGuidesInstance.SetChannelValueToSelection true 1 2.0f
		editGuidesInstance.SelectRoots #{3,4,5}
		editGuidesInstance.SetChannelValueToSelection true 1 3.0f

		local hairFromGuidesInstance = Ox_Hair_from_Guides()
		modPanel.addModToSelection hairFromGuidesInstance ui:on
		hairFromGuidesInstance.denseCount = 100
		hairFromGuidesInstance.viewCountPercentage = 100
		hairFromGuidesInstance.rememberPositions = on
		
		local renderSettings1 = Ox_Render_Settings()
		modPanel.addModToSelection renderSettings1 ui:on
		renderSettings1.groupID = 0 -- global group
		renderSettings1.radius = 1
		renderSettings1.diagData = #( 103, 3, 0.00970874, 0.534881, 3, 0.595146, 0.434884, 3, 1, 0.441863 )

    local renderSettings2 = Ox_Render_Settings()
    modPanel.addModToSelection renderSettings2 ui:on
    renderSettings2.groupID = 2
    renderSettings2.radius = 2
    renderSettings2.thicknessMap = Gradient()
		renderSettings2.diagData = #( 103, 3, 0.0339806, 0.0348786, 3, 0.543689, 0.476741, 3, 1, 0.91861 )
	
		local renderSettings3 = Ox_Render_Settings()
		modPanel.addModToSelection renderSettings3 ui:on
		renderSettings3.thicknessMap = Checker()
		renderSettings3.groupID = 3
		renderSettings3.radius = 3
		renderSettings3.diagData = #( 103, 3, 0.038835, 1, 3, 0.543689, 0.476741, 3, 1, 0 )

		local meshFromStrands = Ox_Mesh_From_Strands()
		modPanel.addModToSelection meshFromStrands ui:on
		local arrayOrig = #()
		for i = 1 to distributionObject.numVerts do
		(
			arrayOrig[i] = distributionObject.verts[i].pos
		)
		deleteModifier distributionObject 1

 		maxOps.CollapseNode distributionObject off
 		distributionObject.distrObj = $Plane003
		distributionObject.displayFraction = 100
		distributionObject.detached = on
		distributionObject.sourceFilePath1 = ""
		distributionObject.sourceFilePath2 = ""
		distributionObject.sourceFilePath3 = ""
		distributionObject.sourceFilePath4 = ""
		distributionObject.sourceFilePath5 = ""

		local meshFromStrands = Ox_Mesh_From_Strands()
		modPanel.addModToSelection meshFromStrands ui:on
		local arrayBaked = #()
		for i = 1 to distributionObject.numVerts do
		(
			arrayBaked[i] = distributionObject.verts[i].pos
		)
		
		deleteModifier distributionObject 1
		
		-- The order of roots in original and baked arrays can be different so use matching instead of exact array comparison
		EAssert.ArePointsAllNearEqual arrayOrig arrayBaked
	
		holdMaxFile()
		fetchMaxFile quiet:true
		distributionObject = GetNodeByName distributionObjectName
		
		select distributionObject
		modPanel.addModToSelection meshFromStrands ui:on
		local arrayReloaded = #()
		for i = 1 to distributionObject.numVerts do
		(
			arrayReloaded[i] = distributionObject.verts[i].pos
		)
		deleteModifier distributionObject 1
		
		EAssert.ArePointsAllNearEqual arrayBaked arrayReloaded
	),
	
	function Test_Grounding =
	(
		local bakedHair = TestUtilities.CreateBakedHairOnPlane()
		local planeForGrounding = Plane lengthsegs:1 widthsegs:1 width:100 length:100 isSelected:on	
			
		local initialVertices = OxGetVertices bakedHair true
			
		bakedHair.distrObj = planeForGrounding
		local verticesAfterAssigningDistributionObject = OxGetVertices bakedHair true
		
		-- After setting distribution objects strands should be re-distributed over the new mesh
		EAssert.ArePointsNotAllNearEqual initialVertices verticesAfterAssigningDistributionObject
		
		-- Ground strands
		bakedHair.detached = off
		local verticesAfterGrounding = OxGetVertices bakedHair true
		
		-- After grounding the strands they should go back to their original positions
		EAssert.ArePointsAllNearEqual initialVertices verticesAfterGrounding
		
		-- Rescaling the grounding plane should spread apart the vertices
		planeForGrounding.width *= 2
		planeForGrounding.length *= 2
		local verticesAfterGroundingAndRescaling = OxGetVertices bakedHair true
		for i = 1 to (verticesAfterGrounding.count) do
		(
			local expectedPoint = initialVertices[i] * 2
			expectedPoint.z = initialVertices[i].z
			
			EAssert.ArePointsNearEqual expectedPoint verticesAfterGroundingAndRescaling[i]
		)
	),
	
	function Test_GroundToRotatedAndScaledTarget =
	(
		local bakedHair = TestUtilities.CreateBakedHairOnPlane()
		
		local alembicFilePath1 = TestUtilities.GetTestFilePath "Test_GroundToRotatedAndScaledTarget.abc"
		OxAlembicExport alembicFilePath1 true 0 0 false 2 false 1
		
		bakedHair.sourceFilePath1 = alembicFilePath1
		
		local initialVertices = TestUtilities.GetHairVerticesInWorldSpace bakedHair
		
		local planeForGrounding = Plane lengthsegs:1 widthsegs:1 width:100 length:100 isSelected:on
		macros.run "Modifier Stack" "Convert_to_Poly"
		max modify mode
		
		-- Move and rotate the plane in such a way that we will scale and rotate it back to align with the hair
		subobjectLevel = 1
		planeForGrounding.EditablePoly.SetSelection #Vertex #{1, 3}
		move planeForGrounding.selectedVerts [200,0,0]
		planeForGrounding.EditablePoly.SetSelection #Vertex #{2, 4}
		move planeForGrounding.selectedVerts [100,0,-100]
		subobjectLevel = 0
		
		-- Transform the mesh so it aligns back with the roots of the hairs
		rotate planeForGrounding (angleaxis 90 [0,-1,0])
		scale planeForGrounding [0.5,0.5,0.5]
		move planeForGrounding [-23,0,-75]
		
		bakedHair.distrObj = planeForGrounding
		local verticesAfterSelectingDistributionSurface = TestUtilities.GetHairVerticesInWorldSpace bakedHair
		EAssert.ArePointsAllNearEqual initialVertices verticesAfterSelectingDistributionSurface
		
		bakedHair.detached = off
		local verticesAfterGrounding = TestUtilities.GetHairVerticesInWorldSpace bakedHair
		EAssert.ArePointsAllNearEqual initialVertices verticesAfterGrounding
		
		deleteFile alembicFilePath1
	),
	
	function Test_LoadTwoAbcFiles =
	(
		max modify mode
		
		-- Export two different hair objects
		local firstObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local alembicFilePath1 = TestUtilities.GetTestFilePath "FirstFile.abc"
		OxAlembicExport alembicFilePath1 true 0 0 false 2 false 1
		
		--resetMaxFile #noPrompt
		
		local secondObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local alembicFilePath2 = TestUtilities.GetTestFilePath "SecondFile.abc"
		OxAlembicExport alembicFilePath2 true 0 0 false 2 false 1
		
		--resetMaxFile #noPrompt
		
		-- Load the two alembic files in a new baked hair object
		local bakedHair = TestUtilities.CreateBakedHairOnPlane denseCount:0
		EAssert.AreEqual 0 (OxGetStrandCount bakedHair)
		
		bakedHair.sourceFilePath1 = alembicFilePath1
		EAssert.AreEqual 4 (OxGetStrandCount bakedHair)
		
		bakedHair.sourceFilePath2 = alembicFilePath2
		EAssert.AreEqual 8 (OxGetStrandCount bakedHair)
		
		bakedHair.sourceFilePath2 = ""
		EAssert.AreEqual 4 (OxGetStrandCount bakedHair)

		-- Cleanup
		bakedHair.sourceFilePath1 = ""
		bakedHair.sourceFilePath2 = ""
		deleteFile alembicFilePath1
		deleteFile alembicFilePath2
	),
	
	function Test_ReloadAbcFileWithPartialDisplay =
	(
		max modify mode
		
		-- Export and load Alembic hair
		local firstObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		modPanel.addModToSelection (HairFromGuidesModifier method:7 viewCountPercentage:1 denseCount:100)
		local alembicFilePath1 = TestUtilities.GetTestFilePath "FirstFile.abc"
		OxAlembicExport alembicFilePath1 true 0 0 false 2 false 1
		delete firstObject
		
		local bakedHair = TestUtilities.CreateBakedHairOnPlane denseCount:0
		bakedHair.displayFraction = 10
		bakedHair.sourceFilePath1 = alembicFilePath1
		local viewHairCount = OxGetStrandCount bakedHair
		local hairVertices = OxGetVertices bakedHair true
		local bakedHairName = bakedHair.name
		
		-- Reload the scene, make sure hair is still there
		holdMaxFile()
		fetchMaxFile quiet:true
		
		bakedHair = GetNodeByName bakedHairName
		EAssert.AreEqual alembicFilePath1 (bakedHair.sourceFilePath1)
		EAssert.AreEqual viewHairCount (OxGetStrandCount bakedHair)
		EAssert.ArePointsAllNearEqual hairVertices (OxGetVertices bakedHair true)

		-- Cleanup
		bakedHair.sourceFilePath1 = ""
		deleteFile alembicFilePath1
	),
	
	function Test_GroundedAbcFileOnAnimatedSurface =
	(
		max modify mode
		
		-- Export four hairs to Alembic
		local baseSurface = Plane lengthsegs:1 widthsegs:1 isSelected:on
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local alembicFilePath = TestUtilities.GetTestFilePath "Test_GroundedAbcFileOnAnimatedSurface.abc"
		OxAlembicExport alembicFilePath true 0 0 false 2 false 1
		delete baseSurface
		
		-- Create an animated surface
		local animatedSurface = Plane lengthsegs:1 widthsegs:1 isSelected:on
		local bendModifier = Bend BendAxis:0
		set animate on
		sliderTime = 5f
		bendModifier.BendAngle = 160
		bendModifier.BendDir = 43
		set animate off
		modPanel.addModToSelection bendModifier
		
		sliderTime = 1f
		local bakedHair = Ox_Baked_Hair displayFraction:100 sourceFilePath1:alembicFilePath
		
		bakedHair.distrObj = animatedSurface
		bakedHair.detached = off
	)
	/*,
	
	function Test_LoadV6GroundStrands =
	(
		local testSceneFilePath = pathConfig.convertPathToAbsolute (getSourceFileName() + "..\\..\\..\\..\\..\\..\\..\\..\\" ) + "External\\TestData\\OxMax6-Max2020-BakedHairGroundStrands.max"
		loadMaxFile testSceneFilePath useFileUnit:true quiet:true
	
		local actualVertices = OxGetVertices $FurBall true
		local expectedVertices = #([-3.93701,-1.9685,0], [-3.93701,-1.9685,0.393701], [-1.9685,-3.93701,0], [-1.9685,-3.93701,0.393701], [1.9685,-1.9685,0], [1.9685,-1.9685,0.393701], [3.93701,-3.93701,0], [3.93701,-3.93701,0.393701], [-3.93701,3.93701,0], [-3.93701,3.93701,0.393701], [-1.9685,1.9685,0], [-1.9685,1.9685,0.393701], [1.9685,3.93701,0], [1.9685,3.93701,0.393701], [3.93701,1.9685,0], [3.93701,1.9685,0.393701])
		EAssert.ArePointsAllNearEqual expectedVertices actualVertices
		
		-- Cleanup
		units.SystemType = #Centimeters
		resetMaxFile #noPrompt
	)*/
)

EUnit.AddTestClass Test_BakedHairObject
ok
