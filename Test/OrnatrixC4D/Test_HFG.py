#----------------
import os
import sys
import c4d
sys.path.insert( 0, os.path.dirname( __file__ ) )
import Ornatrix.Ornatrix as Ox
#reload(Ox)
import TestUtilities
sys.path.pop( 0 )
#----------------
from unittest import TestCase

#
# TestCase
class Test_HFG( TestCase ):
	doc = None
	hair = None
	gfm = None
	hfg = None

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		# Create Sphere
		sphere = c4d.BaseObject( c4d.Osphere )
		self.doc.InsertObject( sphere )
		# Create Ornatrix Hair object
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		self.doc.InsertObject( self.hair )
		# Create GFM modifier inside Hair Object
		self.gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		self.gfm[Ox.res.INPUT_MESH] = sphere
		self.hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		self.doc.InsertObject( self.hfg, self.hair )
		self.doc.InsertObject( self.gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )

	def test_StrandCount( self ):
		hairCount = Ox.GetUserData( self.hfg, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		#print( "GC = " + str( hairCount ) )
		self.assertAlmostEqual( hairCount, 1000, delta=2 )
		# Test output strand count from Hair Object
		outputHairCount = Ox.GetStrandCount( self.hair )
		self.assertEqual( outputHairCount, hairCount )

	def test_VertexCount( self ):
		vertexCount = Ox.GetUserData( self.hfg, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_VertexCount, c4d.DTYPE_LONG )
		#print( "VC = " + str( vertexCount ) )
		self.assertAlmostEqual( vertexCount, 10000, delta=20 )

	def test_StrandIds( self ):
		strandIds = TestUtilities.GetStrandIds( self.hfg )
		self.assertEqual( len( strandIds ), Ox.GetStrandCount( self.hfg ) )

	def test_NullMeshUndoRedo( self ):
		hairCount = Ox.GetUserData( self.hfg, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( hairCount, 1000, delta=2 )
		# Remove distribution mesh, count should be zero
		self.doc.StartUndo()
		self.doc.AddUndo( c4d.UNDOTYPE_CHANGE, self.gfm )
		self.gfm[Ox.res.INPUT_MESH] = None
		self.doc.EndUndo()
		Ox.ComputeStack( self.doc )
		hairCountWithNullMesh = Ox.GetUserData( self.hfg, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertEqual( hairCountWithNullMesh, 0 )
		outputHairCountWithNullMesh = Ox.GetStrandCount( self.hair )
		self.assertEqual( outputHairCountWithNullMesh, 0 )
		# Undo, restore guide count
		# Need to update reference to self.hfg
		self.doc.DoUndo()
		Ox.ComputeStack( self.doc )
		self.hfg = self.doc.SearchObject(self.hfg.GetName())
		hairCountAfterUndo = Ox.GetUserData( self.hfg, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertEqual( hairCountAfterUndo, hairCount )
		outputHairCountAfterUndo = Ox.GetStrandCount( self.hair )
		self.assertEqual( outputHairCountAfterUndo, hairCount )
		# Redo, zero guide count
		# Need to update reference to self.hfg
		self.doc.DoRedo()
		Ox.ComputeStack( self.doc )
		self.hfg = self.doc.SearchObject(self.hfg.GetName())
		hairCountAfterRedo = Ox.GetUserData( self.hfg, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertEqual( hairCountAfterRedo, 0 )
		outputHairCountAfterRedo = Ox.GetStrandCount( self.hair )
		self.assertEqual( outputHairCountAfterRedo, 0 )

	def test_DisableGfmUndoRedo( self ):
		hairCount = Ox.GetUserData( self.hfg, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( hairCount, 1000, delta=2 )
		# Disable GuidesFromMesh, count should be zero
		self.doc.StartUndo()
		self.doc.AddUndo( c4d.UNDOTYPE_CHANGE, self.gfm )
		self.gfm.SetDeformMode( False )
		self.doc.EndUndo()
		Ox.ComputeStack( self.doc )
		# HFG is being ignored, its value cant be tested
		#hairCountWithGfmDisabled = Ox.GetUserData( self.hfg, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		#self.assertEqual( hairCountWithGfmDisabled, 0 )
		# Test output hair only
		outputHairCountWithGfmDisabled = Ox.GetStrandCount( self.hair )
		self.assertEqual( outputHairCountWithGfmDisabled, 0 )
		# Undo, restore guide count
		# Need to update reference to self.hfg
		return
		self.doc.DoUndo()
		Ox.ComputeStack( self.doc )
		self.hfg = self.doc.SearchObject(self.hfg.GetName())
		outputHairCountAfterUndo = Ox.GetStrandCount( self.hair )
		self.assertEqual( outputHairCountAfterUndo, hairCount )
		# Redo, zero guide count
		# Need to update reference to self.hfg
		self.doc.DoRedo()
		Ox.ComputeStack( self.doc )
		self.hfg = self.doc.SearchObject(self.hfg.GetName())
		outputHairCountAfterRedo = Ox.GetStrandCount( self.hair )
		self.assertEqual( outputHairCountAfterRedo, 0 )


#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_HFG )