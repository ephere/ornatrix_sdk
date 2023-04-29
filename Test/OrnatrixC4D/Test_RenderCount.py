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
class Test_RenderCount( TestCase ):
	doc = None
	hair = None
	hairTag = None
	gfm = None
	hfg = None

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		# Create Sphere
		plane = c4d.BaseObject( c4d.Oplane )
		self.doc.InsertObject( plane )
		# Create Ornatrix Hair object
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		self.doc.InsertObject( self.hair )
		self.hairTag = self.hair.MakeTag( Ox.res.ID_OX_DISPLAY_TAG, None )
		# Create GFM modifier inside Hair Object
		self.gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		self.gfm[Ox.res.INPUT_MESH] = plane
		self.gfm[c4d.gfm_GuidePointCountAttribute] = 3
		self.hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		self.hfg[c4d.hfg_RenderCountAttribute] = 1000
		self.hfg[c4d.hfg_ViewportCountFractionAttribute] = 0.1
		self.doc.InsertObject( self.hfg, self.hair )
		self.doc.InsertObject( self.gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )

	def test_HairFromGuidesRenderCount( self ):
		# Test before
		mod = self.hfg
		hairCount = Ox.GetUserData( mod, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( hairCount, 100, delta=2 )
		# HairObject RenderCount
		self.hairTag[c4d.disp_ForceRenderMode] = True
		Ox.ComputeStack( self.doc )
		hairCount = Ox.GetUserData( mod, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( hairCount, 1000, delta=10 )
		# Viewport Count
		self.hairTag[c4d.disp_ForceRenderMode] = False
		Ox.ComputeStack( self.doc )
		hairCount = Ox.GetUserData( mod, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( hairCount, 100, delta=2 )
		# HFG RenderCount
		# Rendercount on modifiers are disabled
		#modTag = mod.MakeTag( Ox.res.ID_OX_DISPLAY_TAG, None )
		#modTag[c4d.disp_ForceRenderMode] = True
		#Ox.ComputeStack( self.doc )
		#hairCount = Ox.GetUserData( mod, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		#self.assertAlmostEqual( hairCount, 1000, delta=10 )

	def test_DetailRenderCount( self ):
		# Test before
		vertexCount = Ox.GetUserData( self.hfg, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_VertexCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( vertexCount, 300, delta=30 )
		# Create Detail tag
		detail = c4d.BaseObject( Ox.res.ID_OX_DETAIL_MOD )
		detail[c4d.dtl_ViewPointCountAttribute] = 5
		detail[c4d.dtl_RenderPointCountAttribute] = 10
		self.doc.InsertObject( detail, self.hair, self.hfg )
		mod = detail
		# HFG + Detail
		self.hairTag[c4d.disp_ForceRenderMode] = True
		Ox.ComputeStack( self.doc )
		vertexCount = Ox.GetUserData( mod, Ox.res.OUTPUT_STRANDS, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_VertexCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( vertexCount, 10000, delta=30 )
		# Viewport Count
		self.hairTag[c4d.disp_ForceRenderMode] = False
		Ox.ComputeStack( self.doc )
		vertexCount = Ox.GetUserData( mod, Ox.res.OUTPUT_STRANDS, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_VertexCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( vertexCount, 500, delta=30 )
		# Detail only
		# Rendercount on modifiers are disabled
		#modTag = mod.MakeTag( Ox.res.ID_OX_DISPLAY_TAG, None )
		#modTag[c4d.disp_ForceRenderMode] = True
		#Ox.ComputeStack( self.doc )
		#vertexCount = Ox.GetUserData( mod, Ox.res.OUTPUT_STRANDS, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_VertexCount, c4d.DTYPE_LONG )
		#self.assertAlmostEqual( vertexCount, 1000, delta=30 )


#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_RenderCount )