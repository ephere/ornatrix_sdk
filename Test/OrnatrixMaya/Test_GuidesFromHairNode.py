import MayaTest
import TestUtilities
import pymel.core as pm

class Test_GuidesFromHairNode( MayaTest.OxTestCase ):

	def test_Create( self ):
		plane = pm.polyPlane( w=1, h=1 )

		# Create hair shape with a hair at each vertex
		hairShape = TestUtilities.AddHairToMesh( plane[0], hairViewportCount = 10, rootGenerationMethod = 4 )
		hairStrandCount = pm.mel.OxGetStrandCount( hairShape )
		hairVertexCount = pm.mel.OxGetVertexCount( hairShape )
		guidesFromHair = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.GuidesFromHairNodeName ) )
		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		guidesStrandCount = pm.mel.OxGetStrandCount( guidesShape )
		guidesVertexCount = pm.mel.OxGetVertexCount( guidesShape )

		self.assertEqual( hairStrandCount, guidesStrandCount )
		self.assertEqual( hairVertexCount, guidesVertexCount )

	def test_InheritGuideChannelsFromHair( self ):
		# Add hair from mesh strips from two strips and turn on the per-strip strand groups
		plane1 = pm.polyPlane( w = 1, h = 10, sx = 1, sh = 10 )
		plane2 = pm.polyPlane( w = 1, h = 10, sx = 1, sh = 10 )
		pm.move( 2, 0, 0, r = True )

		pm.select( plane1 )
		pm.select( plane2, add = True )
		planeShape1 = plane1[1].output.outputs( sh = True )[0]
		planeShape2 = plane2[1].output.outputs( sh = True )[0]

		hairShape = pm.PyNode( pm.mel.OxAddHairFromMeshStrips( [ planeShape1, planeShape2 ] ) )
		hairFromMeshStrips = pm.ls( type = TestUtilities.HairFromMeshStripsNodeName )[0]
		hairFromMeshStrips.usePerStripStrandGroups.set( True )

		# Add guides from hair operator
		guidesFromHair = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.GuidesFromHairNodeName ) )
		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		self.assertEqual( hairShape, guidesShape )

		# Add frizz operator, we will use it to test whether groups were inherited
		frizzNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.FrizzNodeName ) )

		verticesWithAllGroupsFrizzed = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Change frizzed group index to 1. If groups were inherited then the result should differ from frizzing all groups.
		frizzNode.strandGroupPattern.set( '1' )
		verticesWithGroup1Frizzed = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		frizzNode.strandGroupPattern.set( '3' )
		verticesWithNoGroupsFrizzed = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithAllGroupsFrizzed, verticesWithGroup1Frizzed )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithNoGroupsFrizzed, verticesWithGroup1Frizzed )
