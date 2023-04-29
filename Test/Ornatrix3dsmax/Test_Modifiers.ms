struct Test_Modifiers
(
	function TestCleanup =
	(
		delete $objects
		units.SystemType = #Centimeters
	),
	
	function GetGuidesCount guidesNode =
	(
		-- Temporarily collapse guides, get the count from there, and delete the collapsed object
		guidesCopy = instance guidesNode
		convertTo guidesCopy GuidesObject
		result = ( ( ( guidesCopy.GetGuides() ).AsHair() ).NumRoots() )
		delete guidesCopy

		result
	),

	-- Tests that manually changing guide count in MaxScript makes the resulting guides to be recomputed
	function Test_SettingGuideCountFromMaxScriptUpdatesResult =
	(
		local distributionObject = Plane isSelected:on
			
		max modify mode
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		guidesFromSurfaceModifier.count = 300
		guidesFromSurfaceModifier.method = 2
		
		modPanel.addModToSelection guidesFromSurfaceModifier

		-- Initially there are many guides generated
		EAssert.AreEqual 300 ( Test_Modifiers.GetGuidesCount distributionObject )

		-- Now there is only one guide generated
		guidesFromSurfaceModifier.count = 1
		EAssert.AreEqual 1 ( Test_Modifiers.GetGuidesCount distributionObject )
	),

	-- When a user selects guides/roots the selection the modifiers above in the stack should be able to use this selection
	function Test_GuideSelectionIsPassedUpTheStack =
	(
		-- Important to set segment count to 1 so that we have 4 guides in total with vertex distribution
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on

		max modify mode

		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 2
		modPanel.addModToSelection guidesFromSurfaceModifier

		-- Add edit guides modifier and select only the first two strands
		editGuidesModifier = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesModifier
		editGuidesModifier.SelectRoots #{1, 2}

		-- Gets the tip positions of all guides
		initialTipPositions = TestUtilities.GetTipPositions distributionObject

		-- Add a bend modifier
		bendModifier = Bend()
		bendModifier.angle = 100
		modPanel.addModToSelection bendModifier

		-- Get the tip positions again, they should be different only for the first two strands
		newTipPositions = TestUtilities.GetTipPositions distributionObject
		
		-- Exactly two points should differ
		local differingPointCount = 0
		for i = 1 to 4 do
		(
			if ((distance newTipPositions[i] initialTipPositions[i]) > 0.001) then differingPointCount+=1
		)
		
		EAssert.AreEqual 2 differingPointCount
	),
	
	function Test_WithoutSelectionAllGuidesAreAffectedByPointDeformers =
	(
		-- Important to set segment count to 1 so that we have 4 guides in total with vertex distribution
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on

		max modify mode
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 2
		modPanel.addModToSelection guidesFromSurfaceModifier

		-- Gets the tip positions of all guides
		guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		hairCopy = ( guidesCopy.GetGuides() ).AsHair()
		initialTipPosition1 = hairCopy.GetPoint 0 1
		initialTipPosition2 = hairCopy.GetPoint 1 1
		initialTipPosition3 = hairCopy.GetPoint 2 1
		initialTipPosition4 = hairCopy.GetPoint 3 1
		delete guidesCopy

		-- Add a bend modifier
		bendModifier = Bend()
		bendModifier.angle = 100
		modPanel.addModToSelection bendModifier

		-- All points should be affected by the bend modifier
		guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		hairCopy = ( guidesCopy.GetGuides() ).AsHair()
		EAssert.ArePointsNotNearEqual initialTipPosition1 ( hairCopy.GetPoint 0 1 )
		EAssert.ArePointsNotNearEqual initialTipPosition2 ( hairCopy.GetPoint 1 1 )
		EAssert.ArePointsNotNearEqual initialTipPosition3 ( hairCopy.GetPoint 2 1 )
		EAssert.ArePointsNotNearEqual initialTipPosition4 ( hairCopy.GetPoint 3 1 )
		delete guidesCopy
	),

	-- Max allows map channel data to be empty. Since Ornatrix inherits the UVW data from the base mesh's channels ensure that the UVW data
	-- is generated (without crashing) even when the mapping channel is empty
	function Test_UseMapChannelWhichIsUnavailableInDistributionMesh =
	(
		-- Create a plane and apply a UVW modifier to it which modifes some higher channel, thus creating blank map channels in between
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		local uvwMapInstance = Uvwmap()
		modPanel.addModToSelection uvwMapInstance
		uvwMapInstance.mapChannel = 5

		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		modPanel.addModToSelection guidesFromSurfaceModifier

		-- Add Ox Mesh From Strands and make it use the base channel 2, which should be blank
		local meshFromStrandsInstance = Ox_Mesh_From_Strands()
		modPanel.addModToSelection meshFromStrandsInstance
		meshFromStrandsInstance.baseChannel[1] = 2

		-- If we survived this far, we are good!
	),

	-- Tests that all Ornatrix modifiers can properly handle a scenario when there are no incoming strands
	function Test_ApplyModifiersToBlankHair =
	(
		-- Create some guides which don't output any strands because the distribution map is all black
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.count = 1
		guidesFromSurfaceModifier.distrMap = BitmapTexture fileName:""
		modPanel.addModToSelection guidesFromSurfaceModifier

		-- Now add all tested modifiers
		modPanel.addModToSelection (Ox_Strand_Curling())
		modPanel.addModToSelection (Ox_Strand_Frizz())
		modPanel.addModToSelection (Ox_Strand_Clustering())

		-- If no crashing happened at this point, we are good
	),
	
	-- Reproduces an issue
	function Test_CopyModifiersToNewObjectAboveSkin =
	(
		-- Create some guides which don't output any strands because the distribution map is all black
		local plane1 = Plane lengthsegs:1 widthsegs:1
		local plane2 = Plane lengthsegs:1 widthsegs:1
		max modify mode

		select plane1
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		modPanel.addModToSelection guidesFromSurfaceModifier
		local lengthModifierInstance = Ox_Strand_Length()
		modPanel.addModToSelection lengthModifierInstance

		select plane2
		local skinModifierInstance = Skin()
		modPanel.addModToSelection skinModifierInstance
		
		-- Copy two Ox modifiers from first plane to the second
		local guidesFromSurfaceModifierCopy = copy guidesFromSurfaceModifier
		modPanel.addModToSelection guidesFromSurfaceModifierCopy
		local lengthModifierInstanceCopy = copy lengthModifierInstance
		modPanel.addModToSelection lengthModifierInstanceCopy
	),
	
	-- Reproduces an issue
	function Test_SetDisplayRenderHairInViewports =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode

		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		local hairFromGuidesModifierInstance = Ox_Hair_From_Guides viewCountPercentage:10 denseCount:100
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		local viewportHairCount = OxGetStrandCount distributionObject
		
		-- With default viewport representation only 10 hairs should be generated
		EAssert.AreNearEqual 10 viewportHairCount epsilon:4
		
		-- Turn on render display
		OxSetDisplayRenderHairInViewports true true
			
		-- With render representation 100 hairs should be generated
		EAssert.AreNearEqual 100 (OxGetStrandCount distributionObject) epsilon:3
			
		OxSetDisplayRenderHairInViewports false true
		EAssert.AreEqual viewportHairCount (OxGetStrandCount distributionObject)
	),
	
	function SceneUnitsModifierTest modifierInstance =
	(
		-- Initially use centimeters for scene scale
		units.SystemType = #Centimeters
		
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		local distributionObjectName = distributionObject.Name
		max modify mode

		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:10
		
		-- Various modifiers to test for scene scaling issues
		modPanel.addModToSelection modifierInstance
			
		local verticesInCentimeters = OxGetVertices distributionObject true
			
		-- Save, reset the scene, and switch system units to inches
		holdMaxFile()
		TestUtilities.ResetScene()
		units.SystemType = #Inches
			
		-- Reload scene and change its units
		fetchMaxFile quiet:true
			
		distributionObject = GetNodeByName distributionObjectName
		local verticesInInches = OxGetVertices distributionObject true
			
		-- If scene was scaled properly the changes to all procedural parameters should be in such a way that proportions in the hair are preserved
		local scaleFactor =  (verticesInCentimeters[1].x) / (verticesInInches[1].x)
			
		local averageDistanceBetweenPoints = 0
		for i = 1 to verticesInInches.count do
		(
			averageDistanceBetweenPoints += distance verticesInCentimeters[i] (verticesInInches[i] * scaleFactor)
		)
		
		averageDistanceBetweenPoints /= (verticesInInches.count)
		EAssert.IsLessThan 1 averageDistanceBetweenPoints 
	),
	
	function Test_SceneUnitsCurling =
	(
		SceneUnitsModifierTest (Ox_Strand_Curling form:0 magn:2 phase:10 phaseOff:3 Stretch:1)
	),
	
	function Test_SceneUnitsFrizz =
	(
		SceneUnitsModifierTest (Ox_Strand_Frizz amount:70)
	),
	
	function Test_SceneUnitsMultiplier =
	(
		SceneUnitsModifierTest (Ox_Strand_Multiplier Twist:4 spreadRoot:2 spreadTip:6 fluffRoot:1 fluffTip:8 shiftRoot:2 shiftTip:6)
	),
	
	function Test_ModifiersAppliedInCorrectCoordinateSpace =
	(
		OxResetClassParameters (Sphere mapcoords:on)
		OxResetClassParameters (Ox_Guides_from_Surface())
			
		local distributionObject = Sphere mapcoords:on isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface count:10 method:2)
		local boundingBoxWithoutModifier = nodeLocalBoundingBox distributionObject
		
		modPanel.addModToSelection (Ox_Strand_Frizz amount:0 outPercent:0)
		local boundingBoxWithModifier = nodeLocalBoundingBox distributionObject
		
		-- If points were correctly transformed the two bounding boxes should be identical
		EAssert.ArePointsNearEqual boundingBoxWithoutModifier[1] boundingBoxWithModifier[1]
		EAssert.ArePointsNearEqual boundingBoxWithoutModifier[2] boundingBoxWithModifier[2]
	),
	
	function Test_CloneModifiers =
	(
		-- Arnold appears in 2018
		EUnit.SkipBeforeMaxVersion 2018

		-- TODO: Add other modifiers known to have problems when cloning
		local guideModifiersToTest = #( Ox_Resolve_Collisions(), Ox_Oscillator(), Ox_Arnold() )
		
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		for i = 1 to (guideModifiersToTest.count) do
		(
			modPanel.addModToSelection guideModifiersToTest[i]
			distributionObject2 = copy distributionObject
			deleteModifier distributionObject guideModifiersToTest[i]
		)
	),
	
	function Test_CollapseModifiers =
	(
		-- TODO: Add other modifiers known to have problems when cloning
		local hairModifiersToTest = #( Ox_Push_Away_From_Surface() )
		
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		modPanel.addModToSelection (Ox_Hair_from_Guides denseCount:20 viewCountPercentage:100)
		
		local changeWidthMod = Ox_Change_Width()
		
		for i = 1 to (hairModifiersToTest.count) do
		(
			modPanel.addModToSelection hairModifiersToTest[i]
			modPanel.addModToSelection changeWidthMod
			
			-- Collapse to currently tested modifier
			distributionObject2 = copy distributionObject
			maxOps.CollapseNodeTo distributionObject2 2 on
			delete distributionObject2
			
			deleteModifier distributionObject changeWidthMod
			deleteModifier distributionObject hairModifiersToTest[i]
		)
	),
	
	function Test_CollapsingHairPreservesShape =
	(
		-- Add hair and spread it apart using surface comb
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		--modPanel.addModToSelection (Ox_Surface_Comb())
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance

		-- Move control points
		subobjectLevel = 2
		editGuidesInstance.SelectRoots #{1,2}
		editGuidesInstance.MoveSelectedControlPoints [50,0,0]
		modPanel.addModToSelection (Ox_Hair_from_Guides renderCount:20 viewCount:20 )
			--method:4)
		
		
		local verticesBeforeCollapse = OxGetVertices distributionObject true
		maxOps.CollapseNodeTo distributionObject 1 on
		distributionObject.displayFraction = 100
		local verticesAfterCollapse = OxGetVertices distributionObject true
		
		EAssert.ArePointsAllNearEqual verticesBeforeCollapse verticesAfterCollapse
	),

	-- Reproduces strand groups allocation crash in issues #3719 and #3728
	function Test_StrandGroupsAllocationCrash =
	(
		-- Create a downward-facing plane
		local distributionObject = Plane length:40 width:40 lengthSegs:4 widthSegs:4 pos:[0,0,50] isSelected:on
		max modify mode
		modPanel.addModToSelection (Normalmodifier flip:on)
		
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:5

		-- Add EG to enable strand groups
		editGuidesInstance = Ox_Edit_Guides useGuideGroups:true
		modPanel.addModToSelection editGuidesInstance

		local pushAwayFromSurfaceModifierInstance = Ox_Push_Away_From_Surface distance:1
		modPanel.addModToSelection pushAwayFromSurfaceModifierInstance
		
		-- Add collision object
		local collisionObject = Sphere radius:20
		append pushAwayFromSurfaceModifierInstance.objects collisionObject

		local collisionObjectName = collisionObject.name

		-- Save and reload the scene
		holdMaxFile()
		fetchMaxFile quiet:true

		collisionObject = getNodeByName collisionObjectName
		-- Changing a property of the collision object after reloading triggers the crash
		collisionObject.radius = 19
	),
	
	function Test_DeformerOnGuides =
	(
		local distributionObject = Plane lengthSegs:1 widthSegs:1 isSelected:on
		max modify mode
		
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:2
		local verticesBeforeDeformer = OxGetVertices distributionObject true
		modPanel.addModToSelection (FFD_4x4x4 ())
		local verticesAfterDeformer = OxGetVertices distributionObject true
			
		EAssert.ArePointsAllNearEqual verticesBeforeDeformer verticesAfterDeformer
	)
)

EUnit.AddTestClass Test_Modifiers
ok
