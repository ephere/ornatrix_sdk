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
class Test_GuidesFromHair( TestCase ):
	hair = None
	gfm = None
	hfg = None
	guideCount = 0


	#@classmethod
	#def setUpClass( self ):
	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		# Create Plane
		plane = c4d.BaseObject( c4d.Oplane )
		self.doc.InsertObject( plane )
		# Create Ornatrix Hair stack
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		self.hair[c4d.ho_DebugStackOnConsole] = True
		self.gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		self.gfm[Ox.res.INPUT_MESH] = plane
		self.hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		self.hfg[Ox.res.hfg_RootGenerationMethodAttribute] = Ox.res.hfg_RootGenerationMethodAttribute_GUIDE_AS_HAIR
		# Insert
		self.doc.InsertObject( self.hair )
		self.doc.InsertObject( self.hfg, self.hair )
		self.doc.InsertObject( self.gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )
		# Save counts
		self.guideCount = self.gfm[c4d.gfm_GuideCountAttribute]


	def test_HairCount( self ):
		self.doc.SetActiveObject( self.hfg, c4d.SELECTION_NEW )
		TestUtilities.SendDescriptionCommand( self.hfg, Ox.res.bake_Colapse_Button )
		Ox.ComputeStack( self.doc )
		# Get baked hair
		# Cant find a way to properly end baking from Python, HFG is still in the stack
		bakedHair = self.hair.GetDown()
		if bakedHair.GetType() != Ox.res.ID_OX_BAKED_HAIR_MOD:
			bakedHair = self.hfg.GetNext()
		self.assertEqual( bakedHair.GetType(), Ox.res.ID_OX_BAKED_HAIR_MOD, msg="First modifier should have changed to BakedHair" )
		bakedHair[c4d.bkh_DisplayFractionAttribute] = 1
		# Add Guides and hair over baked
		Ox.ComputeStack( self.doc )
		# Test guide count
		bakedOutputHairCount = Ox.GetUserData( bakedHair, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( bakedOutputHairCount, self.guideCount, delta=1, msg="Baked Hair count [%d] should be [%d]" %(bakedOutputHairCount, self.guideCount) )
		# Test final hair
		gfh = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_HAIR_MOD )
		hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		self.doc.InsertObject( hfg, self.hair, bakedHair )
		self.doc.InsertObject( gfh, self.hair, bakedHair )
		Ox.ComputeStack( self.doc )
		# Test hair count
		expectedHairCount = hfg[c4d.hfg_RenderCountAttribute]
		finalHairCount = Ox.GetUserData( hfg, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( finalHairCount, expectedHairCount, delta=1, msg="Final Hair count [%d] should be [%d]" %(finalHairCount, expectedHairCount) )




#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_GuidesFromHair )
