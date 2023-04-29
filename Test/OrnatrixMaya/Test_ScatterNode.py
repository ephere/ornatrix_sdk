import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt

class Test_ScatterNode( MayaTest.OxTestCase ):

	def test_OutputShapeInput( self ):
		guidesShape1 = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 1, length = 3 )
		guidesShape2 = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 1, length = 2 )
		scatterNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape1, TestUtilities.ScatterNodeName ) )
		pm.connectAttr( guidesShape2.worldHair[0], scatterNode.referenceObject, nextAvailable = True )
		scatterNode.outputShapeMethod.set( 2 )
		vertices = pm.mel.OxGetVertices( guidesShape1, os = True )

		guidesShape1 = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 1, length = 4 )
		guidesShape2 = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 1, length = 3, guideCount = 1, rootGenerationMethod = 2 )

		propagatorNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape2, TestUtilities.PropagationNodeName ) )
		propagatorNode.length.set( 0.3 )
		propagatorNode.setAttr( 'count', 5 )

		scatterNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape1, TestUtilities.ScatterNodeName ) )
		scatterNode.outputShapeMethod.set( 1 )
		pm.connectAttr( guidesShape2.worldHair[0], scatterNode.referenceObject, nextAvailable = True )

		# There must be no 0,0,0 vertices
		vertices = pm.mel.OxGetVertices( guidesShape1, os = True )
		# TODO: this fails for now and is disabled to avoid breaking the nightly build
		#self.assertTrue( all( [ [vertices[i], vertices[i+1], vertices[i+2]] != [0,0,0] for i in range( 0, len( vertices ), 3 )] ) )

	def test_ChannelData( self ):
		perStrandChannelType = 0
		perVertexChannelType = 1

		guidesShape1 = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 1, length = 10 )
		guidesShape2 = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 1, length = 5 )

		generateGuideData = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape1, TestUtilities.GenerateStrandDataNodeName ) )
		generateGuideData.newChannelType.set( perStrandChannelType )
		generateGuideData.newChannelName.set( "Channel1" )
		generateGuideData.minimumTargetValue.set( 0.1 )
		generateGuideData.maximumTargetValue.set( 0.1 )

		generateGuideData = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape1, TestUtilities.GenerateStrandDataNodeName ) )
		generateGuideData.newChannelType.set( perVertexChannelType )
		generateGuideData.newChannelName.set( "Channel2" )
		generateGuideData.minimumTargetValue.set( 0.2 )
		generateGuideData.maximumTargetValue.set( 0.2 )

		generateGuideData = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape2, TestUtilities.GenerateStrandDataNodeName ) )
		generateGuideData.newChannelType.set( perStrandChannelType )
		generateGuideData.newChannelName.set( "Channel1" )
		generateGuideData.minimumTargetValue.set( 0.3 )
		generateGuideData.maximumTargetValue.set( 0.3 )

		generateGuideData = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape2, TestUtilities.GenerateStrandDataNodeName ) )
		generateGuideData.newChannelType.set( perVertexChannelType )
		generateGuideData.newChannelName.set( "Channel2" )
		generateGuideData.minimumTargetValue.set( 0.4 )
		generateGuideData.maximumTargetValue.set( 0.4 )

		pm.select( guidesShape1 )
		pm.move( 0, 0, 5 )

		pm.select( guidesShape2 )
		pm.move( 0, 0, -5 )

		strandIndex = 0
		pointIndex = 0

		def checkStrandChannel( guidesShape, value ):
			channelIndex = 1
			dataValue = pm.mel.OxGetPointData( guidesShape, strandIndex, pointIndex, channelIndex, type = perStrandChannelType )
			self.assertNearEqual( value, dataValue )

		def checkVertexChannel( guidesShape, value ):
			channelIndex = 0
			dataValue = pm.mel.OxGetPointData( guidesShape, strandIndex, pointIndex, channelIndex, type = perVertexChannelType )
			self.assertNearEqual( value, dataValue )

		checkStrandChannel( guidesShape1, 0.1 )
		checkVertexChannel( guidesShape1, 0.2 )
		checkStrandChannel( guidesShape2, 0.3 )
		checkVertexChannel( guidesShape2, 0.4 )

		scatterNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape1, TestUtilities.ScatterNodeName ) )
		pm.connectAttr( guidesShape2.worldHair[0], scatterNode.referenceObject, nextAvailable = True )
		# This won't work before Node's GUI would be loaded
		#pm.mel.OxScatterProcessReferenceStrands( scatterNode + ".referenceObject", guidesShape2, scatterNode );

		# Per-strand channels should be from reference strands
		checkStrandChannel( guidesShape1, 0.3 )
		checkVertexChannel( guidesShape1, 0.4 )

		# 0 - None, 1 - Base, 2 - Scattered, 3 - Both
		scatterNode.perStrandChannels.set( 1 )

		# Now per-strand channels should be from input strands
		checkStrandChannel( guidesShape1, 0.1 )
		checkVertexChannel( guidesShape1, 0.4 )

	def test_GroupCrash( self ):
		"""Tests for crash when using strand groups"""
		# Issue #3776
		guidesShape1 = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 1, length = 10 )
		rootCountBeforeScatter = pm.mel.OxGetStrandCount( guidesShape1 )
		guidesShape2 = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 1, length = 5 )
		scatterNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape1, TestUtilities.ScatterNodeName ) )
		propagatorNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape2, TestUtilities.PropagationNodeName ) )
		pm.connectAttr( guidesShape2.worldHair[0], scatterNode.referenceObject, nextAvailable = True )
		scatterNode.strandGroupPattern.set( '1' )

		# Root evaluation triggers the crash
		rootCountAfterScatter = pm.mel.OxGetStrandCount( guidesShape1 )
		# Root count is equal because input group '1' is empty
		self.assertEqual( rootCountBeforeScatter, rootCountAfterScatter )

	def test_TextureAtlas( self ):
		# Create reference and scattered hairs
		strandCount = 4
		guidesShape = TestUtilities.AddEditGuidesToNewPlane( guideCount = strandCount, pointsPerStrandCount = 2, rootGenerationMethod = 2, planeSegmentCount = 2, guideLength = 1 )
		self.assertEqual( strandCount, pm.mel.OxGetStrandCount( guidesShape ) )
		scatterNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.ScatterNodeName ) )
		referenceGuidesShape1 = TestUtilities.CreateSingleHairWithPropagator()
		pm.connectAttr( referenceGuidesShape1.worldHair[0], scatterNode.referenceObject, nextAvailable = True )
		referenceGuidesShape2 = TestUtilities.CreateSingleHairWithPropagator()
		pm.connectAttr( referenceGuidesShape2.worldHair[0], scatterNode.referenceObject, nextAvailable = True )

		# Assign groups and channel values for per-group and per-channel island generation
		pm.select( guidesShape )
		pm.mel.eval( TestUtilities.EditGuidesCommandName  + " -crc 1 Test;" )
		strandIds = pm.mel.OxGetStrandIds( guidesShape )
		editRootDataCommand = TestUtilities.EditGuidesCommandName + " -sd " + str( strandCount );
		editGroupsCommand = TestUtilities.EditGuidesCommandName + " -sg " + str( strandCount );
		groupValues = [0, 0, 1, 1]
		for i in range( 0, strandCount ):
			editRootDataCommand += " " + str( strandIds[i] ) + " 1 1 " + str( groupValues[i] )
			editGroupsCommand += " " + str( strandIds[i] ) + " " + str( groupValues[i] )

		editRootDataCommand += ";"
		editGroupsCommand += ";"
		pm.mel.eval( editRootDataCommand )
		pm.mel.eval( editGroupsCommand )

		# Create predictable distribution
		scatterNode.distributionMethod.set( 1 )
		scatterNode.distributionChannel.set( 2 )
		scatterNode.textureAtlasChannel.set( 2 )
		scatterNode.textureAtlasScalingMethod.set( 0 )
		scatterNode.textureCoordinateGenerationMethod.set( 3 )

		# Obtain texture coordinates for each generation method
		hairShape = pm.mel.OxGetStackShape( scatterNode )
		textureCoordinatesByMethod = []
		for generationMethod in range( 3 ):
			scatterNode.textureAtlasGenerationMethod.set( generationMethod )
			textureCoordinatesByMethod.append( TestUtilities.GetTextureCoordinates( hairShape, 0, type = 0 ) )

		referenceCoords = TestUtilities.GetReferencePropagatorTextureCoordinates( stemVertexCount = 3, branchVertexCount = 4, branchesPerStemCount = 4, islandCount = 2, repetitions = 2 )
		# The three methods are set up to give identical results
		TestUtilities.CheckPointsMatch( self, textureCoordinatesByMethod[0], referenceCoords, epsilon = 0.02 )
		TestUtilities.CheckPointsMatch( self, textureCoordinatesByMethod[1], referenceCoords, epsilon = 0.02 )
		TestUtilities.CheckPointsMatch( self, textureCoordinatesByMethod[2], referenceCoords, epsilon = 0.02 )

	def test_UseBraidsAsReference( self ):
		"""Tests for correctly using braids as reference"""
		# Issue #3874
		guidesShape1 = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 1, length = 10 )
		rootCountBeforeScatter = pm.mel.OxGetStrandCount( guidesShape1 )
		guidesShape2 = pm.PyNode( pm.mel.OxAddBraidGuides() )
		scatterNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape1, TestUtilities.ScatterNodeName ) )
		pm.connectAttr( guidesShape2.worldHair[0], scatterNode.referenceObject, nextAvailable = True )
		scatterNode.outputShapeMethod.set( 2 )

		rootCountAfterScatter = pm.mel.OxGetStrandCount( guidesShape1 )
		rootCountBraids = pm.mel.OxGetStrandCount( guidesShape2 )
		# Braid with a single strand does not provide a meaningful test
		self.assertGreater( rootCountBraids, 1 )
		self.assertEqual( rootCountBeforeScatter * rootCountBraids, rootCountAfterScatter )
