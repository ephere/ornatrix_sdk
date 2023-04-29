struct Test_HairFromMeshStripsObject
(
	function TestCleanup =
	(
		delete $objects
	),
	
	-- Tests that hair from mesh strips can be used with another Ornatrix modifier and the hair goes through the pipeline
	function Test_HairFromMeshStripsWithModifier =
	(
		-- Create a plane with 2x2 segments so we have 9 vertices all together
		local distributionObject = Plane lengthsegs:1 widthsegs:1

		local hairObjectInstance = HairFromMeshStripsObject()
		hairObjectInstance.viewPercentage = 100
		hairObjectInstance.AddStripObject distributionObject
			
		-- Add frizz modifier on top and ensure that all the hairs make it out frizzed
		select hairObjectInstance
		max modify mode
		modPanel.addModToSelection (Ox_Strand_Frizz())

		hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		myHair = hairCopy.GetHair()
		EAssert.AreNotEqual 0 (myHair.NumRoots())
	),
	
	function Test_AddRemoveStrips =
	(
		local stripObject1 = Plane lengthsegs:1 widthsegs:1
		local stripObject2 = Plane lengthsegs:1 widthsegs:3
		local stripObject3 = Plane lengthsegs:5 widthsegs:3

		local hairObjectInstance = HairFromMeshStripsObject hairsPerStrip:5 viewPercentage:100 pointsPerStrand:10 distributionMethod:1
		
		-- No hairs by default
		local hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		myHair = hairCopy.GetHair()
		EAssert.AreEqual 0 (myHair.NumRoots())
		delete hairCopy
			
		hairObjectInstance.AddStripObject stripObject1
			
		-- Hairs on first strip
		hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		myHair = hairCopy.GetHair()
		EAssert.AreEqual 5 (myHair.NumRoots())
		EAssert.AreEqual 50 (myHair.NumVerts())
		delete hairCopy
			
		hairObjectInstance.AddStripObject stripObject2
		hairObjectInstance.AddStripObject stripObject3
			
		-- More hairs as strips are added
		hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		myHair = hairCopy.GetHair()
		EAssert.AreEqual 15 (myHair.NumRoots())
		EAssert.AreEqual 150 (myHair.NumVerts())
		delete hairCopy
			
		hairObjectInstance.RemoveStripObject 1
			
		-- Less hairs as strips are removed
		hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		myHair = hairCopy.GetHair()
		EAssert.AreEqual 10 (myHair.NumRoots())
		EAssert.AreEqual 100 (myHair.NumVerts())
		delete hairCopy
	),
	
	function Test_ChangeHairsPerStripCount =
	(
		stripObject1 = Plane lengthsegs:1 widthsegs:1

		hairObjectInstance = HairFromMeshStripsObject distributionMethod:1
		hairObjectInstance.hairsPerStrip = 5
		hairObjectInstance.viewPercentage = 100
		hairObjectInstance.AddStripObject stripObject1
		
		hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		myHair = hairCopy.GetHair()
		EAssert.AreEqual 5 (myHair.NumRoots())
		delete hairCopy
			
		hairObjectInstance.hairsPerStrip = 7
			
		hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		myHair = hairCopy.GetHair()
		EAssert.AreEqual 7 (myHair.NumRoots())
		delete hairCopy
	),
	
	function Test_ViewPercentage =
	(
		local stripObject1 = Plane lengthsegs:1 widthsegs:1

		local hairObjectInstance = HairFromMeshStripsObject hairsPerStrip:500 viewPercentage:10 distributionMethod:1
		hairObjectInstance.AddStripObject stripObject1
		
		hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		myHair = hairCopy.GetHair()
		EAssert.AreEqual 50 (myHair.NumRoots())
		delete hairCopy
			
		hairObjectInstance.viewPercentage = 1
			
		hairCopy = instance hairObjectInstance
		convertTo hairCopy HairObject
		myHair = hairCopy.GetHair()
		EAssert.AreEqual 5 (myHair.NumRoots())
		delete hairCopy
	),
	
	-- Tests for a bug where mesh from strands guide export generated from mesh from strips object produces an error
	function Test_MeshFromStrandsWithGuideExport =
	(
		local stripObject1 = Plane lengthsegs:1 widthsegs:1
		local hairObjectInstance = HairFromMeshStripsObject()
		hairObjectInstance.hairsPerStrip = 1
		hairObjectInstance.AddStripObject stripObject1
		
		select hairObjectInstance
		max modify mode
		local meshFromStrandsModifierInstance = Ox_Mesh_From_Strands() 
		modPanel.addModToSelection meshFromStrandsModifierInstance
		
		-- This line should not cause an error
		meshFromStrandsModifierInstance.guideMeshExp = on
	),
	
	function Test_StripObjectWithMultipleMeshElements =
	(
		-- Create two planes/strips and attach one to another
		local mainStripObject = Plane lengthsegs:1 widthsegs:1
		local extraStripObject = Plane lengthsegs:1 widthsegs:1
		
		select mainStripObject
		max modify mode
		local editPolyModifierInstance = Edit_Poly()
		modPanel.addModToSelection editPolyModifierInstance
		undo on
		(
		editPolyModifierInstance.Attach extraStripObject editPolyNode:mainStripObject
		)
		
		-- Create a hair from strips object and select the strips we attached into it
		local hairObjectInstance = HairFromMeshStripsObject hairsPerStrip:1 viewPercentage:100 distributionMethod:1
		hairObjectInstance.AddStripObject mainStripObject
		
		-- The strip object should generate 2 strips: one for each element
		myHair = hairObjectInstance.GetHair()
		EAssert.AreEqual 2 (myHair.NumRoots())
	),
	
	function Test_AddAndRemoveStripMeshTurbosmooth =
	(
		OxResetClassParameters( TurboSmooth() )
		OxResetClassParameters( HairFromMeshStripsObject() )
		OxResetClassParameters( Plane() )
		
		local stripObject = Plane lengthsegs:1 widthsegs:1
		
		select stripObject
		max modify mode
		local turboSmoothInstance = TurboSmooth()
		modPanel.addModToSelection turboSmoothInstance
		
		local hairObjectInstance = HairFromMeshStripsObject hairsPerStrip:1 viewPercentage:100 distributionMethod:1
		hairObjectInstance.AddStripObject stripObject
		
		-- Mark the current hair's tip
		local myHair = hairObjectInstance.GetHair()
		local tipPositon = myHair.GetPointT 0 1
		
		-- Delete turbosmooth modifier and check that vartices are in the same place
		select stripObject
		deleteModifier stripObject 1
		
		-- Check the tip position again, it should be about the same
		myHair = hairObjectInstance.GetHair()
		EAssert.ArePointsNearEqual tipPositon (myHair.GetPointT 0 1)
		
		-- Undo the turbosmooth deletion, it should not cause a crash
		-- NOTE: This will undo whole action so it isn't good
		--actionMan.executeAction 0 "40015"
		select stripObject
		modPanel.addModToSelection turboSmoothInstance
		
		-- Turn isoline display on in turbosmooth. This should not make hair generation hang.
		turboSmoothInstance.isolineDisplay = on
		stripObject.widthsegs = 2
		stripObject.lengthsegs = 2
	),
	
	function Test_InheritMeshStripTextureCoordinates =
	(
		local stripObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on
		
		select stripObject
		max modify mode
		
		local hairObjectInstance = HairFromMeshStripsObject hairsPerStrip:1 viewPercentage:100 distributionMethod:1
		hairObjectInstance.AddStripObject stripObject
		
		-- Disable texture coordinate inheritance at first
		hairObjectInstance.inheritUvsFromStrips = off

		-- Hair texture coordiantes should be auto-generated and produce zeroes as UVW since we don't have a distribution mesh
		local myHair = hairObjectInstance.GetHair()
		EAssert.AreEqual [0,0,0] ( myHair.GetPointUVW 0 0 )
		
		-- Enable texture coordinate inheritance and make sure texture coordinates are inherited
		hairObjectInstance.inheritUvsFromStrips = on
		myHair = hairObjectInstance.GetHair()
		EAssert.AreEqual [0,0.5,0] ( myHair.GetPointUVW 0 0 )
	),
	
	-- Checks for a bug which made Max crash when point count is changed after a modifier is applied when Nitrous hair is used
	function Test_ChangePointCountAfterApplyingAModifier =
	(
		local stripObject = Plane lengthsegs:1 widthsegs:1		
		local hairObjectInstance = HairFromMeshStripsObject distributionMethod:1 isSelected:on hairsPerStrip:10 viewPercentage:100 pointsPerStrand:10
		max modify mode
		hairObjectInstance.AddStripObject stripObject
		completeRedraw()
		
		select hairObjectInstance
		local renderSettingsModifierInstance = Ox_Render_Settings()
		modPanel.addModToSelection renderSettingsModifierInstance
		modPanel.setCurrentObject hairObjectInstance
		hairObjectInstance.pointsPerStrand = 60
		completeRedraw()
	),
	
	-- Makes sure that hair can be added to editable poly modifier without crashing the Nitrous viewport
	function Test_AddStripsToEditablePolyAndSelect =
	(
		local stripObject = Plane lengthsegs:1 widthsegs:1
		select stripObject
		macros.run "Modifier Stack" "Convert_to_Poly"
		
		local hairObjectInstance = HairFromMeshStripsObject()
		select hairObjectInstance
		max modify mode
		hairObjectInstance.AddStripObject stripObject
		completeRedraw()
		
		select hairObjectInstance
		local renderSettingsModifierInstance = Ox_Render_Settings()
		modPanel.addModToSelection renderSettingsModifierInstance
		completeRedraw()
		
		-- This shouldn't crash Max
		select stripObject
		completeRedraw()
	),
	
	function Test_HairFromStripsFromMeshFromStrands =
	(
		OxResetClassParameters (Ox_Guides_From_Surface())
		OxResetClassParameters (Ox_Hair_from_Guides())
		OxResetClassParameters (Ox_Mesh_From_Strands())
		OxResetClassParameters (HairFromMeshStripsObject())
			
		local stripObject = Plane lengthsegs:1 widthsegs:1
		select stripObject
		max modify mode
		local guidesFromSurfaceModifierInstance = Ox_Guides_From_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifierInstance
		guidesFromSurfaceModifierInstance.method = 4
		modPanel.addModToSelection guidesFromSurfaceModifierInstance
		
		local hairFromGuidesInstance = Ox_Hair_from_Guides ()
		hairFromGuidesInstance.method = 4 --4
		hairFromGuidesInstance.count = 4
		modPanel.addModToSelection hairFromGuidesInstance
		
		local meshFromStrandsInstance = Ox_Mesh_From_Strands()
		meshFromStrandsInstance.type = 1
		modPanel.addModToSelection meshFromStrandsInstance
		
		local hairObjectInstance = HairFromMeshStripsObject()
		select hairObjectInstance
		max modify mode
		hairObjectInstance.hairsPerStrip = 2
		hairObjectInstance.viewPercentage = 100
		hairObjectInstance.AddStripObject stripObject
		
		local myHair = hairObjectInstance.GetHair()
		EAssert.AreEqual 8 ( myHair.NumRoots() )
	),
	
	function Test_HairsDontFlipOnStripEdgeExtrusion =
	(
		local stripObject = Plane lengthsegs:1 widthsegs:1
		stripObject.width =10
		stripObject.length =20
		
		select stripObject
		macros.run "Modifier Stack" "Convert_to_Poly"
		
		max modify mode
		
		local editPolyModifier = Edit_Poly()
		modPanel.addModToSelection editPolyModifier
		
		local hairObjectInstance = HairFromMeshStripsObject distributionMethod:1 hairsPerStrip:1 pointsPerStrand:2 viewPercentage:100
		hairObjectInstance.AddStripObject stripObject
		
		-- Note the poisition of the strand tip
		local myHair = hairObjectInstance.GetHair()
		local originalTipPosition = myHair.GetPointT 0 0
		
		-- Extrude the edge
		subobjectLevel = 2
		editPolyModifier.SetSelection #Edge #{}
		editPolyModifier.Select #Edge #{4}
		editPolyModifier.ButtonOp #CloneEdge
		editPolyModifier.SetOperation #Transform
		editPolyModifier.MoveSelection  [0,0,4.24751] axis:(transMatrix [0.0000,-11.3953,0.0000])
		undo on
		(
		editPolyModifier.Commit()
		)
		
		-- Make sure that the base of the generated strand hasn't flipped onto another side
		myHair = hairObjectInstance.GetHair()
		EAssert.ArePointsNearEqual originalTipPosition ( myHair.GetPointT 0 0 )
	),
	
	function Test_RotateOrientation =
	(
		local stripObject = Plane lengthsegs:1 widthsegs:1
		stripObject.width =10
		stripObject.length =20
		
		local hairObjectInstance = HairFromMeshStripsObject hairsPerStrip:1 distributionMethod:1 pointsPerStrand:2 viewPercentage:100
		hairObjectInstance.AddStripObject stripObject
		
		local hairInstance = hairObjectInstance.GetHair()
		local initialHairTangent = (hairInstance.GetPointT 0 0 ) - (hairInstance.GetPointT 0 1 )
		
		hairObjectInstance.RotateStripOrientations 0
		
		hairInstance = hairObjectInstance.GetHair()
		local rotatedHairTangent = (hairInstance.GetPointT 0 0 ) - (hairInstance.GetPointT 0 1 )
		
		-- After rotation the hair should be perpendicular to the initial one
		EAssert.AreNearEqual 0 (dot initialHairTangent rotatedHairTangent)
	),
	
	function Test_RotateOrientationWithUVFlow =
	(
		local stripObject = Plane lengthsegs:1 widthsegs:1 width:10 length:20
		local hairObjectInstance = HairFromMeshStripsObject hairsPerStrip:1 distributionMethod:1 pointsPerStrand:2 viewPercentage:100
		hairObjectInstance.AddStripObject stripObject
		
		local hairInstance = hairObjectInstance.GetHair()
		local initialHairTangent = (hairInstance.GetPointT 0 0 ) - (hairInstance.GetPointT 0 1 )
		 
		-- Use UV Flow Down method
		hairObjectInstance.RotateStripOrientations 8
		
		local vertices = OxGetVertices hairObjectInstance true
		EAssert.AreNearEqual 0 (dot [1, 0, 0] (vertices[2] - vertices[1]))
			
		-- Use UV Flow Left method
		hairObjectInstance.RotateStripOrientations 10
		vertices = OxGetVertices hairObjectInstance true
		EAssert.AreNearEqual 0 (dot [0, 1, 0] (vertices[2] - vertices[1]))
	),
	
	-- Makes sure that the strip orientation is preserved if we change it and then move the strip object onto a new layer
	function Test_MoveStripMeshToNewLayerWithRotatedStrip =
	(
		local stripObject = Plane lengthsegs:1 widthsegs:1 transform:(matrix3 [0,-1,0] [0,0,1] [-1,0,0] [0,0,0])
		stripObject.width =20
		stripObject.length =10
		
		local hairObjectInstance = HairFromMeshStripsObject distributionMethod:1
		hairObjectInstance.hairsPerStrip = 1
		hairObjectInstance.pointsPerStrand = 2
		hairObjectInstance.viewPercentage = 100
		hairObjectInstance.AddStripObject stripObject
		hairObjectInstance.RotateStripOrientations 0
		
		local hairInstance = hairObjectInstance.GetHair()
		local initialHairTangent = (hairInstance.GetPointT 0 0 ) - (hairInstance.GetPointT 0 1 )
		
		-- Move the strip object onto another layer
		local layer = LayerManager.newLayer()
		layer.addNode stripObject
		local hairTangentAfterLayerChange = (hairInstance.GetPointT 0 0 ) - (hairInstance.GetPointT 0 1 )
		
		local tangentLength = length initialHairTangent
		EAssert.AreNearEqual (tangentLength*tangentLength) (dot initialHairTangent hairTangentAfterLayerChange)
		
		-- Cleanup
		delete hairObjectInstance
		delete stripObject
		LayerManager.deleteLayerByName layer.Name
	),
	
	function Test_MoveMultipleStripMeshToNewLayerWithRotatedStripAfterSceneReload =
	(
		local stripObject1 = Plane lengthsegs:1 widthsegs:1 transform:(matrix3 [0,-1,0] [0,0,1] [-1,0,0] [0,0,0]) width:20 length:10
		local stripObject2 = Plane lengthsegs:1 widthsegs:1 transform:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [0,0,0]) width:20 length:10
		local stripObject3 = Plane lengthsegs:1 widthsegs:1 transform:(matrix3 [1,0,0] [0,1,0] [1,0,0] [0,0,0]) width:20 length:10
		
		select stripObject1
		macros.run "Modifier Stack" "Convert_to_Poly"
		polyop.attach stripObject1 stripObject2
		polyop.attach stripObject1 stripObject3
		
		local hairObjectInstance = HairFromMeshStripsObject distributionMethod:1
		hairObjectInstance.hairsPerStrip = 1
		hairObjectInstance.pointsPerStrand = 2
		hairObjectInstance.viewPercentage = 100
		hairObjectInstance.AddStripObject stripObject1
		hairObjectInstance.RotateStripOrientations 0
		
		local hairInstance = hairObjectInstance.GetHair()
		local initialHairTangent1 = (hairInstance.GetPointT 0 0 ) - (hairInstance.GetPointT 0 1 )
		local initialHairTangent2 = (hairInstance.GetPointT 1 0 ) - (hairInstance.GetPointT 1 1 )
		local initialHairTangent3 = (hairInstance.GetPointT 2 0 ) - (hairInstance.GetPointT 2 1 )
		
		local stripObjectName = stripObject1.Name
		local hairObjectInstanceName = hairObjectInstance.Name
		
		holdMaxFile()
		fetchMaxFile quiet:true
		
		stripObject1 = getNodeByName stripObjectName
		hairObjectInstance = getNodeByName hairObjectInstanceName
		
		-- Move the strip object onto another layer
		local layer = LayerManager.newLayer()
		layer.addNode stripObject1
		hairInstance = hairObjectInstance.GetHair()
		
		local hairTangentAfterLayerChange1 = (hairInstance.GetPointT 0 0 ) - (hairInstance.GetPointT 0 1 )
		local hairTangentAfterLayerChange2 = (hairInstance.GetPointT 1 0 ) - (hairInstance.GetPointT 1 1 )
		local hairTangentAfterLayerChange3 = (hairInstance.GetPointT 2 0 ) - (hairInstance.GetPointT 2 1 )
		
		local initialTangentLength = length initialHairTangent1
		EAssert.AreNearEqual (initialTangentLength*initialTangentLength) (dot initialHairTangent1 hairTangentAfterLayerChange1)
		EAssert.AreNearEqual (initialTangentLength*initialTangentLength) (dot initialHairTangent2 hairTangentAfterLayerChange2)
		EAssert.AreNearEqual (initialTangentLength*initialTangentLength) (dot initialHairTangent3 hairTangentAfterLayerChange3)
		
		-- Cleanup
		delete hairObjectInstance
		delete stripObject1
		LayerManager.deleteLayerByName layer.Name
	),
	
	function Test_HairStripsOnMeshLinkedToAnimatedMovingObject =
	(
		local movingObject = Sphere()
		set animate on
		sliderTime = 1f
		move movingObject [100,0,0]
		set animate off
		sliderTime = 0f
			
		local stripObject = Plane lengthsegs:1 widthsegs:1 width:20 length:10
		stripObject.parent = movingObject
		
		local hairObjectInstance = HairFromMeshStripsObject()
		hairObjectInstance.hairsPerStrip = 1
		hairObjectInstance.pointsPerStrand = 2
		hairObjectInstance.viewPercentage = 100
		hairObjectInstance.AddStripObject stripObject
		--hairObjectInstance.RotateStripOrientations()
		
		local hairInstance = hairObjectInstance.GetHair()
		local initialPointPosition = hairInstance.GetPointT 0 0
		
		sliderTime = 1f
		local positionAfterMove = hairInstance.GetPointT 0 0
		
		EAssert.ArePointsNotNearEqual initialPointPosition positionAfterMove
		
		sliderTime = 0f
		local positionAfterMoveBack = hairInstance.GetPointT 0 0
		
		EAssert.ArePointsNearEqual initialPointPosition positionAfterMoveBack
		
		-- Cleanup
		--sliderTime = 0f
		--delete movingObject
		--delete hairObjectInstance
		--delete stripObject
	),
	
	-- When rendering a sequence instead of a single image hair created by a deforming mesh strip should updat on each frame
	function Test_DeformingHairStripUpdatesWhenRenderingASequence =
	(
		-- Add an animated deforming strip mesh
		local stripObject = Plane lengthsegs:1 widthsegs:2 width:20 length:10
		
		max modify mode
		select stripObject
		
		local bendModifierInstance = Bend bendAxis:0 bendAngle:0
		set animate on
		sliderTime += 1
		bendModifierInstance.bendAngle = 360
		sliderTime -= 1
		set animate off
			
		modPanel.addModToSelection bendModifierInstance
		
		-- Create a renderable hair strips object
		max create mode
		local hairObjectInstance = HairFromMeshStripsObject hairsPerStrip:1 pointsPerStrand:3 viewPercentage:100 distributionMethod:1
		hairObjectInstance.AddStripObject stripObject
		
		max modify mode
		select hairObjectInstance
		
		local meshFromStrandsModifierInstance = Ox_Mesh_From_Strands type:0 guideMeshExp:off
		modPanel.addModToSelection meshFromStrandsModifierInstance
		
		-- Zoom viewport extents
		viewport.activeViewport = 2
		viewport.setType #view_front
		actionMan.executeAction 0 "310"
		
		-- Set renderer to render frames 0 to 1 in a sequence
		rendTimeType = 3
		rendStart = 0
		rendEnd = 1
		renderWidth = 20
		renderHeight = 20
		rendSaveFile = true
		local expectedFilePathRoot = (GetDir #renderoutput) + "\\Test_DeformingHairStripUpdatesWhenRenderingASequence"
		local expectedFilePath1 = expectedFilePathRoot + "0000.bmp"
		local expectedFilePath2 = expectedFilePathRoot + "0001.bmp"
		rendOutputFilename = expectedFilePathRoot + ".bmp"
		
		hide stripObject
		clearSelection()
		max quick render
		
		-- The two rendered bitmaps should not be the same
		EAssert.IsFalse (CompareBitmaps expectedFilePath1 expectedFilePath2 2 2)
		
		-- Cleanup
		actionMan.executeAction 0 "310" -- Zoom Extents Selected to reset viewport
	),
	
	-- Test volume controls
	function Test_VolumeControls =
	(
		local stripObject = Plane lengthsegs:1 widthsegs:1
		stripObject.width = 20
		stripObject.length = 10
		
		local hairObjectInstance = HairFromMeshStripsObject distributionMethod:1
		hairObjectInstance.hairsPerStrip = 1
		hairObjectInstance.pointsPerStrand = 5
		hairObjectInstance.viewPercentage = 100
		hairObjectInstance.AddStripObject stripObject
		
		-- Get default tips
		local tipDefault = ( ( hairObjectInstance.GetHair() ).GetPointT 0 ( hairObjectInstance.pointsPerStrand - 1 ) )
		
		-- Increase volume
		hairObjectInstance.volumeSelected = 5.0
		local tipInflated = ( ( hairObjectInstance.GetHair() ).GetPointT 0 ( hairObjectInstance.pointsPerStrand - 1 ) )
		
		-- Remove volume
		hairObjectInstance.volumeSelected = 0.0
		local tipDeflated = ( ( hairObjectInstance.GetHair() ).GetPointT 0 ( hairObjectInstance.pointsPerStrand - 1 ) )
	
		-- Check reload scene
		hairObjectInstance.volumeSelected = 5.0
		local systemUnits = units.SystemType
		holdMaxFile()
		resetMaxFile #noprompt
		units.SystemType = systemUnits
		fetchMaxFile quiet:true
		EAssert.AreEqual 2 objects.count
		hairObjectInstance = objects[2]
		local tipReloaded = ( ( hairObjectInstance.GetHair() ).GetPointT 0 ( hairObjectInstance.pointsPerStrand - 1 ) )

		EAssert.ArePointsNearEqual tipDefault tipDeflated
		EAssert.ArePointsNearEqual tipInflated tipReloaded
		EAssert.ArePointsNotNearEqual tipDefault tipInflated
	),

	-- Test per strip settings
	function Test_PerStripSettings =
	(
		-- Dummy planes
		local planeL = Plane lengthsegs:4 widthsegs:2 length:10 width:5 pos:[20, 0, 0]
		local planeR = Plane lengthsegs:4 widthsegs:2 length:10 width:5 pos:[-20, 0, 0]

		-- Create main planes and attach one to another
		local planeC = Plane lengthsegs:4 widthsegs:2 length:10 width:5 pos:[0, 0, 0]
		local plane1 = Plane lengthsegs:4 widthsegs:2 length:10 width:5 pos:[0, 0, 10]
		local plane2 = Plane lengthsegs:4 widthsegs:2 length:10 width:5 pos:[0, 0, 20]

		select planeC
		max modify mode
		local editPolyModifierInstance = Edit_Poly()
		modPanel.addModToSelection editPolyModifierInstance
		editPolyModifierInstance.Attach plane1 editPolyNode:planeC
		editPolyModifierInstance.Attach plane2 editPolyNode:planeC
	
		local hairObjectInstance = HairFromMeshStripsObject distributionMethod:1
		hairObjectInstance.hairsPerStrip = 1
		hairObjectInstance.pointsPerStrand = 5
		hairObjectInstance.viewPercentage = 100
		hairObjectInstance.AddStripObject planeL
		hairObjectInstance.AddStripObject planeC
		hairObjectInstance.AddStripObject planeR
		hairObjectInstance.volumeSelected = 30

		select hairObjectInstance
		max modify mode		
		subObjectLevel = 1
		
		-- Change per-strip settings
		hairObjectInstance.SelectStrips #{2}
		hairObjectInstance.DistributionMethodSelected = 0
		hairObjectInstance.VolumeSelected = 5
		
		hairObjectInstance.SelectStrips #{3}
		hairObjectInstance.DistributionMethodSelected = 1
		hairObjectInstance.HairsPerStripSelected = 10
		hairObjectInstance.VolumeSelected = 10
		
		hairObjectInstance.SelectStrips #{4}
		hairObjectInstance.DistributionMethodSelected = 2
		hairObjectInstance.HairsPerStripSelected = 20
		hairObjectInstance.VolumeSelected = 15
		
		delete planeL
		delete planeR
		
		holdMaxFile()
		fetchMaxFile quiet:true
		
		planeC = objects[1]
		hairObjectInstance = objects[2]

		select hairObjectInstance
		max modify mode		
		subObjectLevel = 1

		-- Strips indices would shifted after planeL deletion
		hairObjectInstance.SelectStrips #{1}
		EAssert.AreEqual 0 hairObjectInstance.DistributionMethodSelected
		EAssert.AreNearEqual 5 hairObjectInstance.VolumeSelected
		
		hairObjectInstance.SelectStrips #{2}
		EAssert.AreEqual 1 hairObjectInstance.DistributionMethodSelected
		EAssert.AreEqual 10 hairObjectInstance.HairsPerStripSelected
		EAssert.AreNearEqual 10 hairObjectInstance.VolumeSelected

		hairObjectInstance.SelectStrips #{3}
		EAssert.AreEqual 2 hairObjectInstance.DistributionMethodSelected
		EAssert.AreEqual 20 hairObjectInstance.HairsPerStripSelected
		EAssert.AreNearEqual 15 hairObjectInstance.VolumeSelected

		local myHair = hairObjectInstance.GetHair()
		-- Check hair count
		EAssert.AreEqual ( 3 + 10 + 20 ) ( myHair.NumRoots() )
	),

	function Test_UndoRedo =
	(
		-- Dummy planes
		local planeL = Plane lengthsegs:4 widthsegs:2 length:10 width:5 pos:[20, 0, 0]
		local planeC = Plane lengthsegs:4 widthsegs:2 length:10 width:5 pos:[0, 0, 0]
		local planeR = Plane lengthsegs:4 widthsegs:2 length:10 width:5 pos:[-20, 0, 0]
	
		local hairObjectInstance = HairFromMeshStripsObject distributionMethod:1
		hairObjectInstance.AddStripObject planeL
		hairObjectInstance.AddStripObject planeC
		hairObjectInstance.AddStripObject planeR

		hairObjectInstance.HairsPerStrip = 2
		hairObjectInstance.PointsPerStrand = 5
		hairObjectInstance.ViewPercentage = 100
		hairObjectInstance.VolumeSelected = 1
		hairObjectInstance.DistributionMethodSelected = 1

		select hairObjectInstance
		max modify mode		
		subObjectLevel = 1

		-- tosses out the restore objects since the last Begin() but still continues holding
		theHold.Accept "setup"
		theHold.begin()
		
		local vertices0 = OxGetVertices hairObjectInstance true
		
		hairObjectInstance.SelectStrips #{1}
		hairObjectInstance.DistributionMethodSelected = 0
		hairObjectInstance.VolumeSelected = 5
		
		hairObjectInstance.SelectStrips #{2}
		hairObjectInstance.DistributionMethodSelected = 1
		hairObjectInstance.HairsPerStripSelected = 10
		hairObjectInstance.VolumeSelected = 10
		
		hairObjectInstance.SelectStrips #{3}
		hairObjectInstance.DistributionMethodSelected = 2
		hairObjectInstance.HairsPerStripSelected = 20
		hairObjectInstance.VolumeSelected = 15
		
		local vertices1 = OxGetVertices hairObjectInstance true
		theHold.Accept "test"

		max undo		
		
		local vertices2 = OxGetVertices hairObjectInstance true
		
		max redo		
		
		local vertices3 = OxGetVertices hairObjectInstance true

		EAssert.AreNotEqual ( vertices0.count ) ( vertices1.count )
		EAssert.AreEqual ( vertices0.count ) ( vertices2.count )
		EAssert.AreEqual ( vertices1.count ) ( vertices3.count )

		for i = 1 to vertices0.count do
		(
			EAssert.ArePointsNearEqual ( vertices0[i] ) ( vertices2[i] )
		)

		for i = 1 to vertices1.count do
		(
			EAssert.ArePointsNearEqual ( vertices1[i] ) ( vertices3[i] )
		)
		
		max undo
		max undo
		max undo
	),
	
	-- Checks for a crash regression relating to lack of generated UVW coordinates
	function Test_DetailModifierOnHairStrips =
	(
		-- Create a plane with 2x2 segments so we have 9 vertices all together
		local distributionObject = Plane lengthsegs:1 widthsegs:1

		local hairObjectInstance = HairFromMeshStripsObject viewPercentage:100 isSelected:on distributionMethod:0
		hairObjectInstance.AddStripObject distributionObject
		
		max modify mode
		modPanel.addModToSelection (Ox_Strand_Detail viewCount:20)
			
		-- If no crash happened we are good
		EAssert.AreEqual 40 (numPoints hairObjectInstance)
	),
	
	-- Tests for a bug where mesh from strands applied to hair from strips didn't render if cylindrical mode is set
	function Test_RenderCylindricalHairsFromStrips =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1

		local hairObjectInstance = HairFromMeshStripsObject viewPercentage:100 isSelected:on distributionMethod:0
		hairObjectInstance.AddStripObject distributionObject
		
		max modify mode
		local meshFromStrandsInstance = Ox_Mesh_From_Strands type:0
		modPanel.addModToSelection meshFromStrandsInstance
		
		hide distributionObject
		local camera1 = Freecamera fov:45 targetDistance:160 pos:[0,0,50]
		local renderResult = render camera:camera1 outputwidth:50 outputheight:50
		
		-- Make sure that bitmap isn't completely black (we have hairs in there)
		EAssert.IsFalse (TestUtilities.AreAllPixelsBlack renderResult)
		
		-- Cleanup
		unDisplay renderResult
	),
	
	function Test_ReloadBakedHairFromMeshStripsObject =
	(
		-- Create and collapse a hair from mesh strips object
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		local hairObjectInstance = HairFromMeshStripsObject viewPercentage:100 isSelected:on distributionMethod:0 pointsPerStrand:2
		hairObjectInstance.AddStripObject distributionObject
		max modify mode
		modPanel.addModToSelection (Ox_Strand_Curling())
		
		maxOps.CollapseNode hairObjectInstance on
		local objectName = hairObjectInstance.Name
			
		local verticesBeforeSave = OxGetVertices hairObjectInstance true
		holdMaxFile()
		fetchMaxFile quiet:true
		hairObjectInstance = GetNodeByName objectName
		local verticesAfterLoad = OxGetVertices hairObjectInstance true
		
		EAssert.ArePointsAllNearEqual verticesBeforeSave verticesAfterLoad
	),


	function Test_SelectNextPrevious =
	(
		-- Dummy planes
		local planeL = Plane lengthsegs:4 widthsegs:2 length:10 width:5 pos:[20, 0, 0]
		local planeR = Plane lengthsegs:4 widthsegs:2 length:10 width:5 pos:[-20, 0, 0]
	
		local hairObjectInstance = HairFromMeshStripsObject distributionMethod:1
		hairObjectInstance.AddStripObject planeL
		hairObjectInstance.AddStripObject planeR

		hairObjectInstance.HairsPerStrip = 2
		hairObjectInstance.PointsPerStrand = 5
		hairObjectInstance.ViewPercentage = 100
		hairObjectInstance.VolumeSelected = 1
		hairObjectInstance.DistributionMethodSelected = 1

		select hairObjectInstance
		max modify mode		
		subObjectLevel = 1
		
		hairObjectInstance.SelectStrips #{1}

		hairObjectInstance.SelectNext()
		hairObjectInstance.SelectNext()

		max undo
		max undo

		max redo

		hairObjectInstance.SelectStrips #{2}

		hairObjectInstance.SelectPrevious()
		hairObjectInstance.SelectPrevious()

		max undo
		max undo

		max redo
	),
	
	function Test_ConvertToGuidesAndAssignGroups =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		local hairObjectInstance = HairFromMeshStripsObject viewPercentage:100 isSelected:on distributionMethod:0 pointsPerStrand:2
		hairObjectInstance.AddStripObject distributionObject
		max modify mode
		modPanel.addModToSelection (Ox_Guides_From_Hair())
		local editGuidesModifierInstance = Ox_Edit_Guides changeTracking:1
		modPanel.addModToSelection editGuidesModifierInstance
		local frizzInstance = Ox_Strand_Frizz amount:30 strandGroupPattern:"1"
		modPanel.addModToSelection frizzInstance
			
		local verticesBeforeAssigningGroup = OxGetVertices hairObjectInstance true
			
		modPanel.setCurrentObject editGuidesModifierInstance
		editGuidesModifierInstance.useGuideGroups = on
		subobjectLevel = 1
		editGuidesModifierInstance.SelectRoots #{1}
		editGuidesModifierInstance.SetSelectedGuidesGroupIndex 1
		subobjectLevel = 0
		modPanel.setCurrentObject frizzInstance
			
		local verticesAfterAssigningGroup = OxGetVertices hairObjectInstance true
			
		-- Since we now have a guide in group 1 it should be curles
		EAssert.ArePointsNotAllNearEqual verticesBeforeAssigningGroup verticesAfterAssigningGroup
	),
	
	function Test_ApplyDetailer =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		local hairObjectInstance = HairFromMeshStripsObject viewPercentage:100 isSelected:on distributionMethod:0 pointsPerStrand:2
		hairObjectInstance.AddStripObject distributionObject
		max modify mode
		
		local boundingBoxBeforeDetailer = TestUtilities.GetPointsBoundingBox (OxGetVertices hairObjectInstance true)
		modPanel.addModToSelection (Ox_Strand_Detail())
		local boundingBoxAfterDetailer = TestUtilities.GetPointsBoundingBox (OxGetVertices hairObjectInstance true)
			
		EAssert.ArePointsNearEqual boundingBoxBeforeDetailer[1] boundingBoxAfterDetailer[1] useDistance:true
		EAssert.ArePointsNearEqual boundingBoxBeforeDetailer[2] boundingBoxAfterDetailer[2] useDistance:true
	),
	
	function Test_CylindricalStrips =
	(
		max modify mode
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		local meshFromStrands = Ox_Mesh_from_Strands type:0 numSides:3 guideMeshExp:off strandGroup:0
		modPanel.addModToSelection meshFromStrands
		
		local hairObjectInstance = HairFromMeshStripsObject viewPercentage:100 isSelected:on distributionMethod:0 pointsPerStrand:2
		hairObjectInstance.AddStripObject distributionObject
		
		EAssert.AreEqual (4*4) (OxGetStrandCount hairObjectInstance)
	),
	
	-- Tests that the rotate operator can function correctly on hair generated from mesh strips
	function Test_RotateHairFromMeshStrips =
	(
		OxResetClassParameters (Ox_Rotate_Strands())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		local hairObjectInstance = HairFromMeshStripsObject viewPercentage:100 isSelected:on distributionMethod:0 pointsPerStrand:2
		hairObjectInstance.AddStripObject distributionObject
		max modify mode
		local rotateModifierInstance = Ox_Rotate_Strands()
		modPanel.addModToSelection rotateModifierInstance
		modPanel.addModToSelection (Ox_Mesh_From_Strands())
		
		local initialMeshVertices = TestUtilities.GetVertexArray hairObjectInstance
		
		-- Change the rotation angle. This should affect the final mesh vertices if hair is rotated.
		rotateModifierInstance.angle = 45
		local verticesAfterRotation = TestUtilities.GetVertexArray hairObjectInstance
		
		EAssert.ArePointsAllNotNearEqual initialMeshVertices verticesAfterRotation
	),
	
	function Test_GroundHairs =
	(
		local distributionMesh = Sphere()
		local stripMesh = Plane lengthsegs:1 widthsegs:1
		local hairObjectInstance = HairFromMeshStripsObject viewPercentage:100 isSelected:on distributionMethod:0 pointsPerStrand:2
		hairObjectInstance.AddStripObject stripMesh
		
		max modify mode
		modPanel.addModToSelection (Ox_Guides_From_Hair())
		local groundStrandsInstance = Ox_Ground_Strands surface:distributionMesh
		modPanel.addModToSelection groundStrandsInstance
		
		-- Ground the strands
		local verticesBeforeGrounding = OxGetVertices hairObjectInstance true
		groundStrandsInstance.detachRoots = off
		local verticesAfterGrounding = OxGetVertices hairObjectInstance true
			
		EAssert.ArePointsNotAllNearEqual verticesBeforeGrounding verticesAfterGrounding
			
		-- Unground
		groundStrandsInstance.detachRoots = on
		local verticesAfterUngrounding = OxGetVertices hairObjectInstance true
		EAssert.ArePointsAllNearEqual verticesBeforeGrounding verticesAfterUngrounding
	)
)

EUnit.AddTestClass Test_HairFromMeshStripsObject
ok
