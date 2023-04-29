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
class Test_GFM( TestCase ):
	doc = None
	hair = None
	gfm = None

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
		self.doc.InsertObject( self.gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )

	def test_GFM_StrandCount( self ):
		guideCount = Ox.GetUserData( self.gfm, Ox.res.OUTPUT_GUIDES, Ox.res.ID_OX_GUIDES_DATA, Ox.res.gd_StrandCount, c4d.DTYPE_LONG )
		#print( "GC = " + str( guideCount ) )
		self.assertEqual( guideCount, 300 )
		# Test output strand count from Hair Object
		outputHairCount = Ox.GetStrandCount( self.hair )
		self.assertEqual( outputHairCount, guideCount )

	def test_GFM_VertexCount( self ):
		vertexCount = Ox.GetUserData( self.gfm, Ox.res.OUTPUT_GUIDES, Ox.res.ID_OX_GUIDES_DATA, Ox.res.gd_VertexCount, c4d.DTYPE_LONG )
		#print( "VC = " + str( vertexCount ) )
		self.assertEqual( vertexCount, 3000 )

	def test_StrandIds( self ):
		strandIds = TestUtilities.GetStrandIds( self.gfm )
		self.assertEqual( len( strandIds ), Ox.GetStrandCount( self.gfm ) )

	def test_GFM_NullMeshUndoRedo( self ):
		guideCount = Ox.GetUserData( self.gfm, Ox.res.OUTPUT_GUIDES, Ox.res.ID_OX_GUIDES_DATA, Ox.res.gd_StrandCount, c4d.DTYPE_LONG )
		self.assertEqual( guideCount, 300 )
		# Remove distribution mesh, count should be zero
		self.doc.StartUndo()
		self.doc.AddUndo( c4d.UNDOTYPE_CHANGE, self.gfm )
		self.gfm[Ox.res.INPUT_MESH] = None
		self.doc.EndUndo()
		Ox.ComputeStack( self.doc )
		guideCountWithoutMesh = Ox.GetUserData( self.gfm, Ox.res.OUTPUT_GUIDES, Ox.res.ID_OX_GUIDES_DATA, Ox.res.gd_StrandCount, c4d.DTYPE_LONG )
		self.assertEqual( guideCountWithoutMesh, 0 )
		# Undo, restore guide count
		# Need to update reference to self.gfm
		self.doc.DoUndo()
		Ox.ComputeStack( self.doc )
		self.gfm = self.doc.SearchObject(self.gfm.GetName())
		guideCountAfterUndo = Ox.GetUserData( self.gfm, Ox.res.OUTPUT_GUIDES, Ox.res.ID_OX_GUIDES_DATA, Ox.res.gd_StrandCount, c4d.DTYPE_LONG )
		self.assertEqual( guideCountAfterUndo, guideCount )
		# Redo, zero guide count
		# Need to update reference to self.gfm
		self.doc.DoRedo()
		Ox.ComputeStack( self.doc )
		self.gfm = self.doc.SearchObject(self.gfm.GetName())
		guideCountAfterRedo = Ox.GetUserData( self.gfm, Ox.res.OUTPUT_GUIDES, Ox.res.ID_OX_GUIDES_DATA, Ox.res.gd_StrandCount, c4d.DTYPE_LONG )
		self.assertEqual( guideCountAfterRedo, 0 )


#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_GFM )