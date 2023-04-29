struct Test_HairClusteringModifier
(
	function TestCleanup =
	(
		delete $objects
	),
	
	-- Prevents regression when clustering modifier popped randomly due to parallel evaluation and cluster strands themselves being clustered
	function disabled_TooSlow_Test_NoFlickeringWithMultithreadedEvaluation =
	(
		-- Create a hair strips object on a default plane without rotating the strands to make sure they are generated along the Y axis
		local hairObjectInstance = Plane lengthsegs:5 widthsegs:5

		select hairObjectInstance
		max modify mode
		
		local noiseModifierInstance = NoiseModifier()
		noiseModifierInstance.strength = [0,0,0]
		noiseModifierInstance.animate = on
		modPanel.addModToSelection noiseModifierInstance

		local guidesFromSurfaceModifier = Ox_Guides_From_Surface()
		guidesFromSurfaceModifier.method = 4
		modPanel.addModToSelection guidesFromSurfaceModifier
		
		local hairFromGuidesModifierInstance = Ox_Hair_From_Guides()
		hairFromGuidesModifierInstance.count = 100
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		local curlingModifierInstance = Ox_Strand_Curling()
		modPanel.addModToSelection curlingModifierInstance
		
		local strandDetailModifier = Ox_Strand_Detail()
		strandDetailModifier.viewCount = 200
		modPanel.addModToSelection strandDetailModifier
		
		local clusteringModifierInstance = Ox_Hair_Clustering()
		clusteringModifierInstance.genCount = 10
		modPanel.addModToSelection clusteringModifierInstance

		local originalPoints = #()
		for i = 1 to (numPoints hairObjectInstance) do
		(
			append originalPoints (getPointPos hairObjectInstance i)
		)
		
		-- Move slider a bunch of times and make sure that all points in the object stay in place, not flicker
		for time = 1 to 5 do
		(
			sliderTime += 1
			
			for i = 1 to (numPoints hairObjectInstance) do
			(
				EAssert.ArePointsNearEqual originalPoints[i] (getPointPos hairObjectInstance i)
			)
		)
	),

	-- Tests that generated clusters are persisted with 3dsmax scene
	function Test_ClustersPreservedWithScene =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Guides_From_Surface())
		OxResetClassParameters (Ox_Hair_From_Guides())
		OxResetClassParameters (Ox_Hair_Clustering())
			
		-- Create a plane with 2x2 segments so we have 9 vertices all together
		local distributionObject = Plane lengthsegs:2 widthsegs:2
		local distributionObjectName = distributionObject.Name

		max modify mode

		-- Add strand propagation
		select distributionObject
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local hairFromGuidesModifierInstance = Ox_Hair_From_Guides viewCountPercentage:100 denseCount:100
		modPanel.addModToSelection hairFromGuidesModifierInstance
		hairFromGuidesModifierInstance.rememberRootPositions = on
			
		local clusteringModifierInstance = Ox_Hair_Clustering genCount:10
		modPanel.addModToSelection clusteringModifierInstance
		
		local verticesBeforeSave = OxGetVertices distributionObject true
		
		-- Save and reload the scene
		holdMaxFile()
		fetchMaxFile quiet:true
		
		distributionObject = GetNodeByName distributionObjectName
		local verticesAfterLoad = OxGetVertices distributionObject true
		
		-- Make sure the vertices haven't changed
		for i = 1 to verticesBeforeSave.count do
		(
			EAssert.ArePointsNearEqual verticesBeforeSave[i] verticesAfterLoad[i]
		)
	),
	
	function Test_RenderClustersAreUsedWhenRendering =
	(
		local distributionObject = Plane lengthsegs:2 widthsegs:2 isSelected:on
		
		max modify mode
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local hairFromGuidesModifierInstance = Ox_Hair_From_Guides viewCountPercentage:50 denseCount:10
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		-- Record the vertex positions of viewport and render hairs before clustering modifier
		local viewportVerticesBeforeClustering = OxGetVertices distributionObject true
		OxSetDisplayRenderHairInViewports true true
		local renderVerticesBeforeClustering = OxGetVertices distributionObject true
		OxSetDisplayRenderHairInViewports false true
		
		-- Add clustering modifier
		local clusteringModifierInstance = Ox_Hair_Clustering genCount:3 thresh:100
		modPanel.addModToSelection clusteringModifierInstance
		
		-- Make sure some viewport vertices have changed
		local viewportVerticesAfterClustering = OxGetVertices distributionObject true
		local haveSomeVerticesChanged = false
		for i = 1 to viewportVerticesBeforeClustering.count do
			if( distance (viewportVerticesBeforeClustering[i]) (viewportVerticesAfterClustering[i]) ) > 0.01 then haveSomeVerticesChanged = true
		
		EAssert.IsTrue haveSomeVerticesChanged
		
		-- Switch to render mode and make sure vertices have changed
		OxSetDisplayRenderHairInViewports true true
		local renderVerticesAfterClustering = OxGetVertices distributionObject true
		haveSomeVerticesChanged = false
		for i = 1 to renderVerticesBeforeClustering.count do
			if( distance (renderVerticesBeforeClustering[i]) (renderVerticesAfterClustering[i]) ) > 0.01 then haveSomeVerticesChanged = true
		
		OxSetDisplayRenderHairInViewports false true
		EAssert.IsTrue haveSomeVerticesChanged
	),
	
	function AddHairClusteringOntoAnimatedMesh =
	(
		local distributionObject = Plane lengthsegs:2 widthsegs:2 isSelected:on
			
		max modify mode
		
		local meltModifierInstance = Melt Melt_Amount:12
		modPanel.addModToSelection meltModifierInstance
		
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local hairFromGuidesModifierInstance = Ox_Hair_From_Guides viewCountPercentage:100 denseCount:10
		modPanel.addModToSelection hairFromGuidesModifierInstance
		
		local clusteringModifierInstance = Ox_Hair_Clustering genCount:3 thresh:10
		modPanel.addModToSelection clusteringModifierInstance
		
		#(distributionObject, clusteringModifierInstance, meltModifierInstance, hairFromGuidesModifierInstance)
	),
	
	-- Ensures that strands will not be popping when clustered with a low threshold on a deforming surface
	function Test_ClusterThresholdDoesNotCauseStrandReassignmentOnDeformingGeometry =
	(
		local params = AddHairClusteringOntoAnimatedMesh()
		local distributionObject = params[1]
		local meltModifierInstance = params[3]
		
		local verticesBeforeDeformation = OxGetVertices distributionObject true
		
		-- Changing the melt amount will slightly make the plane wider, increasing spacing between hair roots
		meltModifierInstance.Melt_Amount = 16
		local verticesAfterDeformation = OxGetVertices distributionObject true
		
		-- Because we didn't move the roots too much the vertices should all be near equal, unless threshold was not precomputed with clusters
		EAssert.ArePointsAllNearEqual verticesBeforeDeformation verticesAfterDeformation epsilon:1
	),
	
	function Test_RenderVsViewportClusters =
	(
		local params = AddHairClusteringOntoAnimatedMesh()
		local distributionObject = params[1]
		local clusteringModifierInstance = params[2]
		local meltModifierInstance = params[3]
		local hairFromGuidesModifierInstance = params[4]
		
		--clusteringModifierInstance.enabled = off
		OxGetVertices distributionObject true
		hairFromGuidesModifierInstance.rememberPositions = on
		
		local meshFromStrandsInstance = Ox_Mesh_from_Strands()
		modPanel.addModToSelection meshFromStrandsInstance
		
		set animate on
		sliderTime = 0f
		meltModifierInstance.Melt_Amount = 0
		sliderTime = 5f
		meltModifierInstance.Melt_Amount = 100
		set animate off
			
		sliderTime = 3f
		meshFromStrandsInstance.enabled = off
		local verticesBeforeRendering = OxGetVertices distributionObject true
		meshFromStrandsInstance.enabled = on
		render()
		meshFromStrandsInstance.enabled = off
		local verticesAfterRendering = OxGetVertices distributionObject true
		meshFromStrandsInstance.enabled = on
		
		EAssert.ArePointsAllNearEqual verticesBeforeRendering verticesAfterRendering
		sliderTime = 1f
		render()
		sliderTime = 3f
		meshFromStrandsInstance.enabled = off
		local verticesAfterRenderingAndFrameSwitch = OxGetVertices distributionObject true
		meshFromStrandsInstance.enabled = on
		
		EAssert.ArePointsAllNearEqual verticesAfterRendering verticesAfterRenderingAndFrameSwitch
	),
	
	function Test_ClustersRemainAfterRememberClustersTurnedOn =
	(
		OxResetClassParameters (Ox_Hair_From_Guides())
		OxResetClassParameters (Ox_Hair_Clustering())
				
		local distributionObject = Plane lengthsegs:2 widthsegs:2 isSelected:on
		
		max modify mode
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local hairFromGuidesModifierInstance = Ox_Hair_From_Guides viewCountPercentage:50 denseCount:40
		modPanel.addModToSelection hairFromGuidesModifierInstance
		hairFromGuidesModifierInstance.rememberRootPositions = on
		
		-- Record the vertex positions of viewport hairs
		local viewportVerticesBeforeClustering = OxGetVertices distributionObject true
		
		local clusteringModifierInstance = Ox_Hair_Clustering genCount:5
		modPanel.addModToSelection clusteringModifierInstance
		
		local viewportVerticesAfterClustering = OxGetVertices distributionObject true
		
		-- Turn on the remember clusters option
		clusteringModifierInstance.useRenderCache = on
			
		-- The clustering should have been applied after remembering clusters, in viewport mode
		local verticesWithCacheOn = OxGetVertices distributionObject true
		EAssert.ArePointsNotAllNearEqual viewportVerticesBeforeClustering verticesWithCacheOn
		EAssert.ArePointsAllNearEqual viewportVerticesAfterClustering verticesWithCacheOn
	)
)

EUnit.AddTestClass Test_HairClusteringModifier
ok
