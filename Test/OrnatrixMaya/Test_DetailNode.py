import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt

class Test_DetailNode( MayaTest.OxTestCase ):

	def test_BasicGuideDetail( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( pointsPerStrandCount = 2 )

		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		detailNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.DetailNodeName ) )
		detailNode.viewPointCount.set( 5 )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		self.assertEqual( 2 * 4, len( verticesBefore ) )
		self.assertEqual( 5 * 4, len( verticesAfter ) )

		detailNode.renderPointCount.set( 6 )
		with TestUtilities.RenderModeScope():
			self.assertEqual( 6 * 4, len( TestUtilities.GetVerticesInObjectCoordinates( guidesShape ) ) )

		self.assertEqual( 5 * 4, len( TestUtilities.GetVerticesInObjectCoordinates( guidesShape ) ) )

	# Tests that we can reduce the detail and no errors will happen
	def test_DecreaseDetail( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		pm.select( guidesShape )
		pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.DetailNodeName )
		detailNode = pm.ls( type = TestUtilities.DetailNodeName )[0]

		# Get vertices just to force the operator to validate
		vertices1 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		self.assertEqual( 10 * 4, len( vertices1 ) )

		# Change point count from 10 to 5
		detailNode.viewPointCount.set( 5 )

		# Evaluate result again, no error should happen
		vertices2 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		self.assertEqual( 5 * 4, len( vertices2 ) )

	# Tests for a bug where UVs are reset when detail node is applied with a strand group specified
	def test_EvalWithGroupsPreservesTextures( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		detailNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.DetailNodeName ) )

		textureCoordinatesWithoutStrandGroups = pm.mel.OxGetTextureCoordinates( hairShape, 0, type = 2 )
		detailNode.strandGroupPattern.set( '1' )
		textureCoordinatesWithStrandGroups = pm.mel.OxGetTextureCoordinates( hairShape, 0, type = 2 )

		self.assertSequenceEqual( textureCoordinatesWithoutStrandGroups, textureCoordinatesWithStrandGroups )

	# Tests that sampling by length produces correct segment lengths
	def test_SampleByLength( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( length = 50, pointsPerStrandCount = 10, lengthRandomness = 1 )
		pm.select( guidesShape )
		detailNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.DetailNodeName ) )
		detailNode.distance.set( 1 )
		detailNode.sampleByLength.set( True )
		self.assertEqual( pm.mel.OxGetStrandPointCount( guidesShape, 0 ), 45 )
		for strandIndex in range( pm.mel.OxGetStrandCount( guidesShape ) ):
			for pointIndex in range( 1, pm.mel.OxGetStrandPointCount( guidesShape, strandIndex ) ):
				segmentLength = dt.Vector( pm.mel.OxGetStrandPointInObjectCoordinates( guidesShape, strandIndex, pointIndex ) ).distanceTo( pm.mel.OxGetStrandPointInObjectCoordinates( guidesShape, strandIndex, pointIndex - 1 ) )
				self.assertLess( segmentLength, 1.2 )
