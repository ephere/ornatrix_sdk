import MayaTest
import TestUtilities
import pymel.core as pm

class Test_UnboxNodes( MayaTest.OxTestCase ):
	def test_UnboxNodes( self ):
		[hairShape, curlNode, frizzNode, lengthNode] = TestUtilities.CreateBoxedPlaneWithOperators()

		# Force graph evaluation
		pm.mel.OxGetVertices( hairShape, os = True )
		pm.mel.OxBoxNodes( [lengthNode, frizzNode, curlNode] )

		self.assertEqual( 1, len( pm.mel.ls( type=TestUtilities.HairShapeName ) ) )
		hairShape =  pm.mel.ls( type = TestUtilities.HairShapeName )[0]
		boxedVertices = pm.mel.OxGetVertices( hairShape, os = True )

		self.assertEqual( 1, len( pm.mel.ls( type = TestUtilities.GroomNodeName ) ) )
		pm.mel.OxUnboxNodes( [pm.mel.ls(type=TestUtilities.GroomNodeName )[0]] )
		self.assertEqual( None, pm.mel.ls( type = TestUtilities.GroomNodeName ) )

		self.assertEqual( 1, len( pm.mel.ls( type = TestUtilities.HairShapeName ) ) )
		hairShape =  pm.mel.ls( type = TestUtilities.HairShapeName )[0]
		unboxedVertices = pm.mel.OxGetVertices( hairShape, os = True )

		TestUtilities.CheckPointsAllNearEqual( self, boxedVertices, unboxedVertices )

	def test_UnboxNodesWithHFGAndEG( self ):
		[verticesBeforeBoxing, hairShape] = TestUtilities.BoxNodesWithHFG( self )
		pm.mel.OxUnboxNodes( [pm.mel.ls(type=TestUtilities.GroomNodeName )[0]] )
		verticesAfterUnboxing = pm.mel.OxGetVertices( hairShape, os = True )

		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeBoxing, verticesAfterUnboxing )
		
		# Don't allow any warnings during this test
		self.assertEqual( 0, pm.mel.OxProfiler( "-warningCount" ) )

	def test_UnboxStack( self ):
		[verticesBeforeBoxing, hairShape] = TestUtilities.BoxNodesWithHFG( self, True )

		pm.mel.OxUnboxNodes( [pm.mel.ls(type=TestUtilities.GroomNodeName )[0]] )

		hairShape = pm.mel.ls( type=TestUtilities.HairShapeName )[0]
		verticesAfterUnboxing = pm.mel.OxGetVertices( hairShape, os = True )

		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeBoxing, verticesAfterUnboxing )

		# Only one hair shape present after unboxing
		self.assertEqual( 1, len( pm.mel.ls( type=TestUtilities.HairShapeName ) ) )

	def test_UnboxingFirstNodeDoesntModifyStack( self ):
		guidesShape = TestUtilities.AddGuidesToNewSphere( rootGenerationMethod = 4, sphereSegmentCount = 3 )
		pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.CurlNodeName ) )
		pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.FrizzNodeName ) )

		verticesBeforeBoxing = pm.mel.OxGetVertices( guidesShape, os = True )

		# Box and unbox GFM node
		groomNode = pm.mel.OxBoxNodes( [pm.mel.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]] )
		pm.mel.OxUnboxNodes( groomNode )

		guidesShape = pm.mel.ls( type = TestUtilities.GuidesShapeName )[0]
		verticesAfterUnboxing = pm.mel.OxGetVertices( guidesShape, os = True )

		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeBoxing, verticesAfterUnboxing )
		self.assertEqual( 1, len( pm.mel.ls( type=TestUtilities.GuidesShapeName ) ) )
		self.assertIsNone( pm.mel.ls( type=TestUtilities.HairShapeName ) )
		self.assertIsNone( pm.mel.ls( type=TestUtilities.GroomNodeName ) )