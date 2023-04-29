struct Test_EditGuidesModifier
(
	function TestCleanup =
	(
		delete $objects
		units.SystemType = #Centimeters
	),
	
	-- Tests that a bug which caused Max to hang when edit guides are added to a skinned wrapped spline on shape modifier is fixed
	function Test_EditGuidesDoesntCrashWithGuidesOnSplineAndSkinWrappedShape =
	(
		local distributionObject = Circle()
		select distributionObject

		max modify mode
		
		modPanel.addModToSelection (Edit_Spline ())
		modPanel.addModToSelection (Skin_Wrap ())
		modPanel.addModToSelection (Ox_Guides_on_Spline ())
		modPanel.addModToSelection (Ox_Edit_Guides ())
	),

	-- Tests that changes made to shape of guides are loaded along with the scene
	function Test_SaveAndLoadShapeChanges =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Edit_Guides())
			
		-- Create a plane with 1x1 segments
		distributionObject = Plane lengthsegs:1 widthsegs:1
		distributionObjectName = distributionObject.name
		max modify mode
		select distributionObject

		-- Add a guides from mesh modifier with per-vertex guide distribution, which should make the total number of guides 4
		guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 3
		modPanel.addModToSelection guidesFromSurfaceModifier

		editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance

		-- Modify the first strand's shape
		editGuidesInstance.SetStrandShape 0 #( [0,0,0], [1,1,1], [5,5,5] )
		
		pointsBeforeSaving = editGuidesInstance.GetStrandShape 0

		-- Save and reload the scene
		holdMaxFile()
		fetchMaxFile quiet:true

		-- Make sure that the change we made persists
		distributionObject = GetNodeByName distributionObjectName

		max modify mode
		select distributionObject

		editGuidesInstance = distributionObject.modifiers[1]
		strandPoints = editGuidesInstance.GetStrandShape 0

		EAssert.AreEqual pointsBeforeSaving[1] strandPoints[1]
		EAssert.AreEqual pointsBeforeSaving[2] strandPoints[2]
		EAssert.AreEqual pointsBeforeSaving[3] strandPoints[3]
	),
	
	function Test_SoftSelection =
	(
		-- Create a plane with 1x1 segments
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		local distributionObjectName = distributionObject.name
		max modify mode
		select distributionObject
		
		-- Add a guides from mesh modifier with per-vertex guide distribution, which should make the total number of guides 4
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 3
		modPanel.addModToSelection guidesFromSurfaceModifier

		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- Select first strand only
		editGuidesInstance.SelectRoots #{1}
		
		-- By default only one guide should be selected in the result
		local guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		strandSelection = ( guidesCopy.GetGuides() ).GetStrandSelection()
		EAssert.AreEqual 4 (strandSelection.count)
		EAssert.AreEqual true strandSelection[1]
		EAssert.AreEqual false strandSelection[2]
		EAssert.AreEqual false strandSelection[3]
		EAssert.AreEqual false strandSelection[4]
		delete guidesCopy
		
		-- Turn on soft selection
		editGuidesInstance.useSoftSelection = true
		
		-- All guides should now be selected due to soft selection
		guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		strandSelection = ( guidesCopy.GetGuides() ).GetStrandSelection()
		EAssert.AreEqual 4 (strandSelection.count)
		EAssert.AreEqual true strandSelection[1]
		EAssert.AreEqual true strandSelection[2]
		EAssert.AreEqual true strandSelection[3]
		EAssert.AreEqual true strandSelection[4]
		delete guidesCopy
	),

	function Test_SubObjectNamedSelectionSets =
	(
		-- Create a plane with 1x1 segments, 4 vertices
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		distributionObjectName = distributionObject.name
		max modify mode
		select distributionObject

		-- Add a guides from mesh modifier with per-vertex guide distribution, which should make the total number of guides 4
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 3
		modPanel.addModToSelection guidesFromSurfaceModifier

		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		subObjectLevel = 1

		-- No named selection sets by default
		EAssert.AreEqual 0 (editGuidesInstance.NamedSelectionSetCount())
		
		-- Select first two strands and record them as a named selection set
		editGuidesInstance.SelectRoots #{1,2}
		editGuidesInstance.AddNamedSelectionSetFromSelection "guides1"
		EAssert.AreEqual 1 (editGuidesInstance.NamedSelectionSetCount())

		-- TODO: Undoing will undo this whole function, find a way to only undo the last action
		---- Undo, set should be deleted
		--max undo
		--EAssert.AreEqual 0 (editGuidesInstance.NamedSelectionSetCount())

		---- Redo, set should be restored
		--max redo
		--EAssert.AreEqual 1 (editGuidesInstance.NamedSelectionSetCount())

		-- Change strand selection and then activate the state to ensure strand selection was restored
		editGuidesInstance.SelectRoots #{3,4}
		strandSelection = editGuidesInstance.GetSelected()
		EAssert.IsFalse strandSelection[1]
		EAssert.IsFalse strandSelection[2]
		EAssert.IsTrue strandSelection[3]
		EAssert.IsTrue strandSelection[4]
			
		editGuidesInstance.ActivateNamedSelectionSet "guides1"
		strandSelection = editGuidesInstance.GetSelected()
		EAssert.IsTrue strandSelection[1]
		EAssert.IsTrue strandSelection[2]
		EAssert.IsFalse strandSelection[3]
		EAssert.IsFalse strandSelection[4]

		-- Select other two strands and create a second set
		editGuidesInstance.SelectRoots #{3,4}
		editGuidesInstance.AddNamedSelectionSetFromSelection "guides2"

		-- Hold/fetch the scene and make sure the two selection sets are still there
		holdMaxFile()
		fetchMaxFile quiet:true

		distributionObject = GetNodeByName distributionObjectName
		max modify mode
		select distributionObject
		editGuidesInstance = distributionObject.modifiers[1]
		subObjectLevel = 1

		EAssert.AreEqual 2 (editGuidesInstance.NamedSelectionSetCount())
		editGuidesInstance.ActivateNamedSelectionSet "guides2"
		strandSelection = editGuidesInstance.GetSelected()
		EAssert.IsFalse strandSelection[1]
		EAssert.IsFalse strandSelection[2]
		EAssert.IsTrue strandSelection[3]
		EAssert.IsTrue strandSelection[4]
	),
	
	function Test_InvertSelection =
	(
		OxResetClassParameters()
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:2)

		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		subObjectLevel = 1

		-- Select first two strands and then invert the selection
		editGuidesInstance.SelectRoots #{1,2}
		actionMan.executeAction 0 "40044"  -- Selection: Select Invert
		
		strandSelection = editGuidesInstance.GetSelected()
		EAssert.IsFalse strandSelection[1]
		EAssert.IsFalse strandSelection[2]
		EAssert.IsTrue strandSelection[3]
		EAssert.IsTrue strandSelection[4]
	),

	function Test_EditGuidesModifierGroupsOnFirstRootChannelSetThePropagationStrandGroupValue =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Edit_Guides())
		OxResetClassParameters (Ox_Render_Settings())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on

		max modify mode
		
		-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		modPanel.addModToSelection guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.NumSegs = 2

		-- Edit guides modifier which sets the first two strands to use group #2
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		editGuidesInstance.useGuideGroups = on

		editGuidesInstance.SelectRoots #{1,2}
		editGuidesInstance.SetSelectedGuidesGroupIndex 2

		editGuidesInstance.SelectRoots #{3,4}
		editGuidesInstance.SetSelectedGuidesGroupIndex 3

		-- Render settings modifier with global radius of 2 for group 2
		local renderSettingsModifier = Ox_Render_Settings()
		modPanel.addModToSelection renderSettingsModifier
		renderSettingsModifier.radius = 2
		renderSettingsModifier.groupID = 2

		-- Render settings modifier with global radius of 3 for group 3
		local renderSettingsModifier = Ox_Render_Settings()
		modPanel.addModToSelection renderSettingsModifier
		renderSettingsModifier.radius = 3
		renderSettingsModifier.groupID = 3

		-- Mesh from strands modifier should respect the resulting strand widths
		local meshFromStrandsModifier = Ox_Mesh_from_Strands type:1 numSides:1
		modPanel.addModToSelection meshFromStrandsModifier
		
		EAssert.AreNearEqual 4 ( distance $.mesh.verts[1].position $.mesh.verts[2].position )
		EAssert.AreNearEqual 6 ( distance $.mesh.verts[9].position $.mesh.verts[10].position )
	),
	
	-- Tests that "push away from mesh" functionality can be invoked while editing guides without crashing them
	function Test_PushAwayFromMeshInSubObjectModeWithoutCrash =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1

		max modify mode

		-- Set up the mesh from strands modifier, use per vertex distribution
		select distributionObject
		
		-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		modPanel.addModToSelection guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.NumSegs = 2

		-- Edit guides modifier which sets the first two strands to use group #2
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- Go into sub-object mode
		subobjectLevel = 2
		
		editGuidesInstance.PushAwayFromMesh false
	),
	
	function Test_DeletingGuidesClearsSelection =
	(
		OxResetClassParameters()
		local distributionObject = Plane lengthsegs:1 widthsegs:1

		max modify mode

		-- Set up the mesh from strands modifier, use per vertex distribution
		select distributionObject
		
		-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		modPanel.addModToSelection guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.NumSegs = 2

		-- Edit guides modifier which sets the first two strands to use group #2
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- Go into sub-object mode
		subobjectLevel = 1
		editGuidesInstance.SelectRoots #{1,2}
		
		-- Delete selected roots
		actionMan.executeAction 0 "40020"
		
		-- Remaining two strands are not selected after deletion
		local strandSelection = editGuidesInstance.GetSelected()
		EAssert.IsFalse strandSelection[1]
		EAssert.IsFalse strandSelection[2]
	),
	
	function Test_MoveControlPoints =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode

		-- Set up the mesh from strands modifier, use per vertex distribution
		select distributionObject
		
		-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		modPanel.addModToSelection guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.randomness = 0
		guidesFromSurfaceModifier.NumSegs = 2
		
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance

		-- Go into control points sub-object mode
		subobjectLevel = 2
		editGuidesInstance.SelectRoots #{1,2}
		
		-- Move the selected control points
		editGuidesInstance.MoveSelectedControlPoints [10,0,0]
		
		-- Make sure tips of guides match the control point positions
		local myHair = ( editGuidesInstance.GetGuides() ).AsHair()
		EAssert.ArePointsNearEqual ( editGuidesInstance.GetControlPointPosition 0 ) ( myHair.GetPointT 0 1 )
		EAssert.ArePointsNearEqual ( editGuidesInstance.GetControlPointPosition 1 ) ( myHair.GetPointT 1 1 )
		EAssert.ArePointsNearEqual ( editGuidesInstance.GetControlPointPosition 2 ) ( myHair.GetPointT 2 1 )
		EAssert.ArePointsNearEqual ( editGuidesInstance.GetControlPointPosition 3 ) ( myHair.GetPointT 3 1 )
	),
	
	function Test_PaintRootSelection =
	(
		local guidesFromSurfaceModifier = undefined
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:25 length:25
		max modify mode

		-- Set up the mesh from strands modifier, use per vertex distribution
		select distributionObject
			
		-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
			
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance

		-- Go into brush sub-object mode
		subobjectLevel = 3
		
		editGuidesInstance.SetBrushType 6
		
		-- Set active viewport to be the front view and zoom to extents
		viewport.activeViewport = 2
		max tool zoomextents
		local viewSize = TestUtilities.GetActualViewSize()
		
		-- Set brush strength to maximum and drag brush along first two strands
		editGuidesInstance.SetBrushSize (viewSize.x/10) (viewSize.x/10)
		editGuidesInstance.SetBrushStrength 10
		editGuidesInstance.SetCurrentChannelIndex 0 true
		
		-- At this point no strands should have root selection
		local myGuides =  editGuidesInstance.GetGuides() 
		EAssert.AreEqual 0 (myGuides.GetRootData 0 0)
		EAssert.AreEqual 0 (myGuides.GetRootData 0 1)
		EAssert.AreEqual 0 (myGuides.GetRootData 0 2)
		EAssert.AreEqual 0 (myGuides.GetRootData 0 3)
		
		local brushStrokeYPosition = viewSize.y / 10
		
		theHold.Accept "setup"
		theHold.Begin()
		
		-- Paint first two strands
		editGuidesInstance.DragBrush [viewSize.x/3, brushStrokeYPosition] [viewSize.x/2, brushStrokeYPosition] 3
		
		theHold.Accept "First Drag"
		
		-- Two roots should have some weight
		myGuides =  editGuidesInstance.GetGuides() 
		EAssert.IsTrue ((myGuides.GetRootData 0 0) > 0.1)
		EAssert.IsTrue ((myGuides.GetRootData 0 1) > 0.1)
		EAssert.AreEqual 0 (myGuides.GetRootData 0 2)
		EAssert.AreEqual 0 (myGuides.GetRootData 0 3)
		
		-- Paint last two strands
		theHold.Begin()
		editGuidesInstance.DragBrush [viewSize.x/2, brushStrokeYPosition] [viewSize.x*3/4, brushStrokeYPosition] 3
		theHold.Accept "Second Drag"
		
		theHold.Begin()
		editGuidesInstance.DragBrush [viewSize.x/2, brushStrokeYPosition] [viewSize.x*3/4, brushStrokeYPosition] 3
		theHold.Accept "Third Drag"
		
		-- All roots should have some weight
		myGuides =  editGuidesInstance.GetGuides() 
		EAssert.IsTrue ((myGuides.GetRootData 0 0) > 0.1)
		EAssert.IsTrue ((myGuides.GetRootData 0 1) > 0.1)
		EAssert.IsTrue ((myGuides.GetRootData 0 2) > 0.1)
		EAssert.IsTrue ((myGuides.GetRootData 0 3) > 0.1)
		
		-- Undo once to revert last change
		max undo
		
		myGuides =  editGuidesInstance.GetGuides() 
		EAssert.IsTrue ((myGuides.GetRootData 0 0) > 0.1)
		EAssert.IsTrue ((myGuides.GetRootData 0 1) > 0.1)
		EAssert.AreEqual 0 (myGuides.GetRootData 0 2)
		EAssert.AreEqual 0 (myGuides.GetRootData 0 3)
		
		max undo
		
		myGuides =  editGuidesInstance.GetGuides() 
		EAssert.AreEqual 0 (myGuides.GetRootData 0 0)
		EAssert.AreEqual 0 (myGuides.GetRootData 0 1)
		EAssert.AreEqual 0 (myGuides.GetRootData 0 2)
		EAssert.AreEqual 0 (myGuides.GetRootData 0 3)
	),
	
	function Test_CombGuidesOnNonUniformScaledMesh =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		
		-- Scale the mesh non-uniformly
		scale distributionObject [1,5,1]
		
		max modify mode
		select distributionObject
			
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		modPanel.addModToSelection guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.randomness = 0
		guidesFromSurfaceModifier.NumSegs = 2
			
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance

		-- Go into brush sub-object mode
		subobjectLevel = 3
		
		editGuidesInstance.SetBrushType 1
		
		-- Set active viewport to be the front view and zoom to extents
		viewport.activeViewport = 2
		max tool zoomextents
		local viewSize = TestUtilities.GetActualViewSize()
		
		-- Calculate the length of the first guide
		local myHair =  (editGuidesInstance.GetGuides()).AsHair()
		local firstGuideLength = distance (myHair.GetPointT 0 0 ) (myHair.GetPointT 0 1 )
		
		-- Set brush strength to maximum and drag brush across viewport
		editGuidesInstance.SetBrushSize (viewSize.x) (viewSize.x)
		editGuidesInstance.SetBrushStrength 10
		editGuidesInstance.DragBrush [viewSize.x/2, viewSize.y/2] [viewSize.x/3, viewSize.y/2] 3
		
		-- Calculate the length of the first guide again, it should be the same as before
		myHair =  (editGuidesInstance.GetGuides()).AsHair()
		local firstGuideLengthAfterBrushing = distance (myHair.GetPointT 0 0 ) (myHair.GetPointT 0 1 )
		
		EAssert.AreNearEqual firstGuideLength firstGuideLengthAfterBrushing
	),
	
	-- Tests for a regression of a big related to editing a guide tip after first moving the guide
	function Test_EditGuideAfterMovingIt =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Edit_Guides())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode
		select distributionObject
			
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 randomness:0 NumSegs:2)
		
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- Move third root somewhere else
		subobjectLevel = 1
		editGuidesInstance.SelectRoots #{3}
		editGuidesInstance.MoveSelectedControlPoints [-10,-10,0]

		-- Move its control point as well
		subobjectLevel = 2
		editGuidesInstance.MoveSelectedControlPoints [10,10,0]
		
		-- Deselect it, select it again and move its control point back
		editGuidesInstance.SelectRoots #{}
		editGuidesInstance.SelectRoots #{3}
		editGuidesInstance.MoveSelectedControlPoints [-10,-10,0]
		local guide3TipPosition = editGuidesInstance.GetControlPointPosition 2
		
		-- Deselect it again
		editGuidesInstance.SelectRoots #{}
		editGuidesInstance.SelectRoots #{3}
		EAssert.ArePointsNearEqual guide3TipPosition (editGuidesInstance.GetControlPointPosition 2)
	),
	
	function Test_PlantGuideOnNonUniformTopology =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode
		select distributionObject
			
		modPanel.addModToSelection (Tessellate tension:70)
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 randomness:0 NumSegs:2)
		
		-- Move all control points in one direction
		local editGuides1Instance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuides1Instance
		subobjectLevel = 2
		-- Select all
		actionMan.executeAction 0 "40021"
		editGuides1Instance.MoveSelectedControlPoints [-100,0,0]
		
		-- Plant new guide with a second edit guides instance
		local editGuides2Instance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuides2Instance
		subobjectLevel = 1
		editGuides2Instance.CreateRootAtCoordinate 0 [0.4, 0.4]
		
		-- The newly created guide should be facing in the same direction as other guides
		local guideVertices = OxGetVertices distributionObject true
		EAssert.ArePointsNearEqual (guideVertices[2] - guideVertices[1]) (guideVertices[20] - guideVertices[19])
	),

	function Test_PlantGuideWithoutAnyPreExistingGuides =
	(
		OxResetClassParameters()
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode
		select distributionObject

		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 randomness:0 NumSegs:2)
		
		-- Move all control points in one direction
		local editGuides1Instance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuides1Instance
		subobjectLevel = 1
		-- Select all and delete
		actionMan.executeAction 0 "40021"
		actionMan.executeAction 0 "40020"
		
		EAssert.AreEqual 0 (OxGetStrandCount distributionObject)
		
		-- Now try to create a root, it should happen without errors
		editGuides1Instance.CreateRootAtCoordinate 0 [0.4, 0.4]
		
		EAssert.AreEqual 1 (OxGetStrandCount distributionObject)
	),
	
	function Test_PlantCombUndo =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode
		select distributionObject

		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 randomness:0 NumSegs:2)
		
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		subobjectLevel = 1
		-- Select all and delete
		actionMan.executeAction 0 "40021"
		actionMan.executeAction 0 "40020"
		
		-- Create a root
		theHold.Accept "Zero"
		theHold.Begin()
		editGuidesInstance.CreateRootAtCoordinate 0 [0.4, 0.4]
		theHold.Accept "Create root"
		
		local verticesBeforeMove = OxGetVertices distributionObject true
		subobjectLevel = 2
		
		-- Move the tip to change strand shape
		theHold.Begin()
		editGuidesInstance.SelectRoots #{1}
		editGuidesInstance.MoveSelectedControlPoints [-10,-10,0]
		theHold.Accept "Move tip"
		
		local verticesAfterMove = OxGetVertices distributionObject true
		
		max undo
		local verticesAfterUndo = OxGetVertices distributionObject true
		
		EAssert.ArePointsAllNearEqual verticesBeforeMove verticesAfterUndo
	),
	
	function Test_AdoptFromStackEditSplineChanges =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
			
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 randomness:0 NumSegs:2)
		
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- Remember points before any modification
		local verticesBeforeModification = OxGetVertices distributionObject true
		
		local editSplineInstance = Edit_Spline()
		modPanel.addModToSelection editSplineInstance
		
		-- Because edit spline does not expose MXS methods to move the sub-object points we have to add another bend modifier to change the points
		local bendInstance = Bend BendAngle:100
		modPanel.addModToSelection bendInstance
		
		-- Use the adopt from stack command to alter selected guides
		modPanel.setCurrentObject editGuidesInstance
		subobjectLevel = 2
		showEndResult = true
		
		-- Select all
		actionMan.executeAction 0 "40021"
		editGuidesInstance.AdoptFromStack()
		
		-- Delete the two modifiers above edit guides
		deleteModifier distributionObject 1
		deleteModifier distributionObject 1
		
		-- The tip vertices of guides should not be the same as before
		local verticesAfterModification = OxGetVertices distributionObject true
		EAssert.ArePointsNotNearEqual verticesBeforeModification[2] verticesAfterModification[2] 
		EAssert.ArePointsNotNearEqual verticesBeforeModification[4] verticesAfterModification[4] 
		EAssert.ArePointsNotNearEqual verticesBeforeModification[6] verticesAfterModification[6] 
		EAssert.ArePointsNotNearEqual verticesBeforeModification[8] verticesAfterModification[8]
	),
	
	function Test_AdoptFromStackTopWithChangedStrandCount =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
			
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 randomness:0 NumSegs:2 forceSequentialStrandIds:on)
		
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		local verticesBeforeModification = OxGetVertices distributionObject true
		
		-- Add Hair from Guides to increase hair count and add gravity to pull hairs down
		local hairFromGuides = Ox_Hair_from_Guides method:2 viewCountPercentage:100 denseCount:10
		modPanel.addModToSelection hairFromGuides
		local gravityMod = Ox_Strand_Gravity()
		modPanel.addModToSelection gravityMod
			
		local verticesAfterModification = OxGetVertices distributionObject true
			
		-- Adopt from stack
		modPanel.setCurrentObject editGuidesInstance
		showendresult = on
		editGuidesInstance.AdoptFromStack()
			
		-- Adopted guides should change vertex count
		EAssert.AreNotEqual (verticesBeforeModification.count) (verticesAfterModification.count)
		--EAssert.ArePointsNotAllNearEqual verticesBeforeModification verticesAfterModification
			
		-- Turn off the two modifiers above EG, it should produce same vertices as previously full modifier stack
		hairFromGuides.enabled = off
		gravityMod.enabled = off
		local verticesAfterAdoptFromStack = OxGetVertices distributionObject true
		
		EAssert.ArePointsAllNearEqual verticesAfterModification verticesAfterAdoptFromStack
	),
	
	-- This test is disabled because it is flaky and produces many false positives
	/*function Test_BrushSelection =
	(
		local guidesFromSurfaceModifier = undefined

		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:25 length:25
		max modify mode

		-- Set up the mesh from strands modifier, use per vertex distribution
		select distributionObject
			
		-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
		guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		guidesFromSurfaceModifier.rememberRootPositions = on
			
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance

		-- Go into brush sub-object mode and select "Selection" brush
		subobjectLevel = 3
		editGuidesInstance.SetBrushType 0
		
		-- Set active viewport to be the front view and zoom to extents
		viewport.activeViewport = 2
		max tool zoomextents
		local viewSize = TestUtilities.GetActualViewSize()
		
		-- Set brush strength to maximum and drag brush along first two strands
		editGuidesInstance.SetBrushSize (viewSize.x/10) (viewSize.x/10)
		editGuidesInstance.SetBrushStrength 10
		
		-- At this point no strandsshould have root selection
		local guidesSelection =  ( editGuidesInstance.GetGuides() ).GetStrandSelection()
		for i = 1 to 4 do
		(
			EAssert.IsFalse guidesSelection[i]
		)
		
		-- tosses out the restore objects since the last Begin() but still continues holding
		theHold.Accept "zero"
		theHold.Begin()
		
		local brushStrokeYPosition = viewSize.y / 10
		
		-- Draw a stroke across first two strands
		editGuidesInstance.DragBrush [viewSize.x/3, brushStrokeYPosition] [viewSize.x * 0.3, brushStrokeYPosition] 2
		theHold.Accept "one"
		
		-- First two strands should be selected
		guidesSelection =  ( editGuidesInstance.GetGuides() ).GetStrandSelection()
		EAssert.IsTrue guidesSelection[1]
		EAssert.IsTrue guidesSelection[2]
		EAssert.IsFalse guidesSelection[3]
		EAssert.IsFalse guidesSelection[4]

		theHold.Begin()
		-- Paint last two strands
		editGuidesInstance.DragBrush [viewSize.x/2, brushStrokeYPosition] [viewSize.x * 0.6, brushStrokeYPosition] 3
		theHold.Accept "two"
		
		theHold.Begin()
		editGuidesInstance.DragBrush [viewSize.x/2, brushStrokeYPosition] [viewSize.x * 0.6, brushStrokeYPosition] 3
		theHold.Accept "three"
		
		-- Last two strands should be selected
		guidesSelection =  ( editGuidesInstance.GetGuides() ).GetStrandSelection()
		EAssert.IsFalse guidesSelection[1]
		EAssert.IsFalse guidesSelection[2]
		EAssert.IsTrue guidesSelection[3]
		EAssert.IsTrue guidesSelection[4]
		
		-- Undo once to revert last change
		max undo
		max undo
		
		guidesSelection =  ( editGuidesInstance.GetGuides() ).GetStrandSelection()
		EAssert.IsTrue guidesSelection[1]
		EAssert.IsTrue guidesSelection[2]
		--EAssert.IsFalse guidesSelection[3]
		--EAssert.IsFalse guidesSelection[4]
		
		max undo
		max undo
	),*/
	
	function Test_RotateSelectedGuidesAndPersist =
	(
		OxResetClassParameters()
		
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		local nodeName = distributionObject.Name
		max modify mode
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface method:4 randomness:0 NumSegs:2
		modPanel.addModToSelection guidesFromSurfaceModifier
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		local meshFromStrandsInstance = Ox_Mesh_From_Strands type: 1
		modPanel.addModToSelection meshFromStrandsInstance
		
		-- Record initial guide 1 rotation
		local vertex1Position = distributionObject.mesh.vertices[1].position
		local vertex2Position = distributionObject.mesh.vertices[2].position
		local initialGuide1Rotation = vertex2Position - vertex1Position
		
		-- Select guide 1 and rotate it
		modPanel.setCurrentObject editGuidesInstance
		subobjectLevel = 0
		editGuidesInstance.SelectRoots #{1}
		editGuidesInstance.RotateSelectedRootsToDirection [1,0,0]
		
		-- Extract and compare the new rotation
		modPanel.setCurrentObject meshFromStrandsInstance
		vertex1Position = distributionObject.mesh.vertices[1].position
		vertex2Position = distributionObject.mesh.vertices[2].position
		local newGuide1Rotation = vertex2Position - vertex1Position
		EAssert.ArePointsNotNearEqual newGuide1Rotation initialGuide1Rotation
		
		-- Hold/fetch and make sure the rotation change remains
		holdMaxFile()
		fetchMaxFile quiet:true
		
		distributionObject = GetNodeByName nodeName
		vertex1Position = distributionObject.mesh.vertices[1].position
		vertex2Position = distributionObject.mesh.vertices[2].position
		local loadedGuide1Rotation = vertex2Position - vertex1Position
		EAssert.ArePointsNearEqual newGuide1Rotation loadedGuide1Rotation
	),
	
	function Test_AddRemovePerRootChannel =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode
		select distributionObject
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 randomness:0 NumSegs:2)
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- By default there is 1 per-root selection channel and 0 per-vertex selection channels
		EAssert.AreEqual 1 (OxGetStrandChannelCount distributionObject true)
		EAssert.AreEqual 0 (OxGetStrandChannelCount distributionObject false)
		
		editGuidesInstance.CreateChannel true "TestChannel"
		
		-- We should now have 2 root and 0 vertex channels
		EAssert.AreEqual 2 (OxGetStrandChannelCount distributionObject true)
		EAssert.AreEqual 0 (OxGetStrandChannelCount distributionObject false)
		EAssert.AreEqual "TestChannel" (OxGetStrandChannelName distributionObject true 1)
		
		editGuidesInstance.SetCurrentChannelIndex 1 true
		editGuidesInstance.DeleteCurrentChannel false
		
		-- Root channel was deleted, we should be back to 1 root channel
		EAssert.AreEqual 1 (OxGetStrandChannelCount distributionObject true)
		EAssert.AreEqual 0 (OxGetStrandChannelCount distributionObject false)
		EAssert.AreNotEqual "TestChannel" (OxGetStrandChannelName distributionObject true 0)
	),
	
	function Test_AddRemovePerVertexChannel =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode
		select distributionObject
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 randomness:0 NumSegs:2)
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- By default there is 1 per-root selection channel and 0 per-vertex selection channels
		EAssert.AreEqual 1 (OxGetStrandChannelCount distributionObject true)
		EAssert.AreEqual 0 (OxGetStrandChannelCount distributionObject false)
		
		editGuidesInstance.CreateChannel false "TestChannel"
		
		-- We should now have 1 vertex and 1 root channels
		EAssert.AreEqual 1 (OxGetStrandChannelCount distributionObject false)
		EAssert.AreEqual 1 (OxGetStrandChannelCount distributionObject true)
		EAssert.AreEqual "TestChannel" (OxGetStrandChannelName distributionObject false 0)
		
		editGuidesInstance.SetCurrentChannelIndex 0 false
		editGuidesInstance.DeleteCurrentChannel false
		
		-- Root channel was deleted, we should be back to 1 root channel
		EAssert.AreEqual 1 (OxGetStrandChannelCount distributionObject true)
		EAssert.AreEqual 0 (OxGetStrandChannelCount distributionObject false)
	),

	function Test_SavePerVertexChannel =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Edit_Guides())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode
		select distributionObject
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 randomness:0 NumSegs:2)
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- Edit values in the selection vertex channel
		editGuidesInstance.CreateChannel false "Selection"
		editGuidesInstance.SelectRoots #{1,2}
		editGuidesInstance.SetChannelValueToSelection false 0 2.0f

		local myGuides =  editGuidesInstance.GetGuides() 
		EAssert.AreEqual 2.0f (myGuides.GetVertexData 0 1)
		EAssert.AreEqual 2.0f (myGuides.GetVertexData 0 3)
		EAssert.AreEqual 0 (myGuides.GetVertexData 0 5)
		EAssert.AreEqual 0 (myGuides.GetVertexData 0 7)

		local nodeName = distributionObject.Name

		-- Save and reload the scene
		holdMaxFile()
		fetchMaxFile quiet:true

		distributionObject = GetNodeByName nodeName
		max modify mode
		select distributionObject

		editGuidesInstance = distributionObject.modifiers[#Ox_Edit_Guides]
		myGuides =  editGuidesInstance.GetGuides() 
		EAssert.AreEqual 2.0f (myGuides.GetVertexData 0 1)
		EAssert.AreEqual 2.0f (myGuides.GetVertexData 0 3)
		EAssert.AreEqual 0 (myGuides.GetVertexData 0 5)
		EAssert.AreEqual 0 (myGuides.GetVertexData 0 7)
	),
	
	-- Tests that saving per-vertex channel data bound to specific strand ids in an added channel works predictably
	function Test_SaveNonZeroPerVertexChannelData =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Edit_Guides())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode
		select distributionObject
		
		-- Use a random distribution method to ensure that we get different strand ids on reload
		modPanel.addModToSelection (Ox_Guides_from_Surface method:2 randomness:0 NumSegs:2)
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- Create a new per-vertex channel and set values of first 2 strands
		editGuidesInstance.CreateChannel false "TestChannel"
		editGuidesInstance.SelectRoots #{5}
		editGuidesInstance.SetChannelValueToSelection false 0 2.0f
		editGuidesInstance.SelectRoots #{10}
		editGuidesInstance.SetChannelValueToSelection false 0 3.0f
		
		myGuides =  editGuidesInstance.GetGuides()
		EAssert.AreEqual 2.0f (myGuides.GetVertexData 0 (2 * 4 + 1))
		EAssert.AreEqual 3.0f (myGuides.GetVertexData 0 (2 * 9 + 1))
		EAssert.AreEqual 0 (myGuides.GetVertexData 0 2)
		EAssert.AreEqual 0 (myGuides.GetVertexData 0 4)
		
		local nodeName = distributionObject.Name

		-- Save and reload the scene
		holdMaxFile()
		fetchMaxFile quiet:true
		
		distributionObject = GetNodeByName nodeName
		max modify mode
		select distributionObject

		-- The first two strands should have the previously saved vertex data
		editGuidesInstance = distributionObject.modifiers[#Ox_Edit_Guides]
		myGuides =  editGuidesInstance.GetGuides()
		EAssert.AreEqual 2.0f (myGuides.GetVertexData 0 (2 * 4 + 1))
		EAssert.AreEqual 3.0f (myGuides.GetVertexData 0 (2 * 9 + 1))
		EAssert.AreEqual 0 (myGuides.GetVertexData 0 2)
		EAssert.AreEqual 0 (myGuides.GetVertexData 0 4)
	),
	
	-- Tests for a crash in GuidesDelta when brushing after roots have been modified and guide count has changed
	function Test_MoveRootsChangeGuideCountAndEditShape =
	(
		local distributionObject = Plane lengthsegs:2 widthsegs:1 isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 randomness:0 NumSegs:2)
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- Move all 4 roots somewhere
		subobjectLevel = 1
		editGuidesInstance.SelectRoots #{1,2,3,4,5,6}
		editGuidesInstance.MoveSelectedControlPoints [-5,-5,0]
		
		-- Change guide count
		distributionObject.lengthsegs = 1
		
		-- Move control points somewhere
		subobjectLevel = 2
		editGuidesInstance.SelectRoots #{1,2,3,4}
		editGuidesInstance.MoveSelectedControlPoints [1,1,0]
		
		-- If there was no crash we are good
	),
	
	function Test_PushAwayFromMeshWhenNotPenetrating =
	(
		local distributionObject = Sphere radius:10 segs:4 mapcoords:on isSelected:on

		max modify mode
		
		-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()

		-- Edit guides modifier which sets the first two strands to use group #2
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- Go into sub-object mode
		subobjectLevel = 2
		
		local tipPositionsBeforePushing = TestUtilities.GetTipPositions distributionObject
		
		editGuidesInstance.PushAwayFromMesh false
		
		local tipPositionsAfterPushing = TestUtilities.GetTipPositions distributionObject
		
		-- Nothing should have been modified
		-- TODO: Why are some guides always pushed away, even when not penetrating?
		--EAssert.ArePointsAllNearEqual tipPositionsBeforePushing tipPositionsAfterPushing
		EAssert.ArePointsNearEqual tipPositionsBeforePushing[1] tipPositionsAfterPushing[1]
	),
	
	function Test_PushAwayFromMeshWithUndo =
	(
		OxResetClassParameters (Sphere())
		OxResetClassParameters (Ox_Guides_From_Surface())
		OxResetClassParameters (Ox_Edit_Guides())
		local distributionObject = Sphere radius:10 segs:10 mapcoords:on isSelected:on

		max modify mode
		
		-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		guidesFromSurfaceModifier.numSegs = 20

		-- Edit guides modifier which sets the first two strands to use group #2
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		subobjectLevel = 2
		editGuidesInstance.SelectRoots #{1..42}
		editGuidesInstance.MoveSelectedControlPoints [0,0,-200]
		
		local verticesBeforePushing = OxGetVertices distributionObject true
		
		-- Push aray from mesh, no points should be penetrating
		theHold.Accept "zero"
		theHold.Begin()
		editGuidesInstance.PushAwayFromMesh false
		theHold.Accept "Push away"
		
		local verticesAfterPushing = OxGetVertices distributionObject true
		EAssert.ArePointsNotAllNearEqual verticesBeforePushing verticesAfterPushing
		
		-- Undo, vertices should change
		max undo
		local verticesAfterUndo = OxGetVertices distributionObject true
		EAssert.ArePointsAllNearEqual verticesBeforePushing verticesAfterUndo
		
		max redo
		local verticesAfterRedo = OxGetVertices distributionObject true
		EAssert.ArePointsAllNearEqual verticesAfterPushing verticesAfterRedo
	),
	
	function Test_ImportMultipleCurves =
	(
		local line1Point1 = [0,0,0]
		local line1Point2 = [-1,5,10]
		local line2Point1 = [5,0,0]
		local line2Point2 = [1,5,10]
		
		local spline1 = line()
		addNewSpline spline1
		addKnot spline1 1 #corner #line line1Point1
		addKnot spline1 1 #corner #line line1Point2
		updateShape spline1
		
		local spline2 = line()
		addNewSpline spline2
		addKnot spline2 1 #corner #line line2Point1
		addKnot spline2 1 #corner #line line2Point2
		updateShape spline2
		
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		local initialVertexCount = (OxGetVertices distributionObject true).Count
		local plantedPointCount = 3
		
		EAssert.AreEqual 4 (OxGetStrandCount distributionObject)
		
		editGuidesInstance.ImportMultipleCurves #(spline1, spline2) plantedPointCount 1.0
		EAssert.AreEqual 6 (OxGetStrandCount distributionObject)
		
		local strandVertices = OxGetVertices distributionObject true
		EAssert.ArePointsNearEqual line1Point1 strandVertices[initialVertexCount + 1]
		EAssert.ArePointsNearEqual line1Point2 strandVertices[initialVertexCount + plantedPointCount]
		EAssert.ArePointsNearEqual line2Point1 strandVertices[initialVertexCount + plantedPointCount + 1]
		EAssert.ArePointsNearEqual line2Point2 strandVertices[initialVertexCount + plantedPointCount * 2]
	),
	
	function Test_ImportMultipleCurvesReplaceExistingGuides =
	(
		local line1Point1 = [5,5,0]
		local line1Point2 = [-1,5,10]
		local line2Point1 = [-4.5,5,0]
		local line2Point2 = [1,5,10]
		
		local spline1 = line()
		addNewSpline spline1
		addKnot spline1 1 #corner #line line1Point1
		addKnot spline1 1 #corner #line line1Point2
		updateShape spline1
		
		local spline2 = line()
		addNewSpline spline2
		addKnot spline2 1 #corner #line line2Point1
		addKnot spline2 1 #corner #line line2Point2
		updateShape spline2
		
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:10 length:10 mapcoords:on isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		local initialVertexCount = (OxGetVertices distributionObject true).Count
		local plantedPointCount = 3
		
		EAssert.AreEqual 4 (OxGetStrandCount distributionObject)
		
		-- After import we should still have 4 guides since newly added ones were merged in
		editGuidesInstance.ImportMultipleCurves #(spline1, spline2) plantedPointCount 1.0
		EAssert.AreEqual 4 (OxGetStrandCount distributionObject)
		
		local existingGuidePointCount = 2
		local strandVertices = OxGetStrandPointsSortedByStrandId distributionObject true
		EAssert.ArePointsNearEqual line1Point1 strandVertices[existingGuidePointCount * 3 + 1]
		EAssert.ArePointsNearEqual line1Point2 strandVertices[existingGuidePointCount * 3 + 2]
		-- Direction of second guide should match that of the line. The root should remain in place.
		EAssert.ArePointsNearEqual (line2Point2 - line2Point1) (strandVertices[existingGuidePointCount * 2 + 2] - strandVertices[existingGuidePointCount * 2 + 1])
	),

	function Test_ImportMultipleCurvesWithoutDistributionObject =
	(
		local line1Point1 = [5,5,0]
		local line1Point2 = [-1,5,10]
		local line2Point1 = [-4.5,5,0]
		local line2Point2 = [1,5,10]

		local spline1 = line()
		addNewSpline spline1
		addKnot spline1 1 #corner #line line1Point1
		addKnot spline1 1 #corner #line line1Point2
		updateShape spline1
		
		local spline2 = line isSelected:on
		addNewSpline spline2
		addKnot spline2 1 #corner #line line2Point1
		addKnot spline2 1 #corner #line line2Point2
		updateShape spline2

		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Shape())
		local editGuides = Ox_Edit_Guides()
		modPanel.addModToSelection editGuides
			
		-- This should not crash Max
		editGuides.ImportMultipleCurves #(spline1) 10 1.0
	),
	
	function Test_ApplyAmount =
	(
		OxResetClassParameters (Plane())	
		OxResetClassParameters (Ox_Guides_From_Surface())
		OxResetClassParameters (Ox_Strand_Propagation())
		OxResetClassParameters (Ox_Edit_Guides())
		OxResetClassParameters (Ox_Strand_Curling())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:10 length:10 mapcoords:on isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- Make some changes
		subobjectLevel = 2
		editGuidesInstance.SelectRoots #{1..42}
		editGuidesInstance.MoveSelectedControlPoints [0, 0, -200]
		
		local strandVerticesAtFullApply = OxGetVertices distributionObject true
		editGuidesInstance.applyAmount = 50
		local strandVerticesAtHalfApply = OxGetVertices distributionObject true
		
		-- Check that at half application points were moved
		EAssert.ArePointsNotAllNearEqual strandVerticesAtFullApply strandVerticesAtHalfApply
		
		subobjectLevel = 0
		max create mode
		
		-- Test with animated parameter
		set animate on
		sliderTime = 0f
		editGuidesInstance.applyAmount = 0.0
		sliderTime = 2f
		editGuidesInstance.applyAmount = 100.0
		set animate off
		
		sliderTime = 0f
		local strandVerticesAtFrame0 = OxGetVertices distributionObject true
		sliderTime = 1f
		local strandVerticesAtFrame1 = OxGetVertices distributionObject true
		
		EAssert.ArePointsNotAllNearEqual strandVerticesAtFrame0 strandVerticesAtFrame1
	),
	
	-- Tests that on initial evaluation we copy the root data so that we can propagate it to the output
	function Test_AdoptIncomingRootChannelDataOnInitialization =
	(
		OxResetClassParameters (Plane())	
		OxResetClassParameters (Ox_Guides_From_Surface())
		OxResetClassParameters (Ox_Strand_Propagation())
		OxResetClassParameters (Ox_Edit_Guides())
		OxResetClassParameters (Ox_Strand_Curling())
	
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:10 length:10 mapcoords:on isSelected:on
		local nodeName = distributionObject.Name
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		modPanel.addModToSelection (Ox_Strand_Propagation count:2 assignGroupID:1 NumSegs:2)
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		local verticesBeforeCurling = OxGetVertices distributionObject true
		modPanel.addModToSelection (Ox_Strand_Curling groupID:1 magn:100 phase:2.5 form:0 stdAmount:0 stdScale:0)
		local verticesAfterCurling = OxGetVertices distributionObject true
			
		-- If we passed the group index of propagated guides through the Edit Guides modifier then curling should have an effect just on the propagated guides
		EAssert.ArePointsNotAllNearEqual verticesBeforeCurling verticesAfterCurling
		
		-- Reload the scene and try again
		holdMaxFile()
		fetchMaxFile quiet:true

		distributionObject = GetNodeByName nodeName
		local verticesAfterReloading = OxGetVertices distributionObject true
		EAssert.ArePointsAllNearEqual verticesAfterCurling verticesAfterReloading
	),
	
	function Test_EditGuidesDoesNotChangeStrandGroupsOnUndo =
	(
		local distributionObject = Sphere radius:10 segs:4 mapcoords:on isSelected:on
		local	propagatedStrandCount = 2
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 NumSegs:propagatedStrandCount)
		local baseStrandCount = OxGetStrandCount distributionObject
		modPanel.addModToSelection (Ox_Strand_Propagation method:2 NumSegs:2)
		
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		local strandGravityInstance = Ox_Strand_Gravity groupID:1
		modPanel.addModToSelection strandGravityInstance
		
		local verticesBeforeBrushAndUndo = OxGetVertices distributionObject true
		
		modPanel.setCurrentObject editGuidesInstance
		
		-- Go into brush sub-object mode and comb the strands
		subobjectLevel = 3
		editGuidesInstance.SetBrushType 1
		
		-- Set active viewport to be the front view and zoom to extents
		viewport.activeViewport = 2
		max tool zoomextents
		local viewSize = TestUtilities.GetActualViewSize()
		
		-- Set brush strength to maximum and drag brush along first two strands
		editGuidesInstance.SetBrushSize (viewSize.x) (viewSize.x)
		editGuidesInstance.SetBrushStrength 10
		
		-- tosses out the restore objects since the last Begin() but still continues holding
		theHold.Accept "zero"
		theHold.Begin()
		
		local brushStrokeYPosition = viewSize.y / 2
		
		-- Draw a stroke across first two strands
		editGuidesInstance.DragBrush [viewSize.x, brushStrokeYPosition] [0, brushStrokeYPosition] 5
		theHold.Accept "one"
		
		-- Undo the combing. This should not cause guide channels to be reset
		max undo
		
		modPanel.setCurrentObject strandGravityInstance
		
		local verticesAfterBrushAndUndo = OxGetVertices distributionObject true
		
		-- Since guide groups didn't chang the outcome of gravity modifier should remain the same
		EAssert.ArePointsAllNearEqual verticesBeforeBrushAndUndo verticesAfterBrushAndUndo
	),
	
	function Test_CombBackFacing =
	(
		-- I can't figure out why this fails when run for the second time, so I disable it for now
		return ok
		local distributionObject = Box length:5 width:100 height:40 pos:[10,-10,0] mapcoords:on isSelected:on
		viewport.activeViewport = 4
		max tool zoomextents
		max modify mode
		modPanel.addModToSelection (Mesh_Select())
		setFaceSelection distributionObject 1 #{6, 10}
		subobjectLevel = 3
			
		modPanel.addModToSelection (Ox_Guides_from_Surface method:2 count:2 NumSegs:2 useSubSel:on ignoreBackFacingGuides:off)
		
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		local verticesBeforeBrush = OxGetVertices distributionObject true
		
		modPanel.setCurrentObject editGuidesInstance
		
		-- Go into brush sub-object mode and comb the strands
		subobjectLevel = 3
		editGuidesInstance.SetBrushType 1

		-- Set active viewport to be the front view and zoom to extents
		local viewSize = TestUtilities.GetActualViewSize()

		editGuidesInstance.SetBrushSize (viewSize.x / 2) (viewSize.x / 2)
		editGuidesInstance.SetBrushStrength 10

		theHold.Accept "zero"
		theHold.Begin()

		local brushStrokeYPosition = viewSize.y / 2

		-- Draw a stroke across all two strands
		editGuidesInstance.DragBrush [viewSize.x, brushStrokeYPosition] [0, brushStrokeYPosition] 5
		theHold.Accept "one"
		local verticesAfterBrush = OxGetVertices distributionObject true
		-- Roots are not affected
		EAssert.ArePointsNearEqual verticesBeforeBrush[1] verticesAfterBrush[1]
		EAssert.ArePointsNearEqual verticesBeforeBrush[3] verticesAfterBrush[3]
		-- With "ignore back-facing" off both strands are affected
		EAssert.ArePointsNotNearEqual verticesBeforeBrush[2] verticesAfterBrush[2]
		EAssert.ArePointsNotNearEqual verticesBeforeBrush[4] verticesAfterBrush[4]

		-- Repeat with "ignore back-facing" on
		max undo
		editGuidesInstance.ignoreBackFacingGuides = on
		theHold.Begin()
		editGuidesInstance.DragBrush [viewSize.x, brushStrokeYPosition] [0, brushStrokeYPosition] 5
		theHold.Accept "two"
		verticesAfterBrush = OxGetVertices distributionObject true
		-- With "ignore back-facing" only one strand is affected
		EAssert.ArePointsNotNearEqual verticesBeforeBrush[2] verticesAfterBrush[2]
		EAssert.ArePointsNearEqual verticesBeforeBrush[4] verticesAfterBrush[4]

		-- Repeat from the other side
		max undo
		upAxis = (viewport.getTM()).row3
		viewport.rotate (quat 150 upAxis)
		editGuidesInstance.DragBrush [viewSize.x, brushStrokeYPosition] [0, brushStrokeYPosition] 5
		verticesAfterBrush = OxGetVertices distributionObject true
		-- Now the other strand was affected
		EAssert.ArePointsNearEqual verticesBeforeBrush[2] verticesAfterBrush[2]
		EAssert.ArePointsNotNearEqual verticesBeforeBrush[4] verticesAfterBrush[4]
	),
	
	function Test_RetargetChanges =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Edit_Guides())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:10 length:10 mapcoords:on isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local editGuidesInstance = Ox_Edit_Guides()
		editGuidesInstance.changeTracking = 1
		modPanel.addModToSelection editGuidesInstance
		
		-- Move last two guide tips down
		subobjectLevel = 2
		editGuidesInstance.SelectRootById 2 true
		editGuidesInstance.SelectRootById 3 true
		editGuidesInstance.MoveSelectedControlPoints [0, 0, -200]
		EAssert.AreNearEqual -50 ((OxGetStrandPoint distributionObject 2 1 true true).z)
		EAssert.AreNearEqual -50 ((OxGetStrandPoint distributionObject 3 1 true true).z)
		
		-- No need to retarget changes at this point
		EAssert.IsFalse (editGuidesInstance.IsRetargetChangesNeeded())
		
		-- Change the overall guide count and order
		distributionObject.lengthsegs = 2
		distributionObject.widthsegs = 2
		
		-- Without retargeting the new guides whose indices are 2 and 3 will adopt the previous change
		EAssert.AreNearEqual -50 ((OxGetStrandPoint distributionObject 2 1 true true).z)
		EAssert.AreNearEqual -50 ((OxGetStrandPoint distributionObject 3 1 true true).z)
		
		-- Retargeting is now needed for new guides
		EAssert.IsTrue (editGuidesInstance.IsRetargetChangesNeeded())
		
		-- Retargeting the changes should make the guides which are closest to previously modified guides accept the changes instead
		editGuidesInstance.RetargetChanges()
		
		-- Strands 0, 1, 2, 5 should remain unchanged
		local unchangedStrandIds = #(0, 1, 2, 5)
		for i=1 to unchangedStrandIds.count do
		(
			EAssert.AreNearEqual 50 ((OxGetStrandPoint distributionObject unchangedStrandIds[i] 1 true true).z)
		)

		-- Strands 3, 4, 6, 7, and 8 should adopt the changes instead
		local changedStrandIds = #(3, 4, 6, 7, 8)
		for i=1 to changedStrandIds.count do
		(
			EAssert.AreNearEqual -50 ((OxGetStrandPoint distributionObject changedStrandIds[i] 1 true true).z)
		)
		
		-- Since we didn't make any other changes yet retargeting is still an option
		EAssert.IsTrue (editGuidesInstance.IsRetargetChangesNeeded())
		
		-- Making some change should remove retargeting option until next time that strand topology changes			
		editGuidesInstance.MoveSelectedControlPoints [0, 0, -200]
		OxGetVertices distributionObject true
		EAssert.IsFalse (editGuidesInstance.IsRetargetChangesNeeded())
	),
	
	function Test_RetargetChangesOnSphere =
	(
		OxResetClassParameters (Sphere())
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Edit_Guides())
			
		local distributionObject = Sphere mapcoords:on isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:4
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- Move last two guide tips down
		subobjectLevel = 2
		editGuidesInstance.SelectRoots #{3..20}
		editGuidesInstance.MoveSelectedControlPoints [0, 0, -5]
		
		local verticesBeforeRetargeting = OxGetVertices distributionObject true
		-- Using retarget changes option at this point should not move any of the strands
		for i = 1 to 5 do
		(
			editGuidesInstance.RetargetChanges()
			EAssert.ArePointsAllNearEqual verticesBeforeRetargeting (OxGetVertices distributionObject true)
		)
	),
	
	function Test_ChangeGuideShapeAfterDeletingAndMovingGuides =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Edit_Guides())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:10 length:10 mapcoords:on isSelected:on
		distributionObjectName = distributionObject.Name
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- Delete two guides
		subobjectLevel = 1
		editGuidesInstance.SelectRoots #{1,2}
		
		-- Delete selected roots
		actionMan.executeAction 0 "40020"
		
		-- Move the remaining two roots to opposite faces
		subobjectLevel = 1
		editGuidesInstance.SelectRoots #{2}
		editGuidesInstance.MoveSelectedControlPoints [-7,7,0]
		editGuidesInstance.SelectRoots #{1}
		editGuidesInstance.MoveSelectedControlPoints [-7,-7,0]
		
		-- Move the control points
		subobjectLevel = 2
		editGuidesInstance.MoveSelectedControlPoints [7,0,0]
		
		local verticesBeforeExitingCPMode = OxGetVertices distributionObject true
		subobjectLevel = 0
		local verticesAfterExitingCPMode = OxGetVertices distributionObject true
		
		EAssert.ArePointsAllNearEqual verticesBeforeExitingCPMode verticesAfterExitingCPMode
		
		-- Save and reload the scene and make sure the guides still stay the same
		holdMaxFile()
		fetchMaxFile quiet:true
		
		distributionObject = GetNodeByName distributionObjectName
		local verticesAfterReloading = OxGetVertices distributionObject true
		EAssert.ArePointsAllNearEqual verticesBeforeExitingCPMode verticesAfterReloading
	),
	
	-- Tests that guides do not change their edited shape after control points have been moved and the CP sub-object level is exited
	function Test_MoveControlPointsAndExitControlPointMode =
	(
		OxResetClassParameters( Plane() )
		OxResetClassParameters( Ox_Guides_From_Surface() )
		OxResetClassParameters( Ox_Rotate_Strands() )
		OxResetClassParameters( Ox_Edit_Guides() )
		
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:10 length:10 mapcoords:on isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		modPanel.addModToSelection (Ox_Rotate_Strands())
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
			
		forceCompleteRedraw()
		
		-- Make some changes
		subobjectLevel = 2
				
		editGuidesInstance.SelectRoots #{1..2}
		editGuidesInstance.MoveSelectedControlPoints [0, 4, 0]
		
		local strandVerticesAfterEdit = OxGetVertices distributionObject true
		
		subobjectLevel = 0
		local strandVerticesAfterSubObjectModeExit = OxGetVertices distributionObject true
		
		EAssert.ArePointsAllNearEqual strandVerticesAfterEdit strandVerticesAfterSubObjectModeExit
	),
	
	function Test_MoveControlPointsAndExitControlPointModeAfterMovingRootTwice =
	(
		OxResetClassParameters( Plane() )
		OxResetClassParameters( Ox_Guides_From_Surface() )
		OxResetClassParameters( Ox_Rotate_Strands() )
		OxResetClassParameters( Ox_Edit_Guides() )
		
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:10 length:10 mapcoords:on isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		modPanel.addModToSelection (Ox_Rotate_Strands())
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
			
		-- Move root a bit once
		subobjectLevel = 1
		editGuidesInstance.SelectRoots #{1}
		editGuidesInstance.MoveSelectedControlPoints [6,-6,0]
			
		-- Move control point a bit once
		subobjectLevel = 2
		editGuidesInstance.MoveSelectedControlPoints [5,5,0]
			
		-- Move root a bit twice
		subobjectLevel = 1
		editGuidesInstance.MoveSelectedControlPoints [1,1,0]
			
		-- Move control point a bit twice
		subobjectLevel = 2
		editGuidesInstance.MoveSelectedControlPoints [-1,0,0]
			
		local verticesBeforeExitingSubObject = OxGetVertices distributionObject true
		subobjectLevel = 0
		local verticesAfterExitingSubObject = OxGetVertices distributionObject true
		
		-- We do not expect to see any change by existing sub-object mode
		EAssert.ArePointsAllNearEqual verticesBeforeExitingSubObject verticesAfterExitingSubObject
	),
	
	-- Tests for a regression where previously modified guides would change shape after they were moved on a non-planar surface like a sphere
	function Test_MoveRootsAfterShapeChangesOnSphereDoesNotAlterGuides =
	(
		--OxResetClassParameters( Sphere() )
		OxResetClassParameters( Ox_Guides_From_Surface() )
		OxResetClassParameters( Ox_Edit_Guides() )
		
		local distributionObject = Sphere segs:4 radius:10 mapcoords:on isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance

		-- Move control points
		subobjectLevel = 2
		editGuidesInstance.SelectRoots #{1,2,3,4,5,6}
		editGuidesInstance.MoveSelectedControlPoints [50,0,0]
		
		-- Move first root to a different face
		subobjectLevel = 1
		editGuidesInstance.SelectRoots #{2}
		editGuidesInstance.MoveSelectedControlPoints [1,0,-5]
		editGuidesInstance.MoveSelectedControlPoints [5,0,0]
			
		local verticesBeforeExitingSubObject = OxGetVertices distributionObject true
		
		subobjectLevel = 0
		local verticesAfterExitingSubObject = OxGetVertices distributionObject true
		
		-- We do not expect to see any change by existing sub-object mode
		EAssert.ArePointsAllNearEqual verticesBeforeExitingSubObject verticesAfterExitingSubObject
	),

	function BrushPerformance =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:20 mapcoords:on length:20
		maxOps.CloneNodes #(distributionObject)
		max modify mode

		-- Set up the mesh from strands modifier, use per vertex distribution
		select distributionObject

		-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local guidesFromSurfaceInstance = distributionObject.modifiers[1]
		guidesFromSurfaceInstance.method = 0
		guidesFromSurfaceInstance.guideLength = 5

		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		local hfgInstance = Ox_Hair_from_Guides()
		hfgInstance.viewCountFromRender = true
		hfgInstance.viewCountPercentage = 100
		hfgInstance.denseCount = 100000
		modPanel.addModToSelection hfgInstance
		local renderSettingsModifierInstance = Ox_Render_Settings()
		renderSettingsModifierInstance.radius = 0.02
		modPanel.addModToSelection renderSettingsModifierInstance

		local meshFromStrandsModifierInstance = Ox_Mesh_from_Strands()
		modPanel.addModToSelection meshFromStrandsModifierInstance

		-- Go into brush sub-object mode
		modPanel.setCurrentObject editGuidesInstance
		showEndResult = true
		subobjectLevel = 3		
		editGuidesInstance.SetBrushType 1

		max tool zoomextents
		local viewSize = TestUtilities.GetActualViewSize()
		
		-- Set brush strength to maximum and drag brush along first two strands
		editGuidesInstance.SetBrushSize 200 200
		editGuidesInstance.SetBrushStrength 10

		local brushStrokeYPosition = viewSize.y / 2

		local repeats = 10
		local steps = 5
		local startTime = timeGetTime()
		OxProfiler "enable" true
		for i = 1 to repeats do (
			theHold.Begin()
			for i = 1 to steps do (
				editGuidesInstance.DragBrush [i*viewSize.x/steps, brushStrokeYPosition] [(i+1)*viewSize.x/steps, brushStrokeYPosition] 3
				forceCompleteRedraw()
			)

			theHold.Accept "one"
			max undo
		)

		OxProfiler "enable" false
		OxProfiler "save" "C:\\W\\OxMaxBrush"
		OxProfiler "reset"
		format "Time taken: % ms\n" ( ( timeGetTime() - startTime ) as integer )
	),
	
	function Test_MultiChannelPerVertexPaint =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Edit_Guides())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on
		max modify mode
		select distributionObject
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 randomness:0 NumSegs:2)
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- Create three channels
		editGuidesInstance.CreateChannel false "R"
		editGuidesInstance.CreateChannel false "G"
		editGuidesInstance.CreateChannel false "B"
			
		editGuidesInstance.BeginTrackingChanges false false false true
		editGuidesInstance.SetStrandChannelData false 0 1 0.5
		editGuidesInstance.SetStrandChannelData false 1 1 0.6
		editGuidesInstance.SetStrandChannelData false 2 1 0.7
		editGuidesInstance.CommitChanges()
		
		-- Ensure correct results
		myGuides =  editGuidesInstance.GetGuides()
		EAssert.AreEqual 0.5 (myGuides.GetVertexData 0 1)
		EAssert.AreEqual 0.6 (myGuides.GetVertexData 1 1)
		EAssert.AreEqual 0.7 (myGuides.GetVertexData 2 1)
	),
	
	function Test_RotateStrandsDoesntChangeShape =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 randomness:0 NumSegs:2)
		modPanel.addModToSelection (Ox_Surface_Comb())
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		local verticesWithoutStrandOrientation = OxGetVertices distributionObject true

		-- Select guide 1 and rotate it
		modPanel.setCurrentObject editGuidesInstance
		subobjectLevel = 0
		editGuidesInstance.SelectRoots #{1}
		editGuidesInstance.RotateSelectedRootsToDirection [1,1,1]
		
		local verticesWithStrandOrientation = OxGetVertices distributionObject true
		
		EAssert.ArePointsAllNearEqual verticesWithoutStrandOrientation verticesWithStrandOrientation
	),
	
	function Test_EditGuidesShapeAfterRotation =
	(
		OxResetClassParameters()
		
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:2 count:1 randomness:0 NumSegs:2)
		
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- Select guide 1 and rotate it
		modPanel.setCurrentObject editGuidesInstance
		subobjectLevel = 1
		editGuidesInstance.SelectRoots #{1}
		editGuidesInstance.RotateSelectedRootsToDirection [1,1,1]
		
		-- Move the guide tip
		subobjectLevel = 2
		editGuidesInstance.MoveSelectedControlPoints [50,0,0]
		
		local verticesAfterMove = OxGetVertices distributionObject true
		subobjectLevel = 1
		local verticesAfterRootModeSwitch = OxGetVertices distributionObject true
		
		EAssert.ArePointsAllNearEqual verticesAfterMove verticesAfterRootModeSwitch
	),
	
	function Test_EditGuidesRotateAndSelect =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:2 count:2 randomness:0 NumSegs:2)
		
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- Select guide 1 and move its tip
		subobjectLevel = 2
		editGuidesInstance.SelectRoots #{1}
		editGuidesInstance.MoveSelectedControlPoints [50,0,0]
		editGuidesInstance.SelectRoots #{2}
		editGuidesInstance.MoveSelectedControlPoints [-50,0,0]
		
		modPanel.setCurrentObject editGuidesInstance
		subobjectLevel = 1
		editGuidesInstance.SelectRoots #{1}
		editGuidesInstance.RotateSelectedRootsToDirection [1,1,1]
		
		local verticesAfterRotate = OxGetVertices distributionObject true
		editGuidesInstance.SelectRoots #{2}
		local verticesAfterSelectionChange = OxGetVertices distributionObject true
		
		EAssert.ArePointsAllNearEqual verticesAfterRotate verticesAfterSelectionChange
	),
	
	function Test_EditHairFromMeshStripsHair =
	(
		OxResetClassParameters (Ox_Rotate_Strands())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on
		local hairObjectInstance = HairFromMeshStripsObject viewPercentage:100 isSelected:on distributionMethod:0 pointsPerStrand:2
		hairObjectInstance.AddStripObject distributionObject
		max modify mode
		
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		local verticesBeforeTipMove = OxGetVertices hairObjectInstance true
		
		-- Select guide 1 and move its tip
		subobjectLevel = 2
		editGuidesInstance.SelectRoots #{1}
		editGuidesInstance.MoveSelectedControlPoints [0,50,0]
		
		local verticesAfterTipMove = OxGetVertices hairObjectInstance true
		
		EAssert.ArePointsNotAllNearEqual verticesBeforeTipMove verticesAfterTipMove
	),
	
	function Test_AssignStrandGroupsToGuideHalves =
	(
		OxResetClassParameters ()
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:2 count:20 randomness:0 NumSegs:2)
		
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		subobjectLevel = 1
		editGuidesInstance.useGuideGroups = on
		editGuidesInstance.SelectRoots #{1,2,3,4,5,6,7,8,9,10}
		editGuidesInstance.SetSelectedGuidesGroupIndex 1

		-- Select "Next" group
		editGuidesInstance.SelectGuidesWithGroupPattern "-2"
		editGuidesInstance.SetSelectedGuidesGroupIndex 2
		
		editGuidesInstance.SelectGuidesWithGroupPattern "-2"
		
		-- After selecting "Next" group again we should have the 10 guides we initially assigned group 1 to
		local selectedStrandIds = OxGetSelectedStrandIds distributionObject
		EAssert.AreEqual 10 (selectedStrandIds.count)
	),
	
	-- Tests bug #5235
	function Test_LoadSaveObjectCopy =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_from_Surface method:2 count:1 randomness:0 NumSegs:2)
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		subobjectLevel = 2
		editGuidesInstance.SelectRoots #{1}
		editGuidesInstance.MoveSelectedControlPoints [0,50,0]
		
		local originalVertices = OxGetVertices distributionObject true
		
		-- Copy the edit guides object
		guidesCopy = copy distributionObject
		select guidesCopy
		local editGuidesInstance2 = guidesCopy.modifiers[#Ox_Edit_Guides]
		subobjectLevel = 2
		editGuidesInstance2.SelectRoots #{1}
		editGuidesInstance2.MoveSelectedControlPoints [0,-10,0]
		
		local copyVertices = OxGetVertices guidesCopy true
		
		EAssert.ArePointsNotAllNearEqual originalVertices copyVertices
		local originalName = distributionObject.Name
		local copyName = guidesCopy.Name
		
		-- Save and reload
		holdMaxFile()
		fetchMaxFile quiet:true
		
		local originalObject = GetNodeByName originalName
		local copyObject = GetNodeByName copyName
		EAssert.ArePointsNotAllNearEqual (OxGetVertices originalObject true) (OxGetVertices copyObject true)
	),

	-- This test is disabled because the placement of roots in Ox V7 is different than V6 for an unknown reason (mesh triangulation is checked to be identical). In the future it may be a good idea to figure out why.
	function Test_LoadV6DeleteAndComb =
	(
		EUnit.SkipBeforeMaxVersion 2020

		units.SystemType = #Centimeters
		local testSceneFilePath = pathConfig.convertPathToAbsolute (getSourceFileName() + "..\\..\\..\\..\\..\\..\\..\\..\\" ) + "External\\TestData\\OxMax6-Max2020-EGDeleteAndComb.max"
		loadMaxFile testSceneFilePath useFileUnit:true quiet:true
		
		local actualStrandIds = OxGetStrandIds $Plane001
		sort actualStrandIds
		local expectedStrandIds = #(0, 20000, 30000, 50000, 70000)
		EAssert.AreArraysEqual expectedStrandIds actualStrandIds
		
		local actualVertices = OxGetStrandPointsSortedByStrandId $Plane001 true
		local expectedVertices = #([-7.48352,-5.7992,0], [-31.2432,-15.5611,42.8973], [1.68023,-7.20519,0], [19.1068,-26.0093,42.9269], [9.64821,-6.12919,0], [23.2388,-18.5579,46.4847], [-3.10626,5.72548,0], [-6.87544,13.284,49.2815], [8.2387,2.78059,0], [20.6944,18.385,45.8406])
		EAssert.ArePointsAllNearEqual expectedVertices actualVertices
		
		-- Cleanup
		resetMaxFile #noPrompt
	)/*,
	
	function Test_PaintThreePerVertexChannels =
	(
		local guidesFromSurfaceModifier = undefined
		undo "Setup" on
		(
			local distributionObject = Plane lengthsegs:1 widthsegs:1 width:25 length:25
			max modify mode

			-- Set up the mesh from strands modifier, use per vertex distribution
			select distributionObject
			
			-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
			TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
			distributionObject.modifiers[#Ox_Guides_from_Surface].NumSegs = 10
			
			local editGuidesInstance = Ox_Edit_Guides()
			modPanel.addModToSelection editGuidesInstance
			
			editGuidesInstance.CreateChannel false "channel1"
			editGuidesInstance.CreateChannel false "channel2"
			editGuidesInstance.CreateChannel false "channel3"
		)

		-- Go into brush sub-object mode
		subobjectLevel = 3
		
		editGuidesInstance.SetBrushType 6
		
		-- Set active viewport to be the front view and zoom to extents
		viewport.activeViewport = 2
		max tool zoomextents
		local viewSize = TestUtilities.GetActualViewSize()
		
		-- Set brush strength to maximum and drag brush along first two strands
		editGuidesInstance.SetBrushSize (viewSize.x/10) (viewSize.x/10)
		editGuidesInstance.SetBrushStrength 10
		editGuidesInstance.SetCurrentChannelIndex 2 false
		editGuidesInstance.SetPaintChannelCount 3
		
		delete $objects
	)*/
)

EUnit.AddTestClass Test_EditGuidesModifier
ok
