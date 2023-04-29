import UETest
import unreal
import TestUtilities

class Test_GroundStrands( UETest.OxTestCase ):
	def test_VertexPositionConsistency( self ):
		planeActor = TestUtilities.CreateBasicActor( 'Plane' )
		planeActor.set_actor_scale3d( unreal.Vector( 2.0, 2.0, 2.0 ) )

		targetHair = self.oxSubsystem.add_hair_to_actor( planeActor )
		guidesFromMesh = targetHair.add_operator( "GuidesFromMeshOperator" )
		guidesFromMesh.generation_method = unreal.GuidesFromMesh_GenerationMethodType.GFM_GENERATION_METHOD_TYPE_FACE_CENTER
		guidesFromMesh.guide_length = 10
		guidesFromMesh.length_randomness = 0.0
		guidesFromMesh.point_count = 2

		# No grounded data initially
		groundStrands = targetHair.add_operator( "GroundOperator" )
		self.assertEqual( 0, len( groundStrands.grounded_strands_surface_positions ) )

		verticesWithGrounding = targetHair.get_vertices( True )

		# The grounding should've been done, which means that the surface coordinates array is no longer empty
		self.assertNotEqual( 0, len( groundStrands.grounded_strands_surface_positions ) )

		groundStrands.enabled = False
		verticesWithoutGrounding = targetHair.get_vertices( True )

		TestUtilities.CheckPointsAllNearEqual( self, verticesWithoutGrounding, verticesWithGrounding )

		# Test subsequent evaluations
		groundStrands.enabled = True
		verticesAfterRegrounding = targetHair.get_vertices( True )
		TestUtilities.CheckPointsAllNearEqual( self, verticesWithoutGrounding, verticesAfterRegrounding )