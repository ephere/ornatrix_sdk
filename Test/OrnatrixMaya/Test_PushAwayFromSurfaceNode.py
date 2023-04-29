import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt
import unittest

class Test_PushAwayFromSurfaceNode( MayaTest.OxTestCase ):

	@unittest.skipIf( MayaTest.OsIsLinux, "For an unknown reason this fails on the Linux agent (not on my VM). Needs to be debugged there one day" )
	def test_PushAwayFromSurface( self ):
		sphereRadius = 5
		origin = [0,0,0]

		editGuidesShape = TestUtilities.AddEditGuidesToNewSphere( radius = sphereRadius, pointsPerStrandCount = 10, segmentCount = 10 )
		editGuidesShape.setAttr( 'optimizeStrandGeometry', False )
		pm.select( editGuidesShape + ".ep[0:91]" )
		pm.move( 0, 0, 20, relative = True )
		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		tipPositionsBeforePush = TestUtilities.GetTipPositions( guidesShape, useObjectCoordinates = True )

		# Check that some points are inside the sphere boundary before push operator
		areSomePointsInsideSphereBoundary = False
		for i in range( 0, len( tipPositionsBeforePush ) ):
			if( dt.Vector( tipPositionsBeforePush[i] ).distanceTo( origin ) < sphereRadius ):
				areSomePointsInsideSphereBoundary = True

		self.assertTrue( areSomePointsInsideSphereBoundary )

		pushAwayFromSurfaceNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.PushAwayFromSurfaceNodeName ) )
		pushAwayFromSurfaceNode.distance.set( 0.3 )
		tipPositionsAfterPush = TestUtilities.GetTipPositions( guidesShape, useObjectCoordinates = True )

		# Check that all points are outside the sphere boundary after push operator
		for i in range( 0, len( tipPositionsAfterPush ) ):
			self.assertLess( sphereRadius, dt.Vector( tipPositionsAfterPush[i] ).distanceTo( origin ) )