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
class Test_ClumpModifier( TestCase ):

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		Ox.DeleteAll( self.doc )
		# Create base object
		self.hair = TestUtilities.AddGuidesToNewPlane( planeSize=400, planeSegmentCount=20, guideCount=20, pointsPerStrandCount=2)
		self.gfm = self.hair.GetDown()
		# Add clump
		self.clump = c4d.BaseObject( Ox.res.ID_OX_CLUMP_MOD )
		self.clump[Ox.res.clump_ClumpCountAttribute] = 20
		self.clump[Ox.res.clump_FlyawayFractionAttribute] = 0
		self.clump.InsertUnderLast( self.hair )
		# Build
		Ox.ComputeStack( self.doc )
		# Generate clumps
		TestUtilities.SendDescriptionCommand (self.clump, Ox.res.clump_Button_CreateClumps )
		# Build
		Ox.ComputeStack( self.doc )
		self.verticesBeforeClump = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.gfm )
		self.verticesAfterClump = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.clump )

	def tearDown( self ):
		#Ox.DeleteAll( self.doc )
		self.doc = None

	# Test if Clump modifier changed original guides
	def test_Clump_StrandsChanged( self ):
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, self.verticesBeforeClump, self.verticesAfterClump )

	# Test if Clump modifier changed original guides
	def test_Clump_Undo( self, undoCount=1 ):
		# Undo second clump
		if undoCount == 2:
			self.doc.DoUndo()
			Ox.ComputeStack( self.doc )
			self.clump = self.doc.SearchObject(self.clump.GetName())
			verticesAfterUndo = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.clump )
			TestUtilities.CheckPointsAllNearEqualByStrandId( self, self.verticesAfterClump, verticesAfterUndo )
		# Undo first clump
		self.doc.DoUndo()
		Ox.ComputeStack( self.doc )
		self.clump = self.doc.SearchObject(self.clump.GetName())
		verticesAfterUndo = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.clump )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, self.verticesBeforeClump, verticesAfterUndo )

	# Test if Clump modifier changed original guides
	def test_Clump_HairSelection( self ):
		# Strand edit mode
		self.doc.SetActiveObject( self.clump, c4d.SELECTION_NEW )
		self.clump[Ox.res.clump_ClumpEditMode] = Ox.res.clump_ClumpEditMode_ClumpHairs
		Ox.ComputeStack( self.doc )
		# Select half clumped strands
		strandIds = TestUtilities.GetStrandIds( self.clump )
		selectedStrandIds = strandIds[0:int(len(strandIds)/2)]
		TestUtilities.SelectStrandsByStrandIds( self.doc, self.clump, selectedStrandIds, Ox.res.clump_Command_SelectClumpsHairsByStrandId )
		Ox.ComputeStack( self.doc )
		# Clump selected strands
		TestUtilities.SendDescriptionCommand (self.clump, Ox.res.clump_Button_CreateClumps )
		Ox.ComputeStack( self.doc )
		# Test
		verticesAfterSecondClump = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.clump )
		nearEqualPercentage = TestUtilities.GetNearEqualPointsPercentage( self, self.verticesAfterClump, verticesAfterSecondClump )
		TestUtilities.CheckPercentageNearEqual( self, nearEqualPercentage, 0.5, 0.01, 'Equal strands' )

	def test_Clump_HairSelection_Undo( self ):
		self.test_Clump_HairSelection()
		self.test_Clump_Undo( 2 )

	# Test if Clump modifier changed original guides
	def test_ClumpClumpSelection( self ):
		# Strand edit mode
		self.doc.SetActiveObject( self.clump, c4d.SELECTION_NEW )
		self.clump[Ox.res.clump_ClumpEditMode] = Ox.res.clump_ClumpEditMode_Clumps
		Ox.ComputeStack( self.doc )
		# Select half clumped strands
		TestUtilities.SelectStrandsByRange( self.doc, self.clump, 0, 10, 20, Ox.res.clump_Command_SelectClumpsByClumpIndex )
		Ox.ComputeStack( self.doc )
		# Clump selected strands
		TestUtilities.SendDescriptionCommand (self.clump, Ox.res.clump_Button_CreateClumps )
		Ox.ComputeStack( self.doc )
		# Test
		verticesAfterSecondClump = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.clump )
		nearEqualPercentage = TestUtilities.GetNearEqualPointsPercentage( self, self.verticesAfterClump, verticesAfterSecondClump )
		TestUtilities.CheckPercentageNearEqual( self, nearEqualPercentage, 0.5, 0.1, 'Equal strands' )

	def test_ClumpClumpSelection_Undo( self ):
		self.test_ClumpClumpSelection()
		self.test_Clump_Undo( 2 )



#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_ClumpModifier )