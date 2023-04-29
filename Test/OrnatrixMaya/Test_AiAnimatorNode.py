import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt
import pymel.core.system as pmsys
import os
import sys
import unittest

TestsPath = os.path.dirname( __file__ ) if '__file__' in globals() and len( __file__ ) > 0 else os.getcwd()
AIPhysicsScriptPath = os.path.normpath( os.path.join( TestsPath, '../../../../../Ornatrix/AIPhysics' ) )
sys.path.append( AIPhysicsScriptPath )

if os.path.exists( AIPhysicsScriptPath ):
    import MayaTrainingGenerator

@unittest.skipIf( not os.path.exists( AIPhysicsScriptPath ), "These tests can run only from the repository" )
class Test_AiAnimatorNode( MayaTest.OxTestCase ):

	def SetupHairSphere( self, strandCount = 10, pointsPerStrandCount = 10, createMoov = True ):
		self.sphere = pm.polySphere( radius = 10, sx = 20, sy = 20 )
		pm.select( self.sphere )
		self.meshShape = self.sphere[0]

		self.topShape = TestUtilities.AddGuidesToMesh( self.meshShape.getShape(), guideCount = strandCount, pointsPerStrandCount = pointsPerStrandCount, rootGenerationMethod = 2, length = 5 )
		topNode = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]

		if createMoov:
			self.moov = pm.PyNode( pm.mel.OxAddStrandOperator( topNode, TestUtilities.MoovPhysicsNodeName ) )

			# Force evaluation of dynamic attributes
			self.moov.getAttr( 'outputStrands', type = True )

			#self.moov.RootStiffness.set( 0.5 )
			#self.moov.StretchingStiffness.set( 1 )
			#self.moov.BendingStiffness.set( 0.5 )
			#self.moov.RootStiffness.set( 0.5 )

			topNode = self.moov

		self.aiAnimator = pm.PyNode( pm.mel.OxAddStrandOperator( topNode, TestUtilities.AiAnimatorNodeName ) )

		# Force model creation
		pm.currentTime( 1 )
		if MayaTest.InBatchMode:
			self.topShape.boundingBox()
		else:
			pm.refresh()


	@unittest.skipIf( not MayaTest.InBatchMode, "Freezes Maya when started in GUI with Parallel evaluation mode" )
	def test_RandomAnimationTraining( self ):
		self.SetupHairSphere( createMoov = True )

		# Generate animation keyframes
		trainingGenerator = MayaTrainingGenerator.TrainingGenerator()
		trainingGenerator.SetParameters( randomSeed = 1000, positionLimits = ( 20, 2, 20 ), rotationLimits = ( 10, 10, 10 ) )

		pm.mel.OxEnableOperator( self.moov, 0 )
		self.assertEqual( 1, self.moov.nodeState.get() )
		pm.select( self.meshShape, replace = True )
		selection = pm.ls( sl = True )
		trainingGenerator.MayaGenerateTrainingAnimation( selection[0], keyframeCount = 5 )
		pm.mel.OxEnableOperator( self.moov, 1 )

		modelFilePath = self.addTempFile( "tempAiModel.h5" )
		self.aiAnimator.filePath.set( modelFilePath )

		pm.mel.OxRecordAnimation( self.aiAnimator, tr = True )
		# Avoid recalculating Moov model, since it is no longer needed
		pm.mel.OxEnableOperator( self.moov, 0 )
		# Needed to trigger model save
		pm.currentTime( 1 )
		if MayaTest.InBatchMode:
			self.topShape.boundingBox()
		else:
			pm.refresh()

		self.assertTrue( os.path.exists( modelFilePath ) )

	def test_ModelLoadAndSimulate( self ):
		self.SetupHairSphere( createMoov = False )

		# Generate animation keyframes
		pm.select( self.meshShape, replace = True )
		transform = pm.ls( sl = True )[0]

		pm.currentTime( 10 )
		pm.setKeyframe( transform )
		pm.currentTime( 30 )
		pm.move( transform, [10, 10, 10], relative = True )
		pm.setKeyframe( transform )

		modelFilePath = os.path.join( TestsPath, "TestAiModel.h5" )
		self.aiAnimator.filePath.set( modelFilePath )

		hairShape = pm.mel.OxGetStackShape( self.aiAnimator )

		# Needed to trigger model save
		pm.currentTime( 1 )
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		pm.currentTime( 2 )
		TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		for time in range( 3, 35 ):
			pm.currentTime( time )
			verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
			TestUtilities.CheckPointsNotAllNearEqual( self, verticesBefore, verticesAfter )
			verticesBefore = verticesAfter

	def test_GenerateAnimationCommand( self ):
		self.SetupHairSphere( createMoov = False )

		pm.mel.OxGenerateAnimation( self.aiAnimator )

		pm.currentTime( 100 )
		position = pm.xform( self.sphere, q=True, t=True )
		self.assertGreater( pm.datatypes.Point( position ).distanceTo( [0, 0, 0] ), 0 )
