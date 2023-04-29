import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt
import unittest
import os

class Test_MeshFromStrandsNode( MayaTest.OxTestCase ):

	def test_BillboardMesh( self ):
		plane = pm.polyPlane()
		pm.select( plane )

		# Create hair shape with 100 hairs
		guidesShape = TestUtilities.AddGuidesToMesh( plane[0] )
		#plane[0].subdivisionsWidth.set( 1 )
		#plane[0].subdivisionsHeight.set( 1 )
		meshFromStrandsNode = pm.createNode( TestUtilities.MeshFromStrandsNodeName )
		guidesShape.outputHair >> meshFromStrandsNode.inputStrands
		plane[0].outMesh >> meshFromStrandsNode.distributionMesh

		meshShape = pm.createNode( 'mesh' )
		meshFromStrandsNode.outputMesh >> meshShape.inMesh

		# Check vertices, there should be two vertices for every hair vertex
		self.assertEqual( pm.mel.OxGetVertexCount( guidesShape ) * 2, len( meshShape.vtx ) )

		# Check UV coordinates
		self.assertEqual( ( 0, 0 ), meshShape.getPolygonUV( 0, 0 ) )
		self.assertEqual( ( 0, 0 ), meshShape.getPolygonUV( 0, 1 ) )
		self.assertEqual( ( 0, 0 ), meshShape.getPolygonUV( 0, 2 ) )
		self.assertEqual( ( 0, 0 ), meshShape.getPolygonUV( 0, 3 ) )
		self.assertLess( abs( 0.1 - meshShape.getPolygonUV( 1, 0 )[0] ), 0.000001, meshShape.getPolygonUV( 1, 0 ) )

		# Check faces, there should be one long polygon face for every hair strand
		# TODO: This needs to be implemented
		#self.assertEqual( pm.mel.OxGetStrandCount( guidesShape ), len( meshShape.f ) )

	# Tests that mesh from strands can also be applied to a hair object as well as guides and can be generated without a distribution object
	def test_BillboardMeshFromHairWithoutDistribution( self ):
		plane = pm.polyPlane()
		pm.select( plane )

		# Create hair shape with 100 hairs
		hairShape = TestUtilities.AddHairToMesh( plane[0], rootGenerationMethod = 4 )

		meshFromStrandsNode = pm.createNode( TestUtilities.MeshFromStrandsNodeName )
		hairShape.outputHair >> meshFromStrandsNode.inputStrands
		#plane[0].outMesh >> meshFromStrandsNode.distributionMesh

		meshShape = pm.createNode( 'mesh' )
		meshFromStrandsNode.outputMesh >> meshShape.inMesh

		# Check vertices, there should be two vertices for every hair vertex
		self.assertEqual( pm.mel.OxGetVertexCount( hairShape ) * 2, len( meshShape.vtx ) )

	def test_CyllindricalMesh( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()

		meshFromStrandsNode = pm.createNode( TestUtilities.MeshFromStrandsNodeName )
		meshFromStrandsNode.meshType.set( 0 )
		meshFromStrandsNode.sideCount.set( 3 )
		guidesShape.outputHair >> meshFromStrandsNode.inputStrands

		meshShape = pm.createNode( 'mesh' )
		meshFromStrandsNode.outputMesh >> meshShape.inMesh

		# With 3 sides there should be 3 times the vertices in the resulting mesh shape
		self.assertEqual( pm.mel.OxGetVertexCount( guidesShape ) * 3, len( meshShape.vtx ) )

	def AssertMeshDerivedUvCoordinates( self, meshShape, strandIds, uvSet = None ):
		strandIndexByIds = {}
		for i in range( 0, len( strandIds ) ):
			strandIndexByIds[strandIds[i]] = i
		self.assertEqual( ( 0, 0 ), meshShape.getPolygonUV( strandIndexByIds[0], 0, uvSet ) )
		self.assertEqual( ( 0, 0 ), meshShape.getPolygonUV( strandIndexByIds[0], 1, uvSet ) )
		self.assertEqual( ( 0, 0 ), meshShape.getPolygonUV( strandIndexByIds[0], 2, uvSet ) )
		self.assertEqual( ( 0, 0 ), meshShape.getPolygonUV( strandIndexByIds[0], 3, uvSet ) )

		self.assertEqual( ( 1, 0 ), meshShape.getPolygonUV( strandIndexByIds[1], 0, uvSet ) )
		self.assertEqual( ( 1, 0 ), meshShape.getPolygonUV( strandIndexByIds[1], 1, uvSet ) )
		self.assertEqual( ( 1, 0 ), meshShape.getPolygonUV( strandIndexByIds[1], 2, uvSet ) )
		self.assertEqual( ( 1, 0 ), meshShape.getPolygonUV( strandIndexByIds[1], 3, uvSet ) )

		self.assertEqual( ( 0, 1 ), meshShape.getPolygonUV( strandIndexByIds[2], 0, uvSet ) )
		self.assertEqual( ( 0, 1 ), meshShape.getPolygonUV( strandIndexByIds[2], 1, uvSet ) )
		self.assertEqual( ( 0, 1 ), meshShape.getPolygonUV( strandIndexByIds[2], 2, uvSet ) )
		self.assertEqual( ( 0, 1 ), meshShape.getPolygonUV( strandIndexByIds[2], 3, uvSet ) )

		self.assertEqual( ( 1, 1 ), meshShape.getPolygonUV( strandIndexByIds[3], 0, uvSet ) )
		self.assertEqual( ( 1, 1 ), meshShape.getPolygonUV( strandIndexByIds[3], 1, uvSet ) )
		self.assertEqual( ( 1, 1 ), meshShape.getPolygonUV( strandIndexByIds[3], 2, uvSet ) )
		self.assertEqual( ( 1, 1 ), meshShape.getPolygonUV( strandIndexByIds[3], 3, uvSet ) )

	def AssertPerVertexUvCoordinates( self, meshShape, faceIndex, uvSet = None ):
		self.assertEqual( ( 1, 1 ), meshShape.getPolygonUV( faceIndex, 0, uvSet ) )
		self.assertEqual( ( 0, 1 ), meshShape.getPolygonUV( faceIndex, 1, uvSet ) )
		self.assertEqual( ( 0, 0 ), meshShape.getPolygonUV( faceIndex, 2, uvSet ) )
		self.assertEqual( ( 1, 0 ), meshShape.getPolygonUV( faceIndex, 3, uvSet ) )

	def AssertPerSegmentUvCoordinates( self, meshShape, faceIndex, uvSet = None ):
		self.assertEqual( ( 0, 0 ), meshShape.getPolygonUV( faceIndex, 0, uvSet ) )
		self.assertEqual( ( 0, 0 ), meshShape.getPolygonUV( faceIndex, 1, uvSet ) )
		self.assertEqual( ( 0, 1 ), meshShape.getPolygonUV( faceIndex, 2, uvSet ) )
		self.assertEqual( ( 0, 1 ), meshShape.getPolygonUV( faceIndex, 3, uvSet ) )

	# Tests proper generation of texture coordinates for billboard meshes
	def test_BillboardTextureCoordinates( self ):
		plane = pm.polyPlane( sx = 1, sy = 1 )
		pm.select( plane )
		guidesShape = TestUtilities.AddGuidesToMesh( plane[0] )

		strandIds = pm.mel.OxGetStrandIds( guidesShape )

		meshFromStrandsNode = pm.createNode( TestUtilities.MeshFromStrandsNodeName )
		meshFromStrandsNode.meshType.set( 1 )
		guidesShape.outputHair >> meshFromStrandsNode.inputStrands
		plane[0].outMesh >> meshFromStrandsNode.distributionMesh

		meshShape = pm.createNode( 'mesh' )
		meshFromStrandsNode.outputMesh >> meshShape.inMesh

		self.AssertMeshDerivedUvCoordinates( meshShape, strandIds )

	def test_BasicProxyMesh( self ):
		distributionObject = pm.polyPlane( sx = 1, sy = 1 ) #pm.polySphere( r = 1, sx = 1, sy = 1, cuv = 2, ch = 1 )
		proxyObject = pm.polyPlane( sx = 1, sy = 1 ) #pm.polyCylinder( r = 1, h = 2, sx = 1, sy = 1, sz = 1, rcp = 0, cuv = 3, ch = 1 )

		pm.select( distributionObject )
		pm.mel.OxQuickHair()
		guidesFromMeshNode = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		hairFromGuidesNode = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		# User per-vertex distribution to get a predictable vertex count
		guidesFromMeshNode.distribution.set( 4 )
		guidesFromMeshNode.lengthRandomness.set( 0 )
		hairFromGuidesNode.distribution.set( 4 )

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		hairShapeName = hairShape.name()
		strandCount = pm.mel.OxGetStrandCount( hairShape )

		pm.mel.OxAddStrandOperator( hairShape, TestUtilities.MeshFromStrandsNodeName )
		meshFromStrandsNode = pm.ls( type = TestUtilities.MeshFromStrandsNodeName )[0]
		pm.connectAttr( proxyObject[0] + ".outMesh", meshFromStrandsNode + ".proxyMeshes[0]" )
		#pm.mel.OxUpdateProxyMeshList( meshFromStrandsNode + ".proxyMeshes" )

		# Set output type to 'proxy'
		meshFromStrandsNode.meshType.set( 2 )
		hairMeshShape = pm.ls( hairShapeName )[0]

		self.assertEqual( strandCount * len( proxyObject[0].vtx ), len( hairMeshShape.vtx ) )
		self.assertEqual( strandCount * 4, pm.polyEvaluate( hairMeshShape, uvcoord = True ) )

		# No 4 consecutive vertices should be in the same place, but they should be same distance relative to the vertices
		for vertexIndex in range(0, 3):
			for strandIndex in range(1, 3):
				distance = hairMeshShape.vtx[( strandIndex - 1 ) * 4 + vertexIndex].getPosition().distanceTo( hairMeshShape.vtx[strandIndex * 4 + vertexIndex].getPosition() )
				self.assertAlmostEqual( 1, distance )

		proxyMeshVertexIndices = proxyObject[0].getPolygonVertices( 0 )
		# Now ensure vertices are connected properly, each vertex should only be present in one face
		for strandIndex in range(0, 4):
			resultVertexIndices = hairMeshShape.getPolygonVertices( strandIndex )
			for vertexIndex in range(0, 4):
				self.assertEqual( proxyMeshVertexIndices[vertexIndex] + strandIndex * 4, resultVertexIndices[vertexIndex] )

		# Try changing the coordinate mapping, no crashes should be happening
		meshFromStrandsNode.usePerStrandMapping.set( [1], type = "Int32Array" )
		pm.polyEvaluate( hairMeshShape, uvcoord = True )

		# Remove the proxy mesh and verify that the resulting hair mesh now has no verts generated
		pm.removeMultiInstance( meshFromStrandsNode + ".proxyMeshes[0]", b = True )
		self.assertEqual( 0, pm.polyEvaluate( hairMeshShape, vertex = True ) )

	def test_ProxyMeshCrash( self ):
		# Crash described in https://ephere.com:3000/issues/4018
		distributionObject = pm.polyPlane( sx = 1, sy = 1 )
		# With a proxy sphere the mesh has both quads and triangles, which triggers the crash
		proxyObject = pm.polySphere( r = 1, sx = 1, sy = 1, cuv = 2, ch = 1 )

		pm.select( distributionObject )
		pm.mel.OxQuickHair()
		guidesFromMeshNode = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		hairFromGuidesNode = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		# User per-vertex distribution to get a predictable vertex count
		guidesFromMeshNode.distribution.set( 4 )
		guidesFromMeshNode.lengthRandomness.set( 0 )
		hairFromGuidesNode.distribution.set( 4 )

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		hairShapeName = hairShape.name()
		strandCount = pm.mel.OxGetStrandCount( hairShape )

		pm.mel.OxAddStrandOperator( hairShape, TestUtilities.MeshFromStrandsNodeName )
		meshFromStrandsNode = pm.ls( type = TestUtilities.MeshFromStrandsNodeName )[0]
		pm.connectAttr( proxyObject[0] + ".outMesh", meshFromStrandsNode + ".proxyMeshes[0]" )
		#pm.mel.OxUpdateProxyMeshList( meshFromStrandsNode + ".proxyMeshes" )

		# Set output type to 'proxy'
		meshFromStrandsNode.meshType.set( 2 )
		hairMeshShape = pm.ls( hairShapeName )[0]

		# Accessing the object triggers the crash
		self.assertEqual( strandCount * len( proxyObject[0].vtx ), len( hairMeshShape.vtx ) )
		# self.assertEqual( strandCount * 4, pm.polyEvaluate( hairMeshShape, uvcoord = True ) )

	#@unittest.skipIf(
	#	MayaTest.OsIsMac or MayaTest.OsIsWindows and MayaTest.ApiVersion == 201700 and MayaTest.InBatchMode,
	#	"For an unknown reason this test fails occasionally on Mac and crashes on Maya2017/Windows/mayapy (passes in GUI)" )
	@unittest.skip( "This feature is deprecated. Run manually to see if it's still working" )
	def test_nCloth( self ):
		pm.namespace( add = 'Test' )
		pm.namespace( set = 'Test' )
		pm.mel.source( "AEMeshFromStrandsNodeTemplate.mel" )

		distributionObject = pm.polyPlane( sx = 1, sy = 1 )

		pm.select( distributionObject )
		pm.mel.OxQuickHair()
		pm.setAttr( "Test:HairFromGuides1.distribution", 4 )

		meshFromStrands = pm.PyNode( pm.mel.OxAddStrandOperator( "", TestUtilities.MeshFromStrandsNodeName ) )
		meshFromStrands.isGuideMesh.set( True )
		pm.mel.OxCreateNClothSimulation( meshFromStrands )
		self.assertTrue( pm.objExists( 'Test:Hair1Temp' ) )

		pm.select( distributionObject[0].getShape() )
		clusterHandle = pm.cluster()[1]

		pm.currentTime( 1 )
		pm.animation.setKeyframe( clusterHandle, attribute = "tz" )
		pm.animation.setKeyframe( clusterHandle, attribute = "tz", value = 5, time = "1sec" )

		positionBefore = pm.mel.OxGetStrandPointInObjectCoordinates( "Test:HairShape1", 0, 0 )
		pm.playbackOptions( maxTime = 100 )
		pm.select( "Test:nCloth1" )
		pm.mel.doCreateNclothCache( 5, ["2", "1", "10", "OneFile", "1", "","0","","0", "add", "1", "1", "1","0","1","mcx"] )
		positionAfter = pm.mel.OxGetStrandPointInObjectCoordinates( "Test:HairShape1", 0, 0 )

		self.assertAlmostEqual( positionAfter[2] - 5.0, positionBefore[2], 1 )

	def test_SingleOutputMappingChannel( self ):
		plane = pm.polyPlane( sx = 1, sy = 1 )
		pm.select( plane )
		guidesShape = TestUtilities.AddGuidesToMesh( plane[0] )

		strandIds = pm.mel.OxGetStrandIds( guidesShape )

		meshFromStrandsNode = pm.createNode( TestUtilities.MeshFromStrandsNodeName )
		meshFromStrandsNode.meshType.set( 1 )
		guidesShape.outputHair >> meshFromStrandsNode.inputStrands
		plane[0].outMesh >> meshFromStrandsNode.distributionMesh

		meshShape = pm.createNode( 'mesh' )
		meshFromStrandsNode.outputMesh >> meshShape.inMesh

		# Test per-vertex mapping
		meshFromStrandsNode.mapChannelCount.set( 1 )
		meshFromStrandsNode.mappingTypes.set( [0], type = "Int32Array" )
		self.AssertMeshDerivedUvCoordinates( meshShape, strandIds )

		# Turn on per-strand uv coordinates, each strand should now have its own set of uv coordinates
		meshFromStrandsNode.usePerStrandMapping.set( [1], type = "Int32Array" )
		for faceIndex in range(0, 4):
			self.AssertPerVertexUvCoordinates( meshShape, faceIndex )

		# Test per-segment mapping
		meshFromStrandsNode.mappingTypes.set( [1], type = "Int32Array" )
		for faceIndex in range(0, 4):
			self.AssertPerSegmentUvCoordinates( meshShape, faceIndex )

	def test_MultipleOutputMappingChannels( self ):
		plane = pm.polyPlane( sx = 1, sy = 1 )
		pm.select( plane )
		guidesShape = TestUtilities.AddGuidesToMesh( plane[0] )

		strandIds = pm.mel.OxGetStrandIds( guidesShape )

		meshFromStrandsNode = pm.createNode( TestUtilities.MeshFromStrandsNodeName )
		meshFromStrandsNode.meshType.set( 1 )
		guidesShape.outputHair >> meshFromStrandsNode.inputStrands
		plane[0].outMesh >> meshFromStrandsNode.distributionMesh

		meshShape = pm.createNode( 'mesh' )
		meshFromStrandsNode.outputMesh >> meshShape.inMesh

		# Test per-vertex mapping on first UV channel and per-segment mapping on second channel
		meshFromStrandsNode.mapChannelCount.set( 2 )
		meshFromStrandsNode.mappingTypes.set( [0,1], type = "Int32Array" )
		meshFromStrandsNode.usePerStrandMapping.set( [0,1], type = "Int32Array" )

		self.AssertMeshDerivedUvCoordinates( meshShape, strandIds, "map1" )

		for faceIndex in range(0, 4):
			self.AssertPerSegmentUvCoordinates( meshShape, faceIndex, "map2" )

	def test_EnableDisable( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		baseShape = pm.PyNode( pm.mel.OxGetStackBaseShape( guidesShape )[0] )

		meshFromStrandsNode = pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.MeshFromStrandsNodeName )
		meshShape = pm.PyNode( pm.mel.OxGetStackShape( baseShape ) )
		pm.addAttr( meshShape, longName = 'aiOrnatrixTestBool', attributeType = 'bool' )
		pm.addAttr( meshShape, longName = 'aiOrnatrixTestLong', attributeType = 'long' )
		pm.addAttr( meshShape, longName = 'aiOrnatrixTestFloat', attributeType = 'float' )
		pm.addAttr( meshShape, longName = 'aiOrnatrixTestString', dataType = 'string' )
		pm.addAttr( meshShape, longName = 'aiOrnatrixTestColor', dataType = 'float3' )
		pm.addAttr( meshShape, longName = 'aiOrnatrixTestMessage', attributeType = 'message' )
		meshShape.aiOrnatrixTestBool.set( True )
		meshShape.aiOrnatrixTestLong.set( 13 )
		meshShape.aiOrnatrixTestFloat.set( 14.0 )
		meshShape.aiOrnatrixTestString.set( "fifteen" )
		meshShape.aiOrnatrixTestColor.set( 1, 2, 3, type = 'float3' )
		self.assertFalse( guidesShape.exists() )
		self.assertEqual( meshShape.nodeType(), u'mesh' )

		pm.mel.OxEnableOperator( meshFromStrandsNode, False )
		self.assertFalse( meshShape.exists() )
		guidesShape2 = pm.PyNode( pm.mel.OxGetStackShape( baseShape ) )
		self.assertEqual( guidesShape2.nodeType(), TestUtilities.HairShapeName )

		pm.mel.OxEnableOperator( meshFromStrandsNode, True )
		self.assertEqual( 0, len( pm.ls( type = TestUtilities.GuidesShapeName ) ) )
		meshShape2 = pm.PyNode( pm.mel.OxGetStackShape( baseShape ) )
		self.assertEqual( meshShape2.nodeType(), u'mesh' )
		self.assertEqual( meshShape2.aiOrnatrixTestBool.get(), True )
		self.assertNearEqual( meshShape2.aiOrnatrixTestLong.get(), 13 )
		self.assertEqual( meshShape2.aiOrnatrixTestFloat.get(), 14.0 )
		self.assertEqual( meshShape2.aiOrnatrixTestString.get(), "fifteen" )
		self.assertEqual( meshShape2.aiOrnatrixTestColor.get(), (1, 2, 3) )

		# TODO: Implement me
		#meshFromStrandsNode.mapChannelCount.set( 4 )
		#uvSetNames = meshShape.getUVSetNames()
		#self.assertEqual( 4, len( uvSetNames ) )

	def test_ShortSegmentStrandsAreNotFlattenedWithCylindricalType( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( length = 0.5, pointsPerStrandCount = 10 )
		baseShape = pm.PyNode( pm.mel.OxGetStackBaseShape( guidesShape )[0] )
		meshFromStrandsNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.MeshFromStrandsNodeName ) )
		# Set cylindrical cross-section
		meshFromStrandsNode.meshType.set( 0 )
		meshFromStrandsNode.sideCount.set( 4 )

		meshShape = pm.PyNode( pm.mel.OxGetStackShape( baseShape ) )

		# The first four vertices should have the same Y coordinate if they are properly oriented
		meshVertices = TestUtilities.GetMeshVertices( meshShape )
		self.assertAlmostEqual( meshVertices[0][1], meshVertices[1][1] )
		self.assertAlmostEqual( meshVertices[0][1], meshVertices[2][1] )
		self.assertAlmostEqual( meshVertices[0][1], meshVertices[3][1] )
		
	def test_ScatterChannel( self ):
		testScenePath = os.path.dirname( os.path.realpath( __file__ ) ) + "\\test_MeshFromStrandsScatterChannel.ma"
		pm.openFile( testScenePath, force = True )
		faces = pm.sets("MeshFromStrands1_genSG1", q=True)
		count = 0
		for face in faces:
			count = count + face.count()
		self.assertEqual( count, 450 )
		if count != 450:
			return

		# check the strand assignment, run `sets -q MeshFromStrands1_genSG1` to get these
		faceAssignments = [ [0, 89], [99, 143], [162, 197], [252, 260], [297, 332], [342, 350], [360, 368], [396, 404], [432, 440], [459, 476], [531, 539], [594, 611], [657, 692], [756, 773], [783, 809], [828, 899] ]
		
		for i in range(0, len(faces)):
			self.assertEqual( faces[i].index(), faceAssignments[i][0] )
			self.assertEqual( faces[i].count(), faceAssignments[i][1] - faceAssignments[i][0] + 1 )
			
	def test_ScatterMultiplier( self ):
		testScenePath = os.path.dirname( os.path.realpath( __file__ ) ) + "\\test_MeshFromStrandsScatterMultiplier.ma"
		pm.openFile( testScenePath, force = True )

		# We changed the default value of useGuideProximity for V3. In ascii scenes default values are not stored, so we must adjust it manually.
		( pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0] ).useGuideProximity.set( 0 )

		faces = pm.sets("MeshFromStrands1_genSG1", q=True)
		count = 0
		for face in faces:
			count = count + face.count()
		self.assertEqual( count, 450 )
		if count != 450:
			return
			
		# check the strand assignment, run `sets -q MeshFromStrands1_genSG1` to get these
		faceAssignments = [ [  9,  26], [ 45,  62], [ 81,  89], [108, 116], [126, 143], [153, 179], [189, 197], [216, 233], [243, 251], [306, 314], [342, 359], [423, 431], [450, 458], [477, 485], [504, 512], [522, 638], [675, 755], [801, 818], [828, 836], [846, 854], [864, 872], [891, 899] ]
		
		for i in range(0, len(faces)):
			self.assertEqual( faces[i].index(), faceAssignments[i][0] )
			self.assertEqual( faces[i].count(), faceAssignments[i][1] - faceAssignments[i][0] + 1 )
			
		# delete strands and check again
		pm.mel.eval("select EditGuides1;")
		pm.mel.eval("OxEditGuides -dg 80 250000 30000 80000 1070000 50000 400000 2740000 390000 1120000 190000 1500000 510000 1940000 210000 770000 370000 2770000 440000 950000 410000 2310000 990000 420000 2720000 590000 1920000 840000 3920000 460000 290000 3540000 620000 1950000 1080000 1580000 560000 1980000 1440000 1460000 1480000 1150000 2410000 1590000 3940000 2030000 2040000 2750000 1560000 630000 1570000 1600000 1640000 1610000 1660000 3160000 1630000 750000 1650000 910000 920000 2300000 1860000 1890000 2280000 2320000 2360000 7320000 2370000 2420000 3560000 3100000 3110000 3140000 3150000 3180000 3190000 2680000 2730000 2790000 2780000;")
		
		faces = pm.sets("MeshFromStrands1_genSG1", q=True)
		count = 0
		for face in faces:
			count = count + face.count()
		self.assertEqual( count, 450 )
		if count != 450:
			return
		
		for i in range(0, len(faces)):
			self.assertEqual( faces[i].index(), faceAssignments[i][0] )
			self.assertEqual( faces[i].count(), faceAssignments[i][1] - faceAssignments[i][0] + 1 )
			
	def test_ScatterMultiplier2( self ):
		# this is a similar test, but works on a scene without forceSequentialStrandIds and without reference material assignment.
		# it will check whether the material assignment before and after guide deletion matches
		testScenePath = os.path.dirname( os.path.realpath( __file__ ) ) + "\\test_MeshFromStrandsScatterMultiplier2.ma"
		pm.openFile( testScenePath, force = True )

		# We changed the default value of useGuideProximity for V3. In ascii scenes default values are not stored, so we must adjust it manually.
		( pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0] ).useGuideProximity.set( 0 )

		faces = pm.sets("MeshFromStrands1_genSG1", q=True)
		faceAssignments = []
		refCount = 0
		for face in faces:
			refCount = refCount + face.count()
			faceAssignments.append([face.index(), face.count()])

		pm.mel.eval("select EditGuides1;")
		pm.mel.eval("OxEditGuides -dg 86 250000 30000 50000 1070000 590000 1920000 80000 440000 950000 2310000 420000 2720000 990000 1080000 1440000 510000 1500000 210000 1940000 1120000 190000 1460000 3970000 1480000 1150000 620000 3540000 1950000 1290000 1980000 1320000 1340000 1630000 1660000 3160000 2030000 1370000 290000 1560000 2040000 2750000 840000 460000 3920000 1570000 2410000 1590000 3940000 560000 1580000 770000 370000 2770000 390000 1600000 400000 2740000 2300000 1860000 1640000 1610000 1890000 410000 1650000 2280000 630000 2320000 750000 2360000 2370000 2420000 910000 920000 2680000 2730000 2780000 2790000 3100000 3560000 3110000 3500000 3140000 3150000 3180000 7320000 3190000;")
		
		faces = pm.sets("MeshFromStrands1_genSG1", q=True)
		count = 0
		for face in faces:
			count = count + face.count()
		self.assertEqual( count, 450 )
		if count != refCount:
			return
		
		for i in range(0, len(faces)):
			self.assertEqual( faces[i].index(), faceAssignments[i][0] )
			self.assertEqual( faces[i].count(), faceAssignments[i][1] )
