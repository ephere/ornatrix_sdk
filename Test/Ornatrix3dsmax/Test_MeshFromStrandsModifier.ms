struct Test_MeshFromStrandsModifier
(
	function TestCleanup =
	(
		delete $objects
	),
	
	function SetupSceneWithTwoProxyObjects addUvwMap:false addPlaneProxy:true =
	(
		-- Create a plane with 3x3 segments (so that there are are no vertices going exactly down the middle)
		local distributionObject = Plane lengthsegs:3 widthsegs:3 isSelected:on

		max modify mode
		
		if addUvwMap then
		(
			modPanel.addModToSelection (Uvwmap ())
		)

		-- Add guides from surface modifier and make sure we'll use vertices to generate strands, so that we always have a predictable strand count
		modPanel.addModToSelection (Ox_Guides_from_Surface method:4)

		-- Add "Ox Mesh From Strands" modifier
		local meshFromStrandsModifierInstance = Ox_Mesh_from_Strands pstrUV:-1 inheritUV:off type:2 strandGroup:0
		modPanel.addModToSelection meshFromStrandsModifierInstance

		-- Add the two proxy objects
		-- Create a simple box and another plane, we'll use these as proxy meshes
		local boxProxy = Box lengthsegs:1 widthsegs:1 heightsegs:1 length:1.5
		meshFromStrandsModifierInstance.addproxyobject boxProxy
		
		if( addPlaneProxy ) then
		(
			local planeProxy = Plane lengthsegs:1 widthsegs:1
			meshFromStrandsModifierInstance.addproxyobject planeProxy
		)
		
		#(distributionObject, meshFromStrandsModifierInstance, boxProxy)
	),
	
	function AreAllUvsPresentInMesh sourceMesh targetMesh sourceMappingChannel:1 targetMappingChannel:1 =
	(
		local areAllUvCoordinatesInResult = true
		for i = 1 to (meshop.getNumMapVerts sourceMesh sourceMappingChannel) do
		(
			local resultContainsCurrentUVCoord = false
			for j = 1 to (meshop.getNumMapVerts  targetMesh targetMappingChannel) do
			(
				if ((distance (meshop.getMapVert targetMesh targetMappingChannel j) (meshop.getMapVert sourceMesh sourceMappingChannel i)) < 0.01) then
				(
					resultContainsCurrentUVCoord = true
				)
			)
			
			if (not resultContainsCurrentUVCoord) then
			(
				areAllUvCoordinatesInResult = false
			)
		)
		
		areAllUvCoordinatesInResult
	),
	
	-- Tests that a distribution map can be assigned to control which proxy mesh is used for which strand in Mesh From Strands modifier
	function Test_ProxyDistributionMap =
	(
		local result = SetupSceneWithTwoProxyObjects()
		-- Assign a new checkers texture to the proxy map slot
		result[2].proxyDistributionMap = Checker()
			
		completeRedraw()

		-- TODO: Add some assertions
	),
	
	-- Tests that a distribution map can be assigned to control which proxy mesh is used for which strand in Mesh From Strands modifier
	function Test_ProxyDistributionMapOnNonZeroChannel =
	(
		local result = SetupSceneWithTwoProxyObjects addUvwMap:true
		local distributionObject = result[1]
		local guidesFromMesh = distributionObject.Modifiers[#Ox_Guides_from_Surface]
		local meshFromStrandsModifierInstance = result[2]
		local uvwMapModifier = distributionObject.Modifiers[#UVW_Map]
		-- Set box map type
		uvwMapModifier.maptype = 4
		uvwMapModifier.mapChannel = 2
		uvwMapModifier.length = 25.5
		uvwMapModifier.width = 25.5
		
		-- Force guide regeneration, so texture coordinates will be recreated
		-- TODO: This should not be needed, find a way to automatically detect incoming texture coordinate changes in GFM
		guidesFromMesh.method = 4
		
		local initialVertexCount = NumPoints distributionObject
		local checkerMap = Checker()
		-- Set the input map channel to match the one we set with UVW map
		checkerMap.coords.mapChannel = 2
		meshFromStrandsModifierInstance.proxyDistributionMap = checkerMap
		local vertexCountAfterMap = NumPoints distributionObject
		EAssert.AreNotEqual initialVertexCount vertexCountAfterMap
		
		checkerMap.coords.mapChannel = 1
		local vertexCountAfterMapChannelChange = NumPoints distributionObject
		EAssert.AreNotEqual vertexCountAfterMap vertexCountAfterMapChannelChange
		
		-- Make sure distribution will be recomputed by changing value back and forth
		checkerMap.coords.mapChannel = 1
		
		-- When both map channel and uvw modifier channel are the same again we should get the same result as when they were both 1
		uvwMapModifier.mapChannel = 1
		
		-- Force guide regeneration, so texture coordinates will be recreated
		-- TODO: This should not be needed, find a way to automatically detect incoming texture coordinate changes in GFM
		guidesFromMesh.method = 4
		
		local vertexCountAfterUvwChannelChange = NumPoints distributionObject
		EAssert.AreEqual vertexCountAfterMap vertexCountAfterUvwChannelChange
	),

	function Test_NoCrashWithProxyInheritUVOnBeforeApplyingModifier =
	(
		local distributionObject = Plane lengthsegs:3 widthsegs:3
		local boxProxy = Box lengthsegs:1 widthsegs:1 heightsegs:1

		max modify mode

		select distributionObject
		local guidesFromSurfaceModifierInstance = Ox_Guides_from_Surface()
		modPanel.addModToSelection guidesFromSurfaceModifierInstance
		guidesFromSurfaceModifierInstance.method = 4

		local meshFromStrandsModifierInstance = TestUtilities.AddDefaultMeshFromStrandsModifierToSelection()
		-- Set inherit UV on before adding the modifier, this shouldn't crash Max afterwards
		meshFromStrandsModifierInstance.inheritUV = on
		meshFromStrandsModifierInstance.type = 2
		meshFromStrandsModifierInstance.addproxyobject boxProxy
	),

	-- When Edit Normals modifier is applied to a proxy object and normals are edited, the custom normals should also appear in the resulting hair mesh
	function Test_ProxyCustomNormalsAreCopied =
	(
		local distributionObject = Plane()
		distributionObject.width = 50
		distributionObject.length = 50
		
		local proxyPlane = Plane lengthsegs:1 widthsegs:1
		proxyPlane.width = 50
		proxyPlane.length = 50

		max modify mode

		-- Edit the proxy plane normals to all point up
		select proxyPlane
		modPanel.addModToSelection (Edit_Normals())
		local editNormalsModifier = $.modifiers[1]
		editNormalsModifier.Select #{1..4}
		editNormalsModifier.Move [100, 100, 0]

		-- Set up the mesh from strands modifier
		-- Add guides from surface modifier and make sure we'll use vertices to generate strands, so that we always have a predictable strand count
		select distributionObject
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		TestUtilities.SetGuidesFromMeshDefaults guidesFromSurfaceModifier
		guidesFromSurfaceModifier.count = 1
		modPanel.addModToSelection guidesFromSurfaceModifier

		-- Add "Ox Mesh From Strands" modifier
		local meshFromStrandsModifierInstance = TestUtilities.AddDefaultMeshFromStrandsModifierToSelection()
		meshFromStrandsModifierInstance.type = 2
		meshFromStrandsModifierInstance.addproxyobject proxyPlane
		meshFromStrandsModifierInstance.uniScale = off

		-- Make sure the normals in the resulting mesh are the same as the normals in the proxy mesh
		modPanel.addModToSelection (Edit_Normals())
		resultEditNormalsModifier = $.modifiers[#Edit_Normals]
		local firstNormal = resultEditNormalsModifier.getnormal 1

		-- The resulting vector should not point directly up, down, etc.
		EAssert.AreNotNearEqual 0.0 firstNormal.x
		EAssert.AreNotNearEqual 1.0 firstNormal.x
		EAssert.AreNotNearEqual 0.0 firstNormal.y
		EAssert.AreNotNearEqual 1.0 firstNormal.y
		EAssert.AreNotNearEqual 0.0 firstNormal.z
		EAssert.AreNotNearEqual 1.0 firstNormal.z
	),
	
	function Test_ProxySmoothingGroupsArePreserved =
	(
		local result = SetupSceneWithTwoProxyObjects addPlaneProxy:false
		local distributionObject = result[1]
		local boxProxy = result[3]
		
		convertTo distributionObject Editable_Poly
		convertTo boxProxy Editable_Poly
		
		for i = 1 to (polyop.getNumFaces boxProxy) do
		(
			EAssert.AreEqual (polyop.getFaceSmoothGroup distributionObject i) (polyop.getFaceSmoothGroup boxProxy i)
		)
	),

	function Test_TwistChannel =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1

		max modify mode

		-- Set up the mesh from strands modifier, use per vertex distribution
		select distributionObject
		
		-- Guides from surface modifier with vertex distribution and 2 points per strand
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		local editGuidesModifier = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesModifier

		-- Create a twist channel
		local twistChannelIndex = editGuidesModifier.CreateChannel true "TwistChannel"
		editGuidesModifier.SelectRoots #{1..4}
		
		-- Add strand rotate modifier which will modify the actual twist values
		local strandRotateInstance = Ox_Rotate_Strands()
		strandRotateInstance.angle = 180
		strandRotateInstance.angleChannel = ( TestUtilities.GetRootChannelIdentifier twistChannelIndex )
		modPanel.addModToSelection strandRotateInstance

		-- Add mesh from strands modifier with bollboard setting and apply the twist channel to twist
		local meshFromStrandsModifier = TestUtilities.AddDefaultMeshFromStrandsModifierToSelection()
		meshFromStrandsModifier.numSides = 1 
		meshFromStrandsModifier.type = 1

		-- Copy resulting mesh vertices before the twist
		verticesBeforeTwist = TestUtilities.GetVertexArray distributionObject.mesh
		
		modPanel.setCurrentObject editGuidesModifier
		subobjectLevel = 1
		editGuidesModifier.SetChannelValueToSelection true twistChannelIndex 0.2
		
		modPanel.setCurrentObject meshFromStrandsModifier
		verticesAfterTwist = TestUtilities.GetVertexArray distributionObject.mesh
		
		-- Make sure that, after we applied the twist channel value, the vertex positions have changed
		for i = 1 to verticesBeforeTwist.count do
		(
			EAssert.AreNotNearEqual verticesBeforeTwist[i].x verticesAfterTwist[i].x
			EAssert.AreNotNearEqual verticesBeforeTwist[i].y verticesAfterTwist[i].y
			-- The Z coordinate will remain the same since twist is only along one axis
		)
	),
	
	function Test_RenderSettingsWidthControlsProxyUniformWidth =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Render_Settings())
		OxResetClassParameters (Box())
		delete $objects
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on

		max modify mode

		-- Set up the mesh from strands modifier, use per vertex distribution
		
		-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.NumSegs = 2
		guidesFromSurfaceModifier.randomness = 0
		guidesFromSurfaceModifier.guideLength = 1
		
		-- Render settings modifier with global radius of 2
		local renderSettingsModifier = Ox_Render_Settings radius:2
		modPanel.addModToSelection renderSettingsModifier
		
		-- Mesh from strands modifier with a box proxyobject and uniform scale on
		local meshFromStrandsModifier = TestUtilities.AddDefaultMeshFromStrandsModifierToSelection()
		meshFromStrandsModifier.type = 2
		meshFromStrandsModifier.uniScale = on
		
		local boxProxy = Box lengthsegs:1 widthsegs:1 heightsegs:1
		meshFromStrandsModifier.addproxyobject boxProxy
		
		-- Measure the resulting proxy mesh box by comparing the first and fifth vertex Z coordinate, it should have the size of 2
		EAssert.AreNearEqual 4 ( distance( distributionObject.mesh.vertices[1].position ) ( distributionObject.mesh.vertices[2].position ) )
		
		-- Change the render settings width to 0.5 and make sure the resulting box sizes change
		renderSettingsModifier.radius = 0.5
		EAssert.AreNearEqual 1 ( distance( distributionObject.mesh.vertices[1].position ) ( distributionObject.mesh.vertices[2].position ) )
	),
	
	function Test_PerStrandUVsOffAndInheritUVsWithProxyMeshes =
	(
		-- Reset used class defaults
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Mesh_from_Strands())
		delete $objects
				
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on

		max modify mode

		-- Set up the mesh from strands modifier, use per vertex distribution		
		-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		modPanel.addModToSelection guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.NumSegs = 2
		
		-- Mesh from strands modifier with a box proxyobject and uniform scale on
		local meshFromStrandsModifierInstance = TestUtilities.AddDefaultMeshFromStrandsModifierToSelection()
		meshFromStrandsModifierInstance.type = 2
			
		local boxProxy = Box lengthsegs:1 widthsegs:1 heightsegs:1 mapcoords: on
		convertTo boxProxy Editable_Poly
		meshFromStrandsModifierInstance.addproxyobject boxProxy
		
		meshFromStrandsModifierInstance.pstrUV = -2
		
		-- First test with inherit UVs on
		meshFromStrandsModifierInstance.inheritUV = on
		
		-- Resulting mesh should have UV vertices from the distribution object for each of the four generated strands
		EAssert.AreEqual ((polyop.getNumMapVerts boxProxy 1) * 4) (polyop.getNumMapVerts distributionObject 1)
		
		-- Compare vertices of the first strand to the proxy mesh
		for i = 1 to (polyop.getNumMapVerts boxProxy 1) do
		(
			EAssert.ArePointsNearEqual (polyop.getMapVert boxProxy 1 i) (polyop.getMapVert distributionObject 1 i)
		)
		
		-- Now turn inherit UVs off. The result should have one UV coordinate for the whole mesh.
		meshFromStrandsModifierInstance.inheritUV = off
		EAssert.AreEqual 1 (polyop.getNumMaps distributionObject)
	),
	
	function Test_RibbonHairExportGuides =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on

		max modify mode
		
		-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
		local guidesFromSurfaceModifier =  TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.NumSegs = 2
		
		-- Mesh from strands modifier with a box proxyobject and uniform scale on
		local meshFromStrandsModifierInstance = Ox_Mesh_from_Strands type:1 guideMeshExp:on strandGroup:0
		modPanel.addModToSelection meshFromStrandsModifierInstance
		
		local guidesFromGuideMeshModifierInstance = Ox_Guides_from_Guide_Mesh()
		modPanel.addModToSelection guidesFromGuideMeshModifierInstance
		
		-- Make sure the mesh is converted back into guides
		local guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		EAssert.AreEqual 4 (((guidesCopy.GetGuides()).AsHair()).numRoots())
		delete guidesCopy
	),

	function Test_PrismaticHairExportGuides =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Guides_from_Guide_Mesh())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1

		max modify mode

		-- Set up the mesh from strands modifier, use per vertex distribution
		select distributionObject
		
		-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
		local guidesFromSurfaceModifier = Ox_Guides_from_Surface()
		modPanel.addModToSelection guidesFromSurfaceModifier
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.NumSegs = 2
		
		-- Mesh from strands modifier with a box proxyobject and uniform scale on
		local meshFromStrandsModifierInstance = TestUtilities.AddDefaultMeshFromStrandsModifierToSelection type:0
		meshFromStrandsModifierInstance.guideMeshExp = on
		
		local guidesFromGuideMeshModifierInstance = Ox_Guides_from_Guide_Mesh()
		modPanel.addModToSelection guidesFromGuideMeshModifierInstance
		
		-- Make sure the mesh is converted back into guides
		local guidesCopy = instance distributionObject
		convertTo guidesCopy GuidesObject
		EAssert.AreEqual 4 (((guidesCopy.GetGuides()).AsHair()).numRoots())
		delete guidesCopy
	),
	
	function Test_PerStrandCoordinatesGenerateUCoordinateAlongStrand =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Guides_from_Surface())
		OxResetClassParameters (Ox_Render_Settings())
		OxResetClassParameters (Ox_Mesh_from_Strands())
			
		local distributionObject = Plane lengthsegs:1 widthsegs:1 mapcoords:on isSelected:on

		max modify mode

		-- Set up the mesh from strands modifier, use per vertex distribution
		
		-- Guides from surface modifier with vertex distribution and 2 points per strand, no randomness in length, and strand length of 1
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		guidesFromSurfaceModifier.method = 4
		guidesFromSurfaceModifier.NumSegs = 2
		
		-- Render settings modifier should taper off the end of the strands which should also have an effect on UVs
		local renderSettingsInstance = Ox_Render_Settings radius:5
		modPanel.addModToSelection renderSettingsInstance
		
		-- TODO: Verify that the tapering occurs by manually specifying diagram points or choosing a preset
		
		-- Mesh from strands modifier
		local meshFromStrandsModifierInstance = Ox_Mesh_from_Strands type:1 numSides:1 guideMeshExp:off pstrUV:-1 strandGroup:0
		modPanel.addModToSelection meshFromStrandsModifierInstance
	
		EAssert.AreNotEqual 0 (polyop.getNumMapVerts distributionObject 1)
		EAssert.ArePointsNearEqual [0,0,0] (polyop.getMapVert distributionObject 1 1)
		EAssert.ArePointsNearEqual [1,0,0] (polyop.getMapVert distributionObject 1 2)
		EAssert.ArePointsNearEqual [0.5,1,0] (polyop.getMapVert distributionObject 1 3)
		EAssert.ArePointsNearEqual [0.5,1,0] (polyop.getMapVert distributionObject 1 4)
	),

	function Test_PerStrandCoordinatesGenerateUCoordinateAlongStrandWithMeshStrips =
	(
		local stripObject1 = Plane lengthsegs:1 widthsegs:1
		local hairObjectInstance = HairFromMeshStripsObject hairsPerStrip:1 viewPercentage:100 pointsPerStrand:2 isSelected:on
		hairObjectInstance.AddStripObject stripObject1

		max modify mode

		-- Set up the mesh from strands modifier, use per vertex distribution		
		local meshFromStrandsModifierInstance = Ox_Mesh_From_Strands type:1 guideMeshExp:off numSides:1 pstrUV:-1 strandGroup:0
		modPanel.addModToSelection meshFromStrandsModifierInstance
		
		EAssert.ArePointsNearEqual [0,0,0] (polyop.getMapVert hairObjectInstance 1 1)
		EAssert.ArePointsNearEqual [1,0,0] (polyop.getMapVert hairObjectInstance 1 2)
		EAssert.ArePointsNearEqual [0,1,0] (polyop.getMapVert hairObjectInstance 1 3)
		EAssert.ArePointsNearEqual [1,1,0] (polyop.getMapVert hairObjectInstance 1 4)
	),
	
	function Test_ProxyMeshShapeDeformedAlongStrands =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1 width:10 length:10
		local proxyObject = Box lengthsegs:1 widthsegs:1 heightsegs:5 width:0.1 length:0.1 height:10
		
		max modify mode
		select distributionObject
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:6
		
		local surfaceCombModifierInstance = Ox_Surface_Comb()
		modPanel.addModToSelection surfaceCombModifierInstance
		
		-- Get the points for the first strand in object coordinates
		local firstStrandPoints = #()
		for i = 1 to ( OxGetStrandPointCount distributionObject 0 ) do
		(
			append firstStrandPoints ( OxGetStrandPoint distributionObject 0 ( i - 1 ) true )
		)
		
		local meshFromStrandsModifierInstance = TestUtilities.AddDefaultMeshFromStrandsModifierToSelection type:2
		meshFromStrandsModifierInstance.proxyMesh = #(proxyObject)
		
		-- Each vertex in the first proxy mesh should be no further from its associated strand point than a specified threshold
		for i = 1 to firstStrandPoints.count do
		(
			local firstStrandPoint = firstStrandPoints[i]
			local closeVertexCount = 0
			for vertexIndex = 1 to distributionObject.mesh.vertices.count do
			(
				local distanceToStrand = distance firstStrandPoint distributionObject.mesh.vertices[vertexIndex].position
				if distanceToStrand < 0.5 then
				(
					closeVertexCount += 1
				)
			)
			
			EAssert.AreEqual 4 closeVertexCount
		)
	),
	
	function Test_RenderProxyHairWithoutPerStrandUVs =
	(
		local distributionObject = Plane lengthsegs:1 widthsegs:1
		local proxyObject = Plane lengthsegs:1 widthsegs:1
		
		max modify mode
		select distributionObject
		local guidesFromSurfaceModifier = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local meshFromStrandsModifierInstance = TestUtilities.AddDefaultMeshFromStrandsModifierToSelection type:2
		meshFromStrandsModifierInstance.proxyMesh = #(proxyObject)
		meshFromStrandsModifierInstance.pstrUV = -2
		
		Render()
		
		-- There should be no error message or crash during rendering
	),
	
	function Test_RandomProxyMeshDistribution =
	(
		OxResetClassParameters( Box() )
		OxResetClassParameters( Uvwmap() )
		OxResetClassParameters( Plane() )
		OxResetClassParameters( Ox_Guides_From_Surface() )
		OxResetClassParameters( Ox_Mesh_from_Strands() )
		
		max modify mode
		
		-- Create a simple box and another plane, we'll use these as proxy meshes. Add randomly sized UV maps to ensure they will have unique UV coordinates.
		local boxProxy = Box lengthsegs:1 widthsegs:1 heightsegs:1 isSelected:on
		local boxUvMapModifier = Uvwmap maptype:0 length:109 height:12
		modPanel.addModToSelection boxUvMapModifier
		
		local planeProxy = Plane lengthsegs:1 widthsegs:1 isSelected:on
		local planeUvMapModifier = Uvwmap maptype:0 length:11 height:81
		modPanel.addModToSelection planeUvMapModifier
		
		-- Create a plane with 3x3 segments (so that there are are no vertices going exactly down the middle)
		local distributionObject = Plane lengthsegs:3 widthsegs:3 isSelected:on

		-- Add guides from surface modifier and make sure we'll use vertices to generate strands, so that we always have a predictable strand count
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local strandCount = OxGetStrandCount distributionObject
		
		-- Add "Ox Mesh From Strands" modifier
		local meshFromStrandsModifierInstance = Ox_Mesh_from_Strands type:2 pstrUV:-2 inheritUV:on strandGroup:0
		modPanel.addModToSelection meshFromStrandsModifierInstance

		-- Add the two proxy objects
		meshFromStrandsModifierInstance.addproxyobject boxProxy
		meshFromStrandsModifierInstance.addproxyobject planeProxy
		
		-- Because proxy objects are distributed randomly we should have a mix of them. We check these by comparing vertex counts.
		EAssert.AreNotEqual ((boxProxy.mesh.numverts) * strandCount) (distributionObject.mesh.numverts)
		EAssert.AreNotEqual ((planeProxy.mesh.numverts) * strandCount) (distributionObject.mesh.numverts)
		
		-- Make sure that all texture coordinates from both box and plane are present somewhere in the result		
		EAssert.IsTrue (AreAllUvsPresentInMesh (boxProxy.mesh) (distributionObject.mesh))
		EAssert.IsTrue (AreAllUvsPresentInMesh (planeProxy.mesh) (distributionObject.mesh))
	),
	
	function Test_CylindricalMeshNormalsOrientation =
	(
		max modify mode
		local distributionObject = Plane width:1 length:1 lengthsegs:1 widthsegs:1 isSelected:on
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local renderSettings =Ox_Render_Settings radius:2
		modPanel.addModToSelection renderSettings
		local meshFromStrandsModifierInstance = Ox_Mesh_from_Strands type:0 numSides:4 guideMeshExp:off strandGroup:0
		modPanel.addModToSelection meshFromStrandsModifierInstance
		
		-- Get the normals of first three vertices, they should be facing away from one another
		local initialDistance = 0.0
		local distanceOffsetByNormals = 0.0
		local previousVertex = [0,0,0]
		local previousVertexOffsetByNormal = [0,0,0]
		for i = 1 to 4 do
		(
			if (i > 1) then
			(
				initialDistance += distance previousVertex (distributionObject.verts[i].pos)
				distanceOffsetByNormals += distance previousVertexOffsetByNormal ((distributionObject.verts[i].pos) + (getNormal distributionObject.mesh i))
			)
			
			previousVertex = (distributionObject.verts[i].pos)
			previousVertexOffsetByNormal = ((distributionObject.verts[i].pos) + (getNormal distributionObject.mesh i))
		)
		
		EAssert.IsLessThan distanceOffsetByNormals initialDistance
	),
	
	function Test_BillboardCrossSectionSegments =
	(
		OxResetClassParameters (Plane())
		OxResetClassParameters (Ox_Guides_From_Surface())
		OxResetClassParameters (Ox_Mesh_from_Strands())
			
		max modify mode
		local distributionObject = Plane width:2 length:2 lengthsegs:1 widthsegs:1 isSelected:on
		local vertexCount = distributionObject.mesh.verts.count
		local lengthSegmentCount = 2
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		local sideCount = 2
		local meshFromStrandsModifierInstance = Ox_Mesh_from_Strands type:1 numSides:sideCount guideMeshExp:off
		modPanel.addModToSelection meshFromStrandsModifierInstance
		
		local guidesMesh = distributionObject.mesh
		EAssert.AreEqual (vertexCount * lengthSegmentCount * ( sideCount + 1 )) (guidesMesh.verts.count)
	),
	
	function Test_RenderedBillboardHair =
	(
		OxResetClassParameters()
		max modify mode
		local distributionObject = Plane width:2 length:2 lengthsegs:1 widthsegs:1 isSelected:on
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		modPanel.addModToSelection (Ox_Render_Settings radius:10)
		modPanel.addModToSelection (Ox_Mesh_from_Strands type:1 numSides:1 guideMeshExp:off strandGroup:0)
		
		-- We should see the hair during the render
		local renderResult = render outputwidth:50 outputheight:50
		
		-- Make sure that bitmap isn't completely black (we have hairs in there)
		EAssert.IsFalse (TestUtilities.AreAllPixelsBlack renderResult)
		
		-- Cleanup
		unDisplay renderResult
	),
	
	function CreateMeshFromStrandsWithTwoStrandGroups =
	(
		max modify mode
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()

		-- Edit guides modifier which sets the first two strands to use group #2
		local editGuidesInstance = Ox_Edit_Guides()
		modPanel.addModToSelection editGuidesInstance
		editGuidesInstance.useGuideGroups = on

		editGuidesInstance.SelectRoots #{1,2}
		editGuidesInstance.SetSelectedGuidesGroupIndex 2

		editGuidesInstance.SelectRoots #{3,4}
		editGuidesInstance.SetSelectedGuidesGroupIndex 3
		
		local meshFromStrands = Ox_Mesh_from_Strands type:1 numSides:1 guideMeshExp:off strandGroup:0
		modPanel.addModToSelection meshFromStrands
		
		return distributionObject
	),
	
	function Test_MeshedStrandGroup =
	(
		local distributionObject = CreateMeshFromStrandsWithTwoStrandGroups()
		local meshFromStrands = distributionObject.modifiers[#Ox_Mesh_from_Strands]
		
		-- By default all strands are meshed (group 0)
		EAssert.AreEqual (4 * 4) (distributionObject.mesh.verts.count)
		
		-- No strands have group index 1
		meshFromStrands.strandGroup = 1
		EAssert.AreEqual 0 (distributionObject.mesh.verts.count)
		EAssert.AreEqual 0 (distributionObject.mesh.faces.count)
		
		-- Changing strand group to 2 should only mesh first two strands, ignoring the other two
		meshFromStrands.strandGroup = 2
		EAssert.AreEqual (2 * 4) (distributionObject.mesh.verts.count)
		EAssert.AreEqual (2 * 2) (distributionObject.mesh.faces.count)
		local group2Vertices = TestUtilities.GetVertexArray distributionObject.mesh
		
		-- Changing group to 3 should produce two strands, but they are different from previous group
		meshFromStrands.strandGroup = 3
		EAssert.AreEqual (2 * 4) (distributionObject.mesh.verts.count)
		EAssert.AreEqual (2 * 2) (distributionObject.mesh.faces.count)
		local group3Vertices = TestUtilities.GetVertexArray distributionObject.mesh
		
		EAssert.ArePointsAllNotNearEqual group2Vertices group3Vertices
	),
	
	function Test_StrandGroupsWithProxy =
	(
		local distributionObject = CreateMeshFromStrandsWithTwoStrandGroups()
		local meshFromStrands = distributionObject.modifiers[#Ox_Mesh_from_Strands]
		
		local boxProxy = Box lengthsegs:1 widthsegs:1 heightsegs:1 isSelected:on
		meshFromStrands.addproxyobject boxProxy
		meshFromStrands.type = 2
		local proxyMeshVertexCount = boxProxy.mesh.verts.count
		local proxyMeshFaceCount = boxProxy.mesh.faces.count
		
		-- By default all strands are meshed (group 0)
		EAssert.AreEqual (4 * proxyMeshVertexCount) (distributionObject.mesh.verts.count)
		
		-- No strands have group index 1
		meshFromStrands.strandGroup = 1
		EAssert.AreEqual 0 (distributionObject.mesh.verts.count)
		EAssert.AreEqual 0 (distributionObject.mesh.faces.count)
		
		-- Changing strand group to 2 should only mesh first two strands, ignoring the other two
		meshFromStrands.strandGroup = 2
		EAssert.AreEqual (2 * proxyMeshVertexCount) (distributionObject.mesh.verts.count)
		EAssert.AreEqual (2 * proxyMeshFaceCount) (distributionObject.mesh.faces.count)
		local group2Vertices = TestUtilities.GetVertexArray distributionObject.mesh
		
		-- Changing group to 3 should produce two strands, but they are different from previous group
		meshFromStrands.strandGroup = 3
		EAssert.AreEqual (2 * proxyMeshVertexCount) (distributionObject.mesh.verts.count)
		EAssert.AreEqual (2 * proxyMeshFaceCount) (distributionObject.mesh.faces.count)
		local group3Vertices = TestUtilities.GetVertexArray distributionObject.mesh
		
		EAssert.ArePointsAllNotNearEqual group2Vertices group3Vertices
	),
	
	function Test_CreateTextureAtlas =
	(
		OxResetClassParameters (Ox_Mesh_From_Strands())
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		
		-- Mesh from strands modifier with a box proxyobject and uniform scale on
		local meshFromStrandsModifierInstance = TestUtilities.AddDefaultMeshFromStrandsModifierToSelection()
		meshFromStrandsModifierInstance.type = 1
		meshFromStrandsModifierInstance.pstrUV = -3
		
		-- Without texture atlas enabled texture coordinates for each strand will be the same
		meshFromStrandsModifierInstance.createTextureAtlas[1] = off
		
		-- Go over each strand's texture vertices and make sure they match
		for i = 1 to 4 do
		(
			for j = 0 to 2 do
			(
				EAssert.ArePointsNearEqual (polyop.getMapVert distributionObject 1 (j * 4 + i)) (polyop.getMapVert distributionObject 1 ( (j + 1) * 4 + i))
			)
		)
		
		-- With texture atlas no two texture points should be the same
		meshFromStrandsModifierInstance.createTextureAtlas[1] = on
		local textureVertexCount = polyop.getNumMapVerts distributionObject 1

		for i = 1 to textureVertexCount do
		(
			for j = 1 to textureVertexCount do
			(
				if i != j then EAssert.ArePointsNotNearEqual (polyop.getMapVert distributionObject 1 i) (polyop.getMapVert distributionObject 1 j)
			)
		)
	),
	
	function Test_TextureMapRadiusAlongWCoordinate =
	(
		OxResetClassParameters (Ox_Mesh_From_Strands())
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:5
		
		local mapInstance = Gradient_Ramp()
		modPanel.addModToSelection (Ox_Render_Settings thicknessMap:mapInstance radius:10)
		
		local meshFromStrandsModifierInstance = TestUtilities.AddDefaultMeshFromStrandsModifierToSelection()
		
		local verticesWithUVMapping = TestUtilities.GetVertexArray distributionObject
		mapInstance.coordinates.UVW_Type  = 2
		local verticesWithUWMapping = TestUtilities.GetVertexArray distributionObject
		
		EAssert.ArePointsNotAllNearEqual verticesWithUVMapping verticesWithUWMapping
	),
	
	function Test_ProxyMeshWithNegativeZValues =
	(
		OxResetClassParameters (Ox_Mesh_From_Strands())
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		local guidesFromMesh = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection pointCount:3
		guidesFromMesh.guideLength = 5
		
		modPanel.addModToSelection (Ox_Surface_Comb minimumBendValue:-0.2 maximumBendValue:0.2 )
	
		local meshFromStrandsModifierInstance = Ox_Mesh_from_Strands type:2 pstrUV:-2 inheritUV:on strandGroup:0
		modPanel.addModToSelection meshFromStrandsModifierInstance
		
		cylinderHeightSegmentCount = 10
		local proxyMesh = Cylinder heightsegs:cylinderHeightSegmentCount sides:3 radius:0.1 isSelected:on

		-- Add the two proxy objects
		meshFromStrandsModifierInstance.addproxyobject proxyMesh
		
		cylinderFaceCount = cylinderHeightSegmentCount * 3 + 2
		
		editPoly = Edit_Poly ()
		modPanel.addModToSelection editPoly ui:on
		subobjectLevel = 5
		editPoly.SetSelection #Face #{}
		editPoly.Select #Face #{1..cylinderFaceCount}
		actionMan.executeAction 0 "50001"  -- Tools: Select and Move
		editPoly.SetOperation #Transform
		editPoly.MoveSelection  [0,0,-3] axis:(transMatrix [0.0000,0.0000,12.5000])
		editPoly.Commit ()
		
		-- No consecutive points should be too far from one another
		local vertices = TestUtilities.GetVertexArray distributionObject
		local lastVertex = vertices[1]
		for i=2 to (3 * cylinderHeightSegmentCount) do
		(
			local currentVertex = vertices[i]
			EAssert.ArePointsNearEqual lastVertex currentVertex epsilon:0.6 useDistance:true
			lastVertex = currentVertex
		)
	),
	
	function GetFaceMaterialIds mesh =
	(
		local result = #()
		for i = 1 to (getNumFaces mesh) do
		(
			append result (getFaceMatID mesh i)
		)
		
		result
	),
	
	function Test_MaterialIdScatterMap =
	(
		OxResetClassParameters()
		
		local distributionObject = Plane lengthsegs:1 widthsegs:1 isSelected:on
		max modify mode
		local guidesFromMesh = TestUtilities.AddDefaultGuidesFromSurfaceModifierToSelection()
		local meshFromStrandsModifierInstance = Ox_Mesh_from_Strands type:1 numMatIDs:2 mapIdStrGroup:off
		modPanel.addModToSelection meshFromStrandsModifierInstance
		
		local materialIdsWithoutScatterMap = GetFaceMaterialIds (distributionObject.mesh)
		meshFromStrandsModifierInstance.scatMap = Checker()
		local materialIdsWithScatterMap = GetFaceMaterialIds (distributionObject.mesh)
		
		EAssert.AreNotAllEqual materialIdsWithoutScatterMap materialIdsWithScatterMap
	)
)

EUnit.AddTestClass Test_MeshFromStrandsModifier
ok
