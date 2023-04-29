#----------------
import os
import sys
import c4d
sys.path.insert( 0, os.path.dirname( __file__ ) )
import Ornatrix.Ornatrix as Ox
import TestUtilities
#import importlib
#importlib.reload(TestUtilities)
sys.path.pop( 0 )
#----------------
from unittest import TestCase


#
# TestCase
class Test_Noise( TestCase ):

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		Ox.DeleteAll( self.doc )
		# Create base object
		self.hair = TestUtilities.AddGuidesToNewPlane( planeSize=400, planeSegmentCount=20, guideCount=10, pointsPerStrandCount=4)
		self.gfm = self.hair.GetDown()
		# Add noise
		self.noise = c4d.BaseObject( Ox.res.ID_OX_NOISE_MOD )
		self.noise[Ox.res.noise_AmountAttribute] = 2
		self.noise[Ox.res.noise_NoiseScaleAttribute] = 0.5
		self.noise[Ox.res.noise_PreserveStrandLengthAttribute] = False
		self.noise.InsertUnderLast( self.hair )
		# Build
		Ox.ComputeStack( self.doc )

	def tearDown( self ):
		#Ox.DeleteAll( self.doc )
		self.doc = None

	# Test if Noise modifier changed original guides
	def test_Noise_StrandsChanged( self ):
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.gfm )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.noise )
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, verticesBefore, verticesAfter )

	# Test if Noise modifier changed strand lengths
	def test_Noise_StrandLength( self ):
		lengthsBefore = TestUtilities.GetStrandLengthsByStrandId( self.gfm )
		lengthsAfter = TestUtilities.GetStrandLengthsByStrandId( self.noise )
		TestUtilities.CheckValuesNotAllNearEqualByStrandId( self, lengthsBefore, lengthsAfter )



#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_Noise )