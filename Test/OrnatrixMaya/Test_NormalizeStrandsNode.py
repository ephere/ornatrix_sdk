import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt
import maya.cmds as cmds
import os

class Test_NormalizeStrandsNode( MayaTest.OxTestCase ):

	def test_BasicNormalizeStrands( self ):
		epsilon = 0.0001

		guidesShape = TestUtilities.AddGuidesToNewPlane( guideCount = 20, rootGenerationMethod = 2, pointsPerStrandCount = 10, length = 5 )

		frizzNodeName = pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.FrizzNodeName )

		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		verticesBeforeCount = len( verticesBefore )
		self.assertGreaterEqual( verticesBeforeCount, 2, 'Expected at least two vertices, instead got: %(vertexCount)s' % { 'vertexCount': verticesBeforeCount } )

		normalizeNodeName = pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.NormalizeStrandsNodeName )

		node = pm.PyNode( normalizeNodeName )

		verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		self.assertEqual( len( verticesAfter ), len( verticesBefore ), 'Vertex count was modified by the operator' )

		wasAtLeastOneVertexDifferent = False
		for vertexIndex in range( 0, len( verticesBefore ) ):
			distance = dt.Vector( verticesAfter[vertexIndex] ).distanceTo( verticesBefore[vertexIndex] )
			if not wasAtLeastOneVertexDifferent:
				wasAtLeastOneVertexDifferent = distance > epsilon

		self.assertTrue( wasAtLeastOneVertexDifferent, 'Operator did not change any vertices' )
