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
class Test_MFS( TestCase ):
	hair = None
	gfm = None
	hfg = None
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
		# Create Sphere
		plane = c4d.BaseObject( c4d.Oplane )
		self.doc.InsertObject( plane )
		# Create Ornatrix Hair stack
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		self.gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		self.gfm[Ox.res.INPUT_MESH] = plane
		self.hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		self.mfs = c4d.BaseObject( Ox.res.ID_OX_MESH_FROM_STRANDS_MOD )
		# Insert
		self.doc.InsertObject( self.hair )
		self.doc.InsertObject( self.mfs, self.hair )
		self.doc.InsertObject( self.hfg, self.hair )
		self.doc.InsertObject( self.gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )
		# Save counts
		self.pointCount = self.gfm[Ox.res.gfm_GuidePointCountAttribute]
		self.strandCount = Ox.GetUserData( self.hfg, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )

	def test_MFS_RibbonPolyCount( self ):
		self.mfs[Ox.res.mfs_MeshTypeAttribute] = Ox.res.mfs_MeshType_Ribbon
		(polyCount,targetPolyCount) = self.CachePolygonAndCalculatePolyCount(1, "RibbonHair")
		self.assertEqual( polyCount, targetPolyCount, msg="Ribbon sides [%d] mesh polygon count [%d] should be [%d]" %(self.mfs[Ox.res.mfs_SideCountAttribute], polyCount, targetPolyCount) )
		(polyCount,targetPolyCount) = self.CachePolygonAndCalculatePolyCount(3, "RibbonHair")
		self.assertEqual( polyCount, targetPolyCount, msg="Ribbon sides [%d] mesh polygon count [%d] should be [%d]" %(self.mfs[Ox.res.mfs_SideCountAttribute], polyCount, targetPolyCount) )

	def test_MFS_CylindricalPolyCount( self ):
		self.mfs[Ox.res.mfs_MeshTypeAttribute] = Ox.res.mfs_MeshType_Cylindrical
		(polyCount,targetPolyCount) = self.CachePolygonAndCalculatePolyCount(3, "CylindricalHair")
		self.assertEqual( polyCount, targetPolyCount, msg="Cylindrical sides [%d] mesh polygon count [%d] should be [%d]" %(self.mfs[Ox.res.mfs_SideCountAttribute], polyCount, targetPolyCount) )
		(polyCount,targetPolyCount) = self.CachePolygonAndCalculatePolyCount(8, "CylindricalHair")
		self.assertEqual( polyCount, targetPolyCount, msg="Cylindrical sides [%d] mesh polygon count [%d] should be [%d]" %(self.mfs[Ox.res.mfs_SideCountAttribute], polyCount, targetPolyCount) )

	def test_MFS_ZProxyPolyCount( self ):
		# Create proxy mesh
		pyramid = c4d.BaseObject( c4d.Opyramid )
		pyramid[c4d.PRIM_PYRAMID_LEN] = c4d.Vector(10,100,10)
		self.doc.InsertObject( pyramid )
		proxyMesh = TestUtilities.CachePrimitiveIntoNewObject(pyramid, self.doc, "Proxy")
		#c4d.utils.SendModelingCommand(command=c4d.MCOMMAND_TRIANGULATE,list=[proxyMesh],mode=c4d.MODELINGCOMMANDMODE_ALL,doc=self.doc)
		Ox.ComputeStack( self.doc )
		proxyPolyCount = proxyMesh.GetPolygonCount()
		meshList = self.mfs[Ox.res.mfs_ProxyMeshesAttribute]
		meshList.InsertObject(proxyMesh,0)
		self.mfs[Ox.res.mfs_ProxyMeshesAttribute] = meshList
		self.mfs[Ox.res.mfs_MeshTypeAttribute] = Ox.res.mfs_MeshType_ProxyMesh
		Ox.ComputeStack( self.doc )
		mesh = TestUtilities.CachePrimitiveIntoNewObject(self.hair, self.doc, "ProxyMeshHair")
		meshPolyCount = mesh.GetPolygonCount()
		targetPolyCount = self.strandCount * proxyPolyCount
		self.assertEqual( meshPolyCount, targetPolyCount, msg="Proxy poly count [%d] mesh polygon count [%d] should be [%d]" %(proxyPolyCount, meshPolyCount, targetPolyCount) )

	def test_MFS_ProxyTransformed( self ):
		# Create proxy mesh
		pyramid = c4d.BaseObject( c4d.Opyramid )
		pyramid[c4d.PRIM_PYRAMID_LEN] = c4d.Vector(10,100,10)
		pyramid[c4d.PRIM_AXIS] = 4
		pyramid.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( 250, 250, 250 ) ) )
		self.doc.InsertObject( pyramid )
		meshList = self.mfs[Ox.res.mfs_ProxyMeshesAttribute]
		meshList.InsertObject(pyramid,0)
		self.mfs[Ox.res.mfs_ProxyMeshesAttribute] = meshList
		self.mfs[Ox.res.mfs_MeshTypeAttribute] = Ox.res.mfs_MeshType_ProxyMesh
		Ox.ComputeStack( self.doc )
		mesh = TestUtilities.CachePrimitiveIntoNewObject(self.hair, self.doc, "ProxyMeshHair")
		meshyCenter = mesh.GetMp()
		meshRadius = mesh.GetRad()
		self.assertAlmostEqual( meshyCenter.x, 0, delta=1, msg="Generated mesh center X is [%d] should be ~0" %(meshyCenter.x) )
		self.assertAlmostEqual( meshyCenter.z, 0, delta=1, msg="Generated mesh center Z is [%d] should be ~0" %(meshyCenter.z) )
		self.assertAlmostEqual( meshyCenter.y, 25, delta=2, msg="Generated mesh center Y is [%d] should be ~0" %(meshyCenter.y) )
		self.assertAlmostEqual( meshRadius.x, 205, delta=1, msg="Generated mesh radius X is [%d] should be ~0" %(meshRadius.x) )
		self.assertAlmostEqual( meshRadius.z, 205, delta=1, msg="Generated mesh radius Z is [%d] should be ~0" %(meshRadius.z) )
		self.assertAlmostEqual( meshRadius.y, 25, delta=2, msg="Generated mesh radius Y is [%d] should be ~0" %(meshRadius.y) )


#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_MFS )