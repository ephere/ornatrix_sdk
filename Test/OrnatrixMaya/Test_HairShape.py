import MayaTest
import TestUtilities
import pymel.core as pm

class Test_HairShape( MayaTest.OxTestCase ):

	def test_HairVertexCountAndBoundingBox( self ):
		plane = pm.polyPlane()
		pm.select( plane )

		# Create hair shape with 100 hairs
		hairShape = TestUtilities.AddHairToMesh( plane[0], hairViewportCount = 10 )
		hairShapeName = hairShape.name()

		# TODO: Set the viewport hair count to a smaller value for quicker test evaluation

		self.assertFalse( pm.mel.OxUsesStrandTopology(hairShapeName) )

		strandCount = pm.mel.OxGetStrandCount(hairShapeName)
		pointCount = pm.mel.OxGetGlobalStrandPointCount(hairShapeName)
		vertexCount = pm.mel.OxGetVertexCount(hairShapeName)

		# Number of vertices in hair should equal number of strands times number of points per strand
		self.assertEqual( vertexCount, strandCount * pointCount )

		# Test that the bounding box of the hair contains every point
		hairShapeNode = pm.general.PyNode( hairShapeName )
		boundingBox = hairShapeNode.boundingBox()

		for strandIndex in range( 0, strandCount ):
			for pointIndex in range( 0, pointCount ):
				strandPoint = pm.mel.OxGetStrandPointInObjectCoordinates( hairShapeName, strandIndex, pointIndex )
				self.assertTrue( boundingBox.contains( strandPoint ), msg = strandPoint )

	def test_CrossPlatformSerialization( self ):
		if MayaTest.OsIsWindows:
			testScenePath = self.findTestFile( 'OxMaya2-8-SerializationTest-Linux.ma' )
		else:
			testScenePath = self.findTestFile( 'OxMaya2-8-SerializationTest-Windows.ma' )
		pm.openFile( testScenePath, force = True )
		hair = pm.ls( type = TestUtilities.GuidesShapeName )[0]
		self.assertEqual( [u'Selection', u'PerStrandChan1', u'PerStrandChan2'], pm.mel.OxGetRootChannels( hair ) )
		self.assertEqual( [u'PerVertChan1', u'PerVertChan2'], pm.mel.OxGetVertexChannels( hair ) )

		if not MayaTest.OsIsMac:
			testScenePath = self.findTestFile( 'OxMaya2-8-SerializationTest-Linux.mb' )
		else:
			# Mac build agent uses Maya 2015 for the tests, the binary scene is from 2016 and won't load
			return
			#testScenePath = self.findTestFile( 'OxMaya2-8-SerializationTest-Windows.mb' )
		pm.openFile( testScenePath, force = True )
		hair = pm.ls( type = TestUtilities.GuidesShapeName )[0]
		self.assertEqual( [u'Selection', u'PerStrandChan1', u'PerStrandChan2'], pm.mel.OxGetRootChannels( hair ) )
		self.assertEqual( [u'PerVertChan1', u'PerVertChan2'], pm.mel.OxGetVertexChannels( hair ) )
