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
class Test_Frizz( TestCase ):
	doc = None
	hair = None
	frizz = None
	verticesBefore = None

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		Ox.DeleteAll( self.doc )
		plane = c4d.BaseObject( c4d.Oplane )
		self.doc.InsertObject( plane )
		# Create stack with Frizz
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		self.gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		self.gfm[Ox.res.INPUT_MESH] = plane
		self.frizz = c4d.BaseObject( Ox.res.ID_OX_FRIZZ_MOD )
		self.frizz[c4d.frz_AmountAttribute] = 50
		self.doc.InsertObject( self.hair )
		self.gfm.InsertUnderLast( self.hair )
		self.frizz.InsertUnderLast( self.hair )
		# Build
		Ox.ComputeStack( self.doc )
		gfm = self.hair.GetDown()
		self.verticesBefore = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( gfm )

	def test_Frizz( self ):
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.frizz )
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, self.verticesBefore, verticesAfter )

	def test_Frizz_OrientationMultiplier( self ):
		TestUtilities.MakeAxisAlignedBlackAndWhiteShaderX( self.frizz, c4d.frz_AmountMultiplierAttribute )
		Ox.ComputeStack( self.doc )
		# half strands not frizzed
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.frizz )
		nearEqualPercentage = TestUtilities.GetNearEqualPointsPercentage( self, self.verticesBefore, verticesAfter )
		TestUtilities.CheckPercentageNearEqual( self, nearEqualPercentage, 0.5, 0.02, 'Equal strands' )


#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_Frizz )