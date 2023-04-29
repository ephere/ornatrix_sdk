import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt
import os.path
import unittest

def CreateHairAbcArchive( filePath, planeSegmentCount = 2 ):
	guidesShape = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = planeSegmentCount )
	pm.mel.OxAlembicExport( filePath, fromTime = 1, toTime = 1, format = 1 )
	pm.delete( guidesShape )

class Test_BakedHairNode( MayaTest.OxTestCase ):

	def test_ReloadBakedHairWithAbcReference( self ):
		abcFilePath = self.addTempFile( "test_ReloadBakedHairWithAbcReference.abc" )
		CreateHairAbcArchive( abcFilePath )

		hairShape, bakedHairNode = TestUtilities.AddBakedHair()

		bakedHairNode.sourceFilePath.set( abcFilePath )
		bakedHairNode.displayFraction.set( 1 )

		self.assertEquals( 9, pm.mel.OxGetStrandCount( hairShape ) )

	def ExportCurvesUsingMayaAlembic( self, curveCount = 2, curveSize = 5 ):
		if not pm.pluginInfo( 'AbcExport', query=True, loaded=True ):
			pm.loadPlugin( 'AbcExport' )

		# Create two curves and export them as an Alembic archive
		halfSize = curveSize / curveCount
		for i in range( 0, curveCount ):
			currentSize = halfSize * i
			pm.curve( p=[(currentSize, 0, currentSize), (currentSize, 1, currentSize), (currentSize * 2, 2, currentSize * 2), (currentSize * 2 * 2, 3, currentSize * 2 * 2)], k=[0, 0, 0, 1, 1, 1] )

		abcFilePath = self.addTempFile( "ExportedCurves.abc" ).replace( '\\', '/' )
		pm.mel.AbcExport( j = "-frameRange 1 1 -dataFormat ogawa -file \"" + abcFilePath + "\"" )

		return abcFilePath

	@unittest.skipIf( MayaTest.OsIsMac, "AbcExport doesn't exist on Mac Maya install (or isn't loaded)" )
	def test_ImportMayaExportedAlembicWithMultipleCurves( self ):
		abcFilePath = self.ExportCurvesUsingMayaAlembic()

		hairShape, bakedHairNode = TestUtilities.AddBakedHair()

		bakedHairNode.sourceFilePath.set( abcFilePath )
		bakedHairNode.displayFraction.set( 1 )

		# If the file import was successful we should get two hairs
		self.assertEquals( 2, pm.mel.OxGetStrandCount( hairShape ) )

	@unittest.skipIf( MayaTest.OsIsMac, "AbcExport doesn't exist on Mac Maya install (or isn't loaded)" )
	def test_ImportCommand( self ):
		abcFilePath = self.ExportCurvesUsingMayaAlembic( curveCount = 3 )

		#pm.importFile( abcFilePath )
		pm.system.cmds.file( abcFilePath, i = True, type = "Ornatrix Alembic Import", ignoreVersion = True, ra = True, mergeNamespacesOnClash = False, namespace = "test", options = "importAs=0",
			pr = True );

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		bakedHairNode = pm.ls( type = TestUtilities.BakedHairNodeName )[0]
		bakedHairNode.displayFraction.set( 1 )

		# If the file import was successful we should get two hairs
		self.assertEquals( 3, pm.mel.OxGetStrandCount( hairShape ) )

	def test_ImportCurvesWithDifferentPointCounts( self ):
		# Use pre-saved scene (part of repository)
		abcFilePath = os.path.dirname( os.path.realpath( __file__ ) ) + "\\ThreeCurves.abc"

		#pm.importFile( abcFilePath )
		pm.system.cmds.file( abcFilePath, i = True, type = "Ornatrix Alembic Import", ignoreVersion = True, ra = True, mergeNamespacesOnClash = False, namespace = "test", options = "importAs=0",
			pr = True );

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		bakedHairNode = pm.ls( type = TestUtilities.BakedHairNodeName )[0]
		bakedHairNode.displayFraction.set( 1 )

		# If the file import was successful we should get two hairs
		self.assertEquals( 4, pm.mel.OxGetStrandCount( hairShape ) )

		# If everything loaded correctly no curve segment should be longer than 2
		strandSegmentLengths = TestUtilities.GetStrandSegmentLengths( hairShape )
		for i in range( 0, len( strandSegmentLengths ) ):
			self.assertLess( strandSegmentLengths[i], 6 )

	def test_MultipleAbcFiles( self ):
		abcFilePath1 = self.addTempFile( "test_MultipleAbcFiles.abc" )
		abcFilePath2 = self.addTempFile( "test_MultipleAbcFiles2.abc" )
		CreateHairAbcArchive( abcFilePath1, planeSegmentCount = 2 )
		CreateHairAbcArchive( abcFilePath2, planeSegmentCount = 3 )

		hairShape, bakedHairNode = TestUtilities.AddBakedHair()

		bakedHairNode.sourceFilePath.set( abcFilePath1 )
		bakedHairNode.displayFraction.set( 1 )
		self.assertEquals( 9, pm.mel.OxGetStrandCount( hairShape ) )

		# Loading the second archive should add more hairs
		bakedHairNode.sourceFilePath2.set( abcFilePath2 )
		self.assertEquals( 25, pm.mel.OxGetStrandCount( hairShape ) )

	def test_ReloadFromFileButton( self ):
		abcFilePath = self.addTempFile( "test_ReloadFromFileButton.abc" )
		abcFilePath16 = self.addTempFile( "test_ReloadFromFileButton_16.abc" )
		CreateHairAbcArchive( abcFilePath, planeSegmentCount = 2 )
		CreateHairAbcArchive( abcFilePath16, planeSegmentCount = 3 )

		hairShape, bakedHairNode = TestUtilities.AddBakedHair()

		bakedHairNode.sourceFilePath.set( abcFilePath )
		bakedHairNode.displayFraction.set( 1 )
		self.assertEquals( 9, pm.mel.OxGetStrandCount( hairShape ) )

		os.remove( abcFilePath )
		os.rename( abcFilePath16, abcFilePath )
		bakedHairNode.wasFileLoaded.set( False )
		self.assertEquals( 16, pm.mel.OxGetStrandCount( hairShape ) )

	def test_FullPreviewDoesNotChangeHairs( self ):
		# Create hair and deform them a bit
		hairShape = TestUtilities.AddHairToNewPlane( planeSize = 10 )
		frizz = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.FrizzNodeName ) )
		frizz.amount.set( 10 )

		# Bake the hair
		pm.mel.OxCollapseStack( frizz )
		bakedHair = pm.ls( type = TestUtilities.BakedHairNodeName )[0]
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		# Test before detaching
		bakedHair.displayFraction.set( 0.99 )
		verticesAtAlmostFullPreview = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		bakedHair.displayFraction.set( 1 )
		verticesAtFullPreview = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		TestUtilities.CheckPointsAllNearEqual( self, verticesAtAlmostFullPreview, verticesAtFullPreview )

	def TestWidthPersistence( self, mayaFileExtension ):
		# Create and bake hair
		hairShape = TestUtilities.AddHairToNewPlane( planeSize = 10 )
		changeWidthsNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ChangeWidthNodeName ) )

		# Grab widths before reloading
		originalWidths = pm.mel.OxGetWidths( hairShape )

		pm.mel.OxCollapseStack( changeWidthsNode )
		bakedHair = pm.ls( type = TestUtilities.BakedHairNodeName )[0]
		bakedHair.displayFraction.set( 1 )

		# Save and reload
		fileName = 'temp.' + mayaFileExtension
		filePath = pm.saveAs( fileName )
		pm.openFile( filePath, force = True )

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		# Reloaded widths should be identical to original ones
		self.assertSequenceEqual( originalWidths, pm.mel.OxGetWidths( hairShape ) )

	def test_WidthPersistenceAscii( self ):
		self.TestWidthPersistence( 'ma' )

	def test_WidthPersistenceBinary( self ):
		self.TestWidthPersistence( 'mb' )
		
	def test_HFG_DisplacementMap( self ):
		hairShape = TestUtilities.AddHairToNewPlane( planeSize = 10 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		noiseTexture = pm.createNode( "noise" )
		pm.setAttr( hairFromGuides + ".displacementOffset", 1.0 )
		pm.connectAttr( noiseTexture + ".outColor", hairFromGuides + ".displacementMultiplier" )

		# Bake the hair
		pm.mel.OxCollapseStack( hairFromGuides )
		bakedHair = pm.ls( type = TestUtilities.BakedHairNodeName )[0]
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		# Test before detaching
		bakedHair.displayFraction.set( 0.99 )
		verticesAtAlmostFullPreview = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		bakedHair.displayFraction.set( 1 )
		verticesAtFullPreview = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		TestUtilities.CheckPointsAllNearEqual( self, verticesAtAlmostFullPreview, verticesAtFullPreview )

	def test_UpdateOnAnimatedMeshVertices( self ):
		# Create and bake the hair
		hairShape = TestUtilities.AddHairToNewPlane( planeSize = 10 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		pm.mel.OxCollapseStack( hairFromGuides )
		bakedHair = pm.ls( type = TestUtilities.BakedHairNodeName )[0]
		bakedHair.displayFraction.set( 1 )
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		planeShape = pm.ls( type = "mesh" )[0]

		# Attach roots to surface
		bakedHair.detachRoots.set( 0 )

		# Animate all plane vertices
		pm.select( planeShape + ".vtx[0:3]" )
		pm.currentTime( 0 )
		pm.setKeyframe()
		pm.currentTime( 1 )
		pm.move( -1, 0, 0, r = True )
		pm.setKeyframe()

		pm.currentTime( 0 )
		hairVerticesAtTime0 = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		pm.currentTime( 1 )
		hairVerticesAtTime1 = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Animated mesh should cause hair to also animate
		TestUtilities.CheckPointsAllNotNearEqual( self, hairVerticesAtTime0, hairVerticesAtTime1 )

	# Tests for a bug where rotations are not changed for hair on an animated mesh when not 100% of baked + grounded hairs are previewed
	def test_RotationsAreSetWhenUsingViewportPreview( self ):
		# Create and bake the hair
		hairShape = TestUtilities.AddHairToNewPlane( planeSize = 10 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		pm.mel.OxCollapseStack( hairFromGuides )
		bakedHair = pm.ls( type = TestUtilities.BakedHairNodeName )[0]

		hairVerticesAt100Percent = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		bakedHair.displayFraction.set( 0.5 )
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		planeShape = pm.ls( type = "mesh" )[0]
		
		# Attach roots to surface
		bakedHair.detachRoots.set( 0 )

		# Animate all plane vertices
		pm.select( planeShape + ".vtx[0:1]" )
		pm.currentTime( 1 )
		pm.setKeyframe()
		pm.currentTime( 2 )
		pm.move( 0, 5, 0, r = True )
		pm.setKeyframe()

		pm.currentTime( 1 )
		hairVerticesAtTime0 = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		pm.currentTime( 2 )
		hairVerticesAtTime1 = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Make sure that the object space tangent of first strand didn't change at frame 1
		TestUtilities.CheckPointsNearEqual( self, dt.Vector( hairVerticesAt100Percent[1] ) - hairVerticesAt100Percent[0], dt.Vector( hairVerticesAtTime0[1] ) - hairVerticesAtTime0[0] )

		# Check that the object space tangent of the first strand changes at frame 2 due to surface rotation
		TestUtilities.CheckPointsNotNearEqual( self, dt.Vector( hairVerticesAtTime1[1] ) - hairVerticesAtTime1[0], dt.Vector( hairVerticesAtTime0[1] ) - hairVerticesAtTime0[0] )

	def test_HairIsntChangedAfterRotatingTransformAndRendering( self ):
		hairShape = TestUtilities.AddHairToNewPlane( planeSize = 10, rootGenerationMethod = 6, reparent = True )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		pm.mel.OxCollapseStack( hairFromGuides )
		bakedHair = pm.ls( type = TestUtilities.BakedHairNodeName )[0]
		bakedHair.displayFraction.set( 1.0 )
		bakedHair.detachRoots.set( 0 )

		verticesBeforeRotateAndRender = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		pm.select( pm.ls( type = "mesh" )[0] )
		pm.rotate( -35, 0, 0, r = True )
		bakedHair.displayFraction.set( 0.99999 )
		verticesAfterRotateAndRender = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeRotateAndRender, verticesAfterRotateAndRender )

	def test_EditGuidesAfterEmptyBakedHair( self ):
		hairShape = TestUtilities.AddHairToNewPlane( strandCount=0, guideCount=0, rootGenerationMethod = 6, guideRootGenerationMethod=6 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.renderCount.set( 0 )
		pm.mel.OxCollapseStack( hairFromGuides )

		bakedHair = pm.ls( type = TestUtilities.BakedHairNodeName )[0]
		bakedHair.detachRoots.set( 0 )

		editGuidesShape = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.EditGuidesShapeName ) )
		TestUtilities.CreateStrand( 0, [0.5, 0.5], 1, 2 )

		self.assertEqual( 1, pm.mel.OxGetStrandCount( editGuidesShape ) )

	def test_HairIsntChangedAfterRendering( self ):
		hairShape = TestUtilities.AddHairToNewPlane( planeSize = 10, rootGenerationMethod = 6, reparent = True )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		pm.mel.OxCollapseStack( hairFromGuides )
		bakedHair = pm.ls( type = TestUtilities.BakedHairNodeName )[0]
		bakedHair.displayFraction.set( 1.0 )
		bakedHair.detachRoots.set( 0 )

		verticesBeforeRender = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		cam = pm.camera()
		pm.render( cam[0] )
		verticesAfterRender = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		pm.render( cam[0] )

		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeRender, verticesAfterRender )

	def test_HairChangesAfterMeshIsModified( self ):
		hairShape, _, meshName = TestUtilities.AddBakedHairToSegmentedPlane( planeSegmentCount = 5, planeSize = 10, hairCount = 50 )

		verticesBeforeModifyingMesh = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		surfacePositionsBeforeModifyingMesh = pm.mel.OxGetSurfaceDependencies( hairShape )
		pm.softSelect( sse = 1 )
		pm.select( meshName + '.f[5]', replace = True )
		pm.move( [0, 2, 0] )
		verticesAfterModifyingMesh = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		surfacePositionsAfterModifyingMesh = pm.mel.OxGetSurfaceDependencies( hairShape )

		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBeforeModifyingMesh, verticesAfterModifyingMesh )

		# Surface positions should've remained the same during mesh deformation. If they changed it means hair was re-grounded, which should only happen once.
		TestUtilities.CheckPointsAllNearEqual( self, surfacePositionsBeforeModifyingMesh, surfacePositionsAfterModifyingMesh )
