import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt

class Test_GuidesFromGuideMeshNode( MayaTest.OxTestCase ):

	def test_GenerateGuidesFromMesh( self ):
		TestUtilities.AddGuidesToNewPlane()
		meshFromStrands = pm.PyNode( pm.mel.OxAddStrandOperator( "", TestUtilities.MeshFromStrandsNodeName ) )
		meshFromStrands.isGuideMesh.set( True )
		pm.mel.OxAddStrandOperator( meshFromStrands, TestUtilities.GuidesFromGuideMeshNodeName )
		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		# We should get our four guides back
		self.assertEqual( 4, pm.mel.OxGetStrandCount( guidesShape ) )

	def test_Delete_GuidesFromGuideMesh_From_NCloth_Simulation( self ):
		guidesFromMesh = TestUtilities.AddGuidesToNewPlane()
		meshFromStrands = pm.PyNode( pm.mel.OxAddStrandOperator( "", TestUtilities.MeshFromStrandsNodeName ) )
		meshFromStrands.isGuideMesh.set( True )
		guidesShape1 = meshFromStrands.outputMesh.outputs( sh = True )[0]
		pm.mel.source( "AEMeshFromStrandsNodeTemplate.mel" ) # for OxCreateNClothSimulation
		pm.mel.OxCreateNClothSimulation( meshFromStrands )
		guidesFromGuideMesh = pm.PyNode( pm.ls( type = 'GuidesFromGuideMeshNode' )[0] )
		guidesShape2 = guidesFromGuideMesh.outputGuides.outputs( sh = True )[0]
		self.assertEqual( guidesShape2, pm.mel.OxGetStackShape( meshFromStrands ) )
		pm.mel.OxDeleteStrandOperator( guidesFromGuideMesh )
		# Here we need to cause a call to TriangleMeshAccess::ValidateMeshTopologyArrays but I could not figure out how

		pm.mel.OxDeleteStrandOperator( 'outputCloth1' )
		pm.mel.OxDeleteStrandOperator( 'nClothShape1' )
		self.assertEqual( 'HairShape1', pm.mel.OxGetStackShape( meshFromStrands ) )
