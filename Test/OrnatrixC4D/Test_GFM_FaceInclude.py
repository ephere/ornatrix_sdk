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
class Test_GFM_FaceInclude( TestCase ):
	useTagAsDistributionMesh = False
	redistributeHair = False
	hair = None
	gfm = None
	poly = None

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		# Create Plane polygon
		plane = c4d.BaseObject( c4d.Oplane )
		plane[c4d.PRIM_PLANE_SUBW] = 2
		plane[c4d.PRIM_PLANE_SUBH] = 2
		self.doc.InsertObject( plane )
		self.poly = TestUtilities.CachePrimitiveIntoNewObject( plane, self.doc )
		self.doc.SetActiveObject( self.poly )
		c4d.CallCommand(12187) # Polygon selection mode
		self.poly.GetPolygonS().Select(0)
		c4d.CallCommand(12552) # Set Selection tag
		sel = self.poly.GetTag(c4d.Tpolygonselection)
		# Create Ornatrix Hair object
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		#self.hair[Ox.res.ho_DebugStackOnConsole] = True
		self.doc.InsertObject( self.hair )
		# Create GFM modifier inside Hair Object
		self.gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		if self.useTagAsDistributionMesh:
			self.gfm[Ox.res.INPUT_MESH] = sel
		else:
			self.gfm[Ox.res.INPUT_MESH] = self.poly
			self.gfm[Ox.res.gfm_FaceIncludeSelectionTagAttribute] = sel
		self.gfm[Ox.res.gfm_GuideCountAttribute] = 100
		self.gfm[Ox.res.gfm_GuidePointCountAttribute] = 2
		self.gfm[Ox.res.gfm_UseFaceIncludeAttribute] = True
		self.gfm[Ox.res.gfm_RedistributeOnFaceSelectionAttribute] = self.redistributeHair
		self.doc.InsertObject( self.gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )

	def test_GFM_FaceInclude_StrandCount( self ):
		guideCount = Ox.GetUserData( self.gfm, Ox.res.OUTPUT_GUIDES, Ox.res.ID_OX_GUIDES_DATA, Ox.res.gd_StrandCount, c4d.DTYPE_LONG )
		#print "GC = " + str( guideCount )
		self.assertEqual( guideCount, 100 if self.redistributeHair else 25 )

	def test_GFM_FaceInclude_Positions( self ):
		vertices = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.gfm )
		allPointsAreNegative = True
		for strandId, points in iter(vertices.items()):
			for p in points:
				if( p[0] > 0 or p[2] > 0 ):
					allPointsAreNegative = False
					break
		self.assertTrue( allPointsAreNegative, 'All vertices coordinates should be negative' )

	def test_GFM_FaceInclude_SubdivisionSurface( self ):
		# face selection must be redistributed to subdivided surfaces
		subdiv = c4d.BaseObject( 1007455 )
		subdiv[c4d.SDSOBJECT_TYPE] = c4d.SDSOBJECT_TYPE_OSD_BILINEAR
		self.doc.InsertObject( subdiv )
		self.poly.InsertUnder( subdiv )
		if not self.useTagAsDistributionMesh:
			self.gfm[Ox.res.INPUT_MESH] = subdiv
		# Build
		Ox.ComputeStack( self.doc )
		# Test
		vertices = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.gfm )
		allPointsAreNegative = True
		for strandId, points in iter(vertices.items()):
			for p in points:
				if( p[0] > 0 or p[2] > 0 ):
					allPointsAreNegative = False
					break
		self.assertTrue( allPointsAreNegative, 'All vertices coordinates should be negative' )



#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_GFM_FaceInclude )
