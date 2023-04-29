#----------------
import os, sys, c4d
sys.path.insert(0, os.path.dirname(__file__))
import Ornatrix.Ornatrix as Ox
import TestUtilities
sys.path.pop(0)
#----------------
from unittest import TestCase

#
# TestCase
class Test_HFMS( TestCase ):
	doc = None
	plane = None
	hair = None
	hairTag = None
	hfms = None

	def setUp(self):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll(self.doc)
		# Create Sphere
		self.plane = c4d.BaseObject( c4d.Oplane )
		self.doc.InsertObject( self.plane )
		# Create Ornatrix Hair object
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		self.hairTag = self.hair.MakeTag( Ox.res.ID_OX_DISPLAY_TAG, None )
		self.doc.InsertObject( self.hair )
		# Create HFMS modifier inside Hair Object
		self.hfms = c4d.BaseObject(Ox.res.ID_OX_HAIR_FROM_MESH_STRIPS_MOD)
		self.hfms[Ox.res.INPUT_MESH] = self.plane
		self.hfms[Ox.res.hfms_HairsPerStripCountAttribute] = 100
		self.hfms[Ox.res.hfms_ViewPercentageAttribute] = 0.1
		self.doc.InsertObject(self.hfms, self.hair)
		# Build
		Ox.ComputeStack( self.doc )

	def test_StrandCount(self):
		strandCount = Ox.GetUserData( self.hfms, Ox.res.OUTPUT_STRANDS, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		targetStrandCount = 10
		self.assertEqual(strandCount, targetStrandCount, "Wrong strand count %d, should be %d" % (strandCount,targetStrandCount))

	def test_VertexCount(self):
		vertexCount = Ox.GetUserData( self.hfms, Ox.res.OUTPUT_STRANDS, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_VertexCount, c4d.DTYPE_LONG )
		targetVertexCount = 50
		self.assertEqual(vertexCount, targetVertexCount, "Wrong vertex count %d, should be %d" % (vertexCount,targetVertexCount))

	def test_Volume(self):
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.hfms )
		self.hfms[Ox.res.hfms_VolumeAttribute] = 100
		Ox.ComputeStack( self.doc )
		# Compare
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.hfms )
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, verticesBefore, verticesAfter )

	def test_OverrideVolume(self):
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.hfms )
		# Select strip
		TestUtilities.SendDescriptionCommand( self.hfms, Ox.res.hfms_SelectNextHairStrip_Button )
		Ox.ComputeStack( self.doc )
		# Change Override value
		self.hfms[Ox.res.hfms_StripVolumeOverride] = 100
		Ox.ComputeStack( self.doc )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.hfms )
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, verticesBefore, verticesAfter )
		# Reset override
		TestUtilities.SendDescriptionCommand( self.hfms, Ox.res.hfms_StripOverrideReset_Button )
		Ox.ComputeStack( self.doc )
		verticesAfterReset = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.hfms )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesBefore, verticesAfterReset )

	def test_StripRotation(self):
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.hfms )
		# Select strip
		TestUtilities.SendDescriptionCommand( self.hfms, Ox.res.hfms_SelectNextHairStrip_Button )
		Ox.ComputeStack( self.doc )
		# Rotate strip
		TestUtilities.SendDescriptionCommand( self.hfms, Ox.res.hfms_RotateStrips_Button )
		Ox.ComputeStack( self.doc )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.hfms )
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, verticesBefore, verticesAfter )
		# Reset rotation
		TestUtilities.SendDescriptionCommand( self.hfms, Ox.res.hfms_StripOverrideReset_Button )
		Ox.ComputeStack( self.doc )
		verticesAfterReset = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.hfms )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesBefore, verticesAfterReset )

	def test_RenderCount( self ):
		# Test before
		strandCount = Ox.GetUserData( self.hfms, Ox.res.OUTPUT_STRANDS, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( strandCount, 10 )
		# HairObject RenderCount
		self.hairTag[c4d.disp_ForceRenderMode] = True
		Ox.ComputeStack( self.doc )
		strandCount = Ox.GetUserData( self.hfms, Ox.res.OUTPUT_STRANDS, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( strandCount, 100 )
		# Viewport Count
		self.hairTag[c4d.disp_ForceRenderMode] = False
		Ox.ComputeStack( self.doc )
		strandCount = Ox.GetUserData( self.hfms, Ox.res.OUTPUT_STRANDS, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		self.assertAlmostEqual( strandCount, 10 )
		# HFG RenderCount
		# Rendercount on modifiers are disabled
		#modTag = self.hfms.MakeTag( Ox.res.ID_OX_DISPLAY_TAG, None )
		#modTag[c4d.disp_ForceRenderMode] = True
		#Ox.ComputeStack( self.doc )
		#strandCount = Ox.GetUserData( self.hfms, Ox.res.OUTPUT_STRANDS, Ox.res.ID_OX_HAIR_DATA, Ox.res.hd_StrandCount, c4d.DTYPE_LONG )
		#self.assertAlmostEqual( strandCount, 100 )

	def test_MultipleMeshes(self):
		cylinder = c4d.BaseObject( c4d.Ocylinder )
		cylinder[c4d.PRIM_CYLINDER_CAPS] = False
		self.doc.InsertObject( cylinder )
		meshList = c4d.InExcludeData()
		meshList.InsertObject( self.plane, 1 )
		meshList.InsertObject( cylinder, 1 )
		self.hfms[Ox.res.hfms_InputMeshStrips] = meshList
		self.hfms[Ox.res.hfms_PointsPerStrandCountAttribute] = 10
		self.hfms[Ox.res.hfms_DistributionMethodAttribute] = Ox.res.hfms_DistributionMethod_PerVertex
		Ox.ComputeStack( self.doc )
		TestUtilities.CheckHairBounds( self, self.hair, c4d.Vector(0,0,0), c4d.Vector(self.plane[c4d.PRIM_PLANE_WIDTH], cylinder[c4d.PRIM_CYLINDER_HEIGHT], self.plane[c4d.PRIM_PLANE_HEIGHT] ), 2 )


#----------------------------------
# main
testObject = None
if __name__=='__main__':
	Ox.runTest(doc, Test_HFMS)