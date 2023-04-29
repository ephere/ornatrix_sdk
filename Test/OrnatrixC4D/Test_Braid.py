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
class Test_Braid( TestCase ):
	hair = None
	braid = None
	mfs = None
	pointCount = 0
	strandCount = 0

	def CachePolygonAndCalculatePolyCount( self, sideCount, polyName ):
		self.mfs[Ox.res.mfs_SideCountAttribute] = sideCount
		Ox.ComputeStack( self.doc )
		poly = TestUtilities.CachePrimitiveIntoNewObject(self.hair, self.doc, polyName)
		polyCount = poly.GetPolygonCount()
		targetPolyCount = self.strandCount * ( self.pointCount - 1 ) * sideCount
		return(polyCount,targetPolyCount)

	#@classmethod
	#def setUpClass( self ):
	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		# Create Ornatrix Hair stack
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		self.braid = c4d.BaseObject( Ox.res.ID_OX_BRAID_GUIDES_MOD )
		self.mfs = c4d.BaseObject( Ox.res.ID_OX_MESH_FROM_STRANDS_MOD )
		# Insert
		self.doc.InsertObject( self.hair )
		self.doc.InsertObject( self.mfs, self.hair )
		self.doc.InsertObject( self.braid, self.hair )
		# Build
		Ox.ComputeStack( self.doc )
		# Save counts
		self.pointCount = self.braid[Ox.res.braid_PointCountAttribute]
		self.strandCount = Ox.GetUserData( self.braid, Ox.res.OUTPUT_GUIDES, Ox.res.ID_OX_GUIDES_DATA, Ox.res.gd_StrandCount, c4d.DTYPE_LONG )

	def test_Braid_StrandCount(self):
		targetStrandCount = 3
		self.assertEqual( self.strandCount, targetStrandCount, "Wrong strand count %d, should be %d" % (self.strandCount,targetStrandCount))

	def test_MFS_RibbonPolyCount( self ):
		(polyCount, targetPolyCount) = self.CachePolygonAndCalculatePolyCount(1, "BraidMesh")
		self.assertEqual( polyCount, targetPolyCount, msg="Ribbon sides [%d] mesh polygon count [%d] should be [%d]" %(self.mfs[Ox.res.mfs_SideCountAttribute], polyCount, targetPolyCount) )
		(polyCount, targetPolyCount) = self.CachePolygonAndCalculatePolyCount(3, "BraidMesh")
		self.assertEqual( polyCount, targetPolyCount, msg="Ribbon sides [%d] mesh polygon count [%d] should be [%d]" %(self.mfs[Ox.res.mfs_SideCountAttribute], polyCount, targetPolyCount) )



#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_Braid )
