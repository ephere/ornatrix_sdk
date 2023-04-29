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
from unittest import TestCase

#
# TestCase
class Test_SurfaceComb( TestCase ):
	doc = None
	hair = None
	gfm = None
	comb = None
	verticesBefore = None

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		Ox.DeleteAll( self.doc )
		plane = c4d.BaseObject( c4d.Oplane )
		self.doc.InsertObject( plane )
		# Create stack with Surface Comb
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		self.gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		self.gfm[Ox.res.INPUT_MESH] = plane
		self.comb = c4d.BaseObject( Ox.res.ID_OX_SURFACE_COMB_MOD )
		self.comb[c4d.comb_ChaosAttribute] = 0
		self.doc.InsertObject( self.hair )
		self.gfm.InsertUnderLast( self.hair )
		self.comb.InsertUnderLast( self.hair )
		# Build
		Ox.ComputeStack( self.doc )
		gfm = self.hair.GetDown()
		self.verticesBefore = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( gfm )

	def CreateSink( self, startPos, endPos ):
		args = c4d.BaseContainer()
		args[0] = startPos
		args[1] = endPos
		command = c4d.BaseContainer()
		command[Ox.res.comb_Command_AddSink] = args
		self.comb.Message(c4d.MSG_BASECONTAINER, command)
		self.doc.ExecutePasses( None, True, True, True, c4d.BUILDFLAGS_INTERNALRENDERER )
		#result = command[Ox.res.comb_Command_AddSink]
		#return result[1000]

	# Default Comb without sinks have no effect
	def test_Comb_NoSinks( self ):
		self.assertEqual( self.comb[c4d.comb_AlgorithmAttribute], Ox.res.comb_AlgotihmType_Triangulation )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.comb )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, self.verticesBefore, verticesAfter )

	def test_Comb_Sink( self ):
		self.CreateSink( c4d.Vector(100, 0, 0), c4d.Vector(300, 0, 0) );
		Ox.ComputeStack( self.doc )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.comb )
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, self.verticesBefore, verticesAfter )
		TestUtilities.CheckStrandDirectionByStrandId( self, verticesAfter, 90.0 )

	def test_Comb_Sinks( self ):
		self.CreateSink( c4d.Vector(100, 0, 0), c4d.Vector(300, 0, 0) );
		self.CreateSink( c4d.Vector(0, 0, 100), c4d.Vector(0, 0, 300) );
		Ox.ComputeStack( self.doc )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.comb )
		TestUtilities.CheckStrandDirectionByStrandId( self, verticesAfter, 45.0, 45.0 )

	def test_Comb_OrientationMultiplier( self ):
		TestUtilities.MakeGrayscaleShader( self.comb, c4d.comb_OrientationMultiplierAttribute, 1.0 - (1.0 / 8.0) )
		self.CreateSink( c4d.Vector(100, 0, 0), c4d.Vector(300, 0, 0) );
		Ox.ComputeStack( self.doc )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.comb )
		TestUtilities.CheckStrandDirectionByStrandId( self, verticesAfter, 180.0, 2.0 )

	# MeshVertex method has some displacement by default
	def test_Comb_MeshVertex( self ):
		self.comb[c4d.comb_AlgorithmAttribute] = Ox.res.comb_AlgotihmType_MeshVertex
		Ox.ComputeStack( self.doc )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.comb )
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, self.verticesBefore, verticesAfter )
		# Check displacement
		TestUtilities.CheckStrandPlaneDisplacementByStrandId( self, verticesAfter, 30, 5 )

	def test_Comb_MeshVertex_OrientationMultiplier( self ):
		self.comb[c4d.comb_AlgorithmAttribute] = Ox.res.comb_AlgotihmType_MeshVertex
		Ox.ComputeStack( self.doc )
		verticesOrient1 = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.comb )
		# Change orientation
		TestUtilities.MakeGrayscaleShader( self.comb, c4d.comb_OrientationMultiplierAttribute, 0.75 )
		Ox.ComputeStack( self.doc )
		verticesOrient2 = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.comb )
		# Check displacement
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, verticesOrient1, verticesOrient2 )





#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_SurfaceComb )
