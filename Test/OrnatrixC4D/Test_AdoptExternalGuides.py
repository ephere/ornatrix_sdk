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
class Test_AdoptExternalGuides( TestCase ):
	doc = None
	hair = None
	reference = None
	control = None
	adopt = None

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		# Create Sphere
		plane = c4d.BaseObject( c4d.Oplane )
		self.doc.InsertObject( plane )
		# Create Hair objects
		self.hair = TestUtilities.AddGuidesToMesh( plane, 300, Ox.res.gfm_RootGenerationMethodAttribute_VERTEX, 10, name="Hair" )
		self.reference = TestUtilities.AddGuidesToMesh( plane, 300, Ox.res.gfm_RootGenerationMethodAttribute_FACE_CENTER, 10, name="Reference" )
		self.control = TestUtilities.AddGuidesToMesh( plane, 300, Ox.res.gfm_RootGenerationMethodAttribute_FACE_CENTER, 10, name="Control" )
		# Add Frixx to modify Control
		frizz = c4d.BaseObject( Ox.res.ID_OX_FRIZZ_MOD )
		frizz[c4d.frz_AmountAttribute] = 50
		self.doc.InsertObject( frizz, self.control, self.control.GetDownLast() )
		# Create AdoptExternalGuides
		self.adopt = c4d.BaseObject( Ox.res.ID_OX_ADOPT_EXTERNAL_GUIDES_MOD )
		self.doc.InsertObject( self.adopt, self.control, self.hair.GetDownLast() )
		# Build
		Ox.ComputeStack( self.doc )

	def test_AdoptExternalGuides( self ):
		verticesBefore = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( self.hair )
		# Adopt
		self.adopt[Ox.res.aeg_ReferenceObjectAttribute] = self.reference
		self.adopt[Ox.res.aeg_ControlObjectAttribute] = self.control
		Ox.ComputeStack( self.doc )
		# Test
		verticesAfter = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( self.hair )
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, verticesBefore, verticesAfter )



#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_AdoptExternalGuides )
