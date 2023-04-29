struct Test_RenderSettingsModifier
(
	function TestCleanup =
	(
		delete $objects
	),
	
	function SetupBasicHairStackWithRenderSettings redrawAfterEachStep:false =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Guides_From_Surface())
		OxResetClassParameters (Ox_Edit_Guides())
		OxResetClassParameters (Ox_Hair_From_Guides())
		OxResetClassParameters (Ox_Render_Settings())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		
		RedrawViews()

		-- Add strand propagation
		select distributionObject
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		if redrawAfterEachStep then RedrawViews()
		modPanel.addModToSelection (Ox_Edit_Guides())
		if redrawAfterEachStep then RedrawViews()
		modPanel.addModToSelection (Ox_Hair_From_Guides method:4)
		if redrawAfterEachStep then RedrawViews()
		local renderSettingsModifierInstance = Ox_Render_Settings()
		modPanel.addModToSelection renderSettingsModifierInstance
		
		renderSettingsModifierInstance
	),
	
	function Test_ThicknessMap =
	(
		local renderSettingsModifierInstance = SetupBasicHairStackWithRenderSettings redrawAfterEachStep:true
		renderSettingsModifierInstance.thicknessMap = Checker()
		
		-- Redraw views to test hair rendering which will use the thickness
		RedrawViews()
		
		-- If no crash happened we are good
	),
	
	function Test_CollapseAllCrashesMaxWhenRenderSettingsIsOnTop =
	(
		SetupBasicHairStackWithRenderSettings()
		RedrawViews()
		
		maxOps.CollapseNode $ off
		
		-- If no crash happened we are good
	)
)

EUnit.AddTestClass Test_RenderSettingsModifier
ok
