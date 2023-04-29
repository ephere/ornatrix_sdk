import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt

class Test_WeaverNode( MayaTest.OxTestCase ):

	def test_CreateWeavePattern( self ):
		curve1 = pm.curve( p=[(0, 0, 1), (0, 0, 0.5), (0, 0, -0.5), (0, 0, -1)], k=[0, 0, 0, 1, 1, 1] )
		pm.mel.OxCreateWeavePattern()

		pattern = pm.ls( selection=True )[0]
		pattern.shapeWidth.set( 2.0 )
		pattern.shapeHeight.set( 2.0 )
		pattern.curveSampleCount.set( 10.0 )

		pm.connectAttr( "curveShape1.worldSpace[0]", "%s.inputCurve[0]" % pattern.name() )
		self.assertEqual( 10, pm.mel.OxGetStrandPointCount( "WeavePattern1Guides", 0 ) )

		strandPoints = pm.mel.OxGetStrandPoints( "WeavePattern1Guides", 0 )
		self.assertAlmostEqual( strandPoints[2], 1.0, msg = "Expecting Z to be 1, actual: " + str( strandPoints ) )
		self.assertAlmostEqual( strandPoints[29], -1.0 )

		pm.select( pattern )
		self.assertEqual( [u'WeavePattern1Guides', u'WeavePattern1', u'curveShape1'], pm.mel.OxGetHairStackDialogNodes() )
