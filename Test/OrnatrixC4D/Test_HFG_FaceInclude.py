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
class Test_HFGFaceInclude( TestCase ):
	hair = None
	hfg = None

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		# Create Plane polygon
		plane = c4d.BaseObject( c4d.Oplane )
		plane[c4d.PRIM_PLANE_SUBW] = 2
		plane[c4d.PRIM_PLANE_SUBH] = 2
		self.doc.InsertObject( plane )
		poly = TestUtilities.CachePrimitiveIntoNewObject(plane, self.doc)
		self.doc.SetActiveObject(poly)
		c4d.CallCommand(12187) # Polygon selection mode
		poly.GetPolygonS().Select(0)
		c4d.CallCommand(12552) # Set Selection tag
		sel = poly.GetTag(c4d.Tpolygonselection)
		# Create Ornatrix Hair object
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		#self.hair[Ox.res.ho_DebugStackOnConsole] = True
		self.doc.InsertObject( self.hair )
		# Create GFM modifier inside Hair Object
		gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		gfm[Ox.res.INPUT_MESH] = poly
		gfm[Ox.res.gfm_GuideCountAttribute] = 100
		gfm[Ox.res.gfm_GuidePointCountAttribute] = 2
		self.hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		self.hfg[Ox.res.hfg_RenderCountAttribute] = 1000
		self.hfg[Ox.res.hfg_ViewportCountFractionAttribute] = 1.0
		self.hfg[Ox.res.hfg_FaceIncludeSelectionTagAttribute] = sel
		self.hfg[Ox.res.hfg_UseFaceIncludeAttribute] = True
		self.doc.InsertObject( self.hfg, self.hair )
		self.doc.InsertObject( gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )

	def test_HFG_FaceInclude_StrandCount( self ):
		strandCount = Ox.GetUserData( self.hfg, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		#print "SC = " + str( strandCount )
		self.assertTrue( strandCount >= 245 and strandCount <= 250 )

	def test_HFG_FaceInclude_Positions( self ):
		vertices = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.hfg )
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
	Ox.runTest( doc, Test_HFGFaceInclude )
