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
class Test_BakedHair( TestCase ):
	hair = None
	hairTag = None
	gfm = None
	hfg = None
	plane = None

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
		#self.hair[c4d.ho_DebugStackOnConsole] = True
		self.hairTag = self.hair.MakeTag( Ox.res.ID_OX_DISPLAY_TAG, None )
		self.gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		self.gfm[Ox.res.INPUT_MESH] = self.plane
		self.hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		self.hfg[c4d.hfg_RenderCountAttribute] = 10000
		# Insert
		self.doc.InsertObject( self.hair )
		self.doc.InsertObject( self.hfg, self.hair )
		self.doc.InsertObject( self.gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )

	def Bake( self ):
		self.doc.SetActiveObject( self.hfg, c4d.SELECTION_NEW )
		TestUtilities.SendDescriptionCommand( self.hfg, Ox.res.bake_Colapse_Button )
		Ox.ComputeStack( self.doc )
		# Revert Temporary force render count set by StartModifiersCollapse()
		self.hairTag[c4d.disp_ForceRenderMode] = False
		Ox.ComputeStack( self.doc )
		# Cant find a way to properly end baking from Python, HFG is still in the stack
		bakedHair = self.hair.GetDown()
		if bakedHair.GetType() != Ox.res.ID_OX_BAKED_HAIR_MOD:
			bakedHair = self.hfg.GetNext()
		return bakedHair

	def test_BakedHairsCount( self ):
		# Get baked hair
		bakedHair = self.Bake()
		self.assertEqual( bakedHair.GetType(), Ox.res.ID_OX_BAKED_HAIR_MOD, msg="First modifier should have changed to BakedHair" )
		# Test baked hair count, should not be not affected by display fraction
		bakedHairCount = Ox.GetUserData( bakedHair, Ox.res.BAKED_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( bakedHairCount, 10000, delta=3 )
		# Test BakedHair output, must reflect display fraction
		bakedOutputHairCount = Ox.GetUserData( bakedHair, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( bakedOutputHairCount, 1000, delta=30 )

	def test_BakedHairOffset( self ):
		TestUtilities.SendDescriptionCommand( self.hfg, Ox.res.bake_Colapse_Button )
		Ox.ComputeStack( self.doc )
		# Move plane, hair must follow
		self.plane.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( 200, 0, 200 ) ) )
		Ox.ComputeStack( self.doc )
		# Roots must be offset
		vertices = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( self.hair )
		for strandId, points in iter(vertices.items()):
			self.assertGreaterEqual( points[0][0], 0, msg='All roots should be X>0, is ' + str( points[0] )  )
			self.assertGreaterEqual( points[0][2], 0, msg='All roots should be Z>0, is ' + str( points[0] )  )

	def test_BakedHairRenderCount( self ):
		# Test before
		bakedHair = self.Bake()
		hairCount = Ox.GetUserData( bakedHair, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( hairCount, 1000, delta=30 )
		# HairObject RenderCount
		self.hairTag[c4d.disp_ForceRenderMode] = True
		Ox.ComputeStack( self.doc )
		hairCount = Ox.GetUserData( bakedHair, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( hairCount, 10000, delta=3 )
		# Viewport Count
		self.hairTag[c4d.disp_ForceRenderMode] = False
		Ox.ComputeStack( self.doc )
		hairCount = Ox.GetUserData( bakedHair, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( hairCount, 1000, delta=30 )
		# HFG RenderCount
		# Rendercount on modifiers are disabled
		#modTag = bakedHair.MakeTag( Ox.res.ID_OX_DISPLAY_TAG, None )
		#modTag[c4d.disp_ForceRenderMode] = True
		#Ox.ComputeStack( self.doc )
		#hairCount = Ox.GetUserData( bakedHair, Ox.res.OUTPUT_HAIR, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		#self.assertAlmostEqual( hairCount, 10000, delta=3 )

	def test_BakedHairsDetachedPosition( self ):
		bakedHair = self.Bake()
		verticesAttached = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( self.hair )
		# Detach, strands must not change
		bakedHair[c4d.gnd_DetachAttribute] = True
		Ox.ComputeStack( self.doc )
		verticesDetached = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( self.hair )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesAttached, verticesDetached )
		# Reattach, strands must not change
		bakedHair[c4d.gnd_DetachAttribute] = False
		Ox.ComputeStack( self.doc )
		verticesReAttached = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( self.hair )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesAttached, verticesReAttached )




#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_BakedHair )