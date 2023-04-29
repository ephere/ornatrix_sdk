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
class Test_GFM_RememberRoots( TestCase ):
	doc = None
	hair = None
	gfm = None

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		Ox.DeleteAll( self.doc )
		# Create Sphere
		sphere = c4d.BaseObject( c4d.Osphere )
		self.doc.InsertObject( sphere )
		# Create Ornatrix Hair object
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		self.doc.InsertObject( self.hair )
		# Create GFM modifier inside Hair Object
		self.gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		self.gfm.SetName( "GFM" )
		self.gfm[Ox.res.INPUT_MESH] = sphere
		self.gfm[Ox.res.gfm_RootGenerationMethodAttribute] = Ox.res.gfm_RootGenerationMethodAttribute_EVEN
		self.doc.InsertObject( self.gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )

	def test_RememberRoots( self ):
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.gfm )
		# Remember roots and change distribution
		self.gfm[Ox.res.gfm_RememberRootPositionsAttribute] = True
		self.gfm[Ox.res.gfm_RootGenerationMethodAttribute] = Ox.res.gfm_RootGenerationMethodAttribute_UNIFORM
		Ox.ComputeStack( self.doc )
		# Compare strands
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.gfm )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesBefore, verticesAfter )

	def test_RememberRoots_SaveLoad( self ):
		verticesBeforeSave = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.gfm )
		# Remember roots and save
		self.gfm[Ox.res.gfm_RememberRootPositionsAttribute] = True
		self.gfm[Ox.res.gfm_RootGenerationMethodAttribute] = Ox.res.gfm_RootGenerationMethodAttribute_UNIFORM
		Ox.ComputeStack( self.doc )
		tempUrl = TestUtilities.SaveTempDocument( self.doc )
		# Load temp doc
		loadedDoc = c4d.documents.LoadDocument( tempUrl, c4d.SCENEFILTER_OBJECTS, None )
		Ox.ComputeStack( loadedDoc )
		loadedGFM = loadedDoc.SearchObject( "GFM" )
		# Compare strands
		verticesAfterLoad = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( loadedGFM )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesBeforeSave, verticesAfterLoad )
		# Cleanup
		c4d.documents.KillDocument( loadedDoc )
		tempUrl.IoDelete( True, True )




#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_GFM_RememberRoots )
