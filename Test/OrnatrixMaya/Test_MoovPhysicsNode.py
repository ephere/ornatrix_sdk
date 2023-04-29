from inspect import Attribute
import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt
import pymel.core.system as pmsys
import os
import unittest

TestsPath = os.path.dirname( __file__ ) if '__file__' in globals() and len( __file__ ) > 0 else os.getcwd()

#MoovPythonFileName = os.path.join( TestsPath, 'MoovHairSimulator.py' )
MoovScriptPath = os.path.normpath( os.path.join( TestsPath, '../../../../../Ornatrix/Python/MoovScripts' ) )
# Commenting the next line ensures that tests fail when they cannot find the import path.
#sys.path.append( MoovScriptPath )

MoovPythonFileName = os.path.join( MoovScriptPath, 'HairSimulator.py' )

MoovPhysicsEngineCount = -1

def SkipIfNoScript( func ):
	if os.path.isfile( MoovPythonFileName ):
		return func
	return unittest.skip( "Moov hair simulator script not found" )( func )

def GetMoovPhysicsEngineCount():
	global MoovPhysicsEngineCount
	if MoovPhysicsEngineCount < 0:
		MayaTest.LoadOrnatrixIfNeeded()
		mpn = pm.createNode( 'MoovPhysicsNode' )
		MoovPhysicsEngineCount = mpn.physicsEngineCount.get()
		pm.delete( mpn )

		global MoovScriptPath
		global MoovPythonFileName
		if not os.path.exists( MoovScriptPath ):
			MoovScriptPath = os.path.join( os.path.dirname( os.path.dirname( pm.pluginInfo( MayaTest.PluginFileName, q = True, path = True ) ) ), 'scripts', 'Moov' )
			MoovPythonFileName = os.path.join( MoovScriptPath, 'HairSimulator.py' )

	return MoovPhysicsEngineCount


def SkipIfNoMoovEngines( func ):
	if GetMoovPhysicsEngineCount() > 0:
		return func
	return unittest.skip( "Moov library not available" )( func )



