#----------------
import os
import sys
import c4d
sys.path.insert( 0, os.path.dirname( __file__ ) )
import Ornatrix.Ornatrix as Ox
import TestUtilities
sys.path.pop( 0 )
#----------------
from unittest import TestCase

#
# TestCase
class Test_EG_Transform( TestCase ):
	baseLength = 100.0
	hair = None
	gfm = None
	eg = None
	guidesLength = None
	hairLength = None
	strandIdsAffectedByLength = []
	perStrandChannel = -1
	perVertexChannel = -1

	# Simulate move strands
	def transformAllStrands( self, editMode, translation=c4d.Vector(0,0,0), rotation=c4d.Vector(0,0,0), scale=c4d.Vector(1,1,1) ):
		# 1. select all strands and Edit Mode
		c4d.CallCommand( 1017448 ) # move tool (neutral mode)
		self.doc.SetActiveObject( self.eg )
		TestUtilities.SendDescriptionCommand (self.eg, editMode )
		TestUtilities.SendDescriptionCommand (self.eg, Ox.res.edg_Button_SelectStrandsAll )
		Ox.ComputeStack( self.doc )
		# 2. simulate user transfrom start, will allow EG to be transformed
		self.eg.Message(c4d.MSG_MOVE_START)
		Ox.ComputeStack( self.doc )
		# 3. transform EG
		mg = TestUtilities.MakeTransformMatrix( translation, rotation, scale )
		self.eg.SetMg( mg )
		Ox.ComputeStack( self.doc )
		# 4. simulate user transform end, will commit and lock EG transform
		self.eg.Message(c4d.MSG_MOVE_FINISHED)
		Ox.ComputeStack( self.doc )

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
		self.gfm[Ox.res.gfm_GuideLengthAttribute] = 100
		self.eg = c4d.BaseObject( Ox.res.ID_OX_EDIT_GUIDES_MOD )
		self.eg.SetName( "EG" )
		self.doc.InsertObject( self.eg, self.hair )
		self.doc.InsertObject( self.gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )

	def test_EG_TransformRoots( self ):
		translation = c4d.Vector(0,0,150)
		self.transformAllStrands( Ox.res.edg_Button_EditMode_Root, translation )
		points = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.eg )
		pointsCount = len(points)
		for strandId, points in iter(points.items()):
			if points[0].z < 0:
				self.assertFail( 'All roots should be Z positive' )
				break

	def test_EG_TransformStrands( self ):
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.eg )
		rotation = c4d.Vector(0.5,0.5,0.5)
		self.transformAllStrands( Ox.res.edg_Button_EditMode_Strand, rotation=rotation )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.eg )
		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, verticesBefore, verticesAfter )

	def test_EG_SaveLoad( self ):
		translation = c4d.Vector(0,0,100)
		self.transformAllStrands( Ox.res.edg_Button_EditMode_Strand, translation )
		verticesBeforeSave = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.eg )
		# Save temp doc
		tempUrl = TestUtilities.SaveTempDocument( self.doc )
		# Load temp doc and vertices
		loadedDoc = c4d.documents.LoadDocument( tempUrl, c4d.SCENEFILTER_OBJECTS, None )
		Ox.ComputeStack( loadedDoc )
		loadedEg = loadedDoc.SearchObject( "EG" )
		verticesAfterLoad = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( loadedEg )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesBeforeSave, verticesAfterLoad )
		# Cleanup
		c4d.documents.KillDocument( loadedDoc )
		tempUrl.IoDelete( True, True )



#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_EG_Transform )
