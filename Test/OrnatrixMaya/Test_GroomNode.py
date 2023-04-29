import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.system as pmsys
import pymel.core.datatypes as dt

class Test_GroomNode( MayaTest.OxTestCase ):

	def test_SetParameterValue( self ):
		# Create a basic stack and bake it to a groom
		hairShape = TestUtilities.AddHairToNewPlane()
		curlNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.CurlNodeName ) )
		curlNode.phase.set( 0.05 )
		guidesFromMesh = pm.PyNode( pm.mel.ls( type=TestUtilities.GuidesFromMeshNodeName )[0] )
		hairFromGuides = pm.PyNode( pm.mel.ls( type=TestUtilities.HairFromGuidesNodeName )[0] )

		pm.mel.OxBoxNodes( [curlNode, hairFromGuides, guidesFromMesh] )

		groomNode = pm.PyNode( pm.mel.ls( type = TestUtilities.GroomNodeName )[0] )

		pm.mel.eval( 'OxGroomInfo -sps \"' + groomNode + """\" \"Parameters = (\
	( 'Value1', 'int', lambda gm, v : gm.set_value( 'Curl1', 'Magnitude.Value', v ) ),\
	( 'Value2', 'int', lambda gm, v : gm.set_value( 'Curl1', 'amount', v ) ) \
)\"""" )

		hairShape = pm.mel.ls( type = TestUtilities.HairShapeName )[0]
		originalVertices = pm.mel.OxGetVertices( hairShape, os = True )
		self.assertTrue( originalVertices != None )

		groomNode.Value1.set( 2 )

		verticesAfterValueChange = pm.mel.OxGetVertices( hairShape, os = True )

		TestUtilities.CheckPointsNotAllNearEqual( self, originalVertices, verticesAfterValueChange )

	def TestSetScriptParameterSerialization( self, binary ):
		# Create a basic stack and bake it to a groom
		hairShape = TestUtilities.AddGuidesToNewPlane()
		curlNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.CurlNodeName ) )
		curlNode.phase.set( 1.5 )
		pm.mel.OxGetVertices( hairShape, os = True )

		pm.mel.OxBoxNodes( [curlNode] )

		groomNode = pm.PyNode( pm.mel.ls( type = TestUtilities.GroomNodeName )[0] )

		pm.mel.eval( 'OxGroomInfo -sps \"' + groomNode + """\" \"Parameters = (\
	( 'Value1', 'int', lambda gm, v : gm.set_value( 'Curl1', 'Magnitude.Value', v ) ),\
	( 'Value2', 'int', lambda gm, v : gm.set_value( 'Curl1', 'amount', v ) ) \
)\"""" )

		# Need at least one evaluation before setting parameters. Probably a good idea to address this in the future.
		pm.mel.OxGetVertices( hairShape, os = True )

		value = 5
		groomNode.Value1.set( value )

		verticesBeforeSave = pm.mel.OxGetVertices( hairShape, os = True )
		self.assertEqual( value, groomNode.Value1.get() )

		TestUtilities.SaveAndReloadScene( self, 'test_SetScriptParameterSerialization', binary )

		# Evaluate once to make sure all attributes are updated
		hairShape = pm.PyNode( pm.mel.ls( type = TestUtilities.HairShapeName )[0] )
		verticesAfterLoad = pm.mel.OxGetVertices( hairShape, os = True )

		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeSave, verticesAfterLoad )

		groomNode = pm.PyNode( pm.mel.ls( type = TestUtilities.GroomNodeName )[0] )
		self.assertEqual( value, groomNode.Value1.get() )

		# Finally, make sure that we can still modify parameters and that they alter the script
		groomNode.Value1.set( 8 )
		verticesAfterSecondParameterChange = pm.mel.OxGetVertices( hairShape, os = True )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAfterLoad, verticesAfterSecondParameterChange )

	def test_SetScriptParameterSerializationBinary( self ):
		self.TestSetScriptParameterSerialization( True )

	def test_SetScriptParameterSerializationAscii( self ):
		self.TestSetScriptParameterSerialization( False )

	def test_BoxedClumpNode( self ):
		# Create a basic stack and bake it to a groom
		hairShape = TestUtilities.AddHairToNewPlane()
		#guidesFromMesh = pm.PyNode( pm.mel.ls( type=TestUtilities.GuidesFromMeshNodeName )[0] )
		#hairFromGuides = pm.PyNode( pm.mel.ls( type=TestUtilities.HairFromGuidesNodeName )[0] )

		verticesBeforeClumping = pm.mel.OxGetVertices( hairShape, os = True )

		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.clumpCount.set( 2 )
		
		pm.mel.OxGetVertices( hairShape, os = True )
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 2, 1 ) )

		verticesBeforeBoxing = pm.mel.OxGetVertices( hairShape, os = True )

		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBeforeClumping, verticesBeforeBoxing )

		pm.mel.OxBoxNodes( [clumpNode] )

		verticesAfterBoxing = pm.mel.OxGetVertices( hairShape, os = True )

		TestUtilities.SaveAndReloadScene( self, 'test_BoxedGroomNode', True )

		verticesAfterLoading = pm.mel.OxGetVertices( pm.mel.ls( type = TestUtilities.HairShapeName )[0], os = True )

		TestUtilities.CheckPointsAllNearEqual( self, verticesAfterBoxing, verticesAfterLoading )