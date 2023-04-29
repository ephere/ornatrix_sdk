import UETest
import unreal
import TestUtilities

#class Undo(object):
#    def __init__(self, context, desc, primary_object=None):
#        self.context = context
#        self.desc = desc
#        self.primary_object = primary_object

#    def __enter__(self):
#        self.transaction = unreal.SystemLibrary.begin_transaction(
#            self.context,
#            self.desc,
#            self.primary_object
#        )

#    def __exit__(self, type, value, traceback):
#        self.exit = unreal.SystemLibrary.end_transaction()

#    def __cancel__(self):
#        unreal.SystemLibrary.cancel_transaction(self.transaction)

class Test_FurballEvaluation( UETest.OxTestCase ):
	
	def CreateFurball( self ):
		# We are adding hair to the newly created actor using our subsystem 
		# Note that the c++ name of this ("BlueprintCallable") method is actually "AddHairToActor"
		return self.oxSubsystem.add_hair_to_actor( TestUtilities.CreateBasicActor( 'Sphere' ), "Fur Ball.oxg.usda" )

	def test_StrandCount( self ):
		testHair = self.CreateFurball()
		
		# AddHair should have generated 999 strands
		self.assertEqual( testHair.get_strand_count(), 999 )
		
	def test_FurBallGroomImportedNameConsistency( self ):
		testHair = self.CreateFurball()
		self.assertEqual( 4, len( testHair.ordered_operators ) )
		self.assertEqual( 'GuidesFromMesh1', testHair.ordered_operators[0].get_node_name() )

		# Adding the same groom again should produce operator components with same names
		testHair2 = self.CreateFurball()
		self.assertEqual( 4, len( testHair2.ordered_operators ) )
		self.assertEqual( 'GuidesFromMesh1', testHair2.ordered_operators[0].get_node_name() )
			
	def CreateStackWithSingleFrizz( self ):
		testHair = self.oxSubsystem.add_hair_to_actor( TestUtilities.CreateBasicActor( 'Plane' ) )
		guidesFromMesh = testHair.add_operator( "GuidesFromMeshOperator" )
		self.assertIsNotNone( guidesFromMesh )
		#print( str( guidesFromMesh.generation_method ) )
		guidesFromMesh.generation_method = unreal.GuidesFromMesh_GenerationMethodType.GFM_GENERATION_METHOD_TYPE_FACE_CENTER
		guidesFromMesh.point_count = 2

		self.assertIsNotNone( testHair.add_operator( "FrizzOperator" ) )

		# Bake the Frizz operator
		self.assertEqual( 'Frizz7', testHair.ordered_operators[1].get_node_name() )

		return testHair

	def test_BakeOperator( self ):
		testHair = self.CreateStackWithSingleFrizz()

		verticesBeforeBaking = testHair.get_vertices( True )
		testHair.bake_operators( 1, 1 )

		# Still 2 operators after baking
		self.assertEqual( 2, len( testHair.ordered_operators ) )

		# But new operator is no longer Frizz
		self.assertEqual( 'BakedNode', testHair.ordered_operators[1].get_node_name() )

		verticesAfterBaking = testHair.get_vertices( True )
		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeBaking, verticesAfterBaking )

	def test_AddSecondEditGuides( self ):
		testHair = self.CreateFurball()
		testHair.add_operator( "ClumpOperator" )
		testHair.add_operator( "EditGuidesOperator" )
		
		# All operator names should be unique
		operatorNames = set()
		for op in testHair.ordered_operators:
			opName = op.get_node_name()
			self.assertFalse( opName in operatorNames, opName + ' already exists' )

			operatorNames.add( opName )

	def test_PropertyChangeCausesSingleEvaluation( self ):
		planeActor = TestUtilities.CreateBasicActor( 'Plane' )
		targetHair = self.oxSubsystem.add_hair_to_actor( planeActor )
		guidesFromMesh = targetHair.add_operator( "GuidesFromMeshOperator" )

		# Adding hair to actor will evaluate once, so we start with evaluation_count of 1
		self.assertEqual( 1, targetHair.evaluation_count )
		targetHair.get_vertices( True )
		self.assertEqual( 2, targetHair.evaluation_count )

		guidesFromMesh.guide_length = 2.0
		targetHair.get_vertices( True )
		self.assertEqual( 3, targetHair.evaluation_count )
		
	'''def test_BakeOperatorUndo( self ):
		testHair = self.CreateStackWithSingleFrizz()
		#verticesBeforeBaking = testHair.get_vertices( True )
		testHair.bake_operators( 1, 1 )

		self.assertEqual( 'BakedNode', testHair.ordered_operators[1].get_node_name() )

		# Undo baking
		unreal.VPBlueprintLibrary.editor_undo()

		self.assertEqual( 'Frizz', testHair.ordered_operators[1].get_node_name() )'''