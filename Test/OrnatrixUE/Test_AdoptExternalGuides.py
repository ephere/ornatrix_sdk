import UETest
import unreal
import TestUtilities

class Test_AdoptExternalGuides( UETest.OxTestCase ):
	def test_Evaluate( self ):
		planeActor = TestUtilities.CreateBasicActor( 'Plane' )

		targetHair = self.oxSubsystem.add_hair_to_actor( planeActor )
		guidesFromMesh = targetHair.add_operator( "GuidesFromMeshOperator" )
		guidesFromMesh.generation_method = unreal.GuidesFromMesh_GenerationMethodType.GFM_GENERATION_METHOD_TYPE_VERTEX
		guidesFromMesh.point_count = 2
		adoptExternalGuides = targetHair.add_operator( "AdoptExternalGuidesOperator" )

		controlHair = self.oxSubsystem.add_hair_to_actor( planeActor )
		guidesFromMesh2 = controlHair.add_operator( "GuidesFromMeshOperator" )
		guidesFromMesh2.generation_method = unreal.GuidesFromMesh_GenerationMethodType.GFM_GENERATION_METHOD_TYPE_VERTEX
		guidesFromMesh2.point_count = 2

		adoptExternalGuides.objects_control = planeActor

		verticesBeforeControlChange = targetHair.get_vertices( True )

		# Change control hair a bit by adding frizz
		controlHair.add_operator( "FrizzOperator" )

		verticesAfterControlChange = targetHair.get_vertices( True )

		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBeforeControlChange, verticesAfterControlChange )