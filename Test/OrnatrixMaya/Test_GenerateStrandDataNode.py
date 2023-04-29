import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt
import unittest

class Test_GenerateStrandDataNode( MayaTest.OxTestCase ):

	def test_BasicGenerateStrandData( self ):
		# Add hair and curl node
		guidesShape = TestUtilities.AddGuidesToNewPlane( pointsPerStrandCount = 5 )

		# Initially only selection channel is present
		self.assertEqual( 1, len( pm.mel.OxGetRootChannels( guidesShape ) ) )
		self.assertEqual( None, pm.mel.OxGetVertexChannels( guidesShape ) )

		generateStrandDataNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GenerateStrandDataNodeName ) )
		curlNode = pm.PyNode( pm.mel.OxAddStrandOperator( generateStrandDataNode, TestUtilities.CurlNodeName ) )

		# Grab the vertices after curling, initially they should all be affected
		verticesAfterCurling = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Create a new constant channel and assign it some value
		generateStrandDataNode.newChannelName.set( "TestChannel" )

		# Test channel is added
		self.assertEqual( [u'Selection', u'TestChannel'], pm.mel.OxGetRootChannels( guidesShape ) )
		self.assertEqual( None, pm.mel.OxGetVertexChannels( guidesShape ) )

		# Assign generated channel to curl node
		curlNode.magnitudeChannel.set( 2 )

		generateStrandDataNode.generationMethod.set( 0 )
		generateStrandDataNode.minimumTargetValue.set( 2.0 )
		verticesWithCurling2 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAfterCurling, verticesWithCurling2 )

		generateStrandDataNode.minimumTargetValue.set( 3.0 )
		verticesWithCurling3 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithCurling2, verticesWithCurling3 )

	def test_IsInsideMovingMesh( self ):
		collisionSphere = pm.polySphere( r = 10 )
		guidesShape = TestUtilities.AddGuidesToNewPlane( pointsPerStrandCount = 10, length = 20 )
		generateStrandDataNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GenerateStrandDataNodeName ) )
		generateStrandDataNode.newChannelName.set( "TestChannel" )
		generateStrandDataNode.newChannelType.set( 1 )
		generateStrandDataNode.generationMethod.set( 6 )

		pm.connectAttr( 'pSphereShape1.worldMesh', str( generateStrandDataNode ) + '.sceneMeshes[0]' );

		# First 5 points on each strand should be inside the mesh
		self.assertEqual( [1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
					 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
					 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
					 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0], pm.mel.OxGetVertexValuesForChannel( guidesShape, 0 ) )

		curlNode = pm.PyNode( pm.mel.OxAddStrandOperator( generateStrandDataNode, TestUtilities.CurlNodeName ) )
		curlNode.magnitudeChannel.set( 1001 )

		# The final vertices should be changing as the collision sphere object is moved
		pm.select( collisionSphere )
		pm.currentTime( 0 )
		pm.move( 1, 35, 1 )
		pm.setKeyframe()
		pm.currentTime( 5 )
		pm.move( 1, 25, 1 )
		pm.setKeyframe()

		verticesAtTime5 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		pm.currentTime( 0 )
		verticesAtTime0 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAtTime0, verticesAtTime5 )

	def test_IsInsideMeshWithHairDistribution( self ):
		collisionSphere = pm.polySphere( r = 4 )
		guidesShape = TestUtilities.AddGuidesToNewPlane( planeSize = 10, pointsPerStrandCount = 2, length = 5, planeSegmentCount = 10 )
		generateStrandDataNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GenerateStrandDataNodeName ) )
		generateStrandDataNode.newChannelName.set( "TestChannel" )
		generateStrandDataNode.newChannelType.set( 0 )
		generateStrandDataNode.generationMethod.set( 6 )
		pm.connectAttr( 'pSphereShape1.worldMesh', str( generateStrandDataNode ) + '.sceneMeshes[0]' );

		hairFromGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.HairFromGuidesNodeName ) )
		hairFromGuidesNode.distributionChannel.set( 2 )
		hairFromGuidesNode.distribution.set( 4 )
		hairFromGuidesNode.interpolation.set( 1 )

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		rootPositions = TestUtilities.GetRootPositions( hairShape )
		self.assertGreater( len( rootPositions ), 0 )

		# At this point all hair roots should be inside the sphere's radius
		for rootPosition in rootPositions:
			self.assertLessEqual( dt.Vector( rootPosition ).distanceTo( [0,0,0] ), 4 )

	def test_IsInside( self ):
		"""Tests numerical correctness of IsInside (issue #4495)"""
		# Generate single strand at origin: (0, 0) projections trigger numerical errors
		guidesShape = TestUtilities.AddGuidesToNewPlane( guideCount = 1, pointsPerStrandCount = 10, length = 20, rootGenerationMethod = 6 )
		generateStrandDataNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GenerateStrandDataNodeName ) )
		generateStrandDataNode.newChannelName.set( "TestChannel" )
		generateStrandDataNode.newChannelType.set( 1 )
		generateStrandDataNode.generationMethod.set( 6 )

		collisionSphere = pm.polySphere( r = 10, sa = 4, sh = 3 )
		pm.connectAttr( 'pSphereShape1.worldMesh', str( generateStrandDataNode ) + '.sceneMeshes[0]' );

		# First 5 points on each strand should be inside the mesh
		self.assertEqual( [1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0], pm.mel.OxGetVertexValuesForChannel( guidesShape, 0 ) )

	def test_GenerateConstantDataOnMesh( self ):
		# Add hair and curl node
		hairShape = TestUtilities.AddHairToNewPlane( pointsPerStrandCount = 2 )

		# Initially only selection channel is present, it was copied from guides
		self.assertEqual( [u'Selection'], pm.mel.OxGetRootChannels( hairShape ) )
		self.assertEqual( None, pm.mel.OxGetVertexChannels( hairShape ) )

		generateStrandDataNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.GenerateStrandDataNodeName ) )
		curlNode = pm.PyNode( pm.mel.OxAddStrandOperator( generateStrandDataNode, TestUtilities.CurlNodeName ) )

		# Grab the vertices after curling, initially they should all be affected
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		verticesAfterCurling = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Create a new constant channel and assign it some value
		generateStrandDataNode.newChannelName.set( "TestChannel" )

		# Test channel is added
		self.assertEqual( [u'Selection', u'TestChannel'], pm.mel.OxGetRootChannels( hairShape ) )
		self.assertEqual( None, pm.mel.OxGetVertexChannels( hairShape ) )

		# Assign generated channel to curl node
		curlNode.magnitudeChannel.set( 2 )

		# Need to set curl phase to a non-integer value to see change with just 2 vertices
		curlNode.phase.set( 1.5 )

		generateStrandDataNode.generationMethod.set( 0 )
		generateStrandDataNode.minimumTargetValue.set( 2.0 )
		verticesWithCurling2 = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAfterCurling, verticesWithCurling2 )

		generateStrandDataNode.minimumTargetValue.set( 3.0 )
		verticesWithCurling3 = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithCurling2, verticesWithCurling3 )

	# Tests that the stored per-vertex values remain valid for their strands when strand order changes. Strand ids should be used to associate values to their strands.
	def test_StoredVertexValuesRemainValidForChangedStrandOrder( self ):
		# Generate per-vertex random values and assign to curling magnitude. Set distribution to random to make sure parallel evaluation is used.
		guidesShape = TestUtilities.AddGuidesToNewPlane( pointsPerStrandCount = 3, rootGenerationMethod = 2, guideCount = 100 )
		generateStrandDataNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GenerateStrandDataNodeName ) )
		curlNode = pm.PyNode( pm.mel.OxAddStrandOperator( generateStrandDataNode, TestUtilities.CurlNodeName ) )

		# Create a new random per-vertex channel and assign it some value
		generateStrandDataNode.targetData.set( 1 )
		generateStrandDataNode.newChannelName.set( "TestChannel" )
		generateStrandDataNode.newChannelType.set( 1 )
		generateStrandDataNode.isStoringValues.set( 1 )
		curlNode.magnitudeChannel.set( 1002 )

		generateStrandDataNode.generationMethod.set( 1 )
		generateStrandDataNode.minimumTargetValue.set( 0.0 )
		generateStrandDataNode.maximumTargetValue.set( 100.0 )
		verticesBeforeStrandOrderChange = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( guidesShape )

		# Force guides root re-evaluation which should modify their order
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		guidesFromMesh.attr( 'count' ).set( 101 )
		TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		guidesFromMesh.attr( 'count' ).set( 100 )
		verticesAfterStrandOrderChange = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( guidesShape )

		# Stored values should remain the same for each strand
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesBeforeStrandOrderChange, verticesAfterStrandOrderChange )

	def test_GenerateStrandChannels( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		# Initially all guides are in 0 group
		self.assertEqual( [0, 0, 0, 0], pm.mel.OxGetStrandGroups( guidesShape ) )

		generateStrandDataNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GenerateStrandDataNodeName ) )
		generateStrandDataNode.targetData.set( 2 )
		generateStrandDataNode.minimumTargetValue.set( 5 )

		# Now we should have all strands in group 5
		self.assertEqual( [5, 5, 5, 5], pm.mel.OxGetStrandGroups( guidesShape ) )

	def test_GeneratedChannelNamesAreUnique( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		generateStrandDataNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GenerateStrandDataNodeName ) )
		generateStrandDataNode.targetData.set( 1 )
		generateStrandDataNode.sampleValueCount.set( 3 )
		generateStrandDataNode.newChannelName.set( "TestChannel" )

		self.assertEqual( [u'Selection', u'TestChannel_1', u'TestChannel_2', u'TestChannel_3'], pm.mel.OxGetRootChannels( guidesShape ) )

	# Tests for a regression which caused a crash when target value range was switched off with non-default target data mode
	def test_DontUseTargetValueRangeWithStrandIdExport( self ):
		guidesShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 2, strandCount = 100, planeSegmentCount = 10 )
		generateStrandDataNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GenerateStrandDataNodeName ) )

		# Strand groups output data
		generateStrandDataNode.targetData.set( 3 )
		generateStrandDataNode.generationMethod.set( 1 )
		generateStrandDataNode.useTargetValueRange.set( 0 )
		TestUtilities.GetVerticesInObjectCoordinatesByStrandId( guidesShape )

		# If no crash happened we are good

		# Although we try to set constant id 3 to all strands, distinct ids are set
		TestUtilities.CheckUniqueStrandIds( self, guidesShape )

	def test_DistanceToObject( self ):
		proxyObject = pm.polyPlane( sx = 1, sy = 1, width = 0.0001, height = 0.0001 )
		pm.select( proxyObject )
		proxyObjectMeshShape = proxyObject[0]

		pm.select( proxyObjectMeshShape )
		pm.move( 10, 0, 0 )

		guidesShape = TestUtilities.AddGuidesToNewPlane( pointsPerStrandCount = 10, length = 20 )
		generateStrandDataNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GenerateStrandDataNodeName ) )
		generateStrandDataNode.newChannelName.set( "TestChannel" )
		generateStrandDataNode.newChannelType.set( 0 )
		generateStrandDataNode.generationMethod.set( 5 )
		generateStrandDataNode.minimumTargetValue.set( 0 )
		generateStrandDataNode.maximumTargetValue.set( 10 )

		pm.connectAttr( str( proxyObjectMeshShape ) + '.worldMesh', str( generateStrandDataNode ) + '.sceneMeshes[0]' );

		# If everything is correct then we should have 2 points almost 10 units away and 2 almost 9 units away (since plane is 1 unit wide)
		TestUtilities.AssertAlmostEqualByKeys( self,
			TestUtilities.MakeDictionaryWithIncrementingKeys( [10.0, 9.0, 10.0, 9.0] ), 
			TestUtilities.MakeDictionary( pm.mel.OxGetStrandIds( guidesShape ), pm.mel.OxGetRootValuesForChannel( guidesShape, 1 ) ), places = 1 )

		# Move the proxy sphere and re-evaluate
		pm.select( proxyObjectMeshShape )
		pm.move( 5, 0, 0 )
		generateStrandDataNode.maximumTargetValue.set( 5 )

		TestUtilities.AssertAlmostEqualByKeys( self,
			TestUtilities.MakeDictionaryWithIncrementingKeys( [5, 4.1, 5, 4.1] ),
			TestUtilities.MakeDictionary( pm.mel.OxGetStrandIds( guidesShape ), pm.mel.OxGetRootValuesForChannel( guidesShape, 1 ) ), places = 1 )

	def test_TextureMap( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( pointsPerStrandCount = 10, length = 20 )
		generateStrandDataNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GenerateStrandDataNodeName ) )
		generateStrandDataNode.newChannelName.set( "TestChannel" )
		generateStrandDataNode.newChannelType.set( 0 )
		generateStrandDataNode.generationMethod.set( 7 )

		# Create a ramp texture and connect it to the GSD node
		rampTexture = pm.createNode( "ramp" )

		pm.mel.connectNodeToAttrOverride( rampTexture, generateStrandDataNode + ".textureMultiplier" )
		pm.connectAttr( rampTexture + ".outColor", generateStrandDataNode + ".textureMultiplier" )

		# Ramp texture should be defining the strand channel data
		actualValues = pm.mel.OxGetRootValuesForChannel( guidesShape, 1 )
		TestUtilities.AssertAlmostEqualByKeys( self,
			TestUtilities.MakeDictionaryWithIncrementingKeys( [0.0, 1.0, 0.0, 1.0] ),
			TestUtilities.MakeDictionary( pm.mel.OxGetStrandIds( guidesShape ), pm.mel.OxGetRootValuesForChannel( guidesShape, 1 ) ), places = 1 )
		
	def test_SeExpr( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( pointsPerStrandCount = 10, length = 20 )
		testChannel = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GenerateStrandDataNodeName ) )
		testChannel.newChannelName.set( "test" )
		testChannel.newChannelType.set( 0 )
		testChannel.generationMethod.set( 2 )
		testChannel.useTargetValueRange.set( 0 )
		
		generateStrandDataNode = pm.PyNode( pm.mel.OxAddStrandOperator( testChannel, TestUtilities.GenerateStrandDataNodeName ) )
		generateStrandDataNode.newChannelName.set( "TestChannel" )
		generateStrandDataNode.newChannelType.set( 0 )
		generateStrandDataNode.generationMethod.set( 9 )
		generateStrandDataNode.useTargetValueRange.set( 0 )
		
		generateStrandDataNode.setAttr("seExprValue", "$index + 1")
		self.assertSequenceAlmostEqual( [1.0, 2.0, 3.0, 4.0], pm.mel.OxGetRootValuesForChannel( guidesShape, 2 ), places = 1 )
		
		generateStrandDataNode.setAttr("seExprValue", "$sc_test")
		self.assertSequenceAlmostEqual( [0.0, 1.0, 2.0, 3.0], pm.mel.OxGetRootValuesForChannel( guidesShape, 2 ), places = 1 )
		
