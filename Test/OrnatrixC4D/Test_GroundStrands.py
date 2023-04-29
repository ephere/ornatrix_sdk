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
class Test_GroundStrands( TestCase ):
	sphere = None
	plane = None
	hair = None
	gfm = None
	hfg = None
	gs = None

	#@classmethod
	#def setUpClass( self ):
	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		self.doc.SetTime( c4d.BaseTime( 0, self.doc.GetFps() ) )
		# Create Sphere (original)
		self.sphere = c4d.BaseObject( c4d.Osphere )
		self.sphere[c4d.PRIM_SPHERE_RAD] = 48
		self.sphere[c4d.ID_BASEOBJECT_XRAY] = True
		self.doc.InsertObject( self.sphere )
		# Create Plane (ground to)
		self.plane = c4d.BaseObject( c4d.Oplane )
		self.plane[c4d.PRIM_PLANE_WIDTH] = 600
		self.plane[c4d.PRIM_PLANE_HEIGHT] = 600
		self.doc.InsertObject( self.plane )
		# Create Ornatrix Hair stack
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		self.gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		self.gfm[Ox.res.INPUT_MESH] = self.sphere
		self.hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		self.hfg[Ox.res.hfg_GuideAreaTypeAttribute] = Ox.res.hfg_GuideAreaTypeAttribute_Circle
		self.gs = c4d.BaseObject( Ox.res.ID_OX_GROUND_STRANDS_MOD )
		self.gs[c4d.gnd_DistributionMeshAttribute] = self.plane
		self.gs[c4d.gnd_GroundClosestEnd] = False
		# Insert
		self.doc.InsertObject( self.hair )
		self.doc.InsertObject( self.hfg, self.hair )
		self.doc.InsertObject( self.gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )

	def test_GroundedGuideRoots_BakedGuides( self ):
		TestUtilities.SendDescriptionCommand( self.gfm, Ox.res.bake_Colapse_Button )
		self.test_GroundedGuideRoots()

	def test_GroundedGuideRoots( self ):
		self.doc.InsertObject( self.gs, self.hair, self.gfm )
		self.gs[Ox.res.gnd_DetachAttribute] = False
		# Build
		Ox.ComputeStack( self.doc )
		# Guide Roots must be on the plane
		guidesVertices = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.gs )
		for strandId, points in iter(guidesVertices.items()):
			self.assertAlmostEqual( points[0][1], 0, places=3, msg='All guide roots should have Y=0, is ' + str( points[0] )  )
			self.assertLess( points[0][0], 50, msg='All guide roots should have -50<X<50, is ' + str( points[0] )  )
			self.assertLess( points[0][2], 50, msg='All guide roots should have 50<Z<50, is ' + str( points[0] )  )
			self.assertGreater( points[0][0], -50, msg='All guide roots should have -50<X<50, is ' + str( points[0] )  )
			self.assertGreater( points[0][2], -50, msg='All guide roots should have -50<Z<50, is ' + str( points[0] )  )
		# Hair Roots must be on the plane
		hairVertices = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.hfg )
		for strandId, points in iter(hairVertices.items()):
			self.assertAlmostEqual( points[0][1], 0, places=3, msg='All hair roots should have Y=0, is ' + str( points[0] )  )
			self.assertLess( points[0][0], 50, msg='All hair roots should have -50<X<50, is ' + str( points[0] )  )
			self.assertLess( points[0][2], 50, msg='All hair roots should have -50<Z<50, is ' + str( points[0] )  )
			self.assertGreater( points[0][0], -50, msg='All hair roots should have -50<X<50, is ' + str( points[0] )  )
			self.assertGreater( points[0][2], -50, msg='All hair roots should have -50<Z<50, is ' + str( points[0] )  )

	def test_GroundedHairRoots_BakedGuides( self ):
		TestUtilities.SendDescriptionCommand( self.gfm, Ox.res.bake_Colapse_Button )
		self.test_GroundedHairRoots()

	def test_GroundedHairRoots( self ):
		self.doc.InsertObject( self.gs, self.hair, self.hfg )
		self.gs[Ox.res.gnd_DetachAttribute] = False
		# Build
		Ox.ComputeStack( self.doc )
		# Roots must be on the plane
		vertices = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.gs )
		for strandId, points in iter(vertices.items()):
			self.assertAlmostEqual( points[0][1], 0, places=3, msg='All hair roots should have Y=0, is ' + str( points[0] )  )
			self.assertLess( points[0][0], 50, msg='All hair roots should have -50<X<50, is ' + str( points[0] )  )
			self.assertLess( points[0][2], 50, msg='All hair roots should have 50<Z<50, is ' + str( points[0] )  )
			self.assertGreater( points[0][0], -50, msg='All hair roots should have -50<X<50, is ' + str( points[0] )  )
			self.assertGreater( points[0][2], -50, msg='All hair roots should have -50<Z<50, is ' + str( points[0] )  )

	def test_GroundedHairRotations( self ):
		# Rotate randomly
		rotate = c4d.BaseObject( Ox.res.ID_OX_ROTATE_MOD )
		rotate[c4d.rtt_ChaosAttribute] = 2.0
		self.doc.InsertObject( rotate, self.hair, self.hfg )
		Ox.ComputeStack( self.doc )
		rotationsBefore = TestUtilities.GetRotationsByStrandId( rotate )
		# Roots must be on the plane
		self.doc.InsertObject( self.gs, self.hair, rotate )
		self.gs[Ox.res.gnd_DetachAttribute] = False
		Ox.ComputeStack( self.doc )
		rotationsGrounded = TestUtilities.GetRotationsByStrandId( rotate )
		TestUtilities.CheckValuesAllNearEqualByStrandId( self, rotationsBefore, rotationsGrounded )

	def DoOffsetTest( self ):
		# Ground
		self.gs[Ox.res.gnd_DetachAttribute] = False
		Ox.ComputeStack( self.doc )
		# Move plane, hair must follow
		self.plane.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( 100, 0, 100 ) ) )
		Ox.ComputeStack( self.doc )
		# Need to move plane again to force Hair matrix to be updated
		self.plane.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( 100, 10, 100 ) ) )
		self.hair.Message(c4d.MSG_UPDATE)
		Ox.ComputeStack( self.doc )
		# Roots must be offset
		vertices = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( self.hair )
		for strandId, points in iter(vertices.items()):
			self.assertGreaterEqual( points[0][0], 0, msg='All roots should have X>0, is ' + str( points[0] )  )
			self.assertGreaterEqual( points[0][2], 0, msg='All roots should have Z>0, is ' + str( points[0] )  )

	def test_GroundedGuidesRootsOffset( self ):
		self.doc.InsertObject( self.gs, self.hair, self.gfm )
		self.DoOffsetTest()

	def test_GroundedHairRootsOffset( self ):
		self.doc.InsertObject( self.gs, self.hair, self.hfg )
		self.DoOffsetTest()

	def test_TransformedDistributionMesh_BakedGuides( self ):
		TestUtilities.SendDescriptionCommand( self.gfm, Ox.res.bake_Colapse_Button )
		self.test_TransformedDistributionMesh()

	def test_TransformedDistributionMesh( self ):
		self.doc.InsertObject( self.gs, self.hair, self.hfg )
		self.gs[Ox.res.gnd_DetachAttribute] = False
		self.sphere.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( 100, 0, 100 ) ) )
		# Build
		Ox.ComputeStack( self.doc )
		# Roots must be on the plane
		vertices = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.hair )
		for strandId, points in iter(vertices.items()):
			self.assertAlmostEqual( points[0][1], 0, places=3, msg='All roots should have Y=0, is ' + str( points[0] )  )
			self.assertGreaterEqual( points[0][0], 0, msg='All roots should have X>0, is ' + str( points[0] )  )
			self.assertGreaterEqual( points[0][2], 0, msg='All roots should have Z>0, is ' + str( points[0] )  )

	def test_TransformedGroundMesh_BakedGuides( self ):
		TestUtilities.SendDescriptionCommand( self.gfm, Ox.res.bake_Colapse_Button )
		self.test_TransformedGroundMesh()

	def test_TransformedGroundMesh( self ):
		self.doc.InsertObject( self.gs, self.hair, self.hfg )
		self.gs[Ox.res.gnd_DetachAttribute] = False
		self.plane.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( 100, 0, 100 ) ) )
		# Build
		Ox.ComputeStack( self.doc )
		# Roots must be on the plane
		vertices = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( self.hair )
		for strandId, points in iter(vertices.items()):
			self.assertAlmostEqual( points[0][1], 0, places=3, msg='All roots should have Y=0, is ' + str( points[0] )  )
			self.assertLess( points[0][0], 50, msg='All roots should have -50<=X<=50, is ' + str( points[0] )  )
			self.assertLess( points[0][2], 50, msg='All roots should have 50<=Z<=50, is ' + str( points[0] )  )
			self.assertGreater( points[0][0], -50, msg='All roots should have -50<=X<=50, is ' + str( points[0] )  )
			self.assertGreater( points[0][2], -50, msg='All roots should have -50<=Z<=50, is ' + str( points[0] )  )

	def test_TransformedDistributionAndGroundMesh_BakedGuides( self ):
		TestUtilities.SendDescriptionCommand( self.gfm, Ox.res.bake_Colapse_Button )
		self.test_TransformedDistributionAndGroundMesh()

	def test_TransformedDistributionAndGroundMesh( self ):
		self.doc.InsertObject( self.gs, self.hair, self.hfg )
		self.gs[Ox.res.gnd_DetachAttribute] = False
		self.sphere.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( 100, 0, 100 ) ) )
		self.plane.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( -100, 0, -100 ) ) )
		# Build
		Ox.ComputeStack( self.doc )
		# Roots must be on the plane
		vertices = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( self.hair )
		for strandId, points in iter(vertices.items()):
			self.assertAlmostEqual( points[0][1], 0, places=3, msg='All roots should have Y=0, is ' + str( points[0] )  )
			self.assertGreaterEqual( points[0][0], 0, msg='All roots should have X>0, is ' + str( points[0] )  )
			self.assertGreaterEqual( points[0][2], 0, msg='All roots should have Z>0, is ' + str( points[0] )  )

	def test_TransformedToPreviousPositionAfterUngrounding( self ):
		self.doc.InsertObject( self.gs, self.hair, self.hfg )
		self.gs[Ox.res.gnd_DetachAttribute] = False
		initMg = self.hair.GetMg()
		self.plane.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( 100, 0, 100 ) ) )
		# Build
		Ox.ComputeStack( self.doc )
		self.assertFalse( initMg == self.hair.GetMg(), "Hair has not moved after grounding.")
		self.gs[Ox.res.gnd_DetachAttribute] = True 
		Ox.ComputeStack( self.doc )
		self.assertTrue( initMg == self.hair.GetMg(), "Hair did not return to original position after ungrounding.")

	def test_GroundsAtFrame0( self ):
		self.doc.InsertObject( self.gs, self.hair, self.hfg )
		# Add a CTrack for the global position
		descId = c4d.DescID( c4d.DescLevel( c4d.ID_BASEOBJECT_GLOBAL_POSITION, c4d.DTYPE_VECTOR, 0 ),
                             c4d.DescLevel( c4d.VECTOR_X, c4d.DTYPE_REAL, 0 ) )
		cTrack = c4d.CTrack( self.plane, descId )
		# Get the CCurve
		cCurve = cTrack.GetCurve()
		# Set the keyframes at frame 0 and 40
		frame0 = c4d.BaseTime( 0, self.doc.GetFps() )
		frame40 = c4d.BaseTime( 40, self.doc.GetFps() )
		TestUtilities.CreateFloatKey( self.doc, cCurve, frame0, 0.0 )
		TestUtilities.CreateFloatKey( self.doc, cCurve, frame40, 100.0 )
		self.plane.InsertTrackSorted( cTrack )
		self.gs[Ox.res.gnd_DetachAttribute] = False
		self.doc.SetTime( frame40 )

		# Build
		Ox.ComputeStack( self.doc )
		# Roots must be on the plane
		vertices = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( self.hair )
		for _, points in iter(vertices.items()):
			self.assertAlmostEqual( points[0][1], 0, places=3, msg='All hair roots should have Y=0, is ' + str( points[0] )  )
			self.assertLess( points[0][0], 150, msg='All hair roots should have 50<X<150, is ' + str( points[0] )  )
			self.assertLess( points[0][2], 50, msg='All hair roots should have -50<Z<50, is ' + str( points[0] )  )
			self.assertGreater( points[0][0], 0, msg='All hair roots should have 50<X<150, is ' + str( points[0] )  )
			self.assertGreater( points[0][2], -50, msg='All hair roots should have -50<Z<50, is ' + str( points[0] )  )

	def test_GroundToObjectWithChildren( self ):
		sphere = c4d.BaseObject( c4d.Osphere )
		sphere[c4d.PRIM_SPHERE_RAD] = 48
		sphere[c4d.ID_BASEOBJECT_XRAY] = True

		plane = c4d.BaseObject( c4d.Oplane )
		plane[c4d.PRIM_PLANE_WIDTH] = 600
		plane[c4d.PRIM_PLANE_HEIGHT] = 600
		plane.InsertUnder( sphere )
		plane.SetRelPos( c4d.Vector( 200, 0, 0 ) )
		self.doc.InsertObject( sphere )

		self.gs[Ox.res.gnd_DetachAttribute] = False
		self.gs[c4d.gnd_DistributionMeshAttribute] = sphere
		self.gs[c4d.gnd_GroundClosestEnd] = False
		Ox.ComputeStack( self.doc )

		self.doc.InsertObject( self.gs, self.hair, self.gfm )
		vertices = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( self.hair )
		self.gs[Ox.res.gnd_DetachAttribute] = False
		Ox.ComputeStack( self.doc )

		newVertices = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( self.hair )
		self.assertTrue( len( vertices ) != len( newVertices ) )


#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_GroundStrands )