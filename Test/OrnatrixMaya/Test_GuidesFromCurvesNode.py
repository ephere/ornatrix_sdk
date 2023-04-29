import MayaTest
import TestUtilities
import pymel.core as pm

class Test_GuidesFromCurvesNode( MayaTest.OxTestCase ):

	def test_Create( self ):
		plane = pm.polyPlane( w=10, h=10 )

		curve1Root = (1, 0, 1)
		curve1Offsets = (0, 3, 0.3)
		curve1 = pm.curve( p=[curve1Root, (1, 1, 1.1), (1, 2, 1.2), ( curve1Root[0] + curve1Offsets[0], curve1Root[1] + curve1Offsets[1], curve1Root[2] + curve1Offsets[2] )], k=[0, 0, 0, 1, 1, 1] )
		curve2 = pm.curve( p=[(-1, 0, 1), (-1, 1, 1.1), (-1, 2, 1.2), (-1, 3, 1.3)], k=[0, 0, 0, 1, 1, 1] )
		curve3 = pm.curve( p=[(-1, 0, -1), (-1, 1, -0.9), (-1, 2, -0.8), (-1, 3, -0.7)], k=[0, 0, 0, 1, 1, 1] )
		curve4 = pm.curve( p=[(1, 0, -1), (1, 1, -0.9), (1, 2, -0.8), (1, 3, -0.7)], k=[0, 0, 0, 1, 1, 1] )

		pm.select( clear = True )
		pm.select( curve1, add = True )
		pm.select( curve2, add = True )
		pm.select( curve3, add = True )
		pm.select( curve4, add = True )
		hairShape = pm.PyNode( pm.mel.OxAddGuidesFromCurves() )
		guidesFromCurves = pm.ls( type = TestUtilities.GuidesFromCurvesNodeName )[0]
		guidesFromCurves.pointCount.set( 2 )

		# Expecting a guide for each input curve
		self.assertEqual( 4, pm.mel.OxGetStrandCount( hairShape ) )
		self.assertEqual( [u'curveShape1', u'curveShape2', u'curveShape3', u'curveShape4'], pm.mel.OxGetStackBaseShape( hairShape ) )

		# Ground the guides
		pm.mel.OxAddStrandOperator( hairShape, TestUtilities.GroundStrandsNodeName )
		groundStrandsNode = pm.ls( selection = True )[0]
		plane[0].outMesh >> groundStrandsNode.distributionMesh
		groundStrandsNode.detachRoots.set( 0 )

		# Insert HairFromGuides
		hairFromGuides = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.HairFromGuidesNodeName ) )

		vertices = pm.mel.OxGetVertices( hairShape, os = True )
		diffX = vertices[3] - vertices[0]
		diffY = vertices[4] - vertices[1]
		diffZ = vertices[5] - vertices[2]

		self.assertAlmostEqual( diffZ, curve1Offsets[2], places = 4 )
		self.assertAlmostEqual( diffX, curve1Offsets[0], places = 4 )
		self.assertAlmostEqual( diffY, curve1Offsets[1], places = 4 )