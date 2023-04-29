import MayaTest
import TestUtilities
import pymel.core as pm
import os.path
import unittest

import sys
if sys.version_info > (3,):
    long = int

class Test_AlembicExport( MayaTest.OxTestCase ):

	def setUp( self ):
		MayaTest.OxTestCase.setUp( self )

		if not pm.pluginInfo( 'AbcImport.mll', query = True, loaded = True ):
			pm.loadPlugin( 'AbcImport.mll' )

	def test_ExportSingleStaticHair( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 2 )
		filePath = self.addTempFile( "SingleStaticHair.abc" )
		# No object specified, exports all HairShape's in the scene
		pm.mel.OxAlembicExport( filePath, fromTime = 1, toTime = 1 )

		self.assertAlembicFileSize( filePath, 2105 )
		strandCount = pm.mel.OxGetStrandCount( guidesShape )

		pm.mel.OxDeleteStrandOperator( guidesShape )
		pm.importFile( filePath, type = 'Alembic' )
		self.assertEqual( len( pm.ls( type = 'nurbsCurve' ) ), strandCount )

	def test_ImportSingleStaticHair( self ):
		guidesShape = TestUtilities.AddHairToNewPlane( planeSegmentCount = 2 )
		filePath = self.addTempFile( "ImportSingleStaticHair.abc" )
		pm.mel.OxAlembicExport( filePath, fromTime = 1, toTime = 1 )
		strandCount = pm.mel.OxGetStrandCount( guidesShape )

		pm.mel.OxDeleteStrandOperator( guidesShape )

		pm.mel.OxAlembicImport( filePath )
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		bakedHairNode = pm.ls( type = TestUtilities.BakedHairNodeName )[0]
		bakedHairNode.displayFraction.set( 1 )

		self.assertEqual( strandCount, pm.mel.OxGetStrandCount( hairShape ) )

	@unittest.skipIf( MayaTest.ApiVersion < 201500, "The .abc file won't load on Maya 2014" )
	def test_ExportSingleAnimatedHair( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 2 )
		guidesShapeType = guidesShape.type()
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		pm.animation.setKeyframe( guidesFromMesh, attribute = "length", value = 10 )
		pm.animation.setKeyframe( guidesFromMesh, attribute = "length", value = 20, time = 10 )

		# Get the hair vertices at frame 1 and 10
		pm.currentTime( 1 )
		verticesAtFrame1 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		pm.currentTime( 10 )
		verticesAtFrame10 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		filePath = self.addTempFile( "SingleAnimatedHair.abc" )

		# Export specific object
		pm.mel.OxAlembicExport( filePath, guidesShape, fromTime = 1, toTime = 10 )

		# Check file size, for a precise value
		self.assertAlembicFileSize( filePath, 5124 )

		# Import the exported hair and check its vertices
		pm.newFile( force = True )
		pm.importFile( filePath, type = 'Alembic' )

		pm.currentTime( 1 )
		curveVertexPositions = []
		for strandIndex in range( 0, 9 ):
			for pointIndex in range( 0, 2 ):
				curveVertexPositions.append( pm.pointPosition( guidesShape + '|' +  guidesShapeType + str( strandIndex + 1 ) + '.cv[' + str( pointIndex ) + ']' ) )

		TestUtilities.CheckPointsAllNearEqual( self, verticesAtFrame1, curveVertexPositions )

		pm.currentTime( 10 )
		curveVertexPositions = []
		for strandIndex in range( 0, 9 ):
			for pointIndex in range( 0, 2 ):
				curveVertexPositions.append( pm.pointPosition( guidesShape + '|' +  guidesShapeType + str( strandIndex + 1 ) + '.cv[' + str( pointIndex ) + ']' ) )

		TestUtilities.CheckPointsAllNearEqual( self, verticesAtFrame10, curveVertexPositions, epsilon = 0.5	)

	def test_ExportMultipleStaticHair( self ):
		guidesShape1 = TestUtilities.AddGuidesToNewPlane()
		guidesShape2 = TestUtilities.AddGuidesToNewPlane()
		filePath = self.addTempFile( "MultipleStaticHair.abc" )
		# No object specified, exports all HairShape/GuidesShape's in the scene
		pm.mel.OxAlembicExport( filePath, fromTime = 1, toTime = 1 )

		self.assertAlembicFileSize( filePath, 2448 )

	def test_ExportRenderHairs( self ):
		plane = pm.polyPlane()
		hairShape = pm.PyNode( pm.mel.OxQuickHair() )
		hairFromGuides = TestUtilities.GetNodeByType( TestUtilities.HairFromGuidesNodeName )
		hairFromGuides.renderCount.set( 200 )

		# Ensure correct hair count
		hairFromGuides.viewportCountFraction.set( 1 )
		hairFromGuides.guideArea.set( 1 )
		self.assertEqual( 200, pm.mel.OxGetStrandCount( hairShape ) )

		hairFromGuides.viewportCountFraction.set( 0.1 )
		filePathNonRender = self.addTempFile( "NonRender.abc" )
		filePathRender = self.addTempFile( "Render.abc" )
		pm.mel.OxAlembicExport( filePathNonRender, fromTime = 1, toTime = 1 )
		pm.mel.OxAlembicExport( filePathRender, fromTime = 1, toTime = 1, renderVersion = True )

		nonRenderFileSize = os.path.getsize( filePathNonRender )
		# CentOS produces different hair count than Windows and Mac due to random algorithm so make sure the result is either one of the provided values
		# self.assertEqual( nonRenderFileSize, 11317 if MayaTest.OsIsLinux else ( 10585 if MayaTest.OsIsMac else 11073 ) )
		self.assertGreater( nonRenderFileSize, 5456 )
		self.assertLess( nonRenderFileSize, 13500 )
		self.assertAlembicFileSize( filePathRender, 37858 )

	def test_ExportVelocities( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 2 )
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		pm.animation.setKeyframe( guidesFromMesh, attribute = "length", value = 10, time = 0 )
		pm.animation.setKeyframe( guidesFromMesh, attribute = "length", value = 20, time = 3 )

		filePathWithoutVelocities = self.addTempFile( "test_ExportVelocitiesWithoutVelocities.abc" )
		filePathWithVelocities = self.addTempFile( "test_ExportVelocitiesWithVelocities.abc" )

		# Export a single frame without and with velocities
		pm.mel.OxAlembicExport( filePathWithoutVelocities, guidesShape, fromTime = 1, toTime = 1 )
		pm.mel.OxAlembicExport( filePathWithVelocities, guidesShape, fromTime = 1, toTime = 1, exportVelocities = True )

		# Check file sizes
		fileSizeWithoutVelocities = os.path.getsize( filePathWithoutVelocities )
		fileSizeWithVelocities = os.path.getsize( filePathWithVelocities )

		# File with velocities should be larger than without them
		self.assertGreater( fileSizeWithVelocities, fileSizeWithoutVelocities )

	def test_UpAxis( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 2 )
		filePathYAxis = self.addTempFile( "testYAxisUp.abc" )
		filePathZAxis = self.addTempFile( "testZAxisUp.abc" )

		# Export hair with Y and Z axis up into separate files
		pm.mel.OxAlembicExport( filePathYAxis, guidesShape, up = 1 )
		pm.mel.OxAlembicExport( filePathZAxis, guidesShape, up = 2 )

		# Import the hairs back into different shapes
		hairShapeYAxis, bakedHairNodeYAxis = TestUtilities.AddBakedHair()
		hairShapeZAxis, bakedHairNodeZAxis = TestUtilities.AddBakedHair()

		bakedHairNodeYAxis.sourceFilePath.set( filePathYAxis )
		bakedHairNodeYAxis.displayFraction.set( 1 )
		bakedHairNodeZAxis.sourceFilePath.set( filePathZAxis )
		bakedHairNodeZAxis.displayFraction.set( 1 )

		# The world coordinates of vertices should be different for the two loaded hair objects
		worldVerticesYAxis = pm.OxGetVertices( hairShapeYAxis, worldSpace = True )
		worldVerticesZAxis = pm.OxGetVertices( hairShapeZAxis, worldSpace = True )

		# If exported properly then the Y and Z axis vertices should not be identical
		TestUtilities.CheckPointsNotAllNearEqual( self, worldVerticesYAxis, worldVerticesZAxis )

	# Move these to another file if needed

	@unittest.skipIf( not MayaTest.OsIsWindows, "There are floating point differences on other platforms" )
	def test_ExportSingleStaticHairToUsd( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 2 )
		filePathText = self.addTempFile( "SingleStaticHair.usda" )
		pm.mel.OxUsdExport( filePathText, guidesShape, exportWidths = True, exportNormals = True, exportStrandData = True, exportTextureCoordinates = True )

		self.assertFileSize( filePathText, 1299 )

	@unittest.skipIf( not MayaTest.OsIsWindows, "There are floating point differences on other platforms" )
	def test_ExportSingleAnimatedHairToUsd( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 2 )
		guidesShapeType = guidesShape.type()
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		pm.animation.setKeyframe( guidesFromMesh, attribute = "length", value = 10 )
		pm.animation.setKeyframe( guidesFromMesh, attribute = "length", value = 20, time = 10 )
		filePathText = self.addTempFile( "SingleAnimatedHair.usda" )
		pm.mel.OxUsdExport( filePathText, guidesShape, fromTime = 1, toTime = 10, exportWidths = True, exportNormals = True, exportStrandData = True, exportTextureCoordinates = True )

		self.assertFileSize( filePathText, 5690 )
