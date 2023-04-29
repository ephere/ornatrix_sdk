import MayaTest
import TestUtilities
import pymel.core as pm

class Test_ShellsNode( MayaTest.OxTestCase ):

	def test_AddShellMeshes( self ):
		pm.mel.source( "AEShellsNodeTemplate.mel" )

		plane1 = pm.polyPlane( w = 2, h = 2, sx = 1, sh = 1 )
		plane1[0].translateY.set(2)

		plane2 = pm.polyPlane( w = 2, h = 2, sx = 1, sh = 1 )
		plane2[0].translateY.set(4)
		plane2[0].translateX.set(2)

		hairShape = TestUtilities.AddHairToNewPlane()
		shellsNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ShellsNodeName ) )

		# connect shells
		pm.select( plane1 )
		pm.mel.OxAddShellMesh_Test( shellsNode + ".shellMeshes" )
		meshShape = plane1[0]

		self.assertTrue( pm.connectionInfo( meshShape + ".outMesh", isSource=True ) )

		pm.select( plane2 )
		pm.mel.OxAddShellMesh_Test( shellsNode + ".shellMeshes" )
		meshShape = plane2[0]

		self.assertTrue( pm.connectionInfo( meshShape + ".outMesh", isSource=True ) )
