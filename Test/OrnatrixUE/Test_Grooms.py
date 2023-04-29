from unittest import skip
import UETest
import unreal
import TestUtilities
import os

class Test_Grooms( UETest.OxTestCase ):
	def test_BakeHairWithEditGuides( self ):
		planeActor = TestUtilities.CreateBasicActor( 'Plane' )

		targetHair = self.oxSubsystem.add_hair_to_actor( planeActor )
		guidesFromMesh = targetHair.add_operator( "GuidesFromMeshOperator" )
		guidesFromMesh.generation_method = unreal.GuidesFromMesh_GenerationMethodType.GFM_GENERATION_METHOD_TYPE_FACE_CENTER
		guidesFromMesh.point_count = 2
		targetHair.add_operator( "EditGuidesOperator" )

		verticesBeforeBaking = targetHair.get_vertices( True )
		strandCountBeforeBaking = targetHair.get_strand_count()
		targetHair.bake_operators( 0, 2 )
		verticesAfterBaking = targetHair.get_vertices( True )
		strandCountAfterBaking = targetHair.get_strand_count()

		self.assertEqual( strandCountBeforeBaking, strandCountAfterBaking )
		#TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeBaking, verticesAfterBaking )

	def ImportTestGroom( fileName ):
		data = unreal.AutomatedAssetImportData()
		data.set_editor_property('destination_path', '/Game/TestGrooms')
		data.set_editor_property('filenames', [os.path.dirname(__file__) + '/' + fileName])
		data.set_editor_property('level_to_load', '/Game/TestLevel')
		data.set_editor_property('replace_existing', True)
		factory = unreal.OrnatrixGroomFactory()
		data.set_editor_property('factory', factory)
		
		return unreal.AssetToolsHelpers.get_asset_tools().import_assets_automated( data )

	def test_BakedHairGroomImport( self ):
		importedAssets = Test_Grooms.ImportTestGroom( 'SimpleBakedHair.oxg.usda' )
		self.assertEqual( 1, len( importedAssets ) )

	@skip( "Enable when external mesh parameters are implemented" )
	def test_PAFSCollisionWithExternalMesh( self ):
		# PAFSCollisionTest contains PAFS with "collide with base mesh" turned off and an exterior mesh specified. This means that if exterior mesh is not set properly the operator (and test) will fail.
		importedAssets = Test_Grooms.ImportTestGroom( 'PAFSCollisionTest.oxg.usda' )
		self.assertEqual( 1, len( importedAssets ) )

		#factory = unreal.OrnatrixGroomFactory()
		#targetHair = unreal.AssetToolsHelpers.get_asset_tools().create_asset( 'Test', '/Game/TestGrooms', None, factory )
		