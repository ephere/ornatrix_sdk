import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt
import os.path
import unittest

def CreateGuidesAbcArchive( filePath, planeSegmentCount = 2, planeSize = 1 ):
	guidesShape = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = planeSegmentCount, planeSize = planeSize )
	pm.mel.OxAlembicExport( filePath, fromTime = 1, toTime = 1, format = 1 )
	pm.delete( guidesShape )

class Test_BakedGuidesNode( MayaTest.OxTestCase ):

	def TestGuideGroupsArePersistedWithScene( self, fileExtension ):
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane( pointsPerStrandCount = 2 )

		# Put one strand into group 1 while leaving others in group 0
		editGuidesShape.useStrandGroups.set( True )
		pm.select( editGuidesShape + ".ep[3]" )
		pm.mel.OxAssignStrandGroup( editGuidesShape, "1" )

		bakedGuides = pm.PyNode( pm.mel.OxCollapseStack( editGuidesShape ) )

		# Save and reload binary
		fileName = 'temp.' + fileExtension
		filePath = pm.saveAs( fileName )
		pm.openFile( filePath, force = True )

		bakedGuides = pm.ls( type = TestUtilities.HairShapeName )[0]

		# Make sure strand groups are preserved after reloading
		strandGroups = pm.mel.OxGetStrandGroups( bakedGuides )

		self.assertEqual( [0, 0, 0, 1], strandGroups )

	def test_GuideGroupsArePersistedWithSceneBinary( self ):
		self.TestGuideGroupsArePersistedWithScene( 'mb' )

	def test_GuideGroupsArePersistedWithSceneAscii( self ):
		self.TestGuideGroupsArePersistedWithScene( 'ma' )

	def test_DistributionMeshNotLostOnReload( self ):
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane( pointsPerStrandCount = 2 )

		pm.mel.OxCollapseStack( editGuidesShape )

		fileName = self.addTempFile( "test_DistributionMeshNotLostOnReload.ma" )
		filePath = pm.saveAs( fileName )

		# First make sure that an empty HairShape has empty distribution mesh
		emptyHair = pm.createNode( 'HairShape' )
		self.assertEqual( pm.mel.OxProfiler( distributionMeshHash = emptyHair ), '' )

		pm.openFile( filePath, force = True )
		self.assertNotEqual( pm.mel.OxProfiler( distributionMeshHash = 'HairShape1' ), '' )

	def test_AssignGuideGroupsAfterSceneReload( self ):
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane( pointsPerStrandCount = 2 )

		bakedGuides = pm.PyNode( pm.mel.OxCollapseStack( editGuidesShape ) )

		# Save and reload
		fileName = 'temp.ma'
		filePath = pm.saveAs( fileName )
		pm.openFile( filePath, force = True )

		bakedGuides = pm.ls( type = TestUtilities.HairShapeName )[0]

		editGuidesShape = pm.PyNode( pm.mel.OxAddStrandOperator( bakedGuides, TestUtilities.EditGuidesShapeName ) )

		# Put one strand into group 1 while leaving others in group 0
		editGuidesShape.useStrandGroups.set( True )
		pm.select( editGuidesShape + ".ep[3]" )
		pm.mel.OxAssignStrandGroup( editGuidesShape, "1" )

		# Make sure strand groups are preserved after reloading
		strandGroups = pm.mel.OxGetStrandGroups( editGuidesShape )

		self.assertEqual( [0, 0, 0, 1], strandGroups )

	def test_GuidesDataOutput( self ):
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane( pointsPerStrandCount = 2 )
		bakedGuides = pm.PyNode( pm.mel.OxCollapseStack( editGuidesShape ) )
		dataType = pm.getAttr( bakedGuides + '.' + pm.mel.OxGetOutputAttribute( bakedGuides ), type = True )
		
		self.assertEqual( "GuidesData", dataType )

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

	def test_ReloadBakedGuidesWithAbcReference( self ):
		abcFilePath = self.addTempFile( "test_ReloadBakedGuidesWithAbcReference.abc" )
		CreateGuidesAbcArchive( abcFilePath )

		guidesShape, bakedGuidesNode = TestUtilities.AddBakedGuides()
		bakedGuidesNode.sourceFilePath.set( abcFilePath )

		self.assertEquals( 9, pm.mel.OxGetStrandCount( guidesShape ) )

	@unittest.skipIf( MayaTest.OsIsMac, "AbcExport doesn't exist on Mac Maya install (or isn't loaded)" )
	def test_ImportMayaExportedAlembicWithMultipleCurves( self ):
		abcFilePath = self.ExportCurvesUsingMayaAlembic()

		guidesShape, bakedGuidesNode = TestUtilities.AddBakedGuides()
		bakedGuidesNode.sourceFilePath.set( abcFilePath )

		# If the file import was successful we should get two hairs
		self.assertEquals( 2, pm.mel.OxGetStrandCount( guidesShape ) )

	@unittest.skipIf( MayaTest.OsIsMac, "AbcExport doesn't exist on Mac Maya install (or isn't loaded)" )
	def test_ImportCommand( self ):
		abcFilePath = self.ExportCurvesUsingMayaAlembic( curveCount = 3 )

		pm.system.cmds.file( abcFilePath, i = True, type = "Ornatrix Alembic Import", ignoreVersion = True, ra = True, mergeNamespacesOnClash = False, namespace = "test", options = "importAs=1", pr = True );

		guidesShape = pm.ls( type = TestUtilities.GuidesShapeName )[0]

		# If the file import was successful we should get two hairs
		self.assertEquals( 3, pm.mel.OxGetStrandCount( guidesShape ) )
