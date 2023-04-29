struct Test_BakedGuidesObject
(
	function TestInitialize =
	(
		OxResetClassParameters()
	),
	
	function TestCleanup =
	(
		delete $objects
	),
	
	-- Tests that changes made by Edit Guides modifier are properly restored when reloading a scene
	function Test_ReloadEditGuidesShapeChanges =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:10 length:10 isSelected:on
		local distributionObject2 = copy distributionObject
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
			
		-- Move control points a bit once
		subobjectLevel = 2
		editGuidesInstance.SelectRoots #{1, 2}
		editGuidesInstance.MoveSelectedControlPoints [5,5,0]
		
		-- Collapse the stack so we get BakedGuidesObject
		maxOps.CollapseNodeTo distributionObject 2 on
		distributionObject.distrObj = distributionObject2
			
		local verticesBeforeReloading = OxGetVertices distributionObject true
		local distributionObjectName = distributionObject.name
		
		-- Save and reload the scene
		holdMaxFile()
		fetchMaxFile quiet:true
		
		distributionObject = GetNodeByName distributionObjectName
		local verticesAfterReloading = OxGetVertices distributionObject true
		
		-- We do not expect to see any change by existing sub-object mode
		EAssert.ArePointsAllNearEqual verticesBeforeReloading verticesAfterReloading
	),
	
	function Test_LoadV6Scene =
	(
		EUnit.SkipBeforeMaxVersion 2020

		local testSceneFilePath = pathConfig.convertPathToAbsolute (getSourceFileName() + "..\\..\\..\\..\\..\\..\\..\\..\\" ) + "External\\TestData\\OxMax6-Max2020-LoadBakedGuidesTest.max"
		loadMaxFile testSceneFilePath useFileUnit:true quiet:true
		local actualVertices = OxGetVertices $Plane001 true
		local expectedVertices = #([-14.9011,-4.79521,0], [-31.8974,-14.8757,14.4341], [-52.9449,-27.359,14.4341], [9.12265,21.558,0], [17.7402,36.2175,12.421], [28.4118,54.3713,12.421], [15.8521,-12.5091,0], [31.0218,-21.3505,12.8252], [49.8075,-32.2993,12.8252], [14.0476,1.85019,0], [33.5254,4.76238,14.3854], [57.6459,8.36873,14.3854], [25.4836,-9.81719,0], [40.6559,-15.4122,11.8119], [59.4446,-22.3409,11.8119])
		
		EAssert.ArePointsAllNearEqual expectedVertices actualVertices
		
		-- Cleanup
		resetMaxFile #noPrompt
	)
)

EUnit.AddTestClass Test_BakedGuidesObject
ok
