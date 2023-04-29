import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt
import unittest

class Test_ResolveCollisionsNode( MayaTest.OxTestCase ):

	def test_ResolveCollisions( self ):
		sphereRadius = 5
		origin = [0,0,0]

		editGuidesShape = TestUtilities.AddEditGuidesToNewSphere( radius = sphereRadius, rootGenerationMethod=2, guideCount=100, pointsPerStrandCount = 10, segmentCount = 10, changeTracking = 0 )
		editGuidesShape.setAttr( 'optimizeStrandGeometry', False )

		# Increase sphere subdivisions to make sure mesh resembles a perfect sphere closer
		sphereShape = pm.ls( type = 'polySphere' )[0]
		sphereShape.subdivisionsAxis.set( 100 )
		sphereShape.subdivisionsHeight.set( 100 )

		pm.select( editGuidesShape + ".ep[0:91]" )
		pm.move( 0, 0, 15, relative = True )
		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		verticesBeforePush = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Check that some points are inside the sphere boundary before push operator
		areSomePointsInsideSphereBoundary = False
		for i in range( 0, len( verticesBeforePush ) ):
			if( dt.Vector( verticesBeforePush[i] ).distanceTo( origin ) < sphereRadius ):
				areSomePointsInsideSphereBoundary = True

		self.assertTrue( areSomePointsInsideSphereBoundary )

		cube = pm.polyCube( depth=5.0, height=5.0, width=5.0 )
		pm.move( 0, 0, 10 )

		resolveCollisionsNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.ResolveCollisionsNodeName ) )

		# Set resolve mode to "Soft" so that strand points are all pushed outside of the mesh
		resolveCollisionsNode.resolveMode.set( 2 )
		pm.connectAttr( cube[0].worldMesh, resolveCollisionsNode.collisionMeshArray, na = True )
		verticesAfterPush = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Check that all points are outside the sphere boundary after push operator
		# Add an epsilon because our mesh isn't a perfect sphere (due to polygons)
		Epsilon = 0.01
		for i in range( 0, len( verticesAfterPush ) ):
			self.assertLess( sphereRadius, dt.Vector( verticesAfterPush[i] ).distanceTo( origin ) + Epsilon )
