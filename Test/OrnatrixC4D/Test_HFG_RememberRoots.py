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
class Test_HFG_RememberRoots( TestCase ):
	doc = None
	hair = None
	hfg = None

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
		gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		gfm[Ox.res.INPUT_MESH] = sphere
		self.hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		self.hfg.SetName( "HFG" )
		self.hfg[Ox.res.hfg_RootGenerationMethodAttribute] = Ox.res.hfg_RootGenerationMethodAttribute_EVEN
		self.hfg[Ox.res.hfg_ViewportCountFractionAttribute] = 1.0
		self.doc.InsertObject( self.hfg, self.hair )
		self.doc.InsertObject( gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )

	def test_RememberRoots( self ):
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.hfg )
		# Remember roots and change distribution
		self.hfg[Ox.res.hfg_RememberRootPositionsAttribute] = True
		Ox.ComputeStack( self.doc )
		self.hfg[Ox.res.hfg_RootGenerationMethodAttribute] = Ox.res.hfg_RootGenerationMethodAttribute_UNIFORM
		Ox.ComputeStack( self.doc )
		# Compare strands
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.hfg )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesBefore, verticesAfter )

	def test_RememberRoots_SaveLoad( self ):
		verticesBeforeSave = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.hfg )
		# Remember roots and save
		self.hfg[Ox.res.hfg_RememberRootPositionsAttribute] = True
		Ox.ComputeStack( self.doc )
		self.hfg[Ox.res.hfg_RootGenerationMethodAttribute] = Ox.res.hfg_RootGenerationMethodAttribute_UNIFORM
		Ox.ComputeStack( self.doc )
		tempUrl = TestUtilities.SaveTempDocument( self.doc )
		# Load temp doc
		loadedDoc = c4d.documents.LoadDocument( tempUrl, c4d.SCENEFILTER_OBJECTS, None )
		Ox.ComputeStack( loadedDoc )
		loadedHFG = loadedDoc.SearchObject( "HFG" )
		# Compare strands
		verticesAfterLoad = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( loadedHFG )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesBeforeSave, verticesAfterLoad )
		# Cleanup
		c4d.documents.KillDocument( loadedDoc )
		tempUrl.IoDelete( True, True )




#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_HFG_RememberRoots )
