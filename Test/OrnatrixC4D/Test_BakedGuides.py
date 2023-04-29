#----------------
import os
import sys
import c4d
sys.path.insert( 0, os.path.dirname( __file__ ) )
import Ornatrix.Ornatrix as Ox
import TestUtilities
sys.path.pop( 0 )
#----------------
from unittest import TestCase

#
# TestCase
class Test_BakedGuides( TestCase ):
	hair = None
	gfm = None
	hfg = None
	plane = None
	guideCount = 0
	hairCount = 0

	#@classmethod
	#def setUpClass( self ):
	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		# Create Plane
		self.plane = c4d.BaseObject( c4d.Oplane )
		self.doc.InsertObject( self.plane )
		# Create Ornatrix Hair stack
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		self.gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		self.gfm[Ox.res.INPUT_MESH] = self.plane
		self.hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		# Insert
		self.doc.InsertObject( self.hair )
		self.doc.InsertObject( self.hfg, self.hair )
		self.doc.InsertObject( self.gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )
		# Save counts
		self.guideCount = Ox.GetUserData( self.gfm, Ox.res.OUTPUT_GUIDES, Ox.res.ID_OX_GUIDES_DATA, Ox.res.gd_StrandCount, c4d.DTYPE_LONG )
		self.hairCount = Ox.GetUserData( self.hfg, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )

	def test_BakedGuidesCount( self ):
		TestUtilities.SendDescriptionCommand( self.gfm, Ox.res.bake_Colapse_Button )
		Ox.ComputeStack( self.doc )
		# Get baked guides
		# Cant find a way to properly end baking from Python, GFM is still in the stack
		bakedGuides = self.hair.GetDown()
		if bakedGuides.GetType() != Ox.res.ID_OX_BAKED_GUIDES_MOD:
			bakedGuides = self.gfm.GetNext()
		self.assertEqual( bakedGuides.GetType(), Ox.res.ID_OX_BAKED_GUIDES_MOD, msg="First modifier should have changed to BakedGuides" )
		# Test hair count
		bakedGuideCount = Ox.GetUserData( bakedGuides, Ox.res.BAKED_GUIDES, Ox.res.ID_OX_GUIDES_DATA, Ox.res.gd_StrandCount, c4d.DTYPE_LONG )
		bakedOutputGuideCount = Ox.GetUserData( bakedGuides, Ox.res.OUTPUT_GUIDES, Ox.res.ID_OX_GUIDES_DATA, Ox.res.gd_StrandCount, c4d.DTYPE_LONG )
		finalHairCount = Ox.GetUserData( self.hfg, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( bakedGuideCount, self.guideCount, delta=1, msg="Baked Guides count [%d] should be [%d]" %(bakedGuideCount, self.guideCount) )
		self.assertAlmostEqual( bakedOutputGuideCount, self.guideCount, delta=1, msg="Baked Guides Output count [%d] should be [%d]" %(bakedOutputGuideCount, self.guideCount) )
		self.assertAlmostEqual( finalHairCount, self.hairCount, delta=1, msg="Final Hair count [%d] should be [%d]" %(finalHairCount, self.hairCount) )

	def test_BakedGuidesOffset( self ):
		TestUtilities.SendDescriptionCommand( self.gfm, Ox.res.bake_Colapse_Button )
		Ox.ComputeStack( self.doc )
		# Move plane, hair must follow
		self.plane.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( 200, 0, 200 ) ) )
		Ox.ComputeStack( self.doc )
		# Roots must be offset
		vertices = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( self.hair )
		for strandId, points in iter(vertices.items()):
			self.assertGreaterEqual( points[0][0], 0, msg='All roots should be X>0, is ' + str( points[0] )  )
			self.assertGreaterEqual( points[0][2], 0, msg='All roots should be Z>0, is ' + str( points[0] )  )


#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_BakedGuides )
