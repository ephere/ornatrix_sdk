#----------------
from json import load
import os
import sys
import unittest
import c4d
sys.path.insert( 0, os.path.dirname( __file__ ) )
import Ornatrix.Ornatrix as Ox
import TestUtilities
sys.path.pop( 0 )
#----------------
from unittest import TestCase

#
# TestCase
class Test_GroundStrands_TransformedSurface( TestCase ):
	hair = None
	groundModifier = None
	baseMesh = None
	surfaceMesh = None

	def setUp( self ):
		bugDocument = TestUtilities.FindTestFile( self, 'GroundStrandsTransformedSurface.c4d' )
		c4d.documents.LoadFile( bugDocument )
		self.doc = c4d.documents.GetActiveDocument()
		self.hair = self.doc.SearchObject( "Ornatrix Hair" )
		self.groundModifier = self.doc.SearchObject( "Ground Strands" )
		self.baseMesh = self.doc.SearchObject( "Plane" )
		self.surfaceMesh = self.doc.SearchObject( "Plane.1" )
		self.doc.SetActiveObject( self.hair )
		Ox.ComputeStack( self.doc )
		
	def test_EG_MoveStrandDeform( self ):
		verticesBeforeGround = TestUtilities.GetVerticesInObjectCoordinatesByStrandId2( self.hair, self.baseMesh.GetMg() )

		self.groundModifier[Ox.res.gnd_DetachAttribute] = False
		Ox.ComputeStack( self.doc )

		verticesAfterGround = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( self.hair )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesBeforeGround, verticesAfterGround )

#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_GroundStrands_TransformedSurface )
