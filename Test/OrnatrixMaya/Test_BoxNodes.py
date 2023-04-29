import MayaTest
import TestUtilities
import pymel.core as pm
import unittest
import os

class Test_BoxNodes( MayaTest.OxTestCase ):
	def test_BoxNodes( self ):
		[hairShape, curlNode, frizzNode, lengthNode] = TestUtilities.CreateBoxedPlaneWithOperators()
		originalVertices = pm.mel.OxGetVertices( hairShape, os = True )

		pm.mel.OxBoxNodes( [lengthNode, frizzNode, curlNode] )

		self.assertEqual( 1, len( pm.mel.ls( type=TestUtilities.HairShapeName ) ) )
		verticesAfterBoxing = pm.mel.OxGetVertices( pm.mel.ls(type=TestUtilities.HairShapeName)[0], os = True )

		TestUtilities.CheckPointsAllNearEqual( self, originalVertices, verticesAfterBoxing )

	def BoxNodesSaveScene( self, binary ):
		[hairShape, curlNode, frizzNode, lengthNode] = TestUtilities.CreateBoxedPlaneWithOperators()
		pm.mel.OxBoxNodes( [lengthNode, frizzNode, curlNode] )

		verticesAfterBoxing = pm.mel.OxGetVertices( pm.mel.ls( type = TestUtilities.HairShapeName )[0], os = True )

		TestUtilities.SaveAndReloadScene( self, 'test_BoxNodesSaveScene', binary )

		verticesAfterLoading = pm.mel.OxGetVertices( pm.mel.ls( type = TestUtilities.HairShapeName )[0], os = True )

		TestUtilities.CheckPointsAllNearEqual( self, verticesAfterBoxing, verticesAfterLoading )

	def test_BoxNodesSaveSceneBinary( self ):
		self.BoxNodesSaveScene( True )

	def test_BoxNodesSaveSceneAscii( self ):
		self.BoxNodesSaveScene( False )

	def test_BoxNodesWithHFG( self ):
		[verticesBeforeBoxing, hairShape] = TestUtilities.BoxNodesWithHFG( self )

		verticesAfterBoxing = pm.mel.OxGetVertices( hairShape, os = True )

		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeBoxing, verticesAfterBoxing )

	def test_BoxNodesWithClump( self ):
		hairShape = TestUtilities.AddHairToNewPlane( 10, 7, planeSize = 10, length = 10 )
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )
		TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		pm.mel.OxEditClumps( clumpNode, d = True, sl = 0 )
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 3, 1 ) )

		changeWidthNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ChangeWidthNodeName ) )

		verticesBeforeBoxing = pm.mel.OxGetVertices( hairShape, os = True )
		pm.mel.OxBoxNodes( [changeWidthNode, clumpNode] )

		verticesAfterBoxing = pm.mel.OxGetVertices( hairShape, os = True )
		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeBoxing, verticesAfterBoxing )

		pm.mel.OxUnboxNodes( [pm.mel.ls(type=TestUtilities.GroomNodeName )[0]] )
		verticesAfterUnboxing = pm.mel.OxGetVertices( hairShape, os = True )

		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeBoxing, verticesAfterUnboxing )

	def test_BoxNodesWithSphereBaseSurfaceAndEG( self ):
		guidesShape = TestUtilities.AddGuidesToNewSphere( guideLength = 1, rootGenerationMethod=7, guideCount=10 )
		editGuides = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.EditGuidesShapeName ) )

		# Move the strand's tip. This will record the movement in surface tracking mode, relative to that face
		pm.select( editGuides + ".ep[8]" )
		pm.move( 0, 0, 20, relative = True )
		pm.mel.OxEditGuides( cc = True )
		
		verticesBeforeBoxing = TestUtilities.GetVerticesInObjectCoordinatesSortedByStrandId( guidesShape )
		pm.mel.OxBoxNodes( [editGuides, pm.mel.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]] )
		verticesAfterUnboxing = TestUtilities.GetVerticesInObjectCoordinatesSortedByStrandId( guidesShape )
		
		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeBoxing, verticesAfterUnboxing )

	def test_DisableBoxedGFMNode( self ):
		guidesShape = TestUtilities.AddGuidesToNewSphere( guideLength = 1, rootGenerationMethod = 7, guideCount = 10 )
		pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.FrizzNodeName )

		self.assertEqual( 2, len( pm.mel.OxGetStackNodes( guidesShape ) ) )

		# Box GFM node
		pm.mel.OxBoxNodes( [pm.mel.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]] )
		self.assertEqual( 2, len( pm.mel.OxGetStackNodes( guidesShape ) ) )
		self.assertEqual( 1, len( pm.mel.ls( type = TestUtilities.HairShapeName ) ) )
		
		pm.mel.OxEnableOperator( pm.mel.ls( type = TestUtilities.GroomNodeName )[0], 0 )

		# We should still have only one hair shape and it should still have both operators
		self.assertEqual( 2, len( pm.mel.OxGetStackNodes( guidesShape ) ) )
		self.assertEqual( 1, len( pm.mel.ls( type = TestUtilities.HairShapeName ) ) )

	def test_BoxNodeAboveBaseBoxedNode( self ):
		guidesShape = TestUtilities.AddGuidesToNewSphere( guideLength = 1, rootGenerationMethod = 7, guideCount = 10 )
		frizzNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.FrizzNodeName ) )

		# Box GFM node
		pm.mel.OxBoxNodes( [pm.mel.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]] )

		# Box frizz node
		pm.mel.OxBoxNodes( [frizzNode] )

		# If no errors occured we are good
		self.assertEqual( 2, len( pm.mel.ls( type = TestUtilities.GroomNodeName ) ) )

	def test_BoxHFGNodeShouldProduceHair( self ):
		guidesShape = TestUtilities.AddGuidesToNewSphere( guideLength = 1, rootGenerationMethod = 7, guideCount = 10 )
		self.assertFalse( pm.mel.OxIsHair( guidesShape ) )

		hfgNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.HairFromGuidesNodeName ) )
		self.assertTrue( pm.mel.OxIsHair( guidesShape ) )
		
		# Box hair from guides node
		pm.mel.OxBoxNodes( [hfgNode] )

		# The output of guidesShape should still be hair and not guides
		self.assertTrue( pm.mel.OxIsHair( guidesShape ) )

	def test_AddEGAboveBoxedGFM( self ):
		guidesShape = TestUtilities.AddGuidesToNewSphere( guideLength = 1, rootGenerationMethod = 7, guideCount = 10 )

		# Add one more operator to the stack
		pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.FrizzNodeName ) )

		# Box GFM node
		pm.mel.OxBoxNodes( [pm.mel.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]] )

		# Add EG shape on top
		pm.PyNode( pm.mel.OxAddStrandOperator( pm.mel.ls( type = TestUtilities.GroomNodeName )[0], TestUtilities.EditGuidesShapeName ) )

		# There should be no warnings and we should have the EG shape
		self.assertEqual( 0, pm.mel.OxProfiler( "-warningCount" ) )
		self.assertEqual( 1, len( pm.mel.ls( type = TestUtilities.EditGuidesShapeName ) ) )

	def test_StoredAndPrecomputed( self ):
		[hairShape, curlNode, frizzNode, lengthNode] = TestUtilities.CreateBoxedPlaneWithOperators()
		originalVertices = pm.mel.OxGetVertices( hairShape, os = True )

		pm.mel.OxBoxNodes( [lengthNode, frizzNode, curlNode] )
		selectedNodes = pm.mel.ls( type=TestUtilities.HairShapeName )

		self.assertEqual( 1, len( selectedNodes ) )

		hair = selectedNodes[0]
		boxNode = pm.mel.ls( type=TestUtilities.GroomNodeName )[0]

		pm.setAttr( boxNode + ".ccb", 1 )
		verticesAfterBoxingStored = pm.mel.OxGetVertices( hair, os = True )
		TestUtilities.CheckPointsAllNearEqual( self, originalVertices, verticesAfterBoxingStored )

		# Switch to "Off" and back to "Stored" to force the use of the cached value.
		pm.setAttr( boxNode + ".ccb", 0 )
		pm.setAttr( boxNode + ".ccb", 1 )
		verticesAfterBoxingStored = pm.mel.OxGetVertices( hair, os = True )
		TestUtilities.CheckPointsAllNearEqual( self, originalVertices, verticesAfterBoxingStored )

		pm.setAttr( boxNode + ".ccb", 2 )
		verticesAfterBoxingPrecomputed = pm.mel.OxGetVertices( hair, os = True )
		TestUtilities.CheckPointsAllNearEqual( self, originalVertices, verticesAfterBoxingPrecomputed )

	def test_GuidesOutput( self ):
		guidesShape = TestUtilities.AddGuidesToNewSphere( rootGenerationMethod = 4 )
		self.assertTrue( pm.mel.OxIsGuides( guidesShape ) )

		# Box GFM node
		pm.mel.OxBoxNodes( [pm.mel.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]] )

		# The output of the stack should be guides
		self.assertTrue( pm.mel.OxIsGuides( guidesShape ) )

	#@unittest.skip( "TODO: fails consistently" )
	def test_ConsistentStrandPositions( self ):
		guidesShape = TestUtilities.AddGuidesToNewSphere( rootGenerationMethod = 4, sphereSegmentCount = 3 )
		verticesBeforeBoxing = pm.mel.OxGetVertices( guidesShape, os = True )

		# Box GFM node
		pm.mel.OxBoxNodes( [pm.mel.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]] )
		verticesAfterBoxing = pm.mel.OxGetVertices( guidesShape, os = True )

		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeBoxing, verticesAfterBoxing )

	def SetDistributionMap( self, hairFromGuides, imageFilePath ):
		fileTexture = pm.shadingNode( "file", asTexture = True, isColorManaged = True )
		place2dTexture = pm.shadingNode( "place2dTexture", asUtility = True )
		pm.connectAttr( place2dTexture.outUV, fileTexture.uv )
		pm.connectAttr( place2dTexture.outUvFilterSize, fileTexture.uvFilterSize )
		pm.connectAttr( fileTexture.outColor, hairFromGuides.distributionMultiplier )

		# Modify UVs and load image as UDIM.
		pm.setAttr( fileTexture + ".fileTextureName", imageFilePath, type = "string" )
		#pm.setAttr( fileTexture + ".uvTilingMode", 3 )

	def test_SaveWithBoxedHFGWithDistributionMap( self ):
		hairShape = TestUtilities.AddHairToNewPlane( 10, 7, planeSize = 10, length = 10 )
		hairFromGuides = pm.PyNode( pm.mel.ls( type = TestUtilities.HairFromGuidesNodeName )[0] )

		# Assign a distribution bitmap to HFG
		imageFilePath = self.findTestFile( "Checker.png" )
		TestUtilities.SetBitmapAttribute( hairFromGuides.distributionMultiplier, imageFilePath )

		verticesBeforeBoxing = pm.mel.OxGetVertices( hairShape, os = True )

		# Box HFG node
		pm.mel.OxBoxNodes( [hairFromGuides] )

		# Save the scene and verify that the resulting file isn't empty
		filePath = pm.saveAs( self.addTempFile( 'test_SaveWithBoxedHFGWithDistributionMap.mb' ) )
		fileSize = os.path.getsize( filePath )
		self.assertGreater( fileSize, 0 )

		pm.newFile( force = True )
		pm.openFile( filePath, force = True )

		verticesAfterLoading = pm.mel.OxGetVertices( pm.mel.ls( type = TestUtilities.HairShapeName )[0], os = True )
		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeBoxing, verticesAfterLoading )

	def test_BoxHFGWithAMap( self ):
		hairShape = TestUtilities.AddHairToNewPlane( 10, 7, planeSize = 10, length = 10 )
		hairFromGuides = pm.PyNode( pm.mel.ls( type = TestUtilities.HairFromGuidesNodeName )[0] )

		TestUtilities.SetBitmapAttribute( hairFromGuides.distributionMultiplier, self.findTestFile( "Checker.png" ) )
		strandCountBeforeBoxing = pm.mel.OxGetStrandCount( hairShape )

		# Box HFG node
		pm.mel.OxBoxNodes( [hairFromGuides] )

		strandCountAfterBoxing = pm.mel.OxGetStrandCount( hairShape )

		self.assertEqual( strandCountBeforeBoxing, strandCountAfterBoxing )