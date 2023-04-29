import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt
import unittest

class Test_TransferNormalsNode( MayaTest.OxTestCase ):

	def test_TransferNormals( self ):
		plane = pm.polyPlane(w = 10, h = 10, sx = 1, sy = 1)
		pm.select( plane )

		# Create hair shape with 5 hairs
		hairShape = TestUtilities.AddHairToMesh( plane[0], pointsPerStrandCount=4, rootGenerationMethod = 4 )

		# Create mesh shape and get normals
		meshFromStrandsNode = pm.PyNode(pm.mel.OxAddStrandOperator( hairShape, "MeshFromStrandsNode"));
		hairMesh = pm.ls( type = "mesh" )[0]
		normalsBefore = TestUtilities.GetVertexNormals(hairMesh)

		# Transfer normals from mesh
		transferNormalsNode = pm.PyNode(pm.mel.OxAddStrandOperator( meshFromStrandsNode, "TransferNormalsNode"));

		# Check that normals are different now
		normalsAfter = TestUtilities.GetVertexNormals(hairMesh)
		self.assertFalse( normalsBefore==normalsAfter )


		# Add external meshes
		normalsBefore = normalsAfter
		sphere = pm.polyCube()
		pm.connectAttr( sphere[0] + ".outMesh", transferNormalsNode + ".extMeshes[0]" )

		# Check that normals are different now
		normalsAfter = TestUtilities.GetVertexNormals(hairMesh)
		self.assertFalse( normalsBefore==normalsAfter )

		# Remove external meshes
		normalsBefore = normalsAfter
		pm.removeMultiInstance(  transferNormalsNode + ".extMeshes[0]", b = True)

		# Check that normals are different now
		normalsAfter = TestUtilities.GetVertexNormals(hairMesh)
		self.assertFalse( normalsBefore==normalsAfter )

	def test_NoDistributionMesh( self ):
		guidesFromCurvesShape = TestUtilities.AddGuidesFromCurves()

		meshFromStrandsNode = pm.PyNode(pm.mel.OxAddStrandOperator( guidesFromCurvesShape, "MeshFromStrandsNode"));

		transferNormalsNode = pm.PyNode(pm.mel.OxAddStrandOperator( meshFromStrandsNode, "TransferNormalsNode"));
		self.assertTrue( transferNormalsNode.exists() )