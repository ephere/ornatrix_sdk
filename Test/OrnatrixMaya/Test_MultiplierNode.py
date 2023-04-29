import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.system as pmsys
import pymel.core.datatypes as dt

class Test_MultiplierNode( MayaTest.OxTestCase ):

	def test_BasicMultiplier( self ):
		# Check that at least some vertices are modified
		TestUtilities.TestStrandOperatorChangesGuideCount( self, TestUtilities.MultiplierNodeName )

	def test_MultiplierRandomizationIsIndependentPerStrand( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( guideCount = 2 )
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		guidesFromMesh.length.set( 5 )
		multiplier = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.MultiplierNodeName ) )

		multiplier.copies.set( 3 )
		verticesBefore = TestUtilities.GetStrandPoints( guidesShape, 3 )

		multiplier.copies.set( 2 )
		verticesAfter = TestUtilities.GetStrandPoints( guidesShape, 2 )

		self.assertEqual( verticesAfter, verticesBefore )

	def test_OutputSmoothness( self ):
		"""Tests for jagginess due to segment transformations (#3877)"""
		guidesShape = pm.PyNode( pm.mel.OxAddBraidGuides() )
		braidGuides = pm.PyNode( pm.mel.ls( type = TestUtilities.BraidGuidesNodeName )[0] )
		braidGuides.frequency.set( 1 )
		multiplier = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.MultiplierNodeName ) )
		strandCount = pm.mel.OxGetStrandCount( guidesShape )
		# Simply testing if any of the strands goes back in the z-direction is enough for this bug
		for strandIndex in range( strandCount ):
			vertices = TestUtilities.GetStrandPoints( guidesShape, strandIndex, True )
			oldVertex = vertices[0]
			for vertex in vertices:
				self.assertGreaterEqual( vertex[2], oldVertex[2] )
				oldVertex = vertex

