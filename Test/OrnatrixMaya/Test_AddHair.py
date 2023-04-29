import MayaTest
import TestUtilities
import pymel.core as pm
import os
import sys

class Test_AddHair( MayaTest.OxTestCase ):

	def test_CorrectDeployment( self ):
		libPrefix = '' if sys.platform == 'win32' else 'lib'
		osDllExtension = '.dll' if sys.platform == 'win32' else '.dylib' if sys.platform == 'darwin' else '.so'
		moduleRoot = pm.moduleInfo( mn = 'Ornatrix', p = True )
		self.assertTrue( os.path.exists( os.path.join( moduleRoot, 'bin', 'Ephere.Licensing.Authorizer.Autodesk.Maya.Ornatrix.exe' ) ) )
		self.assertTrue( os.path.exists( os.path.join( moduleRoot, 'bin', libPrefix + 'Ephere.Moov.Solver' + osDllExtension ) ) )
		if sys.platform != 'darwin':
			self.assertTrue( os.path.exists( os.path.join( moduleRoot, 'bin', libPrefix + 'OpenCL' + osDllExtension ) ) )
		self.assertGreaterEqual( len( os.listdir( os.path.join( moduleRoot, 'icons' ) ) ), 150 )
		self.assertGreaterEqual( len( os.listdir( os.path.join( moduleRoot, 'scripts', 'Autoload' ) ) ), 30 )
		extensionSubDirs = os.listdir( os.path.join( moduleRoot, 'plug-ins', 'extensions' ) )
		self.assertGreaterEqual( len( extensionSubDirs ), 2, extensionSubDirs )
		self.assertGreaterEqual( len( os.listdir( os.path.join( moduleRoot, 'plug-ins', 'PRMan' ) ) ), 2 )
		self.assertGreaterEqual( len( os.listdir( os.path.join( moduleRoot, 'scripts', 'Autoload', 'AETemplates' ) ) ), 40 )
		self.assertGreaterEqual( len( os.listdir( os.path.join( moduleRoot, 'scripts', 'Autoload', 'Others' ) ) ), 30 )
		self.assertGreaterEqual( len( os.listdir( os.path.join( moduleRoot, 'scripts', 'Moov' ) ) ), 3 )

	# Tests 'quick hair' command for when a mesh is selected
	def test_AddHairToPlane( self ):
		plane = pm.polyPlane()
		pm.select( plane )

		previousHairShapeCount = len( pm.ls( type = TestUtilities.HairShapeName ) )
		pm.mel.OxQuickHair()
		self.assertEqual( previousHairShapeCount + 1, len( pm.ls( type = TestUtilities.HairShapeName ) ) )

		# Make sure that top-most hair shape's distribution mesh input is connected
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		self.assertEqual( 1, len( pm.listConnections( hairShape + '.distributionMesh', d=False, s=True ) ) )
		self.assertEqual( True, pm.PyNode( hairShape ).visibleInReflections.get() )
		self.assertEqual( True, pm.PyNode( hairShape ).visibleInRefractions.get() )

		# Save and reload the scene to make sure that the hair is present
		filePath = pm.saveAs( 'temp.mb' )
		pm.newFile( force = True )
		self.assertEqual( 0, len( pm.ls( type = TestUtilities.HairShapeName ) ) )

		pm.openFile( filePath, force = True )
		self.assertEqual( 1, len( pm.ls( type = TestUtilities.HairShapeName ) ) )

		# Make sure hair shape and edit guide shape have same parent
		editGuidesShape = pm.ls( type = TestUtilities.EditGuidesShapeName )[0]
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		self.assertEqual( pm.listRelatives( hairShape, parent = True ), pm.listRelatives( editGuidesShape, parent = True ) )

	def test_AddHairToCurves( self ):
		curve1 = pm.curve( p=[(1, 0, 1), (1, 1, 1.1), (1, 2, 1.2), (1, 3, 1.3)], k=[0, 0, 0, 1, 1, 1] )
		curve2 = pm.curve( p=[(-1, 0, 1), (-1, 1, 1.1), (-1, 2, 1.2), (-1, 3, 1.3)], k=[0, 0, 0, 1, 1, 1] )
		pm.select( curve1, add = True )
		pm.select( curve2, add = True )

		# Guides shape should be created from curves
		guidesShape = pm.PyNode( pm.mel.OxLoadGroom( path = "Hair From Curves" ) )
		self.assertEqual( guidesShape.type(), TestUtilities.HairShapeName )
		self.assertEqual( 'HairFromCurves1', guidesShape.firstParent2().name() )
		self.assertEqual( 'HairFromCurvesShape1', guidesShape.name() )
		self.assertNotEqual( len( guidesShape.instObjGroups[0].outputs() ), 0 )

		# The guides shape created from curves should not be parented under the curves, it needs to be later parented under the distribution mesh
		self.assertIsNone( guidesShape.firstParent2().firstParent2() )

		# OxSetDistributionMesh re-parents the shape under the distribution mesh
		plane = pm.polyPlane()
		pm.mel.source( "OxSetDistributionMeshTool.mel" )
		pm.mel.OxSetDistributionMesh( guidesShape, plane[0], True )
		self.assertEqual( guidesShape.firstParent2().firstParent2(), plane[0] )

	def test_AddHairInPaintSelectionMode( self ):
		plane = pm.polyPlane( sx = 1, sy = 1 )
		pm.select( plane[0] + '.vtx[0]' )
		pm.setToolTo( 'artSelectContext' )
		hairShape = pm.PyNode( pm.mel.OxLoadGroom( path = "Fur Ball" ) )
		self.assertEqual( 'FurBall1', hairShape.firstParent2().name() )
		self.assertEqual( 'FurBallShape1', hairShape.name() )
		self.assertEqual( 1, len( pm.ls( type = TestUtilities.HairFromGuidesNodeName ) ) )

	def test_AddHairToVertexSelection( self ):
		plane = pm.polyPlane( sx = 2, sy = 2 )
		pm.select( plane[0] + '.vtx[1:5]' )
		hairShape = pm.PyNode( pm.mel.OxLoadGroom( path = "Fur Ball" ) )
		guidesFromMesh = pm.PyNode( pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0] )
		self.assertEqual( [1], guidesFromMesh.inputFaceIndices.get() )

	def test_NodeNames( self ):
		plane = pm.polyPlane( sx = 2, sy = 2 )
		hairShape = pm.PyNode( pm.mel.OxLoadGroom( path = "Fur Ball" ) )
		self.assertEqual( 'GuidesFromMesh1', TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName ).name() )
		self.assertEqual( 'EditGuides1', TestUtilities.GetNodeByType( TestUtilities.EditGuidesShapeName ).name() )
		self.assertEqual( 'HairFromGuides1', TestUtilities.GetNodeByType( TestUtilities.HairFromGuidesNodeName ).name() )
		self.assertEqual( 'ChangeWidth1', TestUtilities.GetNodeByType( TestUtilities.ChangeWidthNodeName ).name() )
