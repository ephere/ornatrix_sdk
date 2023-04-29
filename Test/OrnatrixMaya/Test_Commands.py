import os.path
import xml.etree.ElementTree as et
import unittest

import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt

class Test_Commands( MayaTest.OxTestCase ):

	# Tests that "OxSetIsRendering" command properly updates dense hair count and hair node when called. This function is called before and after Maya render from MEL pre/post render callbacks.
	def test_SetIsRendering( self ):
		plane = pm.polyPlane()
		pm.select( plane )

		desiredViewportCount = 10
		hairShape = TestUtilities.AddHairToMesh( plane[0], hairRenderCount = 100, hairViewportCount = desiredViewportCount )

		# Initially we should have viewport count
		# The actual count is 13
		allowedStrandCountError = 5
		self.assertGreaterEqual( allowedStrandCountError, abs( pm.mel.OxGetStrandCount( hairShape ) - desiredViewportCount ) )

		# Set render mode on and make sure we now get render hair count, which should be about 100 (it is possible it's not exactly 100)
		with TestUtilities.RenderModeScope():
			self.assertEqual( 100, pm.mel.OxGetStrandCount(hairShape) )

		# Check again with render mode off
		self.assertGreaterEqual( allowedStrandCountError, abs( pm.mel.OxGetStrandCount( hairShape ) - desiredViewportCount ) )

	def test_RenderHairMemoryUsage( self ):
		if pm.mel.OxProfiler( '-configuration' ) == 'Debug':
			self.skipTest( 'Test is disabled in Debug configuration' )

		pm.mel.OxProfiler( '-enableHairRegistry', True )
		plane = pm.polyPlane()
		hairShape = TestUtilities.AddHairToMesh( plane[0], hairRenderCount = 100000, hairViewportCount = 10 )
		pm.mel.OxAddStrandOperator( '', TestUtilities.CurlNodeName )

		pm.mel.OxGetStrandCount( hairShape )
		memoryBefore = pm.mel.OxProfiler( '-hairMemoryUsage' )
		initialTotalStrands = pm.mel.OxProfiler( '-hairStrandCount' )
		self.assertEqual( memoryBefore, 37 )

		pm.mel.OxLog( 'Before IsRendering=true: strand count: {}, memory usage: {}, total hair objects/strands: {}/{}'
				.format( pm.mel.OxGetStrandCount( hairShape ), memoryBefore, pm.mel.OxProfiler( '-hairObjectCount' ), initialTotalStrands ) )

		with TestUtilities.RenderModeScope():
			# Force evaluation
			renderCount = pm.mel.OxGetStrandCount( hairShape )
			# Then get memory usage
			memoryRender = pm.mel.OxProfiler( '-hairMemoryUsage' )
			pm.mel.OxLog( 'With IsRendering=true: strand count: {}, memory usage: {}, total hair objects/strands: {}/{}'
				.format( renderCount, memoryRender, pm.mel.OxProfiler( '-hairObjectCount' ), pm.mel.OxProfiler( '-hairStrandCount' ) ) )
			self.assertEqual( memoryRender, 37756 )

		memoryAfterIsRenderingFalse = pm.mel.OxProfiler( '-hairMemoryUsage' )
		totalStrands = pm.mel.OxProfiler( '-hairStrandCount' )
		pm.mel.OxLog( 'After IsRendering=false: memory usage: {}, total hair objects/strands: {}/{}'
				.format( memoryAfterIsRenderingFalse, pm.mel.OxProfiler( '-hairObjectCount' ), totalStrands ) )

		# Memory should drop immediately after calling SetIsRendering(false), without having to evaluate the HairShape
		# The amount should be closer to the initial amount than to the render amount
		self.assertEqual( 100242, totalStrands )
		self.assertEqual( memoryAfterIsRenderingFalse, 8738 )

	def test_IHairInterface( self ):
		plane = pm.polyPlane()
		pm.select( plane )
		pm.mel.OxQuickHair()
		hairShape = pm.mel.ls(type=TestUtilities.HairShapeName)[0]

		strandCount = pm.mel.OxGetStrandCount(hairShape)
		self.assertGreater( strandCount, 0 )

		usesStrandTopology = pm.mel.OxUsesStrandTopology(hairShape)
		self.assertIsNotNone( usesStrandTopology )

		usesPerStrandTransformations = pm.mel.OxUsesPerStrandTransformations(hairShape)
		self.assertIsNotNone( usesPerStrandTransformations )

		isUsingPerStrandRotationAngles = pm.mel.OxIsUsingPerStrandRotationAngles(hairShape)
		self.assertIsNotNone( isUsingPerStrandRotationAngles )

		keepsSurfaceDependency = pm.mel.OxKeepsSurfaceDependency(hairShape)
		self.assertIsNotNone( keepsSurfaceDependency )

		keepsGuideDependency = pm.mel.OxKeepsGuideDependency(hairShape)
		self.assertIsNotNone( keepsGuideDependency )

		#usesTopologyOrientation = pm.mel.OxUsesTopologyOrientation(hairShape)
		#self.assertIsNotNone( usesTopologyOrientation )

		globalStrandPointCount = pm.mel.OxGetGlobalStrandPointCount(hairShape)
		self.assertGreater( globalStrandPointCount, 0 )

		mappingChannelCount = pm.mel.OxGetMappingChannelCount(hairShape)
		self.assertGreaterEqual( mappingChannelCount, 0 )

		if usesStrandTopology:
			strandTopology = pm.mel.OxGetStrandTopology(hairShape, 0)
			self.assertEqual( len(strandTopology), 2 )

		strandTransform = pm.mel.OxGetStrandTransform(hairShape, 0)
		self.assertEqual( len(strandTransform), 16 )

		perStrandRotationAngle = pm.mel.OxGetPerStrandRotationAngle(hairShape, 0)
		self.assertIsNotNone( perStrandRotationAngle )

		surfaceDependency = pm.mel.OxGetSurfaceDependency(hairShape, 0)
		self.assertEqual( len(surfaceDependency), 3 )

		guideDependency = pm.mel.OxGetGuideDependency(hairShape, 0)
		self.assertEqual( len(guideDependency), 6 )

		strandPointCount = pm.mel.OxGetStrandPointCount(hairShape, 0)
		self.assertGreater( strandPointCount, 0 )

		firstVertexIndex = pm.mel.OxGetFirstVertexIndex(hairShape, 0)
		self.assertGreaterEqual( firstVertexIndex, 0 )


		strandPoint = pm.mel.OxGetStrandPoint(hairShape, 0, 0)
		self.assertEqual( len(strandPoint), 3 )

		strandPointInObjectCoordinates = pm.mel.OxGetStrandPointInObjectCoordinates(hairShape, 0, 0)
		self.assertEqual( len(strandPointInObjectCoordinates), 3 )

		boundingBox = pm.mel.OxGetBoundingBox(hairShape)
		self.assertEqual( len(boundingBox), 6 )

		strandPoints = pm.mel.OxGetStrandPoints(hairShape, 0)
		self.assertEqual( len(strandPoints), 3 * strandPointCount )

		vertexCount = pm.mel.OxGetVertexCount(hairShape)
		self.assertGreater( vertexCount, 0 )

		vertexCountAbsolute = pm.mel.OxGetVertexCount(hairShape, absolute=True)
		self.assertGreater( vertexCountAbsolute, 0 )

		vertex = pm.mel.OxGetVertex(hairShape, 0)
		self.assertEqual( len(vertex), 3 )

		vertexFrame = pm.mel.OxGetVertex(hairShape, 0, frame=0)
		self.assertEqual( len(vertex), 3 )

		if mappingChannelCount > 0:
			textureCoordinate = pm.mel.OxGetTextureCoordinate(hairShape, 0, 0, 0, type=2 )
			self.assertEqual( len(textureCoordinate), 3 )

			textureCoordinates = pm.mel.OxGetTextureCoordinates(hairShape, 0, type=2)
			self.assertEqual( len(textureCoordinates), 3 * strandCount )

			# For type=2 needs vertex colors, for type=1 or 2 needs guides
			# pointData = pm.mel.OxGetPointData(hairShape, 0, 0, 0, type=2)
			# self.assertIsNotNone( pointData )

	def test_AddEditGuidesToHairShape( self ):
		plane = pm.polyPlane()
		pm.select( plane )

		hairShape = TestUtilities.AddHairToMesh( plane[0] )
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )

		pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.EditGuidesShapeName )

		# Guides shape should have been added
		self.assertEqual( 1, len( pm.ls( type = TestUtilities.EditGuidesShapeName ) ) )

	def test_AddEditGuidesToGuidesShape( self ):
		plane = pm.polyPlane()
		pm.select( plane )

		guidesShape = TestUtilities.AddGuidesToMesh( plane[0] )

		pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.EditGuidesShapeName )

		# One HairShape and one EditGuidesShape should have been added
		self.assertEqual( 1, len( pm.ls( type = TestUtilities.HairShapeName ) ) )
		self.assertEqual( 1, len( pm.ls( type = TestUtilities.EditGuidesShapeName ) ) )

	def test_AddStrandOperator( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		hairFromGuides = hairShape.inputHair.inputs()[0]
		guidesFromMesh = hairFromGuides.inputStrands.inputs()[0]
		hairTransform = hairShape.firstParent2()
		planeTransform = guidesFromMesh.inputMesh.inputs()[0]
		planeShape = guidesFromMesh.inputMesh.inputs( sh = True )[0]
		pm.parent( hairTransform, planeTransform, relative=True )

		# Adding to a non-hair node does nothing (can't check the warning)
		pm.select( planeShape )
		self.assertEqual( '', pm.mel.OxAddStrandOperator( '', TestUtilities.CurlNodeName ) )
		self.assertEqual( 0, len( pm.ls( type = TestUtilities.CurlNodeName ) ) )

		# Adding to the hair transform works by adding below the hair shape
		pm.select( planeTransform )
		self.assertEqual( '', pm.mel.OxAddStrandOperator( '', TestUtilities.CurlNodeName ) )
		self.assertEqual( 0, len( pm.ls( type = TestUtilities.CurlNodeName ) ) )
		pm.undo()

		# Add below HairShape
		curl = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.CurlNodeName ) )
		self.assertTrue( curl.outputStrands.isConnectedTo( hairShape.inputHair ) )
		self.assertTrue( hairFromGuides.outputHair.isConnectedTo( curl.inputStrands ) )

		# Add above HairFromGuides
		frizz = pm.PyNode( pm.mel.OxAddStrandOperator( hairFromGuides, TestUtilities.FrizzNodeName ) )
		self.assertTrue( frizz.outputStrands.isConnectedTo( curl.inputStrands ) )
		self.assertTrue( hairFromGuides.outputHair.isConnectedTo( frizz.inputStrands ) )

		# Branch at hairFromGuides using a Curl
		curl2 = pm.PyNode( pm.mel.OxAddStrandOperator2( hairFromGuides, TestUtilities.CurlNodeName, 1, 1 ) )
		hairShape2 = curl2.outputStrands.outputs( sh = True )[0]
		self.assertEqual( hairShape2, 'HairShape2' )
		self.assertTrue( hairFromGuides.outputHair.isConnectedTo( frizz.inputStrands ) )
		self.assertTrue( hairFromGuides.outputHair.isConnectedTo( curl2.inputStrands ) )
		self.assertEqual( [hairShape2, hairShape], pm.mel.OxGetStackShapes( hairFromGuides ) )

		# Branch again using EditGuides
		editGuides = pm.PyNode( pm.mel.OxAddStrandOperator2( guidesFromMesh, TestUtilities.EditGuidesShapeName, 1, 1 ) )
		guidesShape = editGuides.outputStrands.outputs( sh = True )[0]
		guidesTransform = guidesShape.firstParent2()
		self.assertEqual( guidesShape, 'HairShape3' )
		self.assertEqual( guidesTransform, 'Hair3' )
		self.assertTrue( guidesFromMesh.outputGuides.isConnectedTo( editGuides.inputStrands ) )
		self.assertTrue( guidesFromMesh.outputGuides.isConnectedTo( hairFromGuides.inputStrands ) )
		self.assertEqual( guidesTransform, editGuides.firstParent2() )
		self.assertEqual( planeTransform, guidesTransform.firstParent2() )
		self.assertEqual( [guidesShape, hairShape2, hairShape], pm.mel.OxGetStackShapes( guidesFromMesh ) )

		# Add right after an EditGuidesShape
		curl4 = pm.PyNode( pm.mel.OxAddStrandOperator( editGuides, TestUtilities.CurlNodeName ) )

		# Add below a branching point (without branching)
		curl5 = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.CurlNodeName ) )

	def test_BranchBelowRenderSettings( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		hairFromGuides = hairShape.inputHair.inputs()[0]

		# Add above HairFromGuides
		rs = pm.PyNode( pm.mel.OxAddStrandOperator( hairFromGuides, TestUtilities.RenderSettingsNodeName ) )
		self.assertTrue( rs.outputRenderSettings.isConnectedTo( hairShape.inputRenderSettings ) )
		self.assertTrue( hairFromGuides.outputStack.isConnectedTo( rs.inputStack ) )

		curl = pm.PyNode( pm.mel.OxAddStrandOperator2( hairFromGuides, TestUtilities.CurlNodeName, 1, 1 ) )
		self.assertEqual( 'HairShape2', pm.mel.OxGetStackShape( curl ) )

	def test_ConvertGuidesToMayaHairSystem( self ):
		# First verify that we can't create nHair from curve-based guides
		#guidesShape = TestUtilities.AddGuidesFromCurves()
		#with self.assertRaisesRegexp( pm.MelError, "Stack base shape must be a mesh to create nHair" ):
		#	pm.mel.OxConvertGuidesToMayaHairSystem( guidesShape )

		# Standard test
		guidesShape = TestUtilities.AddGuidesToNewPlane( guideCount = 2 )
		guidesFromMesh = guidesShape.inputHair.inputs()[0]
		hairFromGuides = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.HairFromGuidesNodeName ) )
		hairShape = hairFromGuides.outputHair.outputs( sh = True )[0]
		pfxHair = pm.PyNode( pm.mel.OxConvertGuidesToMayaHairSystem( hairShape ) )
		self.assertTrue( pfxHair.exists() )

		curve1 = pm.PyNode( "curveShape1" )
		self.assertEqual( 4, pm.mel.OxIsStackShape( curve1 ) )

		pm.select( curve1 )
		pm.mel.OxUpdateHairStackDialog()
		self.assertEqual( [u'curveShape1', u'CurvesFromStrands1', u'--- Branch ---', u'GuidesFromMesh1', u'pPlaneShape1' ], pm.mel.OxGetHairStackDialogNodes() )

		# Test branch deletion
		pm.mel.OxDeleteStrandOperator( hairShape )
		self.assertEqual( 0, len( pm.ls( type = TestUtilities.HairShapeName ) ) )
		self.assertFalse( hairFromGuides.exists() )
		pm.mel.OxUpdateHairStackDialog()
		self.assertEqual( [u'curveShape1', u'CurvesFromStrands1', u'GuidesFromMesh1', u'pPlaneShape1' ], pm.mel.OxGetHairStackDialogNodes() )

	def test_FreezeTransformations( self ):
		plane = pm.polyPlane( sx = 1, sy = 1 )[0]
		editGuidesShape = TestUtilities.AddEditGuidesToMesh( plane )
		hairFromGuides = pm.PyNode( pm.mel.OxAddStrandOperator( editGuidesShape, TestUtilities.HairFromGuidesNodeName ) )
		hairFromGuides.distribution.set( 4 )
		hairShapeTransform = pm.ls( type = "transform" )[1]
		pm.select( hairShapeTransform )
		#planeShape = pm.ls( type = "mesh" )[0]
		#pm.select( planeShape )
		pm.scale( 2, 2, 2 )

		pm.makeIdentity( apply = True, t = 1, r = 1, s = 1, n = 0, pn = 1 )

		pm.mel.OxGetStrandCount( pm.ls( type = TestUtilities.HairShapeName )[0] )

	def test_ShowEndResult( self ):
		editGuides = TestUtilities.AddEditGuidesToNewPlane()
		pm.mel.OxAddStrandOperator( editGuides, TestUtilities.HairFromGuidesNodeName )
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		pm.select( editGuides )
		pm.mel.OxShowEndResult( True )
		self.assertTrue( pm.mel.OxShowEndResult( query = True ) )
		self.assertTrue( hairShape.boundingBox().width() > 0 )

		pm.mel.OxShowEndResult( False )
		self.assertFalse( pm.mel.OxShowEndResult( query = True ) )
		self.assertTrue( hairShape.boundingBox().width() == 0 )

		pm.mel.OxShowEndResult( True )
		self.assertTrue( pm.mel.OxShowEndResult( query = True ) )
		self.assertTrue( hairShape.boundingBox().width() > 0 )
