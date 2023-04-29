struct Test_SymmetryModifier
(
	function TestCleanup =
	(
		delete $objects
	),
	
	-- Tests that strand symmetry doubles the number of incoming strands
	function Test_CopyDoubleStrands =
	(
		-- Create a plane with 2x2 segments so we have 9 vertices all together
		local distributionObject = Plane lengthsegs:2 widthsegs:2 isSelected:on

		max modify mode

		-- Add strand symmetry
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		modPanel.addModToSelection guidesFromSurfaceModifier

		strandSymmetryModifier = Ox_Strand_Symmetry()
		strandSymmetryModifier.useIgnoreDistance = off
		modPanel.addModToSelection strandSymmetryModifier
			
		-- Need to collapse the stack to get access to hair
		convertTo distributionObject GuidesObject
		myGuides = ( distributionObject.GetGuides() ).AsHair()

		-- The strand count must be double the initial one
		EAssert.AreEqual 18 ( myGuides.NumRoots() )
	),
	
	-- Test symmetry for X axis
	function Test_SymmetryAxis_X =
	(
		OxResetClassParameters ( Plane() )
		OxResetClassParameters ( Ox_Guides_from_Surface() )
		OxResetClassParameters ( Ox_Edit_Guides() )
		OxResetClassParameters ( Ox_Strand_Symmetry() )
		
		-- Create a plane with 1x1 segments
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		local distributionObjectName = distributionObject.name
		max modify mode
		select distributionObject

		-- Add a guides from mesh modifier with per-vertex guide distribution, which should make the total number of guides 4
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 2
		modPanel.addModToSelection guidesFromSurfaceModifier

		--Edit Guides Modifier
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance

		--Root Mode
		subObjectLevel = 1
		editGuidesInstance.SelectRoots #{1,2}

		-- Edit: Delete Objects
		actionMan.executeAction 0 "40020" 

		-- Modify the first strand's shape
		strand_shape = editGuidesInstance.GetStrandShape 0
		editGuidesInstance.SetStrandShape 0 #( strand_shape[1], strand_shape[2] + [10,10,0] )
		strand_shape = editGuidesInstance.GetStrandShape 1
		editGuidesInstance.SetStrandShape 1 #( strand_shape[1], strand_shape[2] + [-15,-15,0] )

		--Object Mode
		subobjectLevel = 0
		
		-- Symmetry Modifier with X axis
		local strandSymmetryInstance = Ox_Strand_Symmetry ()
		strandSymmetryInstance.useIgnoreDistance = off
		strandSymmetryInstance.mirrorAxis = 0 --X
		strandSymmetryInstance.groundMirroredStrands = off
		modPanel.addModToSelection strandSymmetryInstance

		-- Bake
		local guidesCopy = instance distributionObject
		convertTo guidesCopy HairObject
		myHair = guidesCopy.GetHair()
		EAssert.AreEqual 4 ( myHair.NumRoots() )

		-- Check direction
		EAssert.AreEqual ( ( myHair.GetPointT 1 1 ) - ( myHair.GetPointT 1 0 ) ) ( ( ( myHair.GetPointT 3 1 ) - ( myHair.GetPointT 3 0 ) ) * [-1, 1, 1] )
		EAssert.AreEqual ( ( myHair.GetPointT 0 1 ) - ( myHair.GetPointT 0 0 ) ) ( ( ( myHair.GetPointT 2 1 ) - ( myHair.GetPointT 2 0 ) ) * [-1, 1, 1] )
	),
	
	-- Test symmetry for Y axis
	function Test_SymmetryAxis_Y =
	(
		OxResetClassParameters ( Plane() )
		OxResetClassParameters ( Ox_Guides_from_Surface() )
		OxResetClassParameters ( Ox_Edit_Guides() )
		OxResetClassParameters ( Ox_Strand_Symmetry() )
		
		-- Create a plane with 1x1 segments
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		distributionObjectName = distributionObject.name
		max modify mode
		select distributionObject

		-- Add a guides from mesh modifier with per-vertex guide distribution, which should make the total number of guides 4
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 2
		modPanel.addModToSelection guidesFromSurfaceModifier

		--Edit Guides Modifier
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance

		--Root Mode
		subObjectLevel = 1
		editGuidesInstance.SelectRoots #{1,3}

		-- Edit: Delete Objects
		actionMan.executeAction 0 "40020" 

		-- Modify the first strand's shape
		local strand_shape = editGuidesInstance.GetStrandShape 0
		editGuidesInstance.SetStrandShape 0 #( strand_shape[1], strand_shape[2] + [0,10,0] )
		local strand_shape = editGuidesInstance.GetStrandShape 1
		editGuidesInstance.SetStrandShape 1 #( strand_shape[1], strand_shape[2] + [0,-10,0] )

		--Object Mode
		subobjectLevel = 0
		
		-- Symmetry Modifier with X axis
		local strandSymmetryInstance = Ox_Strand_Symmetry ()
		strandSymmetryInstance.useIgnoreDistance = off
		strandSymmetryInstance.mirrorAxis = 1 --Y
		strandSymmetryInstance.groundMirroredStrands = off
		modPanel.addModToSelection strandSymmetryInstance

		-- Bake
		local guidesCopy = instance distributionObject
		convertTo guidesCopy HairObject
		local myHair = guidesCopy.GetHair()
		EAssert.AreEqual 4 ( myHair.NumRoots() )

		-- Check direction
		EAssert.ArePointsNearEqual ( ( myHair.GetPointT 1 1 ) - ( myHair.GetPointT 1 0 ) ) ( ( ( myHair.GetPointT 3 1 ) - ( myHair.GetPointT 3 0 ) ) * [1, -1, 1] )
		EAssert.ArePointsNearEqual ( ( myHair.GetPointT 0 1 ) - ( myHair.GetPointT 0 0 ) ) ( ( ( myHair.GetPointT 2 1 ) - ( myHair.GetPointT 2 0 ) ) * [1, -1, 1] )
	),
	
	-- Test symmetry for Z axis
	function Test_SymmetryAxis_Z =
	(
		OxResetClassParameters ( Plane() )
		OxResetClassParameters ( Ox_Guides_from_Surface() )
		OxResetClassParameters ( Ox_Edit_Guides() )
		OxResetClassParameters ( Ox_Strand_Symmetry() )
		
		-- Create a plane with 1x1 segments
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		distributionObjectName = distributionObject.name
		max modify mode
		select distributionObject

		-- Add a guides from mesh modifier with per-vertex guide distribution, which should make the total number of guides 4
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 2
		modPanel.addModToSelection guidesFromSurfaceModifier

		--Edit Guides Modifier
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance

		--Root Mode
		subObjectLevel = 1
		editGuidesInstance.SelectRoots #{1,3}

		-- Edit: Delete Objects
		actionMan.executeAction 0 "40020" 

		-- Modify the first strand's shape
		strand_shape = editGuidesInstance.GetStrandShape 0
		editGuidesInstance.SetStrandShape 0 #( strand_shape[1], strand_shape[2] + [0,0,5] )
		strand_shape = editGuidesInstance.GetStrandShape 1
		editGuidesInstance.SetStrandShape 1 #( strand_shape[1], strand_shape[2] + [0,0,-5] )

		--Object Mode
		subobjectLevel = 0
		
		-- Symmetry Modifier with X axis
		local strandSymmetryInstance = Ox_Strand_Symmetry ()
		strandSymmetryInstance.useIgnoreDistance = off
		strandSymmetryInstance.mirrorAxis = 2 --Z
		strandSymmetryInstance.groundMirroredStrands = off
		modPanel.addModToSelection strandSymmetryInstance

		-- Bake
		guidesCopy = instance distributionObject
		convertTo guidesCopy HairObject
		myHair = guidesCopy.GetHair()
		EAssert.AreEqual 4 ( myHair.NumRoots() )

		-- Check direction
		EAssert.ArePointsNearEqual ( ( myHair.GetPointT 1 1 ) - ( myHair.GetPointT 1 0 ) ) ( ( ( myHair.GetPointT 3 1 ) - ( myHair.GetPointT 3 0 ) ) * [1, 1, -1] )
		EAssert.ArePointsNearEqual ( ( myHair.GetPointT 0 1 ) - ( myHair.GetPointT 0 0 ) ) ( ( ( myHair.GetPointT 2 1 ) - ( myHair.GetPointT 2 0 ) ) * [1, 1, -1] )
	),
	
	-- Test Groundifng
	function Test_Grounding = 
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
		guidesFromSurfaceModifier.numSegs = 2
		modPanel.addModToSelection guidesFromSurfaceModifier

		--Edit Guides Modifier
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance

		--Root Mode
		subObjectLevel = 1
		editGuidesInstance.SelectRootById 0 true
		editGuidesInstance.SelectRootById 2 true

		-- Edit: Delete Objects
		actionMan.executeAction 0 "40020" 

		-- Modify the first strand's shape
		strand_shape = editGuidesInstance.GetStrandShape 0
		editGuidesInstance.SetStrandShape 0 #( strand_shape[1], strand_shape[2] + [20,20,0] )
		strand_shape = editGuidesInstance.GetStrandShape 1
		editGuidesInstance.SetStrandShape 1 #( strand_shape[1], strand_shape[2] + [-20,-20,0] )

		--Object Mode
		subobjectLevel = 0

		local strandSymmetryInstance = Ox_Strand_Symmetry ()
		strandSymmetryInstance.mirrorAxis = 0 --X
		strandSymmetryInstance.useIgnoreDistance = off
		strandSymmetryInstance.groundMirroredStrands = on
		modPanel.addModToSelection strandSymmetryInstance

		local hairFromGuidesInstance = Ox_Hair_from_Guides ()
		hairFromGuidesInstance.method = 4 --4
		hairFromGuidesInstance.count = 4
		modPanel.addModToSelection hairFromGuidesInstance

		EAssert.AreEqual 4 ( OxGetStrandCount distributionObject )

		-- Check direction
		local pt1 = OxGetStrandPoint distributionObject 0 1 true true
		local pt2 = OxGetStrandPoint distributionObject 0 0 true true
		local pt3 = OxGetStrandPoint distributionObject 1 1 true true
		local pt4 = OxGetStrandPoint distributionObject 1 0 true true
		EAssert.ArePointsNearEqual ( pt1 - pt2 ) ( ( pt3 - pt4 ) * [-1, 1, 1] )
		
		pt1 = OxGetStrandPoint distributionObject 2 1 true true
		pt2 = OxGetStrandPoint distributionObject 2 0 true true
		pt3 = OxGetStrandPoint distributionObject 3 1 true true
		pt4 = OxGetStrandPoint distributionObject 3 0 true true
		EAssert.ArePointsNearEqual ( pt1 - pt2 ) ( ( pt3 - pt4 ) * [-1, 1, 1] )
	),
	
	-- Test Ignore Distance for Guides
	function Test_GuidesIgnoreDistance =
	(
		-- Create a plane with 1x1 segments
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:25 length:25
		local distributionObjectName = distributionObject.name
		max modify mode
		select distributionObject

		-- Add a guides from mesh modifier with per-vertex guide distribution, which should make the total number of guides 4
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface ()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 2
		modPanel.addModToSelection guidesFromSurfaceModifier

		-- Strand Symmetry Modifier
		local strandSymmetryInstance = Ox_Strand_Symmetry ()
		strandSymmetryInstance.mirrorAxis = 0
		strandSymmetryInstance.groundMirroredStrands = off
		strandSymmetryInstance.useIgnoreDistance = on
		strandSymmetryInstance.ignoreDistance = distributionObject.width
		strandSymmetryInstance.planeTM = (matrix3 [0,1,0] [0,0,1] [1,0,0] [distributionObject.width,0,0])
		modPanel.addModToSelection strandSymmetryInstance
		
		-- Bake as Guides
		guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		myHair = ( guidesCopy.GetGuides() ).AsHair()
		EAssert.AreEqual 6 ( myHair.NumRoots() )
	),
	
	function Test_TextureCoordinatesArePreserved =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (HairFromMeshStripsObject())
		OxResetClassParameters (Ox_Strand_Symmetry())
			
		local stripObject1 = Plane lengthsegs:1 widthsegs:1 mapcoords:on
		
		local hairObjectInstance = HairFromMeshStripsObject hairsPerStrip:1 viewPercentage:100 pointsPerStrand:3 inheritUvsFromStrips:on
		hairObjectInstance.AddStripObject stripObject1
		
		local myHair = hairObjectInstance.GetHair()
		local initialUVWCoordinate = myHair.GetPointUVW 0 1
		
		max modify mode
		select hairObjectInstance
		local symmetryModifierInstance = Ox_Strand_Symmetry useIgnoreDistance:off
		modPanel.addModToSelection symmetryModifierInstance
		
		local hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		myHair = hairCopy.GetHair()
		EAssert.AreEqual initialUVWCoordinate (myHair.GetPointUVW 0 1)
		EAssert.AreEqual initialUVWCoordinate (myHair.GetPointUVW 1 1)
		delete hairCopy
	),
	
	-- Using the ground mirrored strands option shouldn't crash Max if there is no distribution mesh
	function Test_SymmetryGroundingWithoutDistributionMesh =
	(
		local distributionObject = circle()
		
		max modify mode
		select distributionObject
		
		local guidesFromShapeModifierInstance = Ox_Guides_From_Shape()
		modPanel.addModToSelection guidesFromShapeModifierInstance
		
		local symmetryModifierInstance = Ox_Strand_Symmetry()
		modPanel.addModToSelection symmetryModifierInstance
		
		symmetryModifierInstance.groundMirroredStrands = true
		
		-- Shouldn't crash by this point
	),
	
	function Test_StrandRotationsCorrectlyMirrored =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Edit_Guides())
		OxResetClassParameters (Ox_Strand_Symmetry())
				
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on

		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		-- Add edit guides, delete two left guides and move the right guides a bit so they have a defined shape relative to their root rotation
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		
		-- Go into sub-object mode
		subobjectLevel = 1
		editGuidesInstance.SelectRoots #{1,2}
		
		-- Delete selected roots
		actionMan.executeAction 0 "40020"
		
		subobjectLevel = 2
		editGuidesInstance.SelectRoots #{1,2}
		
		-- Move the selected control points
		editGuidesInstance.MoveSelectedControlPoints [10,0,0]
		
		-- Grab the object-space points before applying mirror
		local verticesBeforeMirror = OxGetVertices distributionObject true

		local strandSymmetryModifier = Ox_Strand_Symmetry mirrorAxis:0
		modPanel.addModToSelection strandSymmetryModifier
		
		local verticesAfterMirror = OxGetVertices distributionObject true
		
		-- The position of the first four points (first two strands should be identical)
		for i = 1 to 4 do
			EAssert.ArePointsNearEqual verticesBeforeMirror[i] verticesAfterMirror[i]
	),
	
	-- Tests for a bug regression where symmetry causes hairs to lose their local transformation
	function Test_SymmetryOnHair =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Hair_from_Guides())
		OxResetClassParameters (Ox_Strand_Symmetry())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:25 length:25 isSelected:on
		max modify mode

		modPanel.addModToSelection (Ox_Guides_from_Surface ())
		modPanel.addModToSelection (Ox_Hair_from_Guides ())
			
		local boundingBoxBeforeSymmetry = TestUtilities.GetPointsBoundingBox (OxGetVertices distributionObject true)
		modPanel.addModToSelection (Ox_Strand_Symmetry ())
		local boundingBoxAfterSymmetry = TestUtilities.GetPointsBoundingBox (OxGetVertices distributionObject true)
			
		-- If symmetry was applied correctly the bounding box shouldn't change by much
		EAssert.ArePointsNearEqual boundingBoxBeforeSymmetry[1] boundingBoxAfterSymmetry[1] epsilon:1
		EAssert.ArePointsNearEqual boundingBoxBeforeSymmetry[2] boundingBoxAfterSymmetry[2] epsilon:1
	)
)

EUnit.AddTestClass Test_SymmetryModifier
ok
