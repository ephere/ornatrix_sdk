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
class Test_DistributionMesh( TestCase ):
	hair = None
	gfm = None
	plane = None

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		# Create Plane
		self.plane = c4d.BaseObject( c4d.Oplane )
		self.doc.InsertObject( self.plane )
		# Create Ornatrix Hair object
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		self.doc.InsertObject( self.hair )
		# Create GFM modifier inside Hair Object
		self.gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		self.gfm[Ox.res.INPUT_MESH] = self.plane
		self.gfm[Ox.res.gfm_GuideCountAttribute] = 1000
		self.gfm[Ox.res.gfm_GuideLengthAttribute] = 10
		self.gfm[Ox.res.gfm_GuideLengthRandomnessAttribute] = 0
		self.doc.InsertObject( self.gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )

	def test_DistributionMesh_Primitive( self ):
		expectedCenter = c4d.Vector( 0, 5, 0 )
		expectedSize = c4d.Vector( 400, 10, 400 )
		TestUtilities.CheckHairBounds( self, self.hair, expectedCenter, expectedSize, 2 )

	def test_DistributionMesh_PrimitiveDeformed( self ):
		bend = c4d.BaseObject( c4d.Obend )
		bend[c4d.DEFORMOBJECT_SIZE] = c4d.Vector( 0, 400, 400 )
		bend[c4d.DEFORMOBJECT_STRENGTH] = c4d.utils.DegToRad( 90 )
		bend.SetMg( TestUtilities.MakeTransformMatrix( rotation=c4d.Vector(0,0,90) ) )
		self.doc.InsertObject( bend, self.plane )
		# Build
		Ox.ComputeStack( self.doc )
		# Test
		expectedSize = c4d.Vector( 264.5, 264.5, 400 )
		TestUtilities.CheckHairBounds( self, self.hair, None, expectedSize, 2 )

	def test_DistributionMesh_PrimitiveTree( self ):
		newPlane = c4d.BaseObject( c4d.Oplane )
		newPlane.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector(400,0,0) ) )
		self.doc.InsertObject( newPlane, self.plane )
		# Build
		Ox.ComputeStack( self.doc )
		# Test
		expectedCenter = c4d.Vector( 200, 5, 0 )
		expectedSize = c4d.Vector( 800, 10, 400 )
		TestUtilities.CheckHairBounds( self, self.hair, expectedCenter, expectedSize, 2 )

	def test_DistributionMesh_PrimitiveUV( self ):
		# avoid sampling errors
		self.plane[c4d.PRIM_PLANE_SUBW] = 100
		self.plane[c4d.PRIM_PLANE_SUBH] = 100
		# make gradient shader concentrated in x < 0
		TestUtilities.MakeAxisAlignedBlackAndWhiteShaderX( self.gfm, c4d.gfm_DistributionMapAttribute )
		# Build
		Ox.ComputeStack( self.doc )
		# Test X center
		expectedCenter = c4d.Vector( -100, 5, 0 )
		expectedSize = c4d.Vector( 200, 10, 400 )
		TestUtilities.CheckHairBounds( self, self.hair, expectedCenter, expectedSize, 5 )
		# make gradient shader concentrated in y < 0
		shader = TestUtilities.MakeAxisAlignedBlackAndWhiteShaderZ( self.gfm, c4d.gfm_DistributionMapAttribute )
		Ox.ComputeStack( self.doc )
		# Test Y center
		expectedCenter = c4d.Vector( 0, 5, -100 )
		expectedSize = c4d.Vector( 400, 10, 200 )
		TestUtilities.CheckHairBounds( self, self.hair, expectedCenter, expectedSize, 5 )


#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_DistributionMesh )
