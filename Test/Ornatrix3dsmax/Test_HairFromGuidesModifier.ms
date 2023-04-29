struct Test_HairFromGuidesModifier
(
	function TestInitialize =
	(
		OxResetClassParameters()
	),
	
	function TestCleanup =
	(
		delete $objects
	),
	
	function Test_VertexDistributionWithRandomGuideDistribution =
	(
		-- Create a plane with 2x2 segments so we have 9 vertices all together
		local distributionObject = Plane lengthsegs:2 widthsegs:2 mapcoords:on
		
		-- Copy the vertex positions of the plane into an array, we will compare them to hair roots later on
		local meshVertices = #();
		meshVertices.count = distributionObject.mesh.vertices.count
		for i = 1 to distributionObject.mesh.vertices.count do
		meshVertices[i] = distributionObject.mesh.vertices[i].position

		max modify mode

		-- Add guides from surface, make sure it has a random distribution
		select distributionObject
		guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		
		modPanel.addModToSelection guidesFromSurfaceModifier

		-- Add hair from guides, and set its distribution mode to vertex
		hairFromGuidesModifier = Ox_Hair_from_Guides()
		modPanel.addModToSelection hairFromGuidesModifier
		hairFromGuidesModifier.method = 4
			
		-- Need to collapse the stack to get access to hair
		convertTo distributionObject HairObject
		myHair = distributionObject.GetHair()

		-- The position of each hair root should coincide with the position of each of the plane's vertices
		EAssert.AreEqual ( meshVertices.count ) ( myHair.NumRoots() )
			
		for i = 1 to meshVertices.count do
		(
			rootPoint = myHair.GetPointT ( i - 1 ) 0
			
			-- The root coordinate has to be one of the vertex coordinates of the original mesh
			EAssert.IsTrue( ( findItem meshVertices rootPoint ) != undefined )
		)
	),

	function Test_HairFromGuidesInstancedStrandsWithMeshFromStrandsAndRendering =
	(
		local sphereObject = Sphere radius:10 pos:[0,0,0] mapcoords:on isSelected:on

		max modify mode

		--local hairObject = SetupQuickOrnatrixHairFromSelection()
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		modPanel.addModToSelection guidesFromSurfaceModifier
		local hairFromGuidesModifierInstance = Ox_Hair_from_Guides instance:true denseCount:100
		modPanel.addModToSelection hairFromGuidesModifierInstance

		-- Test rendering of instanced strands with built-in fast raytracer, it should not cause a crash
		-- Set render count to a small number so that the test renders quicker

		-- Add Ox fast raytrace effect and render
		AddOrnatrixFastRaytraceEffectWithSelectedHair()
		render()
		
		-- Add Mesh From Strands modifier, it should not cause a crash
		OxAddModifierEx Ox_Mesh_From_Strands targetNode:sphereObject
	),

	function Test_HavingTwoGuidesCausesNoErrorWhenHairInterpolatesBetweenThreeGuides =
	(
		local distributionObject = Plane mapcoords:on

		max modify mode

		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		
		modPanel.addModToSelection guidesFromSurfaceModifier

		-- In guides from surface modifier set number of guides to 2
		distributionObject.modifiers[#Ox_Guides_from_Surface].count = 2

		modPanel.addModToSelection (Ox_Hair_from_Guides())

		-- Test 1: Set interpolation mode to affine. This should not cause any errors.
		distributionObject.modifiers[#Ox_Hair_from_Guides].interpolation = 1
	),
	
	function Test_RootUpdateOnDistributionMapChangeWhenInCreateMode =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on
		max modify mode

		-- Setup guides with surface comb
		select distributionObject
		guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 2
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		local hairFromGuidesModifierInstance = Ox_Hair_from_Guides()
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		local distributionMap = Checker()
		hairFromGuidesModifierInstance.distrMap = distributionMap
		
		-- Record number of roots at present
		local initialRootCount = OxGetStrandCount distributionObject
			
		-- Go out of sub-object mode
		max create mode
			
		-- Change the checker map to be all black
		distributionMap.color1 = color 255 255 255
			
		-- Number of generated guides should have changed
		EAssert.AreNotEqual initialRootCount (OxGetStrandCount distributionObject)
	),

	function Test_DistributionMapInvert =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on
		
		max modify mode
		select distributionObject
		guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.numSegs = 2
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		local hairFromGuidesModifierInstance = Ox_Hair_from_Guides denseCount:1000 viewCountPercentage:100
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		local distributionMap = Checker()
		hairFromGuidesModifierInstance.distrMap = distributionMap
		
		local countDefaultPos = 0
		local countDefaultNeg = 0
		for i = 1 to (OxGetStrandCount distributionObject) do
		(
			local pointDefault = OxGetStrandPoint distributionObject (i - 1) 0 true
			if ( pointDefault.x * pointDefault.y > 0 ) then
			(
				countDefaultPos += 1
			)
			else
			(
				countDefaultNeg += 1
			)
		)
		
		-- And now do invert checkers distribution map 
		hairFromGuidesModifierInstance.distrMapInvert = true		
		local countInvertedPos = 0
		local countInvertedNeg = 0
			
		for i = 1 to (OxGetStrandCount distributionObject) do
		(
			local pointInverted = OxGetStrandPoint distributionObject ( i - 1 ) 0 true
			if ( pointInverted.x * pointInverted.y > 0 ) then
			(
				countInvertedPos += 1
			)
			else
			(
				countInvertedNeg += 1
			)
		)

		-- With checker map there are same errors near corner lines
		EAssert.IsTrue( countDefaultPos > 450 )
		EAssert.IsTrue( countDefaultNeg < 50 )
		EAssert.IsTrue( countInvertedPos < 50 )
		EAssert.IsTrue( countInvertedNeg > 450 )
	),
	
	-- Tests for a bug where viewport nitrous preview doesn't update vertex indices when hair count goes up
	function Test_ChangeDistributionTypes =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on
		max modify mode

		-- Setup guides with surface comb
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		local hairFromGuidesModifierInstance = Ox_Hair_from_Guides()
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		-- Switch to "guides as hair" method and then to random distribution, which will increase the hair count
		hairFromGuidesModifierInstance.method = 4
		completeRedraw()
		
		hairFromGuidesModifierInstance.method = 1
		completeRedraw()
		
		hairFromGuidesModifierInstance.method = 5
		completeRedraw()
		
		hairFromGuidesModifierInstance.method = 3
		completeRedraw()
		
		-- No crash should have occured
	),

	-- Tests that the same hair from guides modifier can be applied to two objects at the same time
	function Test_ModifierInstancing =
	(
		local distributionObject1 = Plane lengthsegs:1 widthsegs:1 mapcoords:on
		local distributionObject2 = Plane lengthsegs:1 widthsegs:1 mapcoords:on
		max modify mode

		-- Setup guides on both objects
		select distributionObject1
		local guidesFromSurfaceModifierInstance = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifierInstance
		modPanel.addModToSelection guidesFromSurfaceModifierInstance
		
		select distributionObject2
		local guidesFromSurfaceModifierInstance = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifierInstance
		modPanel.addModToSelection guidesFromSurfaceModifierInstance

		-- Create a single hair from guides modifier and add it to both objects
		local hairFromGuidesModifierInstance = Ox_Hair_From_Guides denseCount:10 viewCountPercentage:100
		select distributionObject1
		modPanel.addModToSelection hairFromGuidesModifierInstance
		select distributionObject2
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		EAssert.AreNearEqual 10 (OxGetStrandCount distributionObject1) epsilon:3
		EAssert.AreNearEqual 10 (OxGetStrandCount distributionObject2) epsilon:3

		-- Change the hair count to 5 and make sure that both hair objects with the instanced modifier now have 5 hairs
		hairFromGuidesModifierInstance.denseCount = 5
		hairFromGuidesModifierInstance.count = 5
		
		EAssert.AreNearEqual 5 (OxGetStrandCount distributionObject1) epsilon:3
		
		-- TODO: The two assignments below shouldn't be needed if ModApp instances were properly handled by HFG (each instanced hair should be validated independently of the other)
		hairFromGuidesModifierInstance.denseCount = 5
		hairFromGuidesModifierInstance.count = 5
		EAssert.AreNearEqual 5 (OxGetStrandCount distributionObject2) epsilon:3
	),
	
	--Tests for correct synchronisation between vewport count, percentage, and render count
	function Test_ViewportAndRenderHairCount =
	(
		local distributionObject1 = Plane lengthsegs:1 widthsegs:1 mapcoords:on
		max modify mode

		-- Setup guides on object
		select distributionObject1
		local guidesFromSurfaceModifierInstance = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifierInstance
		modPanel.addModToSelection guidesFromSurfaceModifierInstance	
		
		-- Create a hair from guides modifier and add it to object
		local hairFromGuidesModifierInstance = Ox_Hair_From_Guides viewCountPercentage:20 denseCount:10000
		select distributionObject1
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		--Test Percentage
		EAssert.AreEqual 20.0 hairFromGuidesModifierInstance.viewCountPercentage
		
		--Test Viewport Count by changing Percentage and Render Count
		hairFromGuidesModifierInstance.viewCountPercentage = 30.0
		hairFromGuidesModifierInstance.denseCount = 10000
		
		--Test Viewport Count
		EAssert.AreNearEqual 3000 (OxGetStrandCount distributionObject1)
	),
	
	function Test_DisplacementMapAndOffset = 
	(
		OxResetClassParameters()
		
		-- Create a plane with 1x1 segments (so that there are are no vertices going exactly down the middle)
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode

		-- Setup guides with surface comb
		local guideLength = 10
		modPanel.addModToSelection (Ox_Guides_from_Surface guideLength:guideLength randomness:0 NumSegs:2)
		
		-- Assign Hair From Guides Modifier
		local hairFromGuidesModifierInstance = Ox_Hair_from_Guides method:0 viewCountPercentage:100 denseCount:10
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		-- Without Displacement
		hairFromGuidesModifierInstance.displacementMap = undefined
		hairFromGuidesModifierInstance.displacementOffset = 0	
		
		guidesCopy = instance distributionObject
		convertTo guidesCopy HairObject
		myHair = guidesCopy.GetHair()
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
		DisplacementMap = Checker ()
		DisplacementMap.color1 = DisplacementMap.color2 = (color 255 255 255) / 2
		hairFromGuidesModifierInstance.displacementMap = DisplacementMap
		hairFromGuidesModifierInstance.displacementOffset = guideLength * 2
		guidesCopy2 = instance distributionObject
		convertTo guidesCopy2 HairObject
		myHair2 = guidesCopy2.GetHair()
		DisplaceRoot = 0
		for i = 0 to ( myHair2.NumRoots() - 1 ) do
		(
			hairRootPoint = myHair2.GetPointT i 0
			DisplaceRoot += hairRootPoint.z
		)
		
		EAssert.AreNearEqual ZeroDisplaceTip DisplaceRoot
	),
	
	function Test_BasicDisplacement = 
	(
		OxResetClassParameters()
		
		-- Create a plane with 1x1 segments (so that there are are no vertices going exactly down the middle)
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode

		modPanel.addModToSelection (Ox_Guides_from_Surface guideLength:10 randomness:0 NumSegs:2)
		local hairFromGuidesModifierInstance = Ox_Hair_from_Guides method:0 viewCountPercentage:100 denseCount:10
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		local verticesWithoutDisplacement = OxGetVertices distributionObject true
		hairFromGuidesModifierInstance.displacementMap = Checker ()
		hairFromGuidesModifierInstance.displacementOffset = 1
		local verticesWithDisplacement = OxGetVertices distributionObject true
		hairFromGuidesModifierInstance.displacementOffset = 2
		local verticesWithDisplacement2 = OxGetVertices distributionObject true
		
		EAssert.ArePointsNotAllNearEqual verticesWithoutDisplacement verticesWithDisplacement
		EAssert.ArePointsNotAllNearEqual verticesWithDisplacement verticesWithDisplacement2
	),
	
	function Test_DisplacementMapChannels = 
	(
		OxResetClassParameters (Ox_Hair_From_Guides())

		-- Create a plane with 1x1 segments (so that there are are no vertices going exactly down the middle)
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on
		max modify mode
		
		-- Create Mapping Channels
		select distributionObject
		for i = 1 to 2 do
		(
			local uvwMapModifier = Uvwmap ()
			uvwMapModifier.mapChannel = i
			uvwMapModifier.length = distributionObject.length / i
			uvwMapModifier.width = distributionObject.width / i
			modPanel.addModToSelection uvwMapModifier
		)

		-- Setup guides with surface comb
		select distributionObject
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection method:2
		guidesFromSurfaceModifier.method = 2
		guidesFromSurfaceModifier.uvChannel = 1
		guidesFromSurfaceModifier.count = 100
		guidesFromSurfaceModifier.guideLength = 10
		
		local hairFromGuidesModifierInstance = Ox_Hair_from_Guides()
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		local displacementMap = Checker ()
		displacementMap.color1 = black
		displacementMap.color2 = white
		hairFromGuidesModifierInstance.displacementMap = displacementMap
		hairFromGuidesModifierInstance.displacementOffset = guidesFromSurfaceModifier.guideLength
		
		--Bake 1-st Mapping Channel
		displacementMap.coords.mapChannel = 1
		guidesCopy1 = instance distributionObject
		convertTo guidesCopy1 HairObject
		myHair1 = guidesCopy1.GetHair()
		
		--Bake 2-nd Mapping Channel
		displacementMap.coords.mapChannel = 2
		guidesCopy2 = instance distributionObject
		convertTo guidesCopy2 HairObject
		myHair2 = guidesCopy2.GetHair()
		
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
	
	function Test_RenderHairCaching = 
	(
		-- Create a plane with 1x1 segments
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode

		-- Setup guides with surface comb
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.NumSegs = 2
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		-- Assign Hair From Guides Modifier
		local hairFromGuidesModifierInstance = Ox_Hair_from_Guides method:0 viewCountPercentage:10 denseCount:100
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		EAssert.IsFalse (hairFromGuidesModifierInstance.GetCacheRenderHairPositions())
			
		hairFromGuidesModifierInstance.rememberPositions = true
			
		EAssert.IsTrue (hairFromGuidesModifierInstance.GetCacheRenderHairPositions())
	),

	function Test_UseMaterialID = 
	(
		-- Create a plane with 1x1 segments
		local distributionObject = Plane lengthsegs:2 widthsegs:1 mapcoords:on
		max modify mode
		select distributionObject
		macros.run "Modifier Stack" "Convert_to_Poly"
		subobjectLevel = 4
		distributionObject.EditablePoly.SetSelection #Face #{1}
		distributionObject.EditablePoly.setMaterialIndex 1 1
		distributionObject.EditablePoly.SetSelection #Face #{2}
		distributionObject.EditablePoly.setMaterialIndex 2 1
		
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.NumSegs = 2
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		local hairFromGuidesModifierInstance = Ox_Hair_from_Guides method:0 viewCountPercentage:10 denseCount:100
		modPanel.addModToSelection hairFromGuidesModifierInstance
		forceCompleteRedraw()
		
		local initialRootCount = OxGetStrandCount distributionObject
			
		hairFromGuidesModifierInstance.useMatID = on
		hairFromGuidesModifierInstance.matID = 2
		forceCompleteRedraw()
		
		local hairCountAfterMaterialId = OxGetStrandCount distributionObject
		EAssert.IsTrue ( hairCountAfterMaterialId < initialRootCount)
	),
	
	-- Tests for a bug which would cause hair to dissapear when material id is used and render settings modifier is applied on top of hair
	function Test_AsGuidesDistributionWithMaterialIDAndRenderSettingsOnTop = 
	(
		-- Create a plane with 1x1 segments
		local distributionObject = Plane lengthsegs:2 widthsegs:1 mapcoords:on
		max modify mode
		select distributionObject
		macros.run "Modifier Stack" "Convert_to_Poly"
		subobjectLevel = 4
		distributionObject.EditablePoly.SetSelection #Face #{1}
		distributionObject.EditablePoly.setMaterialIndex 1 1
		distributionObject.EditablePoly.SetSelection #Face #{2}
		distributionObject.EditablePoly.setMaterialIndex 2 1
		
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.NumSegs = 2
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		local hairFromGuidesModifierInstance = Ox_Hair_from_Guides()
		hairFromGuidesModifierInstance.method = 0
		hairFromGuidesModifierInstance.count = 10
		hairFromGuidesModifierInstance.denseCount = 100
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		local renderSettingsModifierInstance = Ox_Render_Settings()
		modPanel.addModToSelection renderSettingsModifierInstance
		
		-- Set the distribution of hair from guides to "As guides"
		hairFromGuidesModifierInstance.method = 5
		
		EAssert.AreEqual 2 (OxGetStrandPointCount distributionObject 0)
	),
	
	function Test_HairFromGuides = 
	(
		OxResetClassParameters (Ox_Hair_From_Guides())

		-- Create a plane with 1x1 segments
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		-- Add hair from guides which exports guides
		local hairFromGuidesModifierInstance1 = Ox_Hair_from_Guides viewCountPercentage:10 denseCount:100 exportType:1
		modPanel.addModToSelection hairFromGuidesModifierInstance1
		
		-- Add another hair from guides modifier and use its Remember button
		local hairFromGuidesModifierInstance2 = Ox_Hair_from_Guides viewCountPercentage:10 denseCount:100 exportType:0
		modPanel.addModToSelection hairFromGuidesModifierInstance2
		hairFromGuidesModifierInstance2.rememberPositions = true
		
		-- Now change the output guide count in first hair from guides, we should not get an error here
		hairFromGuidesModifierInstance1.denseCount = 5
		
		-- Do a render which will evaluate render hairs
		render()
	),
	
	function Test_StrandGroupsArePreservedFromGuidesInGeneratedHairs =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode
		
		-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface method:4 NumSegs:2
		modPanel.addModToSelection guidesFromSurfaceModifier

		-- Edit guides modifier which sets the first two strands to use group #2
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		editGuidesInstance.useGuideGroups = on

		editGuidesInstance.SelectRoots #{1,2}
		editGuidesInstance.SetSelectedGuidesGroupIndex 2

		editGuidesInstance.SelectRoots #{3,4}
		editGuidesInstance.SetSelectedGuidesGroupIndex 3
		
		-- Hair grom guides modifier, also with per-vertex distribution
		local hairFromGuidesModifierInstance1 = Ox_Hair_from_Guides method:1 viewCountPercentage:100 denseCount:50
		modPanel.addModToSelection hairFromGuidesModifierInstance1

		-- Render settings modifier with global radius of 2 for non-existent group 4
		local renderSettings1 = Ox_Render_Settings radius:2 groupID:4
		modPanel.addModToSelection renderSettings1

		-- Render settings modifier with global radius of 3 for non-existent group 5
		local renderSettings2 = Ox_Render_Settings radius:3 groupID:5
		modPanel.addModToSelection renderSettings2

		-- Mesh from strands modifier should respect the resulting strand widths
		modPanel.addModToSelection (Ox_Mesh_from_Strands type:1 numSides:1)
		
		-- At this point we should have the default widths for all strands since render settings were applied to non-existent groups
		local hairVerticesWithNonExistentGroups = TestUtilities.GetVertexArray distributionObject.mesh
		
		-- Changing first render settings to an existing group should produce different radii for half the hairs
		renderSettings1.groupID = 2
		local hairVerticesWithOneExistentGroup = TestUtilities.GetVertexArray distributionObject.mesh
		EAssert.ArePointsNotAllNearEqual hairVerticesWithNonExistentGroups hairVerticesWithOneExistentGroup epsilon:1
		
		-- Changing second render settings to an existing group should produce different radii for all the hairs
		renderSettings2.groupID = 3
		local hairVerticesWithTwoExistentGroups = TestUtilities.GetVertexArray distributionObject.mesh
		EAssert.ArePointsNotAllNearEqual hairVerticesWithNonExistentGroups hairVerticesWithTwoExistentGroups epsilon:1
		EAssert.ArePointsNotAllNearEqual hairVerticesWithOneExistentGroup hairVerticesWithTwoExistentGroups epsilon:1
	),
	
	function Test_ChangeAdditionalRootDataSettings =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode
		
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface method:4 NumSegs:2
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		local hairFromGuidesModifierInstance = Ox_Hair_from_Guides method:4
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		-- Toggle output settings
		hairFromGuidesModifierInstance.rootTM = off
		hairFromGuidesModifierInstance.surfaceDependency = off
		hairFromGuidesModifierInstance.guideDep = off
		
		-- If no crash happens, we are successful
		EAssert.AreEqual 4 (OxGetStrandCount distributionObject)
	),
	
	function Test_GuideProximity =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:20 length:20 mapcoords:on isSelected:on
		max modify mode
		
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface method:4 NumSegs:2
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		local hairFromGuidesModifierInstance = Ox_Hair_from_Guides method:2 viewCountPercentage:100 denseCount:100
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		local hairCountWithoutGuideProximity = OxGetStrandCount distributionObject
		hairFromGuidesModifierInstance.guideProx = on
		hairFromGuidesModifierInstance.gpDist = 5
		local hairCountWithGuideProximity = OxGetStrandCount distributionObject
		
		EAssert.IsLessThan hairCountWithoutGuideProximity hairCountWithGuideProximity
	),
	
	function Test_HairFromZeroLengthBakedGuidesProducesNonFlickeringResults =
	(
		local distributionObject = Sphere radius:10 mapcoords:on isSelected:on
		local hairObj = copy distributionObject
	
		max modify mode
		local bendModifier = Bend()
		set animate on
		sliderTime = 10f
		bendModifier.BendAngle = 5
		set animate off
		sliderTime = 0f
		modPanel.addModToSelection bendModifier
	
		select hairObj
		
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:6
		guidesFromSurfaceModifier.guideLength = 0.000001
		maxOps.CollapseNode hairObj off
		
		hairObj.distrObj = distributionObject
		hairObj.topoOrient = on
		local hairFromGuidesModifierInstance = Ox_Hair_from_Guides method:4
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		local frizzInstance = Ox_Strand_Frizz amount:50
		modPanel.addModToSelection frizzInstance
		
		-- From frame to frame hair vertices should not be jumping too much
		local verticesAtPreviousFrame = OxGetVertices hairObj true
		for i = 1 to 10 do
		(
			sliderTime += 1f
			local verticesAtCurrentFrame = OxGetVertices hairObj true
			EAssert.ArePointsAllNearEqual verticesAtPreviousFrame verticesAtCurrentFrame useDistance:true epsilon:1
		)
	),
	
	function Test_ChangingRootChannelUpdatesHairFromGuidesOutput =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode
		
		-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()

		-- Edit guides modifier which sets the first two strands to use group #2
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		editGuidesInstance.useGuideGroups = on

		editGuidesInstance.SelectRoots #{1,2}
		editGuidesInstance.SetSelectedGuidesGroupIndex 2

		editGuidesInstance.SelectRoots #{3,4}
		editGuidesInstance.SetSelectedGuidesGroupIndex 3
		
		local hairFromGuidesModifier = Ox_Hair_From_Guides method:4 interpolation:1 useUVCoords:off
		modPanel.addModToSelection hairFromGuidesModifier
		
		-- Render settings modifier with global radius of 2 for group 2
		local renderSettingsModifier = Ox_Render_Settings radius:2 groupID:2
		modPanel.addModToSelection renderSettingsModifier

		-- Render settings modifier with global radius of 3 for group 3
		local renderSettingsModifier = Ox_Render_Settings radius:3 groupID:3
		modPanel.addModToSelection renderSettingsModifier

		-- Mesh from strands modifier should respect the resulting strand widths
		local meshFromStrandsModifier = Ox_Mesh_from_Strands type:1 numSides:1
		modPanel.addModToSelection meshFromStrandsModifier
		
		EAssert.AreNearEqual 4 ( distance distributionObject.mesh.verts[1].position distributionObject.mesh.verts[2].position )
		EAssert.AreNearEqual 6 ( distance distributionObject.mesh.verts[9].position distributionObject.mesh.verts[10].position )
		
		-- Modify the channel values, it should update the resulting mesh
		modPanel.setCurrentObject editGuidesInstance
		subobjectLevel = 1
		editGuidesInstance.SelectRoots #{3,4}
		editGuidesInstance.SetSelectedGuidesGroupIndex 2
		modPanel.setCurrentObject meshFromStrandsModifier
		
		EAssert.AreNearEqual 4 ( distance distributionObject.mesh.verts[1].position distributionObject.mesh.verts[2].position )
		EAssert.AreNearEqual 4 ( distance distributionObject.mesh.verts[9].position distributionObject.mesh.verts[10].position )
	),
	
	function Test_TurnOffPerRootTransformMatrices =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local hairFromGuidesModifier = Ox_Hair_From_Guides method:4 interpolation:1 rootTM:on
		modPanel.addModToSelection hairFromGuidesModifier
		
		local verticesWithPerRootTransforms = OxGetVertices distributionObject true
		hairFromGuidesModifier.rootTM = off
		local verticesWithoutPerRootTransforms = OxGetVertices distributionObject true
		
		EAssert.ArePointsAllNearEqual verticesWithPerRootTransforms verticesWithoutPerRootTransforms
		
		-- To reset default setting
		hairFromGuidesModifier.rootTM = on
	),
	
	function Test_DistributionGuideChannel =
	(
		OxResetClassParameters()
		
		local distributionObject = Plane lengthsegs:6 widthsegs:6 mapcoords:on isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		local pointObject = Point pos:[0,-30,0]
		local generateGuideDataModifierInstance = GenerateGuideDataModifier createNewChannel:on newChannelName:"TestChannel" generationMethod:5 Targetobject:pointObject maximumSourceValue:35 minimumTargetValue:1 maximumTargetValue:0
		modPanel.addModToSelection generateGuideDataModifierInstance
		
		local hairFromGuidesModifier = Ox_Hair_From_Guides method:4
		modPanel.addModToSelection hairFromGuidesModifier
		
		local strandCountBeforeDistributionChannel = OxGetStrandCount distributionObject
		hairFromGuidesModifier.distrChan = 2
		local strandCountAfterDistributionChannel = OxGetStrandCount distributionObject
		
		EAssert.IsGreaterThan strandCountAfterDistributionChannel strandCountBeforeDistributionChannel
		
		-- Reset defaults
		hairFromGuidesModifier.distrChan = 0
	),
	
	function Test_DistributionGuideChannelCorrectInterpolation =
	(
		OxResetClassParameters (Sphere mapcoords:on)
		OxResetClassParameters (GenerateGuideDataModifier())
		OxResetClassParameters (Ox_Hair_From_Guides())
					
		local distributionObject = Sphere radius:10 segs:20 mapcoords:on isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		local generateGuideDataModifierInstance = GenerateGuideDataModifier createNewChannel:on newChannelName:"TestChannel" generationMethod:0 minimumTargetValue:0.4
		modPanel.addModToSelection generateGuideDataModifierInstance
		
		local hairFromGuidesModifier = Ox_Hair_From_Guides method:4
		modPanel.addModToSelection hairFromGuidesModifier
		
		-- Bounding box before and after changing distribution values
		local boundingBoxWithFullHair = TestUtilities.GetPointsBoundingBox (OxGetVertices distributionObject true)
		hairFromGuidesModifier.distrChan = 2
		local boundingBoxWithPartialHair = TestUtilities.GetPointsBoundingBox (OxGetVertices distributionObject true)
		
		-- The difference in bounding box corners shouldn't be huge since hairs should be generated all around the sphere in both cases
		EAssert.ArePointsNearEqual boundingBoxWithFullHair[1] boundingBoxWithPartialHair[1] epsilon:10 useDistance:true
		EAssert.ArePointsNearEqual boundingBoxWithFullHair[2] boundingBoxWithPartialHair[2] epsilon:10 useDistance:true
	),
	
	function Test_ChangeInterpolationCountWithRememberedRoots =
	(
		OxResetClassParameters (Sphere mapcoords:on)
		OxResetClassParameters (Ox_Hair_From_Guides())
			
		local distributionObject = Sphere mapcoords:on isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		-- Set interpolation type to affine and interpolation count to 2
		local hairFromGuidesModifier = Ox_Hair_From_Guides method:2 viewCountPercentage:100 denseCount:10 interpNum:2 interpolation:1
		modPanel.addModToSelection hairFromGuidesModifier
		
		-- Remember roots so that hairs and their interpolations are cached
		hairFromGuidesModifier.rememberPositions = on
			
		-- Change interpolation count to 3. This should not cause any errors or a crash due to only 2 guides being used for interpolation earlier.
		hairFromGuidesModifier.interpNum = 3
	),
	
	function Test_PolarInterpolationSetsCorrectStrandRotation =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		-- Add Edit Guides modifier and move all 4 strand tips in the same direction
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance

		subobjectLevel = 2
		editGuidesInstance.SelectRoots #{1,2,3,4}
		editGuidesInstance.MoveSelectedControlPoints [10,0,0]
		
		-- Add HairFromGuidesModifier with polar interpolation
		local hairFromGuidesModifier = Ox_Hair_From_Guides method:2 viewCountPercentage:100 denseCount:10 interpNum:1 interpolation:0 guideProx:off
		modPanel.addModToSelection hairFromGuidesModifier
		
		-- All hairs should be slanted in the same direction if they're oriented correctly
		local hairVertices = OxGetVertices distributionObject true
		local firstStrandDirection = hairVertices[2] - hairVertices[1]
		firstStrandDirection.z = 0
		for i = 1 to (hairVertices.Count / 2) do
		(
			local currentStrandDirection = hairVertices[i * 2] - hairVertices[i * 2 - 1]
			currentStrandDirection.z = 0
			EAssert.IsGreaterThan 0 (dot currentStrandDirection firstStrandDirection)
		)
	),
	
	function Test_UVGenerationMethodOnMeshWithoutUVs =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:off isSelected:on
		max modify mode
		gfm = Ox_Guides_From_Surface method:2 count:5
		gfm.surfaceTangentComputeMethod = 1
		modPanel.addModToSelection gfm
		modPanel.addModToSelection (Ox_Hair_From_Guides method:2 viewCountPercentage:100 denseCount:10 useUVCoords:on)
		
		-- If there was no crash we are good
		OxResetClassParameters (Plane mapcoords:on)
		OxResetClassParameters (Ox_Guides_From_Surface())
		OxResetClassParameters (Ox_Hair_From_Guides())
	),
	
	function Test_ExportGuidesProducesConsistentResultsWithMultiThreading =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on
		max modify mode
		local guidesFromMesh = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		guidesFromMesh.rememberPositions = on
		
		-- Add surface comb to spread hairs apart
		modPanel.addModToSelection (Ox_Surface_Comb())
		
		local hairFromGuidesModifier = Ox_Hair_From_Guides method:4 viewCountPercentage:1 exportType:0
		modPanel.addModToSelection hairFromGuidesModifier
			
		local verticesAsHair = OxGetVertices distributionObject true
		hairFromGuidesModifier.exportType = 1
			
		-- Switching to guide export shouldn't modify the vertices
		local verticesAsGuides = OxGetVertices distributionObject true
		EAssert.ArePointsAllNearEqual verticesAsHair verticesAsGuides
			
		-- Switch the distribution back and forth, vertices shouldn't be changed
			
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Guides_From_Surface())
		OxResetClassParameters (Ox_Hair_From_Guides())
	),
	
	function Test_ChangeInputGuidesDistribution =
	(
		OxResetClassParameters (Ox_Guides_From_Surface())
			
		local distributionObject = Plane lengthsegs:10 widthsegs:10 mapcoords:on isSelected:on
		max modify mode
		local guideLength = 5
		local guidesFromSurface = Ox_Guides_From_Surface method:2 count:5 NumSegs:2 randomness:0 guideLength:guideLength
		modPanel.addModToSelection guidesFromSurface
		
		local denseStrandCount = 100
		modPanel.addModToSelection (Ox_Hair_From_Guides method:2 viewCountPercentage:100 denseCount:denseStrandCount guideProx:off)
		
		guidesFromSurface.method = 1
		
		EAssert.AreEqual denseStrandCount (OxGetStrandCount distributionObject)
		
		local vertices = OxGetVertices distributionObject true
		
		-- All Strands must have had shapes generated
		for i = 1 to denseStrandCount do
		(
			EAssert.AreNearEqual guideLength (distance vertices[(i - 1) * 2 + 1] vertices[(i - 1) * 2 + 2])
		)
	),
	
	-- Tests that if input guides are animated then the resulting hair should also be animated
	function Test_RespectGuideAnimation =
	(
		OxResetClassParameters (Ox_Guides_From_Surface())
		OxResetClassParameters (Ox_Hair_From_Guides())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_From_Surface method:4 NumSegs:2 randomness:0)
		modPanel.addModToSelection (Noisemodifier strength: [10,10,10] scale:10 animate:on)
		modPanel.addModToSelection (Ox_Hair_From_Guides method:4)
		
		sliderTime = 0f
		local verticesAtFrame0 = OxGetVertices distributionObject true
		
		sliderTime = 20f
		local verticesAtFrame20 = OxGetVertices distributionObject true
		
		EAssert.ArePointsNotAllNearEqual verticesAtFrame0 verticesAtFrame20
	),
	
	function Test_PostRenderHairDoesntChangeAtFullPreview =
	(
		local cylinderHair = TestUtilities.AddBendingCylinder()
		
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4 numSegs:2 guideLength:10 randomness:0)
		local hairFromGuides = Ox_Hair_From_Guides method:7 denseCount:100 viewCountPercentage:100
		modPanel.addModToSelection hairFromGuides
		
		sliderTime = 10f
		TestUtilities.RenderAndTestHairsForChanges cylinderHair
	),
	
	-- Tests for a crash when hair is switched to face coordinates and we get an infinite recursion during a texture coordinate query
	function Test_GenericFaceDistributionOnTeapot =
	(
		local distributionObject = Teapot mapcoords:on isSelected:on
		max modify mode
		modPanel.addModToSelection (Ox_Guides_From_Surface method:4 NumSegs:2 randomness:0)
		modPanel.addModToSelection (Ox_Hair_From_Guides method:6)
		
		-- Should not crash here
	),
	
	function TestRootsDontChangeOnEditedMesh distributionMethod =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:5 length:5 mapcoords:on isSelected:on
		max modify mode
		
		local editPolyModifier = Edit_Poly()
		modPanel.addModToSelection editPolyModifier
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4)
		modPanel.addModToSelection (Ox_Hair_From_Guides method:distributionMethod)
		
		local surfaceDependenciesBeforeDeformation = OxGetSurfaceDependencies distributionObject
		
		modPanel.setCurrentObject editPolyModifier
		subobjectLevel = 1
		editPolyModifier.SetSelection #Vertex #{}
		editPolyModifier.Select #Vertex #{3}
		editPolyModifier.SetOperation #Transform
		editPolyModifier.MoveSelection [-5,5,0] parent:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [0,0,0])
		editPolyModifier.Commit()

		modPanel.setCurrentObject distributionObject.modifiers[1]		
		local surfaceDependenciesAfterDeformation = OxGetSurfaceDependencies distributionObject
		
		EAssert.AreArraysEqual surfaceDependenciesBeforeDeformation surfaceDependenciesAfterDeformation
	),
	
	function Test_RootsDontChangeOnEditedMesh_Even =
	(
		TestRootsDontChangeOnEditedMesh 7
	),
	
	function Test_LoadV6WithVertexColorDistributionChannel =
	(
		EUnit.SkipBeforeMaxVersion 2020

		units.SystemType = #Centimeters
		local testSceneFilePath = pathConfig.convertPathToAbsolute (getSourceFileName() + "..\\..\\..\\..\\..\\..\\..\\..\\" ) + "External\\TestData\\OxMax6-Max2020-HFGWithVertexColorDistribution.max"
		loadMaxFile testSceneFilePath useFileUnit:true quiet:true
		
		local strandCount = OxGetStrandCount $FurBall
		EAssert.AreEqual 393 strandCount
		--EAssert.AreNearEqual 401 strandCount epsilon:5
		
		-- Cleanup
		resetMaxFile #noPrompt
	),
	
	function Test_LoadV7WithVertexColorDistributionChannelOnSphere =
	(
		EUnit.SkipBeforeMaxVersion 2020

		units.SystemType = #Centimeters
		local testSceneFilePath = pathConfig.convertPathToAbsolute (getSourceFileName() + "..\\..\\..\\..\\..\\..\\..\\..\\" ) + "External\\TestData\\OxMax7-Max2020-HFGWithVertexColorDistributionSphere.max"
		loadMaxFile testSceneFilePath useFileUnit:true quiet:true
		
		local strandCount = OxGetStrandCount $FurBall
		EAssert.AreEqual 3 strandCount
		
		local vertices = OxGetVertices $FurBall true
		
		EAssert.ArePointsAllNearEqual vertices #([0,0,30],
[1.40353e-07,0,35.438],
[2.80705e-07,0,40.876],
[4.21058e-07,0,46.314],
[5.6141e-07,0,51.752],
[7.01763e-07,0,57.19],
[8.42116e-07,0,62.628],
[9.82468e-07,0,68.066],
[1.12282e-06,0,73.5041],
[1.26317e-06,0,78.9421],
[-1.31134e-06,30,-1.31134e-06],
[-1.91523e-06,34.6796,-1.5529e-06],
[-2.51912e-06,39.3591,-1.79445e-06],
[-3.12301e-06,44.0387,-2.03601e-06],
[-3.7269e-06,48.7183,-2.27756e-06],
[-4.33079e-06,53.3979,-2.51912e-06],
[-4.93468e-06,58.0774,-2.76068e-06],
[-5.53856e-06,62.757,-3.00223e-06],
[-6.14245e-06,67.4366,-3.24379e-06],
[-6.74634e-06,72.1162,-3.48534e-06],
[0,0,-30],
[-1.36653e-07,1.36653e-07,-35.2947],
[-2.73307e-07,2.73306e-07,-40.5894],
[-4.0996e-07,4.0996e-07,-45.884],
[-5.46613e-07,5.46613e-07,-51.1787],
[-6.83266e-07,6.83266e-07,-56.4734],
[-8.1992e-07,8.1992e-07,-61.7681],
[-9.56573e-07,9.56573e-07,-67.0627],
[-1.09323e-06,1.09323e-06,-72.3574],
[-1.22988e-06,1.22988e-06,-77.6521] )
		
		-- Cleanup
		resetMaxFile #noPrompt
	),
	
	-- Reproduces #5733
	function Test_HairOnGroundedStrands =
	(
		OxResetClassParameters ()
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on
		local splines = line isSelected:on
		addNewSpline splines
		addNewSpline splines
		addNewSpline splines
		addKnot splines 1 #corner #line [-5,-5,0]
		addKnot splines 1 #corner #line [-5,-5,10]
		addKnot splines 2 #corner #line [0,5,0]
		addKnot splines 2 #corner #line [0,5,10]
		addKnot splines 3 #corner #line [5,-5,0]
		addKnot splines 3 #corner #line [5,-5,10]
		updateShape splines
		
		max modify mode
		
		modPanel.addModToSelection (Ox_Guides_From_Shape pointCount:2)
		
		local groundStrandsModifierInstance = Ox_Ground_Strands surface:distributionObject topoOrient:on
		modPanel.addModToSelection groundStrandsModifierInstance
		groundStrandsModifierInstance.detachRoots = off
		
		local hairFromGuidesModifier = Ox_Hair_From_Guides method:4
		modPanel.addModToSelection hairFromGuidesModifier
		
		-- 4 hairs should have been generated
		EAssert.AreEqual 4 (OxGetStrandCount splines)
	),
	
	function Test_VertexColorDistributionOnFirstMappingChannel = 
	(
		EUnit.SkipBeforeMaxVersion 2024

		units.SystemType = #Centimeters
		local testSceneFilePath = pathConfig.convertPathToAbsolute (getSourceFileName() + "..\\..\\..\\..\\..\\..\\..\\..\\" ) + "External\\TestData\\OxMax8-Max2024-HFGWithVertexColorDistributionSphere.max"
		loadMaxFile testSceneFilePath useFileUnit:true quiet:true
		
		EAssert.AreEqual 9 (OxGetStrandCount $FurBall)
	)
)

EUnit.AddTestClass Test_HairFromGuidesModifier
ok
