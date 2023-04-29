import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt

class Test_AdoptExternalGuidesNode( MayaTest.OxTestCase ):

	def test_BasicAdopt( self ):
		plane = pm.polyPlane( sx=1, sy=1, width=10.0, height=10.0 )
		pm.select( plane )
		meshShape = plane[0]

		controlGuidesFromMesh = pm.PyNode( TestUtilities.AddGuidesToMesh( meshShape, guideCount=10, rootGenerationMethod=2, length=7.0, randomSeed=1, pointsPerStrandCount=7 ) )

		adoptedGuidesFromMesh = pm.PyNode( TestUtilities.AddGuidesToMesh( meshShape, guideCount=20, rootGenerationMethod=2, length=7.0, randomSeed=2, pointsPerStrandCount=15 ) )
		adoptedGuidesFromMeshName = str( adoptedGuidesFromMesh )

		adoptExternalGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( adoptedGuidesFromMesh, TestUtilities.AdoptExternalGuidesNodeName ) )
		pm.connectAttr( controlGuidesFromMesh.worldHair, adoptExternalGuidesNode.controlObject )
		pm.connectAttr( controlGuidesFromMesh.worldHair, adoptExternalGuidesNode.referenceObject )
		pm.disconnectAttr( controlGuidesFromMesh.worldHair, adoptExternalGuidesNode.referenceObject )

		controlTipPositionsBefore = TestUtilities.GetTipPositions( controlGuidesFromMesh, useObjectCoordinates=True )
		adoptedTipPositionsBefore = TestUtilities.GetTipPositions( adoptedGuidesFromMesh, useObjectCoordinates=True )

		gravityNode = pm.PyNode( pm.mel.OxAddStrandOperator( controlGuidesFromMesh, TestUtilities.GravityNodeName ) )
		gravityNode.rotate.set( 0.0, 0.0, 1.0 )

		controlTipPositionsAfter = TestUtilities.GetTipPositions( controlGuidesFromMesh, useObjectCoordinates=True )
		adoptedTipPositionsAfter = TestUtilities.GetTipPositions( adoptedGuidesFromMesh, useObjectCoordinates=True )

		controlAverageMove = TestUtilities.GetAveragePointMove( self, controlTipPositionsBefore, controlTipPositionsAfter )
		adoptedAverageMove = TestUtilities.GetAveragePointMove( self, adoptedTipPositionsBefore, adoptedTipPositionsAfter )

		self.assertLess( dt.Vector( controlAverageMove ).distanceTo( adoptedAverageMove ), 0.01 )

		# Save and reload the scene to make sure that the hair is present
		filePath = pm.saveAs( 'temp.mb' )
		pm.newFile( force=True )
		self.assertEqual( 0, len( pm.ls( type=TestUtilities.AdoptExternalGuidesNodeName ) ) )

		pm.openFile( filePath, force=True )
		self.assertEqual( 1, len( pm.ls( type=TestUtilities.AdoptExternalGuidesNodeName ) ) )

		adoptedGuidesFromMesh = pm.PyNode( adoptedGuidesFromMeshName )

		adoptedTipPositionsReload = TestUtilities.GetTipPositions( adoptedGuidesFromMesh, useObjectCoordinates=True )

		# Use CheckPointsMatch because point order might change after scene reload
		TestUtilities.CheckPointsMatch( self, adoptedTipPositionsAfter, adoptedTipPositionsReload )

	def test_AdoptSelfWithSingleGuide( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod = 6 )
		pm.mel.source( "OxAEGProcessControlGuides.mel" )
		adoptExternalGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.AdoptExternalGuidesNodeName ) )
		#pm.connectAttr( guidesShape.worldHair, adoptExternalGuidesNode.controlObject )
		pm.mel.OxAEGProcessControlGuides( adoptExternalGuidesNode + ".controlObject", guidesShape, adoptExternalGuidesNode );

		# Evaluate. This shouldn't cause a crash.
		pm.mel.OxGetStrandCount( guidesShape )
		self.assertEqual( 1, pm.mel.OxProfiler( "-errorCount" ) )
