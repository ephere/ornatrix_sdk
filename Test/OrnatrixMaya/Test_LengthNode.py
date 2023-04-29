import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.system as pmsys
import pymel.core.datatypes as dt

class Test_LengthNode( MayaTest.OxTestCase ):

	def SetupLengthNode( self, nodeName ):
		node = pm.PyNode( nodeName )
		node.value.set( 0.5 )

	def test_BasicGuideLength( self ):
		TestUtilities.TestStrandOperatorChangingGuides( self, TestUtilities.LengthNodeName, False, 0.05, operatorSetup = self.SetupLengthNode )

	def test_BoundingBox( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		length = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.LengthNodeName ) )
		box = guidesShape.boundingBox()
		self.assertAlmostEqual( box.height(), 50, 2 )
		length.value.set( 2 )
		box = guidesShape.boundingBox()
		self.assertAlmostEqual( box.height(), 100, 2 )

	def test_LengthChannelControlledByVertexColors( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		length = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.LengthNodeName ) )

		# Paint vertex color info on the two vertices of the plane
		plane = pm.ls( type = "mesh" )[0]
		pm.select( plane + '.vtx[0:1]' )
		pm.polyColorPerVertex( rgb = ( 1, 1, 1 ), cdo = True )

		# Assign the vertex color channel to the length attribute
		length.valueChannel.set( 2001 )

		# We should now have two strands at full length and two strands at 0.05
		# Since min value is 0.01, 50 * 0.01 = 0.05
		tipPositions = TestUtilities.GetTipPositions( guidesShape )
		fullGuideLength = 50
		self.assertAlmostEqual( fullGuideLength, tipPositions[0][2] )
		self.assertAlmostEqual( fullGuideLength, tipPositions[1][2] )
		self.assertAlmostEqual( 25, tipPositions[2][2], 3 )
		self.assertAlmostEqual( 25, tipPositions[3][2], 3 )
