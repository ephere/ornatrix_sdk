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
from unittest import TestCase, skip

#
# TestCase
class Test_BakeModifiers( TestCase ):
	doc = None
	hair = None
	gfm = None
	length = None
	eg = None
	hfg = None
	frizz = None
	cw = None

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
		self.gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		self.gfm[Ox.res.INPUT_MESH] = plane
		self.gfm[c4d.gfm_GuideCountAttribute] = 300
		self.length = c4d.BaseObject( Ox.res.ID_OX_LENGTH_MOD )
		self.length[Ox.res.len_ValueAttribute] = 2
		self.eg = c4d.BaseObject( Ox.res.ID_OX_EDIT_GUIDES_MOD )
		self.hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		self.hfg[c4d.hfg_RenderCountAttribute] = 1000
		self.hfg[c4d.hfg_ViewportCountFractionAttribute] = 1
		self.frizz = c4d.BaseObject( Ox.res.ID_OX_FRIZZ_MOD )
		self.cw = c4d.BaseObject( Ox.res.ID_OX_CHANGE_WIDTH_MOD )
		# Insert
		self.doc.InsertObject( self.hair )
		self.doc.InsertObject( self.cw, self.hair )
		self.doc.InsertObject( self.frizz, self.hair )
		self.doc.InsertObject( self.hfg, self.hair )
		self.doc.InsertObject( self.eg, self.hair )
		self.doc.InsertObject( self.length, self.hair )
		self.doc.InsertObject( self.gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )

	def Bake( self, selectedModifiers ):
		for idx, val in enumerate( selectedModifiers ):
			self.doc.SetActiveObject( val, c4d.SELECTION_NEW if idx == 0 else c4d.SELECTION_ADD )
		TestUtilities.SendDescriptionCommand( self.hfg, Ox.res.bake_BakeModifiers_Button )
		Ox.ComputeStack( self.doc )

	def test_GuidesOnly( self ):
		# Disable HFG
		self.hfg.SetDeformMode( False )
		Ox.ComputeStack( self.doc )
		hairCountBefore = Ox.GetStrandCount( self.hair )
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinatesSorted( self.cw )
		# Bake modifiers
		self.Bake( [self.length, self.eg, self.hfg, self.frizz] )
		self.assertEqual( len( self.hair.GetChildren() ), 3 )
		# Test baked hair count, should not be not affected by display fraction
		hairCountAfter = Ox.GetStrandCount( self.hair )
		self.assertAlmostEqual( hairCountBefore, 300, delta=3 )
		self.assertAlmostEqual( hairCountBefore, hairCountAfter, delta=3 )
		# Test BakedHair output, must reflect display fraction
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesSorted( self.cw )
		TestUtilities.CheckPointsAllNearEqual( self, verticesBefore, verticesAfter )

	def test_GuidesToHair( self ):
		hairCountBefore = Ox.GetStrandCount( self.hair )
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinatesSorted( self.cw )
		# Bake modifiers
		self.Bake( [self.length, self.eg, self.hfg, self.frizz] )
		self.assertEqual( len( self.hair.GetChildren() ), 3)
		# Test baked hair count, should not be not affected by display fraction
		hairCountAfter = Ox.GetStrandCount( self.hair )
		self.assertAlmostEqual( hairCountBefore, 1000, delta=3 )
		self.assertAlmostEqual( hairCountBefore, hairCountAfter, delta=3 )
		# Test BakedHair output, must reflect display fraction
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesSorted( self.cw )
		TestUtilities.CheckPointsAllNearEqual( self, verticesBefore, verticesAfter )

	def test_SingleOperator( self ):
		hairCountBefore = Ox.GetStrandCount( self.hair )
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinatesSorted( self.cw )
		# Bake modifiers
		self.Bake( [self.length] )
		self.assertEqual( len( self.hair.GetChildren() ), 6 )
		# Test baked hair count, should not be not affected by display fraction
		hairCountAfter = Ox.GetStrandCount( self.hair )
		self.assertAlmostEqual( hairCountBefore, 1000, delta=3 )
		self.assertAlmostEqual( hairCountBefore, hairCountAfter, delta=3 )
		# Test BakedHair output, must reflect display fraction
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesSorted( self.cw )
		TestUtilities.CheckPointsAllNearEqual( self, verticesBefore, verticesAfter )


#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_BakeModifiers )