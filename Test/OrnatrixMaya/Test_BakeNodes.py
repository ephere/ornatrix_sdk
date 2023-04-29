import MayaTest
import TestUtilities
import pymel.core as pm
import unittest

class Test_BakeNodes( MayaTest.OxTestCase ):
	def test_BakeTwoFrizzOperators( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( pointsPerStrandCount = 2 )
		frizz1 = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.FrizzNodeName ) )
		frizz2 = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.FrizzNodeName ) )

		verticesBeforeBaking = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Bake the two frizz nodes into a single EG node
		pm.mel.OxBakeNodes( [frizz2, frizz1] )

		verticesAfterBaking = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeBaking, verticesAfterBaking )

	def test_BakeFrizz( self ):
		pm.openFile( self.findTestFile( 'OxMayaV3_FrizzBake.ma' ), force = True )
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		frizz = pm.ls( type = TestUtilities.FrizzNodeName )[0]

		# Some nodes missed the DistributionMesh parameter. These should be auto-wired to the distribution mesh when loading an older scene
		dmesh = pm.PyNode( "polySurfaceShape1" )
		self.assertTrue( dmesh.outMesh.isConnectedTo( frizz.distributionMesh ) )

		verticesBeforeBaking = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		pm.mel.OxBakeNodes( [frizz] )
		verticesAfterBaking = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeBaking, verticesAfterBaking )
