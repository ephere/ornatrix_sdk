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
class Test_EG( TestCase ):
	hair = None
	gfm = None
	eg = None

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		# Create Sphere
		sphere = c4d.BaseObject( c4d.Osphere )
		self.doc.InsertObject( sphere )
		# Create Ornatrix Hair
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		self.doc.InsertObject( self.hair )
		self.gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		self.gfm[Ox.res.INPUT_MESH] = sphere
		self.gfm[c4d.gfm_GuideCountAttribute] = 200
		self.gfm[Ox.res.gfm_GuideLengthAttribute] = 100
		self.eg = c4d.BaseObject( Ox.res.ID_OX_EDIT_GUIDES_MOD )
		self.eg.SetName( "EG1" )
		self.doc.InsertObject( self.eg, self.hair )
		self.doc.InsertObject( self.gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )

	def test_StrandGroups( self ):
		verticesEditGuides = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.eg )
		# Add Frizz
		frizz = c4d.BaseObject( Ox.res.ID_OX_FRIZZ_MOD )
		frizz[c4d.frz_AmountAttribute] = 100
		self.doc.InsertObject( frizz, self.hair, self.eg )
		Ox.ComputeStack( self.doc )
		verticesFrizz = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( frizz )
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, verticesEditGuides, verticesFrizz )
		# Assign Frizz to Group 1 (not existent yet)
		frizz[c4d.frz_StrandGroupPatternAttribute] = "1"
		self.doc.InsertObject( frizz, self.hair, self.eg )
		Ox.ComputeStack( self.doc )
		verticesNoFrizz = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( frizz )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesEditGuides, verticesNoFrizz )
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, verticesFrizz, verticesNoFrizz )
		# Select all guides and assign to Group 1
		TestUtilities.SendDescriptionCommand (self.eg, Ox.res.edg_Button_SelectStrandsAll )
		Ox.ComputeStack( self.doc )
		self.eg[Ox.res.edg_UseStrandGroupsAttribute] = True
		TestUtilities.SendDescriptionCommand (self.eg, Ox.res.edg_Button_StrandGroups_AssignUnique )
		Ox.ComputeStack( self.doc )
		# Frizz should now be in effect again
		verticesFrizzGroup = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( frizz )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesFrizzGroup, verticesFrizz )

	def test_StrandGroupsInheritance( self ):
		verticesEditGuides = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.eg )
		# Select half guides and assign to Group 1
		TestUtilities.SelectStrandsByRange( self.doc, self.eg, 0, 100, 200 )
		Ox.ComputeStack( self.doc )
		self.eg[Ox.res.edg_UseStrandGroupsAttribute] = True
		TestUtilities.SendDescriptionCommand (self.eg, Ox.res.edg_Button_StrandGroups_AssignUnique )
		#self.eg[Ox.res.edg_CurrentStrandGroupAttribute] = "1"
		#TestUtilities.SendDescriptionCommand( self.eg, Ox.res.edg_Button_StrandGroups_Assign )
		Ox.ComputeStack( self.doc )
		# Create a second EG
		eg2 = c4d.BaseObject( Ox.res.ID_OX_EDIT_GUIDES_MOD )
		eg2.SetName( "EG2" )
		self.doc.InsertObject( eg2, self.hair, self.eg )
		Ox.ComputeStack( self.doc )
		TestUtilities.GetVerticesInObjectCoordinatesByStrandId( eg2 )
		# Select other half guides and assign to Group 2
		TestUtilities.SelectStrandsByRange( self.doc, eg2, 100, 100, 200 )
		Ox.ComputeStack( self.doc )
		eg2[Ox.res.edg_UseStrandGroupsAttribute] = True
		# NOTE: Using edg_Button_StrandGroups_Assign instead of edg_Button_StrandGroups_AssignUnique becaus the latter generates unique group of 1
		# while testing and correct value when doing the test manually. For some reason input hair into EG2 doesn't contain groups when running the test.
		#TestUtilities.SendDescriptionCommand (eg2, Ox.res.edg_Button_StrandGroups_AssignUnique )
		eg2[Ox.res.edg_CurrentStrandGroupAttribute] = "2"
		TestUtilities.SendDescriptionCommand( eg2, Ox.res.edg_Button_StrandGroups_Assign )
		Ox.ComputeStack( self.doc )
		# Add Frizz
		frizz = c4d.BaseObject( Ox.res.ID_OX_FRIZZ_MOD )
		frizz[c4d.frz_AmountAttribute] = 100
		self.doc.InsertObject( frizz, self.hair, eg2 )
		Ox.ComputeStack( self.doc )
		verticesFrizz = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( frizz )
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, verticesFrizz, verticesEditGuides )
		# Assign Frizz to Group 1
		frizz[c4d.frz_StrandGroupPatternAttribute] = "1"
		Ox.ComputeStack( self.doc )
		verticesGroup1 = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( frizz )
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, verticesGroup1, verticesEditGuides )
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, verticesGroup1, verticesFrizz )
		# Assign Frizz to Group 2
		frizz[c4d.frz_StrandGroupPatternAttribute] = "2"
		Ox.ComputeStack( self.doc )
		verticesGroup2 = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( frizz )
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, verticesGroup2, verticesEditGuides )
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, verticesGroup2, verticesFrizz )
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, verticesGroup2, verticesGroup1 )
		# Assign Frizz to Group 3 (no effect)
		frizz[c4d.frz_StrandGroupPatternAttribute] = "3"
		Ox.ComputeStack( self.doc )
		verticesGroup3 = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( frizz )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesGroup3, verticesEditGuides )



#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_EG )