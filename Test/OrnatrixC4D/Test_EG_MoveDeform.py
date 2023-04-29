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
class Test_EG_MoveDeform( TestCase ):
	eg = None

	def setUp( self ):
		bugDocument = TestUtilities.FindTestFile( self, 'OxC4d_Test_EG_MoveDeform_scene.c4d' )
		c4d.documents.LoadFile( bugDocument )
		self.doc = c4d.documents.GetActiveDocument()
		self.eg = self.doc.SearchObject( "b_shape" )
		self.doc.SetActiveObject( self.eg )
		Ox.ComputeStack( self.doc )

	@unittest.skip('Test is expected to fail, made to recreate bug')
	def test_EG_MoveStrandDeform( self ):
		moveStrandIdIndex = 1
		strandIds = TestUtilities.GetStrandIds( self.eg )
		moveStrandId = strandIds[moveStrandIdIndex]

		verticesBeforeMove = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.eg )
		verticesBeforeMove.pop( moveStrandId )
		TestUtilities.SendDescriptionCommand( self.eg, Ox.res.edg_Button_EditMode_Root )
		TestUtilities.SelectStrandsByStrandIds( self.doc, self.eg, [moveStrandId] )
		Ox.ComputeStack( self.doc )

		# Simulate user transfrom start, will allow EG to be transformed
		self.eg.Message(c4d.MSG_MOVE_START)
		Ox.ComputeStack( self.doc )
		# Transform EG
		mg = TestUtilities.MakeTransformMatrix( position=c4d.Vector(-0.5, 0.0, 0.0) )
		self.eg.SetMg( mg )
		Ox.ComputeStack( self.doc )
		# Simulate user transform end, will commit and lock EG transform
		self.eg.Message(c4d.MSG_MOVE_FINISHED)
		Ox.ComputeStack( self.doc )

		verticesAfterMove = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.eg )
		verticesAfterMove.pop( moveStrandId )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesBeforeMove, verticesAfterMove )

#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_EG_MoveDeform )
