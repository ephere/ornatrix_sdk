import MayaTest
import TestUtilities
import pymel.core as pm
import sys
import unittest
import maya.mel

if sys.version_info > (3,):
	long = int

''' The tests below are specific to graft grooms, grooms which can be "fitted" onto any base surface by placing a graft surface on it'''
class Test_GraftGrooms( MayaTest.OxTestCase ):

	# Update these values based on installed grooms
	expectedEyebrowHash = '9726219005237696972'
	expectedEyebrowGroomCount = 1

	expectedScalpHash = '17726461644882178118'
	expectedScalpGroomCount = 2

	# Make sure that graft grooms expected for the rest of the tests are installed
	def test_InstalledGraftGrooms( self ):
		# Check available base surfaces, change these when base surfaces are modified or added/removed
		self.assertEqual( ['Eyebrow', 'Scalp'], sorted( pm.mel.OxGroomInfo( q = True, abm = True ) ) )

		# And their hashes (these will change if the topology of base surfaces or the algorithm for hash calculation changes)
		# There is a problem with the commands below: Python orders the arguments differently depending on Maya version, and the order must be fixed to avoid MEL interpreting the string as an object name
		self.assertEqual( self.expectedEyebrowHash, maya.mel.eval( 'OxGroomInfo -baseSurfaceTopologyHash "Eyebrow" -q' ) )
		self.assertEqual( self.expectedScalpHash, maya.mel.eval( 'OxGroomInfo -baseSurfaceTopologyHash "Scalp" -q' ) )

		# One installed eyebrow groom
		self.assertEqual( self.expectedEyebrowGroomCount, len( pm.mel.OxGroomInfo( groomsList = self.expectedEyebrowHash ) ) )
		self.assertEqual( self.expectedScalpGroomCount, len( pm.mel.OxGroomInfo( groomsList = self.expectedScalpHash ) ) )

	def GraftEyebrowGroomToSphere( self ):
		sphere = pm.polySphere()[0]

		eyebrowGroom = pm.mel.OxGroomInfo( groomsList = self.expectedEyebrowHash )[0]

		# Select Eyebrow groom as the current one
		pm.mel.OxGroomInfo( selectedGraftGroom = eyebrowGroom )
		groomNode = pm.mel.OxGraftGroom( sphere.getShape(), 0, 0.0, 0.5, 200, 0.0, 0.5 )

		return groomNode

	# Checks that unboxing a grafted groom will not alter the hair output
	@unittest.skip( "TODO: Remove when #6128 is implemented" )
	def test_UnboxGraftedGroom( self ):
		groomNode = self.GraftEyebrowGroomToSphere()

		self.assertGreater( len( groomNode ), 0 )
		hairShape = pm.mel.OxGetStackShape( groomNode )

		boxedVertices = pm.mel.OxGetVertices( hairShape, os = True )

		# Unbox the groom
		pm.mel.OxUnboxNodes( groomNode )

		hairShape = pm.mel.ls( type = TestUtilities.HairShapeName )[0]
		unboxedVertices = pm.mel.OxGetVertices( hairShape, os = True )

		# Unboxed hair shouldn't be different from boxed one
		TestUtilities.CheckPointsAllNearEqual( self, boxedVertices, unboxedVertices )

	# Makes sure that any changes we made to parameters of a graft groom survive scene serialization
	@unittest.skipIf( MayaTest.OsIsLinux, "TODO: Test fail on Linux, investigate why" )
	def test_SaveLoadGraftedGroom( self ):
		groomNode = pm.PyNode( self.GraftEyebrowGroomToSphere() )

		# Decrease hair count to speed up the test
		groomNode.HairCount.set( 20 )

		hairShape = pm.mel.OxGetStackShape( groomNode )

		self.assertLessEqual( pm.mel.OxGetStrandCount( hairShape ), groomNode.HairCount.get() )

		verticesBeforeParameterChange = TestUtilities.GetVerticesInObjectCoordinatesSortedByStrandId( hairShape )

		# Change some groom parameters
		groomNode.RandomSeed.set( 5 )

		verticesAfterParameterChange = TestUtilities.GetVerticesInObjectCoordinatesSortedByStrandId( hairShape )

		# Modifying the random seed should either change strand/vertex count or their positions
		if len( verticesBeforeParameterChange ) == len( verticesAfterParameterChange ):
			TestUtilities.CheckPointsNotAllNearEqual( self, verticesBeforeParameterChange, verticesAfterParameterChange )

		TestUtilities.SaveAndReloadScene( self, 'test_SaveLoadGraftedGroom' )

		hairShape = pm.mel.ls( type = TestUtilities.HairShapeName )[0]
		verticesAfterReload = TestUtilities.GetVerticesInObjectCoordinatesSortedByStrandId( hairShape )

		TestUtilities.CheckPointsAllNearEqual( self, verticesAfterParameterChange, verticesAfterReload )