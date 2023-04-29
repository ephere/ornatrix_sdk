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
class Test_GFC( TestCase ):
	doc = None
	arc1 = None
	arc2 = None
	hair = None
	gfc = None

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		# Create Sphere
		self.arc1 = c4d.BaseObject( c4d.Osplinearc )
		self.arc2 = c4d.BaseObject( c4d.Osplinearc )
		self.arc2.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( 0, 0, 50 ) ) )
		self.doc.InsertObject( self.arc1 )
		self.doc.InsertObject( self.arc2 )
		# Create Ornatrix Hair object
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		self.doc.InsertObject( self.hair )
		curves = c4d.InExcludeData()
		curves.InsertObject( self.arc1, 1 )
		curves.InsertObject( self.arc2, 1 )
		self.gfc = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_CURVES_MOD )
		self.gfc[Ox.res.gfc_InputCurves] = curves
		self.doc.InsertObject( self.gfc, self.hair )
		# Build
		Ox.ComputeStack( self.doc )

	def test_GFC_StrandCount( self ):
		guideCount = Ox.GetUserData( self.gfc, Ox.res.OUTPUT_GUIDES, Ox.res.ID_OX_GUIDES_DATA, Ox.res.gd_StrandCount, c4d.DTYPE_LONG )
		self.assertEqual( guideCount, 2 )
		# Test output strand count from Hair Object
		outputHairCount = Ox.GetStrandCount( self.hair )
		self.assertEqual( outputHairCount, guideCount )

	def test_GFC_VertexCount( self ):
		vertexCount = Ox.GetUserData( self.gfc, Ox.res.OUTPUT_GUIDES, Ox.res.ID_OX_GUIDES_DATA, Ox.res.gd_VertexCount, c4d.DTYPE_LONG )
		#print( "VC = " + str( vertexCount ) )
		self.assertEqual( vertexCount, 20 )

	def test_GFC_StrandIds( self ):
		strandIds = TestUtilities.GetStrandIds( self.gfc )
		self.assertEqual( len( strandIds ), Ox.GetStrandCount( self.gfc ) )

	def test_GroundCurves( self ):
		# Move curves up
		self.arc1.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( 0, 50, 0 ) ) )
		self.arc2.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( 0, 50, 50 ) ) )
		Ox.ComputeStack( self.doc )
		verticesBefore = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( self.gfc )
		# Create Plane and ground to it
		plane = c4d.BaseObject( c4d.Oplane )
		self.doc.InsertObject( plane )
		gs = c4d.BaseObject( Ox.res.ID_OX_GROUND_STRANDS_MOD )
		gs[c4d.gnd_DistributionMeshAttribute] = plane
		gs[Ox.res.gnd_DetachAttribute] = False
		self.doc.InsertObject( gs, self.hair, self.gfc )
		Ox.ComputeStack( self.doc )
		verticesAfterGrounding = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( gs )
		for strandId, points in iter(verticesBefore.items()):
			self.assertAlmostEqual( points[0][1], 50, places=3, msg='Original roots hould have Y=50, but Y=' + str( points[0][1] )  )
		for strandId, points in iter(verticesAfterGrounding.items()):
			self.assertAlmostEqual( points[0][1], 0, places=3, msg='Grounded roots hould have Y=0, but Y=' + str( points[0][1] )  )
		# Move plane and check if strands moved
		plane.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( 0, 0, 50 ) ) )
		Ox.ComputeStack( self.doc )
		# Need to move plane again to force Hair matrix to be updated
		plane.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( 0, 0, 100 ) ) )
		self.hair.Message(c4d.MSG_UPDATE)
		Ox.ComputeStack( self.doc )
		# Compare
		verticesAfterMove = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( gs )
		for strandId, points in iter(verticesAfterMove.items()):
			strandBeforeMove = verticesAfterGrounding[strandId]
			rootDifference = points[0][2] - strandBeforeMove[0][2]
			self.assertAlmostEqual( rootDifference, 100, places=3, msg='Root should have moved 100, moved ' + str( rootDifference )  )




#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_GFC )
