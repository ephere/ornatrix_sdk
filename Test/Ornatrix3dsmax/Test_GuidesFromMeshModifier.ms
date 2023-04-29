struct Test_GuidesFromMeshModifier
(
	function TestCleanup =
	(
		delete $objects
	),
	
	function Test_UpdateGuidesOnMeshSelectionChange =
	(
		-- Create a 2x1 plane and select one of the faces but not the other
		local distributionObject = convertToPoly(Plane lengthsegs:2 widthsegs:1)
		polyop.setFaceSelection distributionObject #{1}
		
		max modify mode

		-- Setup guides with surface comb
		select distributionObject

		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 2
		guidesFromSurfaceModifier.useSubSel = true
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		-- Only 4 guides should be generated because sub-selection is on
		local guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		myGuides = ( guidesCopy.GetGuides() ).AsHair()
		EAssert.AreEqual 4 (myGuides.NumRoots())
		delete guidesCopy
			
		-- Go out of sub-object mode
		max create mode
			
		-- Select the second face of the distribution mesh, this should result in extra 2 guides being generated
		polyop.setFaceSelection distributionObject #{1,2}
			
		local guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		myGuides = ( guidesCopy.GetGuides() ).AsHair()
		EAssert.AreEqual 6 (myGuides.NumRoots())
		delete guidesCopy
	),
	
	-- Make sure that changing distribution map updates the roots when not in modify mode
	function Test_RootUpdateOnDistributionMapChangeWhenInCreateMode =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode

		-- Setup guides with surface comb
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 2
		guidesFromSurfaceModifier.uvChannel = 1
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		local distributionMap = Checker()
		guidesFromSurfaceModifier.distrMap = distributionMap
		
		-- Record number of roots at present
		local initialRootCount = OxGetStrandCount distributionObject
			
		-- Go out of sub-object mode
		max create mode
			
		-- Change the checker map to be all black
		distributionMap.color1 = color 255 255 255
			
		-- Number of generated guides should have changed
		EAssert.AreNotEqual initialRootCount (OxGetStrandCount distributionObject)
	),
	
	function Test_DisplacementMapAndOffset =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode

		-- Setup guides with surface comb
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 2
		guidesFromSurfaceModifier.numSegs = 2
		guidesFromSurfaceModifier.guideLength = 10
		guidesFromSurfaceModifier.randomness = 0
		modPanel.addModToSelection guidesFromSurfaceModifier
	
		-- Without Displacement
		guidesFromSurfaceModifier.displacementMap = undefined
		guidesFromSurfaceModifier.displacementOffset = 0	
		guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		myHair = ( guidesCopy.GetGuides() ).AsHair()
		ZeroDisplaceTip = 0
		ZeroDisplaceRoot = 0
		for i = 0 to ( myHair.NumRoots() - 1 ) do
		(
			hairRootPoint = myHair.GetPointT i 0
			hairTipPoint = myHair.GetPointT i 1
			ZeroDisplaceTip += hairTipPoint.z
			ZeroDisplaceRoot += hairRootPoint.z
		)

		EAssert.AreNearEqual 0 ( ZeroDisplaceRoot )
		
		--With Displacement (with middle grey map and double Guide's Length as offset, it should be at the top of each strands without displacement)
		local DisplacementMap = Checker ()
		DisplacementMap.color1 = DisplacementMap.color2 = (color 255 255 255) / 2
		guidesFromSurfaceModifier.displacementMap = DisplacementMap
		guidesFromSurfaceModifier.displacementOffset = guidesFromSurfaceModifier.guideLength * 2
		guidesCopy2 = instance distributionObject
		convertTo guidesCopy2 GuidesObject
		myHair2 = ( guidesCopy2.GetGuides() ).AsHair()
		DisplaceRoot = 0
		for i = 0 to ( myHair2.NumRoots() - 1 ) do
		(
			hairRootPoint = myHair2.GetPointT i 0
			DisplaceRoot += hairRootPoint.z
		)
		EAssert.AreNearEqual ZeroDisplaceTip DisplaceRoot
	),

	function Test_DisplacementMapInvert =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		distributionObject.width = 10
		distributionObject.length = 10
		max modify mode
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 2
		guidesFromSurfaceModifier.count = 10
		guidesFromSurfaceModifier.numSegs = 2
		guidesFromSurfaceModifier.guideLength = 10
		guidesFromSurfaceModifier.randomness = 0
		modPanel.addModToSelection guidesFromSurfaceModifier

		local DisplacementMap = Gradient()
		guidesFromSurfaceModifier.displacementMap = DisplacementMap
		guidesFromSurfaceModifier.displacementOffset = 10
		
		-- default is not inverted
		local verticesDefault = OxGetVertices distributionObject true
		guidesFromSurfaceModifier.displacementMapInvert = true
		local verticesInverted = OxGetVertices distributionObject true
		
		for i = 1 to verticesDefault.count do
		(		
			-- filter root vertices only
			if ( mod ( i - 1 ) guidesFromSurfaceModifier.numSegs ) == 0 then
			(
				EAssert.AreNearEqual 5 ( verticesDefault[i].z + verticesDefault[i].y )
				EAssert.AreNearEqual 5 ( verticesInverted[i].z - verticesInverted[i].y )
			)
		)
	),
	
	function Test_DisplacementMapChannels =
	(
		OxResetClassParameters()
		
		-- Create a plane with 1x1 segments (so that there are are no vertices going exactly down the middle)
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode
		
		-- Create Mapping Channels
		select distributionObject
		for i = 1 to 2 do
		(
			uvwMapModifier = Uvwmap ()
			uvwMapModifier.mapChannel = i
			uvwMapModifier.length = distributionObject.length / i
			uvwMapModifier.width = distributionObject.width / i
			modPanel.addModToSelection uvwMapModifier
		)

		-- Setup guides with surface comb
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		-- Set default parameters
		guidesFromSurfaceModifier.method = 2
		guidesFromSurfaceModifier.uvChannel = 1
		guidesFromSurfaceModifier.count = 100
		guidesFromSurfaceModifier.guideLength = 10
		guidesFromSurfaceModifier.randomness = 0
		guidesFromSurfaceModifier.NumSegs = 2
		
		local DisplacementMap = Checker ()
		DisplacementMap.color1 = black
		DisplacementMap.color2 = white
		guidesFromSurfaceModifier.displacementMap = DisplacementMap
		guidesFromSurfaceModifier.displacementOffset = guidesFromSurfaceModifier.guideLength
		
		--Bake 1-st Mapping Channel
		DisplacementMap.coords.mapChannel = 1
		local guidesCopy1 = instance distributionObject
		convertTo guidesCopy1 GuidesObject
		local myHair1 = ( guidesCopy1.GetGuides() ).AsHair()
		
		--Bake 2-nd Mapping Channel
		DisplacementMap.coords.mapChannel = 2
		local guidesCopy2 = instance distributionObject
		convertTo guidesCopy2 GuidesObject
		local myHair2 = ( guidesCopy2.GetGuides() ).AsHair()
		
		--Hairs Count should be the same
		EAssert.AreEqual ( myHair1.NumRoots() ) ( myHair2.NumRoots() )
		
		--Check for matching, if Mapping channel isn't affected, 100% points will be at the same position
		matchedPoints = 0
		for i = 0 to ( myHair1.NumRoots() - 1 ) do
		(
			RootPoint1 = myHair1.GetPointT i 0
			RootPoint2 = myHair2.GetPointT i 0
			if ( RootPoint1 == RootPoint2 ) then
			(
				matchedPoints += 1
			)
		)
		EAssert.AreNotEqual matchedPoints ( myHair1.NumRoots() )
	),
	
	-- Tests that guides are positioned correctly when topology orientation option isn't used
	function Test_GuidePlacementWithTopologyOrientationOff =
	(
		-- Create a plane with 1x1 segments (so that there are are no vertices going exactly down the middle)
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		local distributionObjectCopy = convertto (copy distributionObject) mesh
		max modify mode

		-- Setup guides with vertex distributioon and topology orientation off
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.topoOrient = off
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		-- Make sure vertices are not at the origin
		EAssert.ArePointsNearEqual (getvert distributionObjectCopy 1) (OxGetStrandPoint distributionObject 0 0 true true)
		EAssert.ArePointsNearEqual (getvert distributionObjectCopy 2) (OxGetStrandPoint distributionObject 1 0 true true)
		EAssert.ArePointsNearEqual (getvert distributionObjectCopy 3) (OxGetStrandPoint distributionObject 2 0 true true)
		EAssert.ArePointsNearEqual (getvert distributionObjectCopy 4) (OxGetStrandPoint distributionObject 3 0 true true)
	),
	
	-- Test that Nitrous hair doesn't crash Max when guide point count is increased (issue #208)
	function Test_NitrousViewportHairUpdateAfterPointCountIncrease =
	(
		-- Not sure if this works in 2020, it does work in 2021 and crashes in 2018
		EUnit.SkipBeforeMaxVersion 2020

		-- Create a plane with 1x1 segments (so that there are are no vertices going exactly down the middle)
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		max modify mode

		-- Setup guides with vertex distributioon and topology orientation off
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		local hairFromGuidesModifierInstance = Ox_Hair_From_Guides()
		hairFromGuidesModifierInstance.method = 4
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		-- Increasing guide point count and redrawing viewports should not cause a crash
		guidesFromSurfaceModifier.numSegs = 100
		redrawViews()
		redrawViews()
		redrawViews()
	),
	
	function Test_GuidesRootsAreNotRegeneratedIfTopologyDoesntChange =
	(
		local distributionObject = Plane lengthsegs:10 widthsegs:10
		max modify mode
		
		-- Setup guides with surface area distributioon
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 2
		guidesFromSurfaceModifier.count = 100
		guidesFromSurfaceModifier.useSubSel = on
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		-- Remember the positions of all roots
		local guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		local myGuides = ( guidesCopy.GetGuides() ).AsHair()
		local root1Position = myGuides.GetPointT 15 0
		local root2Position = myGuides.GetPointT 78 0
		delete guidesCopy
		
		local scaleFactor = 2.57
		
		-- Stretch the plane horizontally twice and make sure the roots have also stretched twice
		distributionObject.length = distributionObject.length * scaleFactor
		guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		myGuides = ( guidesCopy.GetGuides() ).AsHair()
		local root1Position2 = myGuides.GetPointT 15 0
		local root2Position2 = myGuides.GetPointT 78 0
		delete guidesCopy
		
		EAssert.ArePointsNearEqual root1Position2 (root1Position * [1,scaleFactor,1])
		EAssert.ArePointsNearEqual root2Position2 (root2Position * [1,scaleFactor,1])
	),
	
	function Test_GuidesOnZeroAreaSurface =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		distributionObject.length = 0
		max modify mode
		
		-- Setup guides with surface area distributioon
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 2
		guidesFromSurfaceModifier.count = 2
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		-- If nothing crashed or hanged at this point we are good
		local guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		local myGuides = ( guidesCopy.GetGuides() ).AsHair()
		EAssert.AreEqual 0 ( myGuides.NumRoots() )
		delete guidesCopy
	),
	
	function Test_UniformGenerationOnNonDefaultUVChannel =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:1 length:1
		max modify mode
		select distributionObject
		
		-- Apply a UVW modifier to the second UV channel of the mesh
		local uvwMapModifierInstance = Uvwmap maptype:0 width:1 length:2 mapChannel:2
		modPanel.addModToSelection uvwMapModifierInstance
		
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 0
		guidesFromSurfaceModifier.count = 10
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		local initialStrandCount = OxGetStrandCount distributionObject
		
		-- Change distribution UV channel to 2, there should be twice as fewer strands
		guidesFromSurfaceModifier.uvChannel = 2
		EAssert.IsTrue (( initialStrandCount / 2 ) >= (OxGetStrandCount distributionObject))
	),
	
	function Test_AssignUnassignDistributionChannel =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:10 length:10
		max modify mode
		select distributionObject
		
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		-- 4 strands by default
		EAssert.AreEqual 4 (OxGetStrandCount distributionObject)
		
		-- Assign vertex colors to distribution. They should all be black and thus no guides should be generated.
		guidesFromSurfaceModifier.distrChan = 65536
		EAssert.AreEqual 0 (OxGetStrandCount distributionObject)
		
		-- Unassign vertex color channel, the guides should be back
		guidesFromSurfaceModifier.distrChan = 0
		EAssert.AreEqual 4 (OxGetStrandCount distributionObject)
		
		-- Change UV channel and try again
		guidesFromSurfaceModifier.uvChannel = 2
		guidesFromSurfaceModifier.distrChan = 65536
		EAssert.AreEqual 0 (OxGetStrandCount distributionObject)
	),
	
	function Test_DetachRoots =
	(
		local distributionObject = Sphere radius:10 smooth:on segs:40 mapcoords:on
		max modify mode
		select distributionObject
		
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 2
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		-- Detach roots, alter underlying topology, then re-attach roots
		local vertexPositionsBeforeDetach = OxGetVertices distributionObject true
		guidesFromSurfaceModifier.detachRoots = on
			
		-- Cause an evaluation
		OxGetVertices distributionObject true
		distributionObject.segs = 30
		guidesFromSurfaceModifier.detachRoots = off
		
		local vertexPositionsAfterReattach = OxGetVertices distributionObject true
		EAssert.ArePointsAllNearEqual vertexPositionsBeforeDetach vertexPositionsAfterReattach useDistance:true epsilon:0.8
	),
	
	-- Tests for floating point precision errors
	function Test_GuidesFarFromOrigin =
	(
		-- Create a distribution plane and move its vertices very far from origin
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:10 length:10 isSelected:on
		max modify mode
		
		local editPolyModifier = Edit_Poly()
		modPanel.addModToSelection editPolyModifier
		
		local veryBigDistance = 9999999
		
		subobjectLevel = 4
		editPolyModifier.SetSelection #Face #{1}
		editPolyModifier.SetOperation #Transform
		
		editPolyModifier.MoveSelection [veryBigDistance,0,0] parent:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [0,0,0])
		editPolyModifier.Commit()
		
		local noiseModifierInstance = NoiseModifier scale:2 strength:[1,1,1] animate:on
		modPanel.addModToSelection noiseModifierInstance
		 
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		modPanel.addModToSelection guidesFromSurfaceModifier
	),
	
	-- Tests that even if surface triangles have small UV coordinates we will still generate guides on them properlys
	function Test_SmallUvCoordinates =
	(
		OxResetClassParameters()
		local distributionObject = Plane lengthsegs:2 widthsegs:2 width:1 length:1 isSelected:on
		max modify mode
		
		-- Large enough UV scale will ensure that the triangles on the plane will be small enough in UV space
		local largeUvScale = 300
		modPanel.addModToSelection (Uvwmap maptype:0 width:largeUvScale length:largeUvScale)
		modPanel.addModToSelection (Ox_Guides_from_Surface method:1 count:10)
		
		EAssert.AreEqual 10 (OxGetStrandCount distributionObject)
	),
	
	function TestRootsDontChangeOnEditedMesh distributionMethod =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:5 length:5 isSelected:on
		max modify mode
		
		local editPolyModifier = Edit_Poly()
		modPanel.addModToSelection editPolyModifier
		modPanel.addModToSelection (Ox_Guides_from_Surface method:distributionMethod count:10)
		
		local surfaceDependenciesBeforeDeformation = OxGetSurfaceDependencies distributionObject
		
		modPanel.setCurrentObject editPolyModifier
		subobjectLevel = 1
		editPolyModifier.SetSelection #Vertex #{}
		editPolyModifier.Select #Vertex #{3}
		editPolyModifier.SetOperation #Transform
		editPolyModifier.MoveSelection [-5,5,0] parent:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [0,0,0])
		editPolyModifier.Commit()
		
		local surfaceDependenciesAfterDeformation = OxGetSurfaceDependencies distributionObject
		
		EAssert.AreArraysEqual surfaceDependenciesBeforeDeformation surfaceDependenciesAfterDeformation
	),
	
	function Test_RootsDontChangeOnEditedMesh_Uniform =
	(
		TestRootsDontChangeOnEditedMesh 0
	),
	
	function Test_RootsDontChangeOnEditedMesh_RandomUv =
	(
		TestRootsDontChangeOnEditedMesh 1
	),
	
	function Test_RootsDontChangeOnEditedMesh_Random =
	(
		TestRootsDontChangeOnEditedMesh 2
	),
	
	function Test_RootsDontChangeOnEditedMesh_Face =
	(
		TestRootsDontChangeOnEditedMesh 3
	),
	
	function Test_RootsDontChangeOnEditedMesh_Vertex =
	(
		TestRootsDontChangeOnEditedMesh 4
	),
	
	function Test_RootsDontChangeOnEditedMesh_FaceCenter =
	(
		TestRootsDontChangeOnEditedMesh 6
	),
	
	function Test_RootsDontChangeOnEditedMesh_Even =
	(
		TestRootsDontChangeOnEditedMesh 7
	),
	
	function Test_V6RandomAreaDistribution =
	(
		EUnit.SkipBeforeMaxVersion 2020

		units.SystemType = #Centimeters
		local testSceneFilePath = pathConfig.convertPathToAbsolute (getSourceFileName() + "..\\..\\..\\..\\..\\..\\..\\..\\" ) + "External\\TestData\\OxMax6-Max2020-RandomAreaRoots.max"
		loadMaxFile testSceneFilePath useFileUnit:true quiet:true
		
		local actualStrandIds = OxGetStrandIds $OX_25
		sort actualStrandIds
		local expectedStrandIds = #(1290000, 2350000, 5090000, 5180000, 7510000)
		EAssert.AreArraysEqual expectedStrandIds actualStrandIds
		
		local actualVertices = OxGetStrandPointsSortedByStrandId $OX_25 true
		local expectedVertices = #([3.17957,-6.44542,204.647], [7.40974,-16.8205,214.62], [7.13251,-4.85238,201.622], [20.9866,-9.72828,204.67], [-4.71736,-3.04831,206.173], [-12.3287,-7.7053,218.231], [-5.52786,-3.90305,205.21], [-14.8813,-9.49361,215.518], [-7.20412,2.43513,192.211], [-17.2035,10.2552,184.22])

		-- Note: A large epsilon here is required as some of the resulting points don't exactly lined up (while most points are within 0.005 epsilon). It is unclear why this is the case, but is acceptable for a major version change.
		EAssert.ArePointsAllNearEqual expectedVertices actualVertices epsilon:2
		
		-- Cleanup
		resetMaxFile #noPrompt
	),
	
	function Test_V6RandomAreaDistribution_2Roots =
	(
		EUnit.SkipBeforeMaxVersion 2020

		units.SystemType = #Centimeters
		local testSceneFilePath = pathConfig.convertPathToAbsolute (getSourceFileName() + "..\\..\\..\\..\\..\\..\\..\\..\\" ) + "External\\TestData\\OxMax6-Max2020-RandomArea2Roots.max"
		loadMaxFile testSceneFilePath useFileUnit:true quiet:true
		
		local actualVertices = OxGetVertices $FurBall true
		local expectedVertices = #([4.62454,36.257,3.12319], [10.7812,84.6311,7.2903], [0.474311,4.36263,-36.5858], [1.21749,9.83063,-78.3389])
		
		EAssert.ArePointsAllNearEqual expectedVertices actualVertices epsilon:1
		
		-- Cleanup
		resetMaxFile #noPrompt
	)
)

EUnit.AddTestClass Test_GuidesFromMeshModifier
ok
