import MayaTest
import TestUtilities
import os
import pymel.core as pm
import pymel.core.system as pmsys
import pymel.core.datatypes as dt
import unittest

class Test_PropagationNode( MayaTest.OxTestCase ):

	def test_BasicPropagation( self ):
		# Check that at least some vertices are modified
		TestUtilities.TestStrandOperatorChangesGuideCount( self, TestUtilities.PropagationNodeName )

	def test_MultiplePropagationGroups( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		propagation = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.PropagationNodeName ) )
		propagation.generationMethod.set( 2 )
		verticesBeforeGravity = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		gravity = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GravityNodeName ) )
		verticesAfterGravity = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBeforeGravity, verticesAfterGravity )

		# Change gravity input group
		gravity.strandGroupPattern.set( "1" )
		verticesAfterGravityStrandGroup1 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAfterGravity, verticesAfterGravityStrandGroup1 )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBeforeGravity, verticesAfterGravityStrandGroup1 )

	def test_ChangeWidthOnTopOfPropagation( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		propagation = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.PropagationNodeName ) )
		propagation.generationMethod.set( 2 )
		propagation.verticesPerRootCount.set( 2 )

		# Add mesh from strands and calculate width at base by measuring distance between first two vertices
		meshFromStrands = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.MeshFromStrandsNodeName ) )

		meshShape = pm.ls( type = "mesh" )[0]
		meshVerticesBeforeWidthChange = TestUtilities.GetMeshVertices( meshShape )

		changeWidth = pm.PyNode( pm.mel.OxAddStrandOperator( propagation, TestUtilities.ChangeWidthNodeName ) )

		changeWidth.width.set( 1 )
		meshVerticesAfterWidthChange = TestUtilities.GetMeshVertices( meshShape )
		widthAtFirstStrandBase =  meshVerticesAfterWidthChange[0].distanceTo( meshVerticesAfterWidthChange[1] )
		self.assertAlmostEqual( 1.0, widthAtFirstStrandBase )

		vertexCount = len( meshVerticesAfterWidthChange )

		changeWidth.width.set( 2 )
		meshVerticesAfterWidthChange = TestUtilities.GetMeshVertices( meshShape )
		widthAtFirstStrandBase =  meshVerticesAfterWidthChange[vertexCount - 4].distanceTo( meshVerticesAfterWidthChange[vertexCount - 3] )
		self.assertAlmostEqual( 2.0, widthAtFirstStrandBase )

	def test_ChangeWidthOnTopOfPropagationWithGuidesFromCurves( self ):
		guidesShape = TestUtilities.AddGuidesFromCurves()

		propagation = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.PropagationNodeName ) )
		propagation.generationMethod.set( 2 )
		propagation.verticesPerRootCount.set( 2 )

		# Add mesh from strands and calculate width at base by measuring distance between first two vertices
		meshFromStrands = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.MeshFromStrandsNodeName ) )

		meshShape = pm.ls( type = "mesh" )[0]
		meshVerticesBeforeWidthChange = TestUtilities.GetMeshVertices( meshShape )

		changeWidth = pm.PyNode( pm.mel.OxAddStrandOperator( propagation, TestUtilities.ChangeWidthNodeName ) )

		changeWidth.width.set( 1 )
		meshVerticesAfterWidthChange = TestUtilities.GetMeshVertices( meshShape )
		widthAtFirstStrandBase =  meshVerticesAfterWidthChange[0].distanceTo( meshVerticesAfterWidthChange[1] )
		self.assertAlmostEqual( 1.0, widthAtFirstStrandBase, places = 4 )

		vertexCount = len( meshVerticesAfterWidthChange )

		changeWidth.width.set( 2 )
		meshVerticesAfterWidthChange = TestUtilities.GetMeshVertices( meshShape )
		widthAtFirstStrandBase =  meshVerticesAfterWidthChange[vertexCount - 4].distanceTo( meshVerticesAfterWidthChange[vertexCount - 3] )
		self.assertAlmostEqual( 2.0, widthAtFirstStrandBase, places = 4 )

	def test_IncreasePerStrandVertexCount( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod = 6 )
		propagation = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.PropagationNodeName ) )

		# Cause evaluation
		TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		propagation.verticesPerRootCount.set( 10 )

		# Cause another evaluation after changing vertex count, this should not cause a crash
		verticesAfterChangingVertexCount = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )


	def test_MultiplePropagationLevelsGenerateUniqueIds( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod = 6 )
		propagation1 = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.PropagationNodeName ) )
		propagation2 = pm.PyNode( pm.mel.OxAddStrandOperator( propagation1, TestUtilities.PropagationNodeName ) )

		strandIds = pm.mel.OxGetStrandIds( guidesShape )
		self.assertEqual( len( strandIds ), len( set( strandIds ) ) )

	def test_LoadingSavedSceneGeneratesSameIds( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod = 6 )
		propagation1 = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.PropagationNodeName ) )
		propagation2 = pm.PyNode( pm.mel.OxAddStrandOperator( propagation1, TestUtilities.PropagationNodeName ) )

		strandIds1 = sorted( pm.mel.OxGetStrandIds( guidesShape ) )

		fileName = 'propagatorSaveLoadTest.mb'
		pmsys.saveAs( fileName, force = True )
		pmsys.newFile( force = True )
		pmsys.openFile( fileName, force = True )
		os.remove( pmsys.sceneName() )

		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		strandIds2 = sorted( pm.mel.OxGetStrandIds( guidesShape ) )

		self.assertEqual( strandIds1, strandIds2 )

	def GetAverageLengthDisparity( self, hairShape, firstStrandIndex ):
		strandLengths = TestUtilities.GetStrandLengths( hairShape )
		result = 0
		strandCount = len( strandLengths )
		for i in range( firstStrandIndex + 1, strandCount ):
			result += strandLengths[i - 1] - strandLengths[i]

		result /= strandCount - firstStrandIndex - 1
		return result

	def test_LengthRandomness( self ):
		# Generate a single guide in the middle of a plane
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod = 6 )
		propagation = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.PropagationNodeName ) )
		propagation.verticesPerRootCount.set( 2 )
		propagation.sideCount.set( 1 )

		# Remove any variation in length along strand length due to length ramp
		pm.setAttr( propagation + ".lengthRamp[0].lengthRamp_FloatValue", 1.0 )
		pm.setAttr( propagation + ".lengthRamp[1].lengthRamp_FloatValue", 1.0 )
		pm.setAttr( propagation + ".lengthRamp[3].lengthRamp_FloatValue", 1.0 )

		# Without length randomness first two strands have the same length
		propagation.lengthRandomness.set( 0 )
		verticesWithoutRandomLength = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		self.assertAlmostEqual( self.GetAverageLengthDisparity( guidesShape, 1 ), 0, places = 5 )

		# With length randomness first two strands have different lengths
		propagation.lengthRandomness.set( 1 )
		verticesWithRandomLength = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		strand1Length = dt.Vector( verticesWithRandomLength[2] ).distanceTo( verticesWithRandomLength[3] )
		strand2Length = dt.Vector( verticesWithRandomLength[4] ).distanceTo( verticesWithRandomLength[5] )
		self.assertNotAlmostEqual( self.GetAverageLengthDisparity( guidesShape, 1 ), 0, places = 2 )

	def test_ChangeOverrideOfSelectSide( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod = 6 )
		propagation = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.PropagationNodeName ) )

		propagation.generationMethod.set( 0 )
		propagation.sideCount.set( 5 )
		propagation.selectedSide.set( 1 )

		verticesBeforeOverrideTwist = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		#propagation.perStrandSideIndices.set( [1], type = "Int32Array" )
		#propagation.perStrandSideTwists.set( [0.8], type = "floatArray" )
		propagation.twistOverride.set( 0.8 )
		#propagation.twist.set( 0.8 )

		verticesAfterOverrideTwist = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBeforeOverrideTwist, verticesAfterOverrideTwist )

		propagation.twistRandomnessOverride.set( 0.5 )        
		verticesAfterOverrideTwistRandomness = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBeforeOverrideTwist, verticesAfterOverrideTwistRandomness )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAfterOverrideTwist, verticesAfterOverrideTwistRandomness )

	# @unittest.skip( "Test doesn't work" )
	def test_ResetSide( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod = 6 )
		propagation = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.PropagationNodeName ) )

		propagation.generationMethod.set( 2 )
		propagation.sideCount.set( 5 )
		propagation.selectedSide.set( 1 )

		verticesBeforeReset = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		propagation.twistOverride.set( 0.8 )
		pm.mel.OxEditPropagation( propagation, r = True )

		verticesAfterReset = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Hairs should not be affected
		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeReset, verticesAfterReset )

	def test_LoadingSavedOverrideAttribute( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod = 6 )
		propagation = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.PropagationNodeName ) )

		propagation.generationMethod.set( 0 )
		propagation.sideCount.set( 5 )
		propagation.selectedSide.set( 1 )
		propagation.twistOverride.set( 0.8 )

		verticesBeforeSave = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		fileName = 'overrideAttrSaveLoadTest.mb'
		pmsys.saveAs( fileName, force = True )
		pmsys.newFile( force = True )
		pmsys.openFile( fileName, force = True )
		os.remove( pmsys.sceneName() )

		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		verticesAfterload = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeSave, verticesAfterload )

	def test_FlatMapTextureCoordinates( self ):
		# Create reference and scattered hairs
		sideCount = 5
		guidesShape = TestUtilities.CreateSingleHairWithPropagator( sideCount = sideCount )
		textureCoordinates = TestUtilities.GetTextureCoordinates( guidesShape, 0, type = 0 )
		referenceCoords = TestUtilities.GetReferencePropagatorTextureCoordinates( stemVertexCount = 3, branchVertexCount = 4, branchesPerStemCount = 4, sideCount = 5, islandCount = 1, repetitions = 1 )
		TestUtilities.CheckPointsAllNearEqual( self, textureCoordinates, referenceCoords, epsilon = 0.02 )
