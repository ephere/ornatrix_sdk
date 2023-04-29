import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt

# TODO: Add tests for all public Ox MEL commands
class Test_MelCommands( MayaTest.OxTestCase ):

	def setUp( self ):
		MayaTest.OxTestCase.setUp( self )
		self.hairShape = TestUtilities.AddHairToNewPlane()
		pm.mel.OxAddStrandOperator( self.hairShape, TestUtilities.RenderSettingsNodeName )
		self.renderSettings = self.hairShape.inputRenderSettings.inputs()[0]
		self.hairFromGuides = self.hairShape.inputHair.inputs()[0]
		self.guidesFromMesh = self.hairFromGuides.inputStrands.inputs()[0]
		self.baseShape = self.guidesFromMesh.inputMesh.inputs( sh = True )[0]

	def addMeshFromStrandsWithGroups( self ):
		self.meshFromStrands = pm.PyNode( pm.mel.OxAddStrandOperator( self.hairShape, TestUtilities.MeshFromStrandsNodeName ) )
		self.hairShape = pm.PyNode( "HairShape1" )

		# force compute to update generated mesh
		mesh = pm.nodetypes.Mesh( self.hairShape )
		mesh.numVertices();
		mesh.numFaces();
		mesh.numEdges();

		# addShaderSlot is in AEMeshFromStrandsNodeTemplate.mel
		pm.mel.source( "AEMeshFromStrandsNodeTemplate.mel" )
		pm.mel.addShaderSlot( "", self.meshFromStrands )
		pm.PyNode( "lambert1" ).message.connect( self.meshFromStrands.shaderList[0] )
		# Call OxMeshFromStrandsGenerateGroupIds immediately instead of waiting for Maya to call it on idle
		pm.mel.OxMeshFromStrandsGenerateGroupIds( self.meshFromStrands )

		self.assertEqual( self.hairShape.instObjGroups[0].objectGroups[0].get()[0][0], 'f[0:3]' )

		pm.mel.addShaderSlot( "", self.meshFromStrands )
		pm.PyNode( "lambert1" ).message.connect( self.meshFromStrands.shaderList[1] )
		pm.mel.OxMeshFromStrandsGenerateGroupIds( self.meshFromStrands )

		# This material id assignment is generated randomly and depends on the random generator properties. Update the expected values if randomization ever changes
		self.assertEqual( self.hairShape.instObjGroups[0].objectGroups[0].get()[0][0], 'f[2]' )
		self.assertEqual( self.hairShape.instObjGroups[0].objectGroups[1].get()[0][0], 'f[0:1]' )

	def test_OxGetStackShape( self ):
		self.assertEqual( self.hairShape, pm.mel.OxGetStackShape( self.hairShape ) )
		self.assertEqual( self.hairShape, pm.mel.OxGetStackShape( self.renderSettings ) )
		self.assertEqual( self.hairShape, pm.mel.OxGetStackShape( self.hairFromGuides ) )
		self.assertEqual( self.hairShape, pm.mel.OxGetStackShape( self.baseShape ) )
		self.assertEqual( [self.hairShape], pm.mel.OxGetStackShapes( self.hairShape ) )
		self.assertEqual( [self.hairShape], pm.mel.OxGetStackShapes( self.renderSettings ) )
		self.assertEqual( [self.hairShape], pm.mel.OxGetStackShapes( self.hairFromGuides ) )
		self.assertEqual( [self.hairShape], pm.mel.OxGetStackShapes( self.baseShape ) )

		self.addMeshFromStrandsWithGroups()
		self.assertEqual( self.hairShape, pm.mel.OxGetStackShape( self.meshFromStrands ) )
		self.assertEqual( [self.hairShape], pm.mel.OxGetStackShapes( self.meshFromStrands ) )

	def test_OxGetStackShapeAfterMeshEditing( self ):
		self.meshFromStrands = pm.PyNode( pm.mel.OxAddStrandOperator( self.hairShape, TestUtilities.MeshFromStrandsNodeName ) )
		self.hairShape = pm.PyNode( "HairShape1" )
		pm.mel.eval( "polyPoke -ws 1  -tx 0 -ty 0 -tz 0 -ch 1 HairShape1.f[1]" )

		self.assertEqual( self.hairShape, pm.mel.OxGetStackShape( self.meshFromStrands ) )
		self.assertEqual( [self.hairShape], pm.mel.OxGetStackShapes( self.meshFromStrands ) )

	def test_OxGetStackShapeWithDeformerOverMesh( self ):
		self.meshFromStrands = pm.PyNode( pm.mel.OxAddStrandOperator( self.hairShape, TestUtilities.MeshFromStrandsNodeName ) )
		self.hairShape = pm.PyNode( "HairShape1" )
		pm.select( self.hairShape )
		pm.mel.eval( "lattice  -divisions 2 5 2 -objectCentered true  -ldv 2 2 2" )

		self.assertEqual( 'HairShape1Orig', pm.mel.OxGetStackShape( self.meshFromStrands ) )
		self.assertEqual( ['HairShape1Orig'], pm.mel.OxGetStackShapes( self.meshFromStrands ) )

	def test_OxGetStackBelow( self ):
		self.assertEqual( self.renderSettings, pm.mel.OxGetStackBelow( self.hairShape ) )
		self.assertEqual( self.hairFromGuides, pm.mel.OxGetStackBelow( self.renderSettings ) )
		self.assertEqual( "", pm.mel.OxGetStackBelow( self.guidesFromMesh ) )
		self.assertEqual( "", pm.mel.OxGetStackBelow( self.baseShape ) )
		curlNode = pm.PyNode( pm.mel.OxAddStrandOperator( self.renderSettings, TestUtilities.CurlNodeName ) )
		self.assertEqual( curlNode, pm.mel.OxGetStackBelow( self.hairShape ) )

		self.addMeshFromStrandsWithGroups()
		self.assertEqual( self.meshFromStrands, pm.mel.OxGetStackBelow( self.hairShape ) )
		self.assertEqual( curlNode, pm.mel.OxGetStackBelow( self.meshFromStrands ) )

	def test_OxGetStackAbove( self ):
		self.assertIsNone( pm.mel.OxGetStackAbove( self.hairShape ) )
		self.assertIsNone( pm.mel.OxGetStackAbove( self.renderSettings ) )
		self.assertEqual( [self.renderSettings], pm.mel.OxGetStackAbove( self.hairFromGuides ) )
		self.assertEqual( [self.guidesFromMesh], pm.mel.OxGetStackAbove( self.baseShape ) )

		self.addMeshFromStrandsWithGroups()
		self.assertIsNone( pm.mel.OxGetStackAbove( self.hairShape ) )
		self.assertIsNone( pm.mel.OxGetStackAbove( self.meshFromStrands ) )

	def test_OxGetTargetStackNode( self ):
		editGuides = pm.PyNode( pm.mel.OxAddStrandOperator( self.guidesFromMesh, TestUtilities.EditGuidesShapeName ) )
		self.assertEqual( editGuides, pm.mel.OxGetStackBelow( self.hairFromGuides ) )
		#hairShape.worldHair.get( type = True )
		self.assertEqual( 'GuidesData', editGuides.outputStrands.get( type = True ) )
		self.assertEqual( 2, pm.mel.OxFindOutputDataType( editGuides ) )
		self.assertEqual( 2, pm.mel.OxGetInputDataType( pm.mel.OxGetInputAttribute( TestUtilities.GuidesShapeName ) ) )
		# Since HfG can be applied to hair as well as guides, OxGetTargetStackNode returns hairFromGuides instead of editGuides
		self.assertEqual( self.hairFromGuides, pm.mel.OxGetTargetStackNode( self.hairShape, TestUtilities.HairFromGuidesNodeName ) )

	def test_OxIsStackShape( self ):
		self.assertTrue( pm.mel.OxIsStackShape( self.hairShape ) )
		self.assertFalse( pm.mel.OxIsStackShape( self.renderSettings ) )
		self.assertFalse( pm.mel.OxIsStackShape( self.hairFromGuides ) )
		self.assertFalse( pm.mel.OxIsStackShape( self.baseShape ) )

		bakedHair = pm.PyNode( pm.mel.OxCollapseStack( self.hairFromGuides ) )

		self.assertTrue( pm.mel.OxIsStackShape( self.hairShape ) )
		self.assertFalse( pm.mel.OxIsStackShape( bakedHair ) )

	def test_OxGetStackTops( self ):
		self.assertEqual( [self.renderSettings], pm.mel.OxGetStackTops( self.hairShape ) )
		self.assertEqual( [self.renderSettings], pm.mel.OxGetStackTops( self.renderSettings ) )
		self.assertEqual( [self.renderSettings], pm.mel.OxGetStackTops( self.hairFromGuides ) )
		self.assertEqual( [self.renderSettings], pm.mel.OxGetStackTops( self.baseShape ) )

		curl = pm.mel.OxAddStrandOperator2( self.guidesFromMesh, TestUtilities.CurlNodeName, 1, 1 )
		self.assertEqual( [curl, self.renderSettings], pm.mel.OxGetStackTops( self.baseShape ) )