@SkipIfNoMoovEngines
class Test_MoovPhysicsNode( MayaTest.OxTestCase ):

	@staticmethod
	def SetupPlaneGuidesMoov( pointsPerStrandCount = 10, guideCount = None, guideLength = None, rootGenerationMethod = None, scriptName = None, gravity = None, planeSize = 1 ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( pointsPerStrandCount = pointsPerStrandCount, planeSize = planeSize )
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )

		if guideCount is not None:
			guidesFromMesh.attr( 'count' ).set( guideCount )
		if guideLength is not None:
			guidesFromMesh.length.set( guideLength )
		if rootGenerationMethod is not None:
			guidesFromMesh.distribution.set( rootGenerationMethod )

		moovName = pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.MoovPhysicsNodeName )
		moov = pm.PyNode( moovName )

		if scriptName is not None:
			moov.fileName.set( scriptName )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )

		if gravity is not None:
			moov.Gravity.set( gravity )

		return guidesShape, moov

	@staticmethod
	def RunMoovSimulation( moov, timeRange ):
		"""Runs simulation setting time and forcing MoovPhysicsNode compute at each step in timeRange.

			Call this method when stepping simulation without querying hair in each step.
		"""
		for time in timeRange:
			pm.currentTime( time )
			# Force compute
			moov.getAttr( 'outputStrands', type = True )


	def test_PythonAttributes( self ):
		scriptName = os.path.join( TestsPath, 'MoovPhysicsNode_BasicScript.py' )
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 5, scriptName = scriptName )

		# Check for existing dynamic attributes
		intAttr = moov.getAttr( 'IntTestParam' )
		self.assertIsNotNone( intAttr )
		self.assertEqual( intAttr, 1 )

		# Check if dynamic attributes from default script have been deleted
		self.assertRaises( pm.MayaAttributeError, moov.getAttr, 'LatticeCount' )

	@SkipIfNoScript
	def test_BasicDynamics( self ):
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 5, scriptName = MoovPythonFileName, gravity = -1000.00 )

		pm.currentTime( 1 )
		verticesAtFrame1 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		pm.currentTime( 2 )
		verticesAtFrame2 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# The vertices should have moved at frame 3
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAtFrame1, verticesAtFrame2 )


	def SetupDeformingMeshWithHairDynamics( self, startTime = 0, endTime = 5, animatedVertexOffset = 6 ):
		plane = pm.polyPlane( sx = 1, sy = 1, h = 10, w = 10 )
		meshShape = plane[0]

		# Animate plane vertices
		pm.select( meshShape + ".vtx[0] ", replace = True )
		pm.select( meshShape + ".vtx[2]", add = True  )

		pm.currentTime( startTime )
		pm.setKeyframe()

		pm.currentTime( endTime )
		pm.move( -animatedVertexOffset, 0, 0, r = True )
		pm.setKeyframe()

		pm.select( meshShape + ".vtx[1] ", replace = True )
		pm.select( meshShape + ".vtx[3]", add = True  )

		pm.currentTime( startTime )
		pm.setKeyframe()

		pm.currentTime( endTime )
		pm.move( animatedVertexOffset, 0, 0, r = True )
		pm.setKeyframe()

		pm.select( plane, replace = True )
		guidesShape = TestUtilities.AddGuidesToMesh( meshShape, 4, 4, 5, 0, 10 )

		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		moovName = pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.MoovPhysicsNodeName )
		moov = pm.PyNode( moovName )
		moov.fileName.set( MoovPythonFileName )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )

		# Make gravity go up
		moov.Gravity.set( 1000.00 )
		moov.baseMeshAnimType.set( 2 ) # set base mesh deformable

		return guidesShape, meshShape, moov

	def test_DeformingBaseMesh( self ):
		# This test currently works even without any physics adapters
		guidesAndMesh = self.SetupDeformingMeshWithHairDynamics()
		guidesShape = guidesAndMesh[0]
		meshShape = guidesAndMesh[1]
		# LINUX ONLY: A dynamic attribute needs to be accessed (set or get) before setting currentTime, otherwise the test hangs
		guidesAndMesh[2].Gravity.get()

		# At all frames the roots of the strands must be in the same positions as mesh vertices
		for frame in range( 0, 10 ):
			pm.currentTime( frame )
			for vertexIndex in range( 0, 4 ):
				meshVertices = TestUtilities.GetMeshVertices( meshShape )
				rootPosition = TestUtilities.GetStrandPointPositionsAtIndex( guidesShape, 0, True )
				TestUtilities.CheckPointsAllNearEqual( self, meshVertices, rootPosition )

	@SkipIfNoScript
	def test_ExternalForce( self ):

		airField = pm.mel.air( magnitude = 15.0 )[0]
		airFieldNode = pm.PyNode( airField )

		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 10, guideLength = 4, scriptName = MoovPythonFileName, gravity = 0 )

		pm.connectAttr( airFieldNode.message, moov.forceFields, na = True )
		self.assertEqual( moov.forceFields.evaluateNumElements(), 1 )

		pm.currentTime( 1 )
		verticesAtFrame1 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		pm.currentTime( 2 )
		verticesAtFrame2 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# The vertices should have moved at frame 2 but not by too much
		averagePointDistance = TestUtilities.GetAveragePointDistance( self, verticesAtFrame1, verticesAtFrame2 )
		self.assertLess( averagePointDistance, 0.3 )
		self.assertGreater( averagePointDistance, 0.01 )

	@unittest.skipIf( MayaTest.InBatchMode, "Requires GUI" )
	def test_AddRemoveExternalForce( self ):
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 10, guideLength = 4, gravity = 0 )

		pm.mel.source( "AEDynamics.mel" )

		fieldCount = 3
		airFields = []
		for fieldIndex in range( fieldCount ):
			airFields.append( pm.mel.air( magnitude = 15.0 )[0] )
			pm.mel.OxConnectForceField( airFields[fieldIndex], moov.forceFields )
			self.assertEqual( moov.forceFields.evaluateNumElements(), fieldIndex + 1 )
			pm.select( clear = True )

		pm.mel.OxUpdateForceFieldList( moov.forceFields )
		listControlName = pm.mel.OxDynamicsUniqueControlName( moov.forceFields, "forceFieldList" )
		self.assertEqual( pm.mel.textScrollList( listControlName, query = True, numberOfItems = True ), fieldCount )

		for fieldIndex in range( fieldCount ):
			pm.mel.textScrollList( listControlName, edit = True, selectItem = airFields[fieldIndex] )
			pm.mel.OxRemoveForceField( moov.forceFields )
			self.assertEqual( moov.forceFields.evaluateNumElements(), fieldCount - fieldIndex - 1 )

		self.assertEqual( pm.mel.textScrollList( listControlName, query = True, numberOfItems = True ), 0 )

	@SkipIfNoScript
	def test_PropagatedHairs( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( guideCount = 10, pointsPerStrandCount = 10 )
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		guidesFromMesh.length.set( 4 )
		sourceStrandCount = pm.mel.OxGetStrandCount( guidesShape )
		propagation = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.PropagationNodeName ) )
		propagation.generationMethod.set( 2 )
		# Make sure propagated roots are far from source roots so they can move
		propagation.lowRange.set( 0.3 )
		moov = pm.PyNode( pm.mel.OxAddStrandOperator( propagation, TestUtilities.MoovPhysicsNodeName ) )
		moov.fileName.set( MoovPythonFileName )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )
		moov.Gravity.set( -100.0 )
		moov.ModelType.set( 2 )


		pm.currentTime( 1 )
		# Ids can be negative and do not sort propagated strands last; use indices first and then sort by id
		originalRootPositions = TestUtilities.GetStrandPointPositionsAtIndex( guidesShape, 0, True, False )
		strandIds = pm.mel.OxGetStrandIds( guidesShape )
		originalSourceRoots = [x for _, x in sorted( zip( strandIds[0:sourceStrandCount], originalRootPositions[0:sourceStrandCount] ) )]
		originalPropagatedRoots = [x for _, x in sorted( zip( strandIds[sourceStrandCount:], originalRootPositions[sourceStrandCount:] ) )]

		for frame in range( 2, 10 ):
			pm.currentTime( frame )
			finalRootPositions = TestUtilities.GetStrandPointPositionsAtIndex( guidesShape, 0, True, False )
			strandIds = pm.mel.OxGetStrandIds( guidesShape )
			finalSourceRoots = [x for _, x in sorted( zip( strandIds[0:sourceStrandCount], finalRootPositions[0:sourceStrandCount] ) )]
			finalPropagatedRoots = [x for _, x in sorted( zip( strandIds[sourceStrandCount:], finalRootPositions[sourceStrandCount:] ) )]
			# Source roots should keep their position, propagated roots should move
			TestUtilities.CheckPointsAllNearEqual( self, originalSourceRoots, finalSourceRoots )
			TestUtilities.CheckPointsAllNotNearEqual( self, originalPropagatedRoots, finalPropagatedRoots )

	@SkipIfNoScript
	def test_ChangeHairDetail( self ):
		"""Test if increasing hair detail updates the hair model correctly."""
		guidesShape = TestUtilities.AddGuidesToNewPlane( pointsPerStrandCount = 10 )
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		detailNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.DetailNodeName ) )
		detailNode.viewPointCount.set( 10 )
		moov = pm.PyNode( pm.mel.OxAddStrandOperator( detailNode, TestUtilities.MoovPhysicsNodeName ) )
		moov.fileName.set( MoovPythonFileName )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )
		# LINUX ONLY: A dynamic attribute needs to be accessed (set or get) before setting currentTime, otherwise the test hangs
		# TODO: Find out why
		moov.Gravity.set( -1000.0 )

		pm.currentTime( 1 )
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		# Run simulation without detail
		pm.currentTime( 2 )
		verticesDetail10 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		# Make sure simulation has actually run, without throwing Python exceptions
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBefore, verticesDetail10 )

		detailNode.viewPointCount.set( 30 )
		moov.getAttr( 'outputStrands', type = True )

		pm.currentTime( 1 )
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		# Run simulation with detail
		pm.currentTime( 2 )
		verticesDetail30 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		# Make sure simulation has actually run, without throwing Python exceptions
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBefore, verticesDetail30 )

		self.assertEqual( 10 * 4, len( verticesDetail10 ) )
		self.assertEqual( 30 * 4, len( verticesDetail30 ) )

	def test_AddCollisionMeshes( self ):

		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 10, guideLength = 4, gravity = 0 )

		sphere1 = pm.polySphere( r = 1 )
		pm.move( sphere1[0], 2, 2, 2 )

		sphere2 = pm.polySphere( r = 1 )
		pm.move( sphere2[0], -2, -2, 2 )

		pm.connectAttr( sphere1[0].worldMesh, moov.collisionMeshArray, na = True )
		pm.connectAttr( sphere2[0].worldMesh, moov.collisionMeshArray, na = True )

		self.assertEqual( moov.collisionMeshArray.evaluateNumElements(), 2 )

		# Crash https://ephere.com:3000/issues/2350
		pm.mel.OxDeleteStrandOperator( moov )

	def SetupMovingCollisionMesh( self, startTime = 0, endTime = 5, animatedVertexOffset = 6 ):
		sphere = pm.polySphere( r = 1 )
		pm.currentTime( startTime )
		pm.move( sphere[0], -animatedVertexOffset, 0, 0 )
		pm.setKeyframe()

		pm.currentTime( endTime )
		pm.move( sphere[0], animatedVertexOffset, 0, 0 )
		pm.setKeyframe()

		return sphere

	def test_MovingCollisionMesh( self ):
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 10, guideLength = 4, gravity = 0 )

		sphere = self.SetupMovingCollisionMesh()
		pm.connectAttr( sphere[0].worldMesh, moov.collisionMeshArray, na = True )
		self.assertEqual( moov.collisionMeshArray.evaluateNumElements(), 1 )

		pm.currentTime( 0 )
		verticesAtFrame0 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		# Run simulation with detail
		pm.currentTime( 1 )
		verticesAtFrame1 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		# Make sure simulation has actually run, without throwing Python exceptions
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAtFrame0, verticesAtFrame1 )

	def SetupScaledRotatedCollisionMesh( self, moov, zDistance ):
		sphere = pm.polySphere( r = 1 )
		pm.scale( sphere[0], [2 * zDistance, 1, 2 * zDistance] )
		pm.rotate( sphere[0], ['90deg', 0, 0], absolute = True )
		pm.move( sphere[0], 0, 0, zDistance )
		pm.connectAttr( sphere[0].worldMesh, moov.collisionMeshArray, na = True )
		self.assertEqual( moov.collisionMeshArray.evaluateNumElements(), 1 )
		return sphere

	def test_ScaledCollisionMesh( self ):
		distanceFromOrigin = 5
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 10, guideLength = 4, gravity = 0, planeSize = 2 * distanceFromOrigin )
		sphere = self.SetupScaledRotatedCollisionMesh( moov, distanceFromOrigin )
		moov.getAttr( 'outputStrands', type = True )
		moov.setAttr( 'CollideWithMeshes', True )
		# Need three steps to make comparison more precise (no displacement in non-colliding strands between steps 2 and 3; first step is different)
		pm.currentTime( 0 )
		tipsAtFrame0 = TestUtilities.GetTipPositions( guidesShape, True )
		pm.currentTime( 1 )
		tipsAtFrame1 = TestUtilities.GetTipPositions( guidesShape, True )
		pm.currentTime( 2 )
		tipsAtFrame2 = TestUtilities.GetTipPositions( guidesShape, True )

		for index in range( len( tipsAtFrame0 ) ):
			if tipsAtFrame0[index][2] == distanceFromOrigin:
				self.assertLessEqual( 0.1, dt.Vector( tipsAtFrame2[index] ).distanceTo( tipsAtFrame1[index] ) )
			else:
				self.assertGreaterEqual( 0.001, dt.Vector( tipsAtFrame2[index] ).distanceTo( tipsAtFrame1[index] ) )

	def test_AnimatedScaledCollisionMesh( self ):
		distanceFromOrigin = 5
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 10, guideLength = 4, gravity = 0, planeSize = 2 * distanceFromOrigin )

		sphere = pm.polySphere( r = 1 )
		pm.rotate( sphere[0], ['90deg', 0, 0], absolute = True )
		pm.move( sphere[0], 0, distanceFromOrigin, 0 )

		pm.currentTime( 0 )
		pm.scale( sphere[0], [distanceFromOrigin, distanceFromOrigin, 1] )
		pm.setKeyframe()
		pm.currentTime( 12 )
		pm.scale( sphere[0], [2 * distanceFromOrigin, 2 * distanceFromOrigin, distanceFromOrigin/2] )
		pm.setKeyframe()
		pm.currentTime( 0 )

		pm.connectAttr( sphere[0].worldMesh, moov.collisionMeshArray, na = True )

		moov.getAttr( 'outputStrands', type = True )
		moov.setAttr( 'LatticeSize', 0.0 )
		moov.setAttr( 'SubstepCount', 1 )
		moov.setAttr( 'CollideWithMeshes', True )
		# Need three steps to make comparison more precise (no displacement in non-colliding strands between steps 2 and 3; first step is different)
		tipsAtFrame0 = TestUtilities.GetTipPositions( guidesShape, True )
		self.RunMoovSimulation( moov, range( 14 ) )
		tipsAtFrame2 = TestUtilities.GetTipPositions( guidesShape, True )

		for index in range( len( tipsAtFrame0 ) ):
			self.assertLessEqual( 0.1, dt.Vector( tipsAtFrame2[index] ).distanceTo( tipsAtFrame0[index] ) )

	def test_AnimatedCollisionMeshViaShapeParameters( self ):
		"""Tests collision meshes animated via non-transform shape attributes such as radius"""
		distanceFromOrigin = 5
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 10, guideLength = 4, gravity = 0, planeSize = 2 * distanceFromOrigin )

		sphere = pm.polySphere( r = 1 )
		pm.move( sphere[0], 0, distanceFromOrigin, 0 )

		pm.currentTime( 0 )
		pm.setKeyframe( sphere[1], attribute = 'radius', value = distanceFromOrigin )
		pm.currentTime( 12 )
		pm.setKeyframe( sphere[1], attribute = 'radius', value = 1.5 * distanceFromOrigin )
		pm.currentTime( 0 )

		pm.connectAttr( sphere[0].worldMesh, moov.collisionMeshArray, na = True )

		moov.getAttr( 'outputStrands', type = True )
		moov.setAttr( 'LatticeSize', 0.0 )
		moov.setAttr( 'SubstepCount', 1 )
		moov.setAttr( 'CollideWithMeshes', True )
		# Need three steps to make comparison more precise (no displacement in non-colliding strands between steps 2 and 3; first step is different)
		tipsAtFrame0 = TestUtilities.GetTipPositions( guidesShape, True )
		self.RunMoovSimulation( moov, range( 14 ) )
		tipsAtFrame2 = TestUtilities.GetTipPositions( guidesShape, True )

		for index in range( len( tipsAtFrame0 ) ):
			self.assertLessEqual( 0.1, dt.Vector( tipsAtFrame2[index] ).distanceTo( tipsAtFrame0[index] ) )

	def test_CollisionPenetrationOpenMesh( self ):
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 10, guideLength = 4, gravity = -1000, planeSize = 1 )
		plane = pm.polyPlane( sx = 1, sy = 1, h = 10, w = 10 )
		pm.connectAttr( plane[0].worldMesh, moov.collisionMeshArray, na = True )
		self.assertEqual( moov.collisionMeshArray.evaluateNumElements(), 1 )
		moov.getAttr( 'outputStrands', type = True )
		moov.setAttr( 'CollideWithMeshes', True )
		# For now only a large collision tolerance prevents open mesh penetration
		moov.setAttr( 'CollisionTolerance', 1.0 )
		self.RunMoovSimulation( moov, range( 10 ) )
		vertices = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		for vertex in vertices:
			self.assertGreaterEqual( vertex[1], -0.1 )

	@SkipIfNoScript
	def test_CaptureInitialState( self ):
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 10, guideLength = 4, scriptName = MoovPythonFileName, gravity = -1000.0 )

		# Run simulation
		self.RunMoovSimulation( moov, range( 1, 4 ) )

		# Create capture
		filename = moov.captureFileName.get()
		self.assertTrue( filename is None or len( filename ) == 0 )
		moov.captureInitialState.set( 1 )
		moov.getAttr( 'outputStrands', type = True )

		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		pm.currentTime( 1 )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesBefore, verticesAfter )

		# Remove capture
		moov.captureInitialState.set( 2 )
		moov.getAttr( 'outputStrands', type = True )

		pm.currentTime( 1 )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBefore, verticesAfter )

	def test_SaveLoadSceneWithCapture( self ):
		"""Tests if capture file is saved and loaded correctly with the scene"""

		sceneName = "Test_MoovPhysics_SaveLoadScene.mb"

		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 10, guideLength = 4, scriptName = MoovPythonFileName, gravity = -1000.0 )

		# Run simulation to make sure captured hair differs from newly created
		self.RunMoovSimulation( moov, range( 1, 4 ) )

		moov.captureInitialState.set( 1 )
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		pmsys.renameFile( sceneName )
		pmsys.saveFile()
		filename = moov.captureFileName.get()
		self.assertNotEqual( len( filename ), 0 )

		sceneNamePath = pmsys.sceneName()
		self.assertTrue( sceneNamePath.isfile() )
		fullCaptureName = os.path.join( sceneNamePath.dirname(), filename )
		self.assertTrue( os.path.isfile( fullCaptureName ) )

		pmsys.openFile( sceneName )
		guidesShapeNodes = pm.ls( type = TestUtilities.HairShapeName )
		self.assertGreater( len( guidesShapeNodes ), 0 )
		moovNodes = pm.ls( type = TestUtilities.MoovPhysicsNodeName )
		self.assertGreater( len( moovNodes ), 0 )
		guidesShape = guidesShapeNodes[0]
		moov = moovNodes[0]
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )
		# LINUX ONLY: A dynamic attribute needs to be accessed (set or get) before setting currentTime, otherwise the test hangs
		moov.Gravity.get()

		# Test we have capture
		filename = moov.captureFileName.get()
		self.assertNotEqual( len( filename ), 0 )

		# Test restored vertices are identical to the captured ones
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesBefore, verticesAfter )

		#moov.captureInitialState.set( 2 )
		#moov.getAttr( 'outputStrands', type = True )
		#newFilename = moov.captureFileName.get()
		#self.assertEqual( len( newFilename ), 0 )

		# Delete saved files
		#pmsys.newFile()
		sceneNamePath.remove()
		os.remove( fullCaptureName )

	def test_CaptureRestState( self ):
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 10, guideLength = 10, scriptName = MoovPythonFileName, gravity = -1000.0 )
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		print( str( moov ) + ".captureInitialState" )
		pm.mel.MoovSimulateRestState( str( moov ) + ".captureInitialState" )
		moov.getAttr( 'outputStrands', type = True )
		verticesRest = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBefore, verticesRest )

		# Test if simulation from rest state moves significantly
		self.RunMoovSimulation( moov, range( 1, 4 ) )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesRest, verticesAfter, epsilon=0.2 )

	@SkipIfNoScript
	def test_UnstableEquilibrium( self ):
		"""Tests if up-pointing vertical strands leave the unstable equilibrium state."""

		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 5, scriptName = MoovPythonFileName, gravity = -1000.0 )
		# Uncomment for zero horizontal displacements, which should fail the test
		# moov.LatticeSize.set( 0 )

		pm.currentTime( 1 )
		verticesAtFrame1 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		self.RunMoovSimulation( moov, [2] )
		pm.currentTime( 3 )
		verticesAtFrame3 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Horizontal (XZ) positions of vertices should have moved at frame 3
		self.assertEqual( len( verticesAtFrame1 ), len( verticesAtFrame3 ) )
		for index in range( len( verticesAtFrame1 ) ):
			verticesAtFrame1[index][1] = verticesAtFrame3[index][1]
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAtFrame1, verticesAtFrame3 )

	def test_CosseratRootRotation( self ):
		"""Tests if Cosserat root orientations rotate with the base mesh"""
		startTime = 1
		endTime = 10

		# Setup rotating base mesh
		plane = pm.polyPlane( sx = 1, sy = 1, h = 10, w = 10 )
		meshShape = plane[0]

		# Rotate plane
		pm.select( meshShape, replace = True )
		pm.rotate( '90deg', 0, 0, r = True )
		pm.currentTime( startTime )
		pm.setKeyframe()
		pm.currentTime( endTime )
		pm.rotate( '-90deg', '90deg', 0, r = True )
		pm.setKeyframe()

		pm.select( plane, replace = True )
		guidesShape = TestUtilities.AddGuidesToMesh( meshShape.getShape(), 4, 4, 5, 0, 10 )

		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		moovName = pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.MoovPhysicsNodeName )
		moov = pm.PyNode( moovName )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )
		moov.setAttr( 'Drag', 0.1 )
		# Cosserat
		moov.setAttr( 'ModelType', 2 )
		# Sets the randomization amplitude for particle positions to 0
		moov.setAttr( 'LatticeSize', 0.0 )

		# Make gravity go up
		moov.Gravity.set( 1000.00 )
		moov.baseMeshAnimType.set( 1 ) # set base mesh trasnforming

		# Make sure guides are being transformed
		guidesTransform = pm.listRelatives( guidesShape, allParents = True, type = 'transform' )[0]
		self.assertEqual( guidesTransform, meshShape )

		# After relaxation strands should point exactly up
		self.RunMoovSimulation( moov, range( startTime, 20 + endTime ) )

		strandCount = pm.mel.OxGetStrandCount( guidesShape )
		for strandIndex in range( strandCount ):
			# Check that strand is straight up
			strandPoints = TestUtilities.GetStrandPoints( guidesShape, strandIndex, True )
			root = dt.Vector( strandPoints[0] )
			root.y = 0
			for vertexIndex in range( 1, 4 ):
				vertex = dt.Vector( strandPoints[vertexIndex] )
				vertex.y = 0
				self.assertLessEqual( vertex.distanceTo( root ), 0.05 )


	def test_CosseratRootRotationNearMinusZDirection( self ):
		"""Tests Cosserat root orientations bug when root direction passes near -z direction, #3933"""

		startTime = 1
		endTime = 10
		# Set up rotating plane to have the normal pass through the -z direction
		plane = pm.polyPlane( sx = 1, sy = 1, h = 10, w = 10 )
		meshShape = plane[0]
		# Rotate plane
		pm.select( plane, replace = True )
		pm.currentTime( startTime )
		pm.rotate( 0, '90deg', '135deg', absolute = True )
		pm.setKeyframe()
		pm.currentTime( endTime )
		pm.rotate( 0, '90deg', '180deg', absolute = True )
		pm.setKeyframe()
		pm.currentTime( startTime )
		# Only need a single strand
		editGuidesShape = TestUtilities.AddEditGuidesToMesh( meshShape.getShape(), 1, 2, 5, 10, 0 )
		# Edit guides to have strand slightly off normal (bug manifests for orientations close to, but not exacty -z)
		pm.select( editGuidesShape + '.ep[0]', replace = True )
		pm.move( 0, -5, 0, relative = True )
		pm.mel.OxEditGuides( cc = True )
		moovName = pm.mel.OxAddStrandOperator( editGuidesShape, TestUtilities.MoovPhysicsNodeName )
		moov = pm.PyNode( moovName )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )
		moov.setAttr( 'Drag', 0.1 )
		# Cosserat
		moov.setAttr( 'ModelType', 2 )
		# Sets the randomization amplitude for particle positions to 0
		moov.setAttr( 'LatticeSize', 0.0 )

		# Make sure guides are being transformed
		guidesTransform = pm.listRelatives( editGuidesShape, allParents = True, type = 'transform' )[0]
		self.assertEqual( guidesTransform, meshShape )

		hairShape = pm.mel.OxGetStackShape( moov )
		initialStrandPoints = TestUtilities.GetStrandPoints( hairShape, 0, True )

		pm.select( hairShape, replace = True )
		self.RunMoovSimulation( moov, range( startTime, 20 + endTime ) )

		# Check that the strand has dropped under gravity compared to initial position
		strandPoints = TestUtilities.GetStrandPoints( hairShape, 0, True )
		for strandPoint, initialStrandPoint in zip( strandPoints, initialStrandPoints ):
			self.assertGreaterEqual( initialStrandPoint[1], strandPoint[1] )


	def test_CosseratSlantedRoots( self ):
		"""Tests if Cosserat slanted roots have correct orientations"""
		startTime = 1
		endTime = 3

		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane( guideCount = 4, pointsPerStrandCount = 4 )

		pm.select( editGuidesShape + ".ep[0:3]" )
		pm.move( 0, 0, 5, relative = True )
		pm.mel.OxEditGuides( cc = True )

		moov = pm.PyNode( pm.mel.OxAddStrandOperator( editGuidesShape, TestUtilities.MoovPhysicsNodeName ) )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )
		moov.setAttr( 'Gravity', 0 )
		# Eliminate random displacements
		moov.setAttr( 'LatticeSize', 0 )
		# Cosserat
		moov.setAttr( 'ModelType', 2 )

		guidesShape = TestUtilities.GetNodeByType( TestUtilities.HairShapeName )
		# After simulation strands should not have changed since there is no gravity
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		self.RunMoovSimulation( moov, range( startTime, endTime ) )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		TestUtilities.CheckPointsAllNearEqual( self, verticesBefore, verticesAfter )


	def test_CosseratRestoreCapture( self ):
		"""Tests if restored capture guides jump at start (bug https://ephere.com:3000/issues/2751)"""
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 10, guideLength = 4, scriptName = MoovPythonFileName, gravity = -1000.0 )
		# Cosserat (non-elaston models fail this test)
		moov.setAttr( 'ModelType', 2 )
		moov.setAttr( 'BendingStiffness', 0.1 )
		moov.setAttr( 'Drag', 0.1 )

		# Run simulation until rest
		averageDistance = 0
		pm.currentTime( 1 )
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		for frame in range( 2, 30 ):
			pm.currentTime( frame )
			verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
			averageDistance = TestUtilities.GetAveragePointDistance( self, verticesBefore, verticesAfter )
			if averageDistance < 0.005:
				break
			verticesBefore = verticesAfter

		# Create capture
		moov.captureInitialState.set( 1 )
		moov.getAttr( 'outputStrands', type = True )

		pm.currentTime( 1 )
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		pm.currentTime( 2 )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesBefore, verticesAfter )
		newAverageDistance = TestUtilities.GetAveragePointDistance( self, verticesBefore, verticesAfter )
		self.assertLess( newAverageDistance, averageDistance )

	@unittest.skipIf( not os.path.isfile( os.path.join( MoovScriptPath, 'WetHair.py' ) ), "Missing WetHair.py" )
	def test_WetHairDataChannelsCreated( self ):
		"""Tests if wetness-related data channels are created"""
		wetHairScript = os.path.join( MoovScriptPath, 'WetHair.py' )
		guidesShape, moov = self.SetupPlaneGuidesMoov( scriptName = wetHairScript )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )

		self.assertEqual( [u'Wetness', u'WetnessCompl', u'WetnessInAir'], pm.mel.OxGetVertexChannels( guidesShape ) )

	@unittest.skipIf( not os.path.isfile( os.path.join( MoovScriptPath, 'WetHair.py' ) ), "Missing WetHair.py" )
	def test_WetHairWetnessValues( self ):
		"""Tests computed wetness values"""
		wetHairScript = os.path.join( MoovScriptPath, 'WetHair.py' )
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 10, guideLength = 4, scriptName = wetHairScript, gravity = 0.0, planeSize = 2 )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )

		cube = pm.polyCube( w = 1, d = 1, h = 1 )
		pm.scale( cube[0], [10, 10, 10] )
		pm.connectAttr( cube[0].worldMesh, moov.FluidObjects[0] )
		self.assertEqual( moov.FluidObjects.evaluateNumElements(), 1 )

		moov.setAttr( 'WettingSpeed', 1.0 )

		self.RunMoovSimulation( moov, [1, 2] )
		for value in pm.mel.OxGetVertexValuesForChannel( guidesShape, 0 ):
			self.assertGreater( value, 0.9 )

		pm.move( cube[0], 0, -10, 0 )

		self.RunMoovSimulation( moov, [1, 2] )
		for value in pm.mel.OxGetVertexValuesForChannel( guidesShape, 0 ):
			self.assertEqual( value, 0.0 )


	def SetupComputeModelPerformance( self, modelTypeEnumIndex = None, useCompliantConstraints = None ):
		self.topShape = TestUtilities.AddGuidesToNewPlane( guideCount = 300, pointsPerStrandCount = 10, rootGenerationMethod = 2, planeSegmentCount = 2, planeSize = 100 )
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		moov = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.MoovPhysicsNodeName ) )

		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )

		if modelTypeEnumIndex is not None:
			moov.ModelType.set( modelTypeEnumIndex )
		if useCompliantConstraints is not None:
			moov.UseCompliantConstraints.set( useCompliantConstraints )

		# Use faster root update (once per step instead of once per substep)
		moov.SlowMovingRoots.set( True )

		# Force model creation
		pm.currentTime( 1 )
		if MayaTest.InBatchMode:
			self.topShape.boundingBox()
		else:
			pm.refresh()

	def EvaluatePerformance( self, frameCount = 99 ):
		# Start from frame 2 to avoid model creation
		MayaTest.RunAnimation( self.topShape, startFrame = 2, frameCount = frameCount )

	def test_ComputePerformance_Cosserat( self ):
		# Only the default model (Cosserat)
		self.assertPerformance( 'MayaTest.Self.SetupComputeModelPerformance()', 'MayaTest.Self.EvaluatePerformance()' )

	@unittest.skip( "Only for detailed benchmarking" )
	def test_ComputePerformance_AllModels( self ):
		self.assertPerformance( 'MayaTest.Self.SetupComputeModelPerformance( 0, False )', 'MayaTest.Self.EvaluatePerformance()', "DistanceOnly" )
		self.assertPerformance( 'MayaTest.Self.SetupComputeModelPerformance( 0, True )', 'MayaTest.Self.EvaluatePerformance()', "DistanceOnly.Compliant" )
		self.assertPerformance( 'MayaTest.Self.SetupComputeModelPerformance( 1, False )', 'MayaTest.Self.EvaluatePerformance()', "DistanceBending" )
		self.assertPerformance( 'MayaTest.Self.SetupComputeModelPerformance( 1, True )', 'MayaTest.Self.EvaluatePerformance()', "DistanceBending.Compliant" )
		self.assertPerformance( 'MayaTest.Self.SetupComputeModelPerformance( 2, False )', 'MayaTest.Self.EvaluatePerformance()', "Cosserat" )
		self.assertPerformance( 'MayaTest.Self.SetupComputeModelPerformance( 2, True )', 'MayaTest.Self.EvaluatePerformance()', "Cosserat.Compliant" )
		self.assertPerformance( 'MayaTest.Self.SetupComputeModelPerformance( 3, False )', 'MayaTest.Self.EvaluatePerformance()', "CosseratDistance" )
		self.assertPerformance( 'MayaTest.Self.SetupComputeModelPerformance( 3, True )', 'MayaTest.Self.EvaluatePerformance()', "CosseratDistance.Compliant" )
		self.assertPerformance( 'MayaTest.Self.SetupComputeModelPerformance( 4, False )', 'MayaTest.Self.EvaluatePerformance()', "CosseratElaston" )
		self.assertPerformance( 'MayaTest.Self.SetupComputeModelPerformance( 4, True )', 'MayaTest.Self.EvaluatePerformance()', "CosseratElaston.Compliant" )

	def SetupHairSphere( self, modelTypeEnumIndex = None, enableBaseMeshCollisions = None, enableAttractToInitialShape = None ):
		sphere = pm.polySphere( radius = 10, sx = 20, sy = 20 )
		pm.select( sphere )
		meshShape = sphere[0]

		strandCount = 300
		pointsPerStrandCount = 10
		self.topShape = TestUtilities.AddGuidesToMesh( meshShape, guideCount = 300, pointsPerStrandCount = 10, rootGenerationMethod = 2, length = 5 )
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		moov = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.MoovPhysicsNodeName ) )

		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )

		if modelTypeEnumIndex is not None:
			moov.ModelType.set( modelTypeEnumIndex )

		moov.RootStiffness.set( 0.5 )
		moov.StretchingStiffness.set( 1 )
		#moov.BendingStiffness.set( 0.5 )
		#moov.RootStiffness.set( 0.5 )
		moov.AttractToInitialShape_Stiffness.set( 5 )
		# Use faster root update (once per step instead of once per substep)
		moov.SlowMovingRoots.set( True )

		if enableBaseMeshCollisions is not None:
			moov.CollideWithBaseMesh.set( enableBaseMeshCollisions )
		if enableAttractToInitialShape is not None:
			moov.AttractToInitialShape.set( enableAttractToInitialShape )

		# Force model creation
		pm.currentTime( 1 )
		if MayaTest.InBatchMode:
			self.topShape.boundingBox()
		else:
			pm.refresh()

	def test_ComputePerformanceBaseMeshCollisions( self ):
		self.assertPerformance( 'MayaTest.Self.SetupHairSphere( enableBaseMeshCollisions = True )', 'MayaTest.Self.EvaluatePerformance( frameCount = 30 )' )

	@unittest.skip( "Only for detailed benchmarking" )
	def test_ComputePerformanceAttractToInitialShape( self ):
		self.assertPerformance( 'MayaTest.Self.SetupHairSphere( enableAttractToInitialShape = True )', 'MayaTest.Self.EvaluatePerformance( frameCount = 30 )' )

	def SetupRotatingHairSphere( self, enableBaseMeshCollisions = None, enableAttractToInitialShape = None ):
		sphere = pm.polySphere( radius = 10, sx = 20, sy = 20 )
		pm.select( sphere )
		meshShape = sphere[0].getShape()

		#pm.select( sphere, replace = True )
		pm.currentTime( 1 )
		pm.rotate( 0, '90deg', '135deg', absolute = True )
		pm.setKeyframe()
		pm.currentTime( 20 )
		pm.rotate( 0, '90deg', '225deg', absolute = True )
		pm.setKeyframe()
		pm.currentTime( 1 )

		strandCount = 500
		pointsPerStrandCount = 10
		self.topShape = TestUtilities.AddGuidesToMesh( meshShape, guideCount = 300, pointsPerStrandCount = 5, rootGenerationMethod = 2, length = 5 )
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		moov = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.MoovPhysicsNodeName ) )

		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )

		moov.StretchingStiffness.set( 0.5 )
		moov.BendingStiffness.set( 0.1 )
		moov.AttractToInitialShape_Stiffness.set( 5 )

		if enableBaseMeshCollisions is not None:
			moov.CollideWithBaseMesh.set( enableBaseMeshCollisions )
		if enableAttractToInitialShape is not None:
			moov.AttractToInitialShape.set( enableAttractToInitialShape )

		# Force model creation
		pm.currentTime( 1 )
		if MayaTest.InBatchMode:
			self.topShape.boundingBox()
		else:
			pm.refresh()

	def test_RootUpdatePerformance( self ):
		self.assertPerformance( 'MayaTest.Self.SetupRotatingHairSphere()', 'MayaTest.Self.EvaluatePerformance( frameCount = 30 )',  )


	# Tests compatibility of Moov with animation cache operator
	@unittest.skipIf( not MayaTest.InBatchMode, "Freezes Maya when started in GUI with Parallel evaluation mode" )
	def test_RecordSimulationToAbcFile( self ):
		guidesShape = TestUtilities.AddEditGuidesToNewSphere( radius = 20, pointsPerStrandCount = 10, guideLength = 40 )
		moov = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.MoovPhysicsNodeName ) )
		animationCacheNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.AnimationCacheNodeName ) )

		# Record the animated guides to file
		filePath = self.addTempFile( "test_RecordSimulationToAbcFile.abc" )
		pm.mel.OxLog( filePath )
		animationCacheNode.filePath.set( filePath )
		animationCacheNode.importTextureCoordinates.set( True )
		pm.mel.OxRecordAnimation( animationCacheNode, startTime = 0, endTime = 5 )

		self.assertGreaterEqual( os.path.getsize( filePath ), 4785 )

	def test_MultipleScriptReloads( self ):
		scriptName = os.path.join( TestsPath, 'MoovPhysicsNode_HairSimulatorImport.py' )
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 5, scriptName = scriptName )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )
		for index in range( 5 ):
			moov.setAttr( 'explicitUpdate', True )
			moov.getAttr( 'outputStrands', type = True )
		# Make sure script is read
		self.assertEqual( moov.getAttr( 'Gravity' ), -981 )

	def test_HairUpdateWithMultiplePropagationLevels( self ):
		"""Tests if hair is updated correctly (following the propagation depth) when multiple propagation levels are present."""
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod = 0, guideCount = 10, pointsPerStrandCount = 10, length = 4, planeSize = 10 )
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		sourceStrandCount = pm.mel.OxGetStrandCount( guidesShape )
		propagationLevelCount = 2
		propagationRootCount = 3
		propagationNodes = []
		for propagationLevel in range( propagationLevelCount ):
			propagation = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape if propagationLevel == 0 else propagation, TestUtilities.PropagationNodeName ) )
			propagation.setAttr( "generationMethod", 0 )
			propagation.setAttr( "count", propagationRootCount )
			propagation.setAttr( "verticesPerRootCount", 4 )
			propagation.setAttr( "resultStrandGroup", propagationLevel + 1 )
			propagation.setAttr( "strandGroupPattern", str( propagationLevel ) )
			# Make sure propagated roots are far from source roots so they can move
			propagation.setAttr( "length", 0.5 )
			propagation.setAttr( "lengthRandomness", 0 )
			propagation.setAttr( "lengthRamp[0].lengthRamp_FloatValue", 1 )
			propagation.setAttr( "lengthRamp[1].lengthRamp_FloatValue", 1 )
			propagation.setAttr( "lengthRamp[2].lengthRamp_FloatValue", 1 )
			propagation.setAttr( "distributionRamp[0].distributionRamp_FloatValue", 1 )
			propagation.setAttr( "distributionRamp[1].distributionRamp_FloatValue", 1 )
			propagation.setAttr( "distributionRamp[2].distributionRamp_FloatValue", 1 )
			#propagation.setAttr( "lowRange", 0.3 )
			propagationNodes.append( propagation )

		moov = pm.PyNode( pm.mel.OxAddStrandOperator( propagation, TestUtilities.MoovPhysicsNodeName ) )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )
		moov.setAttr( 'Gravity', -100.0 )
		moov.setAttr( 'ModelType', 3 )
		moov.setAttr( 'UseCompliantConstraints', True )
		moov.setAttr( 'StretchingStiffness', 1 )
		moov.setAttr( 'BendingStiffness', 1 )

		pm.currentTime( 1 )
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		pm.currentTime( 2 )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		averageDistance = TestUtilities.GetAveragePointDistance( self, verticesBefore, verticesAfter )
		self.assertLess( averageDistance, 0.1 )
		self.assertGreater( averageDistance, 0.01 )

	def test_PropagatedStrandsUpdateOrder( self ):
		"""Tests if propagated strand sources are updated first (needed for correct calculation of strand transforms)."""
		# For some reason this does not fail properly with a plane surface and less than 129 propagated guides
		sphere = pm.polySphere( radius = 1, sx = 20, sy = 20 )
		pm.select( sphere )
		meshShape = sphere[0]
		guidesShape = TestUtilities.AddGuidesToMesh( meshShape, guideCount = 1, pointsPerStrandCount = 4, rootGenerationMethod = 2, length = 5 )
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		pm.rotate( guidesShape, ['90deg', 0, 0], absolute = True )
		propagation = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.PropagationNodeName ) )
		propagation.setAttr( "generationMethod", 0 )
		propagation.setAttr( "count", 130 )
		propagation.setAttr( "verticesPerRootCount", 4 )
		propagation.setAttr( "length", 0.5 )
		propagation.setAttr( "lengthRandomness", 0 )
		propagation.setAttr( "lengthRamp[0].lengthRamp_FloatValue", 1 )
		propagation.setAttr( "lengthRamp[1].lengthRamp_FloatValue", 1 )
		propagation.setAttr( "lengthRamp[2].lengthRamp_FloatValue", 1 )
		propagation.setAttr( "distributionRamp[0].distributionRamp_FloatValue", 1 )
		propagation.setAttr( "distributionRamp[1].distributionRamp_FloatValue", 1 )
		propagation.setAttr( "distributionRamp[2].distributionRamp_FloatValue", 1 )
		moov = pm.PyNode( pm.mel.OxAddStrandOperator( propagation, TestUtilities.MoovPhysicsNodeName ) )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )
		moov.setAttr( 'Gravity', -100.0 )
		moov.setAttr( 'ModelType', 2 )
		moov.setAttr( 'StretchingStiffness', 0.5 )
		moov.setAttr( 'BendingStiffness', 0.5 )

		pm.currentTime( 1 )
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		self.RunMoovSimulation( moov, range( 2, 6 ) )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		maxDistance = TestUtilities.GetMaxPointDistance( self, verticesBefore, verticesAfter )
		self.assertLess( maxDistance, 1.55 )

	def test_PropagatedStrandsIdOrder( self ):
		"""Tests if Moov simulation gives correct results when propagated strands order differently by id."""
		# The Moov hair scripts order strands by id, so different propagator ids may result in different strand ordering
		# https://ephere.com:3000/issues/3088

		propagatorIds = [1, 2]

		resultVertices = []
		startVertices = []
		resultVerticesOrdered = []
		strandIndicesOrderedByStrandId = []

		for propagatorId in propagatorIds:
			pmsys.newFile( force = True )

			sphere = pm.polySphere( radius = 1, sx = 20, sy = 20 )
			pm.select( sphere )
			meshShape = sphere[0]
			guidesShape = TestUtilities.AddGuidesToMesh( meshShape, guideCount = 1, pointsPerStrandCount = 4, rootGenerationMethod = 2, length = 5 )
			guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
			pm.rotate( guidesShape, ['90deg', 0, 0], absolute = True )
			propagation = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.PropagationNodeName ) )
			propagation.setAttr( "generationMethod", 0 )
			propagation.setAttr( "count", 4 )
			propagation.setAttr( "verticesPerRootCount", 4 )
			propagation.setAttr( "length", 0.5 )
			propagation.setAttr( "lengthRandomness", 0 )
			propagation.setAttr( "lengthRamp[0].lengthRamp_FloatValue", 1 )
			propagation.setAttr( "lengthRamp[1].lengthRamp_FloatValue", 1 )
			propagation.setAttr( "lengthRamp[2].lengthRamp_FloatValue", 1 )
			propagation.setAttr( "distributionRamp[0].distributionRamp_FloatValue", 1 )
			propagation.setAttr( "distributionRamp[1].distributionRamp_FloatValue", 1 )
			propagation.setAttr( "distributionRamp[2].distributionRamp_FloatValue", 1 )

			propagation.setAttr( "operatorId", propagatorId )

			moov = pm.PyNode( pm.mel.OxAddStrandOperator( propagation, TestUtilities.MoovPhysicsNodeName ) )
			# Force evaluation of dynamic attributes
			moov.getAttr( 'outputStrands', type = True )
			# Determines the randomization amplitude for positions
			moov.setAttr( 'LatticeSize', 0 )
			moov.setAttr( 'ModelType', 0 )
			pm.currentTime( 1 )

			# Get order of strand ids
			strandIds = pm.mel.OxGetStrandIds( guidesShape )
			strandIndicesOrderedByStrandId.append( sorted( range( len( strandIds ) ), key=strandIds.__getitem__ ) )
			startVertices.append( TestUtilities.GetVerticesInObjectCoordinates( guidesShape ) )
			self.RunMoovSimulation( moov, range( 2, 4 ) )
			resultVertices.append( TestUtilities.GetVerticesInObjectCoordinates( guidesShape ) )

		# Make sure that strands are ordered differently by strand id for each propagator id
		for testIds in strandIndicesOrderedByStrandId[1:]:
			self.assertFalse( all( x == y for x, y in zip( strandIndicesOrderedByStrandId[0], testIds ) ) )

		for testVertices in startVertices[1:]:
			TestUtilities.CheckPointsAllNearEqual( self, startVertices[0], testVertices, epsilon = 1e-2 )

		for testVertices in resultVertices[1:]:
			TestUtilities.CheckPointsAllNearEqual( self, resultVertices[0], testVertices, epsilon = 5e-2 )

	@staticmethod
	def CreateSphereGuidesAndMoov( guideCount, pointsPerStrandCount, rootGenerationMethod, length ):
		sphere = pm.polySphere( radius = 1, sx = 20, sy = 20 )
		pm.select( sphere )
		meshShape = sphere[0]
		guidesShape = TestUtilities.AddGuidesToMesh( meshShape, guideCount = guideCount, pointsPerStrandCount = pointsPerStrandCount, rootGenerationMethod = rootGenerationMethod, length = length )
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		pm.rotate( guidesShape, ['90deg', 0, 0], absolute = True )
		moov = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.MoovPhysicsNodeName ) )
		return guidesShape, moov


	@unittest.skipIf( MayaTest.ApiVersion < 201600, "Evaluation manager is not present before Maya 2016" )
	@unittest.skipIf( MayaTest.OsIsLinux, "Moov crashes in parallel evaluation mode under Linux" )
	def test_ParallelEvaluationMode( self ):
		"""Tests if Moov simulation gives correct results for different evaluation modes."""
		# https://ephere.com:3000/issues/3130
		oldEvaluationMode = pm.animation.evaluationManager( query=True, mode=True )

		modesToTest = [u'off', u'serial', u'parallel']

		endVerticesForFirstMode = None

		for evaluationMode in modesToTest:
			pm.animation.evaluationManager( mode = evaluationMode )
			pmsys.newFile( force = True )

			guidesShape, moov = self.CreateSphereGuidesAndMoov( guideCount = 1, pointsPerStrandCount = 4, rootGenerationMethod = 2, length = 5 )
			guidesShape2, moov2 = self.CreateSphereGuidesAndMoov(guideCount = 1, pointsPerStrandCount = 4, rootGenerationMethod = 2, length = 5 )
			pm.move( guidesShape2, [10, 0, 0], relative = True )

			for time in range( 1, 4 ):
				pm.currentTime( time )
				# Force compute
				moov.getAttr( 'outputStrands', type = True )
				moov2.getAttr( 'outputStrands', type = True )

			endVertices = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
			endVertices2 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape2 )
			# Compare the two hair shapes
			TestUtilities.CheckPointsAllNearEqual( self, endVertices, endVertices2 )
			# Compare the graph evaluation modes
			if endVerticesForFirstMode is None:
				endVerticesForFirstMode = endVertices
			else:
				TestUtilities.CheckPointsAllNearEqual( self, endVertices, endVerticesForFirstMode )

		if len( oldEvaluationMode ) > 0:
			pm.animation.evaluationManager( mode = oldEvaluationMode[0] )


	@unittest.skipIf( MayaTest.ApiVersion < 201600, "Evaluation manager is not present before Maya 2016" )
	@unittest.skipIf( MayaTest.OsIsLinux, "Moov crashes in parallel evaluation mode under Linux" )
	def test_ParallelEvaluationModeCrash( self ):
		"""Tests if Moov simulation crashes in parallel mode when returning to initial time. Crashes intermittently."""
		# https://ephere.com:3000/issues/3130
		oldEvaluationMode = pm.animation.evaluationManager( query=True, mode=True )

		evaluationMode = u'parallel'
		pm.animation.evaluationManager( mode = evaluationMode )

		guidesShape, moov = self.CreateSphereGuidesAndMoov( guideCount = 1, pointsPerStrandCount = 4, rootGenerationMethod = 2, length = 5 )
		guidesShape2, moov2 = self.CreateSphereGuidesAndMoov(guideCount = 1, pointsPerStrandCount = 4, rootGenerationMethod = 2, length = 5 )
		pm.move( guidesShape2, [10, 0, 0], relative = True )

		# Returning to initial time causes a crash in parallel mode
		for time in [1, 2, 3, 1]:
			pm.currentTime( time )
			# Force compute
			moov.getAttr( 'outputStrands', type = True )
			moov2.getAttr( 'outputStrands', type = True )

		endVertices = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		endVertices = TestUtilities.GetVerticesInObjectCoordinates( guidesShape2 )

		if len( oldEvaluationMode ) > 0:
			pm.animation.evaluationManager( mode = oldEvaluationMode[0] )

	def test_AttachmentCreation( self ):
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 4, guideCount = 10, guideLength = 10, rootGenerationMethod = 2, planeSize = 1 )
		sphere = pm.polySphere( r = 5.5 )
		pm.move( sphere[0], 0, 10, 0 )
		pm.connectAttr( sphere[0].worldMesh, moov.AttachmentMeshes[0] )
		self.assertEqual( moov.AttachmentMeshes.evaluateNumElements(), 1 )

	def test_AttachmentDynamics( self ):
		plane = pm.polyPlane( sx = 2, sy = 2, h = 10, w = 10 )
		meshShape = plane[0]

		# Move plane vertices to create rooftop geometry
		pm.select( meshShape + ".vtx[3] ", replace = True )
		pm.select( meshShape + ".vtx[4]", add = True  )
		pm.select( meshShape + ".vtx[5]", add = True  )
		pm.move( 0, 4, 0, r = True )

		guidesShape = TestUtilities.AddGuidesToMesh( meshShape, guideCount = 10, rootGenerationMethod = 2, pointsPerStrandCount = 5, lengthRandomness = 0, length = 10 )
		moov = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.MoovPhysicsNodeName ) )

		moov.getAttr( 'outputStrands', type = True )
		moov.setAttr( 'ModelType', 0 )
		moov.setAttr( 'AttachmentDensity', 1 )
		moov.setAttr( 'AttachmentStiffness', 1 )
		attachmentMesh = pm.polyCube( h = 5, d = 30, w = 20 )
		pm.move( attachmentMesh[0], 0, 10, 0 )
		pm.connectAttr( attachmentMesh[0].worldMesh, moov.AttachmentMeshes[0] )
		#pm.hide( attachmentMesh[0] )
		verticesBefore = TestUtilities.GetTipPositions( guidesShape, useObjectCoordinates = True )
		#verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		self.RunMoovSimulation( moov, range( 5 ) )
		verticesAfter = TestUtilities.GetTipPositions( guidesShape, useObjectCoordinates = True )
		#verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Check that hair does not fall down because it is held by the attachment
		for vertexBefore, vertexAfter in zip( verticesBefore, verticesAfter ):
			self.assertGreater( 2, vertexBefore[1] - vertexAfter[1] )
		#self.assertLess( TestUtilities.GetMaxPointDistance( self, verticesBefore, verticesAfter ), 0.31 )

		# Test attachment release
		#moov.setAttr( 'AttachmentReleaseTime', 4 )
		#self.RunMoovSimulation( moov, [5, 6] )
		#verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		#self.assertGreater( TestUtilities.GetMaxPointDistance( self, verticesBefore, verticesAfter ), 0.5 )


	def test_DuplicatedParameters( self ):
		scriptName = os.path.join( TestsPath, 'MoovPhysicsNode_DuplicatedParameter.py' )
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 5, scriptName = scriptName )

		# Check the attribute was created
		duplicatedAttr = moov.getAttr( 'DuplicatedParam' )
		self.assertIsNotNone( duplicatedAttr )
		self.assertEqual( duplicatedAttr, 1 )

		# Check that Maya does not crash on reload
		moov.setAttr( 'explicitUpdate', True )
		moov.getAttr( 'outputStrands', type = True )

		self.assertEqual( 2, pm.mel.OxProfiler( "-warningCount" ) )


	def test_AttractToInitialShapeWithoutGlobalStrandPointCount( self ):
		"""Tests if AttractToInitialShape works correctly when strands have different point counts (issue #3468)"""

		sceneName = os.path.join( TestsPath, "Test_MoovPhysics_AttractToInitialShape.ma" )
		pmsys.openFile( sceneName )
		guidesShapeNodes = pm.ls( type = TestUtilities.HairShapeName )
		self.assertGreater( len( guidesShapeNodes ), 0 )
		moovNodes = pm.ls( type = TestUtilities.MoovPhysicsNodeName )
		self.assertGreater( len( moovNodes ), 0 )
		guidesShape = guidesShapeNodes[0]
		moov = moovNodes[0]
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )
		# LINUX ONLY: A dynamic attribute needs to be accessed (set or get) before setting currentTime, otherwise the test hangs
		moov.Gravity.get()

		pm.currentTime( 1 )
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		self.RunMoovSimulation( moov, range( 2, 4 ) )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesBefore, verticesAfter, epsilon = 0.2 )


	def test_LongRoots( self ):
		"""Tests setting up long non-dynamic roots with RootVertexCount"""
		strandPointCount = 10
		guidesShape, moov = self.CreateSphereGuidesAndMoov( guideCount = 10, pointsPerStrandCount = strandPointCount, rootGenerationMethod = 2, length = 10 )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )
		lastRootIndex = 5
		moov.setAttr( 'RootVertexCount', lastRootIndex )
		# Setting root stiffness to 0.5 enables root holder
		moov.setAttr( 'RootStiffness', 0.5 )

		pm.currentTime( 1 )
		verticesBefore = []
		for strandPointIndex in range( strandPointCount ):
			verticesBefore.append( TestUtilities.GetStrandPointPositionsAtIndex( guidesShape, strandPointIndex ) )
		self.RunMoovSimulation( moov, range( 2, 4 ) )
		for strandPointIndex in range( strandPointCount ):
			verticesAfter = TestUtilities.GetStrandPointPositionsAtIndex( guidesShape, strandPointIndex )
			# Check that roots haven't moved and tips have
			if strandPointIndex <= lastRootIndex:
				TestUtilities.CheckPointsAllNearEqual( self, verticesBefore[strandPointIndex], verticesAfter )
			else:
				TestUtilities.CheckPointsAllNotNearEqual( self, verticesBefore[strandPointIndex], verticesAfter, epsilon = 0.05 )

	def test_LongRootOrientations( self ):
		"""Tests Cosserat root orientations with RootVertexCount (https://ephere.com:3000/issues/3638)"""
		strandPointCount = 10
		guidesShape, moov = self.CreateSphereGuidesAndMoov( guideCount = 10, pointsPerStrandCount = strandPointCount, rootGenerationMethod = 2, length = 10 )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )
		lastRootIndex = 4
		moov.setAttr( 'RootVertexCount', lastRootIndex )
		# Setting root stiffness to 0.5 enables root holder
		moov.setAttr( 'RootStiffness', 0.5 )
		moov.setAttr( 'BendingStiffness', 1.0 )
		moov.setAttr( 'StretchingStiffness', 1.0 )
		moov.setAttr( 'Gravity', 0 )

		pm.currentTime( 1 )
		verticesBefore = []
		for strandPointIndex in range( strandPointCount ):
			verticesBefore.append( TestUtilities.GetStrandPointPositionsAtIndex( guidesShape, strandPointIndex ) )
		self.RunMoovSimulation( moov, range( 2, 4 ) )
		# Check that tips have not moved in zero gravity (incorrect root orientations make them move)
		for strandPointIndex in range( lastRootIndex + 2, strandPointCount ):
			verticesAfter = TestUtilities.GetStrandPointPositionsAtIndex( guidesShape, strandPointIndex )
			TestUtilities.CheckPointsAllNearEqual( self, verticesBefore[strandPointIndex], verticesAfter, epsilon = 0.1 )


	def test_TwoPointStrands( self ):
		# issue #3632 https://ephere.com:3000/issues/3632
		guidesShape, moov = self.CreateSphereGuidesAndMoov( guideCount = 10, pointsPerStrandCount = 2, rootGenerationMethod = 2, length = 10 )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )

		pm.currentTime( 1 )
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		self.RunMoovSimulation( moov, range( 2, 4 ) )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesBefore, verticesAfter )


	def test_ReferencingKeepsParameterValues( self ):
		# issue #3631 https://ephere.com:3000/issues/3631
		#guidesShape, moov = self.CreateSphereGuidesAndMoov( guideCount = 10, pointsPerStrandCount = 10, rootGenerationMethod = 2, length = 10 )
		hairShape = TestUtilities.AddHairToNewSphere( guideCount = 10, pointsPerStrandCount = 10, rootGenerationMethod = 2 )
		moov = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.MoovPhysicsNodeName ) )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )

		# Set attribute to value other than default
		attrNames = ['BendingStiffness', 'StretchingStiffness' ]
		attrValue = 0.0
		moov.setAttr( attrNames[0], attrValue )

		# Create original scene
		pmsys.saveAs( "Test_MoovPhysics_ReferencedScene.mb" )
		referencedFilePath = pmsys.sceneName()
		self.assertTrue( referencedFilePath.isfile() )

		# Create reference
		pmsys.newFile()
		pm.createReference( referencedFilePath )
		moov = pm.ls( type = TestUtilities.MoovPhysicsNodeName )[0]
		self.assertEqual( moov.getAttr( attrNames[0] ), attrValue )
		# Set another attribute to check reference edits
		moov.setAttr( attrNames[1], attrValue )
		moov.getAttr( 'outputStrands', type = True )
		pmsys.saveAs( "Test_MoovPhysics_ReferencingScene.mb" )
		referencingFilePath = pmsys.sceneName()

		# Reload reference
		pmsys.newFile()
		pmsys.openFile( referencingFilePath, force = True )
		moov = pm.ls( type = TestUtilities.MoovPhysicsNodeName )[0]
		moov.getAttr( 'outputStrands', type = True )
		# Check if value from original referenced file is preserved
		self.assertEqual( moov.getAttr( attrNames[0] ), attrValue )
		# Check if reference edit is preserved
		self.assertEqual( moov.getAttr( attrNames[1] ), attrValue )

		referencingFilePath.remove()
		referencedFilePath.remove()

	def test_StrandGroups( self ):
		strandCount = 4
		guidesShape = TestUtilities.AddEditGuidesToNewPlane( guideCount = strandCount, pointsPerStrandCount = 4, rootGenerationMethod = 2, planeSegmentCount = 2, guideLength = 10 )
		self.assertEqual( strandCount, pm.mel.OxGetStrandCount( guidesShape ) )
		# One strand in group 1
		pm.select( guidesShape + ".ep[3]" )
		pm.mel.OxAssignStrandGroup( guidesShape, "1" )

		pm.select( guidesShape )
		moov = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.MoovPhysicsNodeName ) )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )

		hairShape = pm.mel.OxGetStackShape( moov )
		verticesBefore = TestUtilities.GetTipPositions( hairShape, useObjectCoordinates = True )
		self.RunMoovSimulation( moov, range( 5 ) )
		verticesAfter = TestUtilities.GetTipPositions( hairShape, useObjectCoordinates = True )
		# Default empty group should have simulated all strands
		TestUtilities.CheckPointsAllNotNearEqual( self, verticesAfter, verticesBefore )

		# Set strand group to 1
		moov.setAttr( 'StrandGroup', '1' )
		self.RunMoovSimulation( moov, range( 5 ) )
		verticesAfterGroup1Sim = TestUtilities.GetTipPositions( hairShape, useObjectCoordinates = True )
		# Group 0 strands should remain unchanged
		TestUtilities.CheckPointsAllNearEqual( self, verticesAfterGroup1Sim, verticesBefore, pointsCount = 3 )
		TestUtilities.CheckPointsAllNearEqual( self, verticesAfterGroup1Sim, verticesAfter, points1Start = 3, points2Start = 3, pointsCount = 1 )

	def test_GroupHolder( self ):
		strandCount = 10
		mesh = pm.polySphere( r = 10 )
		guidesShape = TestUtilities.AddEditGuidesToMesh( mesh[0], guideCount = strandCount, pointsPerStrandCount = 10, rootGenerationMethod = 2, guideLength = 10 )
		self.assertEqual( strandCount, pm.mel.OxGetStrandCount( guidesShape ) )
		# five strand in group 1
		pm.select( guidesShape + ".ep[5:9]" )
		pm.mel.OxAssignStrandGroup( guidesShape, "1" )

		pm.select( guidesShape )
		moov = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.MoovPhysicsNodeName ) )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )

		# Set up group holder
		moov.setAttr( 'UseGroupHolder', True )
		moov.setAttr( 'GroupHolderPosMin', 3 )
		moov.setAttr( 'GroupHolderPosMax', 8 )
		moov.setAttr( 'GroupStiffness', 1.0 )

		hairShape = pm.mel.OxGetStackShape( moov )
		verticesBefore = TestUtilities.GetTipPositions( hairShape, useObjectCoordinates = True )
		self.RunMoovSimulation( moov, range( 10 ) )
		verticesAfter = TestUtilities.GetTipPositions( hairShape, useObjectCoordinates = True )
		# Points should be close to initial values due to the group holder
		TestUtilities.CheckPointsAllNearEqual( self, verticesAfter, verticesBefore, 4 )


	def test_ParameterReset( self ):
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 5 )

		attrNames = ['StretchingStiffness', 'StretchingStiffnessCurve[0].StretchingStiffnessCurve_FloatValue']
		oldValues = [moov.getAttr( attrName ) for attrName in attrNames]
		newValues = [oldValue - 0.1 for oldValue in oldValues]
		for attrName, newValue in zip( attrNames, newValues ):
			moov.setAttr( attrName, newValue )
			self.assertAlmostEqual( newValue, moov.getAttr( attrName ) )

		moov.resetParameters.set( 1 )
		moov.getAttr( 'outputStrands', type = True )
		for attrName, oldValue in zip( attrNames, oldValues ):
			self.assertAlmostEqual( oldValue, moov.getAttr( attrName ) )

	def test_ParameterSaveLoad( self ):
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 5 )

		filePath = self.addTempFile( "test_ParameterSaveLoad.py" )
		pm.mel.OxLog( filePath )

		attrName = 'StretchingStiffness'
		oldValue = moov.getAttr( attrName )
		newValue = oldValue - 0.1
		moov.setAttr( attrName, newValue )
		self.assertAlmostEqual( newValue, moov.getAttr( attrName ) )

		# Save parameters
		moov.setAttr( 'paramFileName', filePath )
		moov.setAttr( 'paramSaveLoad', 1 )
		moov.getAttr( 'outputStrands', type = True )
		self.assertTrue( os.path.isfile( filePath ) )

		# Change value again to check if saved parameters are loaded correctly
		moov.setAttr( attrName, oldValue )
		moov.getAttr( 'outputStrands', type = True )

		# Load parameters
		moov.setAttr( 'paramSaveLoad', 0 )
		moov.getAttr( 'outputStrands', type = True )
		self.assertAlmostEqual( newValue, moov.getAttr( attrName ) )

	def test_ParameterLoadPartialSet( self ):
		"""Tests simple parameter and ramp curve update on load"""
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 5 )

		filePath = os.path.join( TestsPath, 'MoovPhysicsNode_ParameterSetImport.py' )

		self.assertNotAlmostEqual( moov.getAttr( 'StretchingStiffness' ), 0.1 )
		self.assertNotAlmostEqual( moov.getAttr( 'StretchingStiffnessCurve[0].StretchingStiffnessCurve_FloatValue' ), 0.1 )
		self.assertNotAlmostEqual( moov.getAttr( 'StretchingStiffnessCurve[1].StretchingStiffnessCurve_FloatValue' ), 0.5 )
		self.assertNotAlmostEqual( moov.getAttr( 'StretchingStiffnessCurve[2].StretchingStiffnessCurve_FloatValue' ), 0.9 )

		# Load parameters
		moov.setAttr( 'paramFileName', filePath )
		moov.setAttr( 'paramSaveLoad', 0 )
		moov.getAttr( 'outputStrands', type = True )

		self.assertAlmostEqual( moov.getAttr( 'StretchingStiffness' ), 0.1 )
		self.assertAlmostEqual( moov.getAttr( 'StretchingStiffnessCurve[0].StretchingStiffnessCurve_FloatValue' ), 0.1 )
		self.assertAlmostEqual( moov.getAttr( 'StretchingStiffnessCurve[1].StretchingStiffnessCurve_FloatValue' ), 0.5 )
		self.assertAlmostEqual( moov.getAttr( 'StretchingStiffnessCurve[2].StretchingStiffnessCurve_FloatValue' ), 0.9 )

	@unittest.skip( 'Only for solver profiling' )
	def test_ProfileSolver( self ):
		"""Profiles the solver running a standard simulation. Moov has to be built with profiling enabled."""
		import ephere_moov as moov
		profiler = moov.Profiler.GetInstance()
		profiler.Reset()
		# Comment this to profile only the simulation, not the setup
		#profiler.SetEnabled( True )
		self.SetupHairSphere( enableBaseMeshCollisions = True, enableAttractToInitialShape = False )
		profiler.SetEnabled( True )
		self.EvaluatePerformance( frameCount = 300 )
		profiler.SetEnabled( False )
		#dump = profiler.Dump( True )
		#callGraph = profiler.GetCallGraph()
		enabledTime = profiler.GetEnabledTime()
		profiledTime = profiler.GetProfiledTime()
		print( "Enabled time: {}; profiled time: {}".format( enabledTime, profiledTime ) )
		# Replace with file locations on the test machine
		profiler.SaveToFiles( 'D:/Documents/Ephere-docs/4893/profileTest_25568', 'D:/opt/graphviz-2.38/bin/dot.exe' )

	def test_SettleMode( self ):
		"""Tests the settle mode of the solver."""
		guidesShape, moov = self.CreateSphereGuidesAndMoov( guideCount = 10, pointsPerStrandCount = 5, rootGenerationMethod = 2, length = 10 )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )

		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		self.RunMoovSimulation( moov, range( 3 ) )
		verticesAfterSimulation = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		pm.currentTime( 0 )
		moov.setAttr( 'useSettleMode', True )
		# large threshold ensures all iterations are done
		moov.setAttr( 'settleModeVelocityThreshold', 1000 )
		moov.setAttr( 'settleModeMaxIterations', 5 )
		verticesAfterSettle = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBefore, verticesAfterSettle )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAfterSimulation, verticesAfterSettle )
		moov.setAttr( 'settleModeMaxIterations', 2 )
		verticesAfterSettle = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesAfterSimulation, verticesAfterSettle )

	def test_InitialUpdate( self ):
		"""Tests initial update without full solver reset."""
		guidesShape, moov = self.CreateSphereGuidesAndMoov( guideCount = 5, pointsPerStrandCount = 5, rootGenerationMethod = 2, length = 10 )
		# Force evaluation of dynamic attributes
		moov.getAttr( 'outputStrands', type = True )

		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		self.RunMoovSimulation( moov, range( 3 ) )
		verticesAfterSimulation = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Going back to the initial frame should invoke update without reset
		pm.currentTime( 0 )
		verticesAfterInitialUpdate = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesBefore, verticesAfterInitialUpdate )

		self.RunMoovSimulation( moov, range( 3 ) )
		verticesAfterSimulationWithInitialUpdate = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesAfterSimulation, verticesAfterSimulationWithInitialUpdate )

	def test_AlembicCollider( self ):
		abcFilePath = os.path.dirname( os.path.realpath( __file__ ) ) + "\\MovingCube.abc"
		collider = pm.other.AbcImport( abcFilePath, mode="import" )
		cubeShape = pm.ls( type = 'mesh' )[0]
		guidesShape, moov = self.SetupPlaneGuidesMoov( pointsPerStrandCount = 5, scriptName = MoovPythonFileName, gravity = 0.0, guideLength = 3 )
		pm.connectAttr( cubeShape.worldMesh, moov.collisionMeshArray, na = True )
		# Eliminate random particle displacements
		moov.setAttr( 'LatticeSize', 0 )
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		self.RunMoovSimulation( moov, range( 10 ) )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBefore, verticesAfter )

	def test_GuidesFromCurves( self ):
		guidesShape = TestUtilities.AddGuidesFromCurvesGrounded()
		moov = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.MoovPhysicsNodeName ) )
		# Force evaluation
		pm.mel.OxGetStrandCount( guidesShape )
