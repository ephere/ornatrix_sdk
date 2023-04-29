import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt

class Test_GroundStrandsNode( MayaTest.OxTestCase ):

	# Tests grounding of strands
	def test_Create( self ):
		size = 5
		halfSize = size / 2
		plane = pm.polyPlane( w = size, h = size, subdivisionsWidth = 1, subdivisionsHeight = 1 )

		curve1 = pm.curve( p=[(halfSize, 0, halfSize), (halfSize, 1, halfSize), (halfSize, 2, halfSize), (halfSize, 3, halfSize)], k=[0, 0, 0, 1, 1, 1] )
		curve2 = pm.curve( p=[(-halfSize, 0, halfSize), (-halfSize, 1, halfSize), (-halfSize, 2, halfSize), (-halfSize, 3, halfSize)], k=[0, 0, 0, 1, 1, 1] )
		curve3 = pm.curve( p=[(-halfSize, 0, -halfSize), (-halfSize, 1, -halfSize), (-halfSize, 2, -halfSize), (-halfSize, 3, -halfSize)], k=[0, 0, 0, 1, 1, 1] )
		curve4 = pm.curve( p=[(halfSize, 0, -halfSize), (halfSize, 1, -halfSize), (halfSize, 2, -halfSize), (halfSize, 3, -halfSize)], k=[0, 0, 0, 1, 1, 1] )

		# Create ungrounded guides from 4 curves at the edges of the plane
		pm.select( clear=True )
		pm.select( curve1, add=True )
		pm.select( curve2, add=True )
		pm.select( curve3, add=True )
		pm.select( curve4, add=True )
		pm.mel.OxAddGuidesFromCurves()

		guidesShape = pm.ls( selection = True )[0]

		# Record the positions of hair strand vertices before grounding them
		verticesBeforeGrounding = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Add ground strands node on top of the guides generated from the curves
		pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GroundStrandsNodeName )
		groundStrandsNode = pm.ls( selection = True )[0]
		plane[0].outMesh >> groundStrandsNode.distributionMesh

		# Test strand count here to weave out a bug which reset strand count after initial evaluation of ground node
		self.assertEqual( 4, pm.mel.OxGetStrandCount( guidesShape ) )

		# Ground strands
		groundStrandsNode.detachRoots.set( 0 )

		# Compare vertices before and after grounding, they should be the same
		verticesAfterGrounding = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		for vertexIndex in range( 0, len( verticesBeforeGrounding ) ):
			self.assertAlmostEqual( verticesBeforeGrounding[vertexIndex][0], verticesAfterGrounding[vertexIndex][0], places = 2 )
			self.assertAlmostEqual( verticesBeforeGrounding[vertexIndex][1], verticesAfterGrounding[vertexIndex][1], places = 2 )
			self.assertAlmostEqual( verticesBeforeGrounding[vertexIndex][2], verticesAfterGrounding[vertexIndex][2], places = 2 )

		# Scale the plane, this should move the vertices apart
		self.assertEqual( 1, len( pm.ls( type = TestUtilities.PolyPlaneShapeName ) ) )
		planeShape = pm.ls( type = TestUtilities.PolyPlaneShapeName )[0]
		planeShape.width.set( size * 2 )
		planeShape.height.set( size * 2 )

		verticesAfterResize = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		for vertexIndex in range( 0, len( verticesBeforeGrounding ) ):
			distance = dt.Vector( verticesBeforeGrounding[vertexIndex] ).distanceTo( verticesAfterResize[vertexIndex] )
			self.assertGreater( distance, 1.0,
				'vertexIndex: %(vertexIndex)d, vertexPosition: %(tipPosition)s, expected anything but: %(expected)s' % { 'vertexIndex': vertexIndex, 'tipPosition': verticesBeforeGrounding[vertexIndex], 'expected' : verticesAfterGrounding[vertexIndex] } )

		filePath = pm.saveAs( 'temp.ma' )
		pm.openFile( filePath, force = True )
		verticesAfterReload = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesAfterResize, verticesAfterReload )

	# Tests grounding of strands of two hair systems based on curves
	def test_GroundStrandsOfTwoHairSystemsToSameMesh( self ):
		size = 5
		halfSize = size / 2
		plane = pm.polyPlane( w = size, h = size, subdivisionsWidth = 1, subdivisionsHeight = 1 )

		curve1 = pm.curve( p=[(halfSize, 0, halfSize), (halfSize, 1, halfSize), (halfSize, 2, halfSize), (halfSize, 3, halfSize)], k=[0, 0, 0, 1, 1, 1] )
		curve2 = pm.curve( p=[(-halfSize, 0, halfSize), (-halfSize, 1, halfSize), (-halfSize, 2, halfSize), (-halfSize, 3, halfSize)], k=[0, 0, 0, 1, 1, 1] )
		curve3 = pm.curve( p=[(-halfSize, 0, -halfSize), (-halfSize, 1, -halfSize), (-halfSize, 2, -halfSize), (-halfSize, 3, -halfSize)], k=[0, 0, 0, 1, 1, 1] )
		curve4 = pm.curve( p=[(halfSize, 0, -halfSize), (halfSize, 1, -halfSize), (halfSize, 2, -halfSize), (halfSize, 3, -halfSize)], k=[0, 0, 0, 1, 1, 1] )

		# Create ungrounded guides from 4 curves at the edges of the plane
		#
		# First hair system
		pm.select( clear=True )
		pm.select( curve1, add=True )
		pm.select( curve2, add=True )
		pm.mel.OxAddGuidesFromCurves()

		guidesShape = pm.ls( selection = True )[0]

		# Add ground strands node on top of the guides generated from the curves
		pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GroundStrandsNodeName )
		groundStrandsNode = pm.ls( selection = True )[0]
		plane[0].outMesh >> groundStrandsNode.distributionMesh

		#
		# Second hair system
		pm.select( clear=True )
		pm.select( curve3, add=True )
		pm.select( curve4, add=True )
		pm.mel.OxAddGuidesFromCurves()

		guidesShape = pm.ls( selection = True )[0]

		# Add ground strands node on top of the guides generated from the curves
		pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GroundStrandsNodeName )
		groundStrandsNode = pm.ls( selection = True )[0]
		plane[0].outMesh >> groundStrandsNode.distributionMesh

		# Assert we have two connections on the plane.outMesh
		self.assertEqual(2, len(pm.mel.listConnections(plane[0].outMesh)))

	def test_GroundToMeshWithRotation( self ):
		plane = pm.polyPlane( w = 5, h = 5, subdivisionsWidth = 1, subdivisionsHeight = 1 )
		curve = pm.curve( p=[(0, 0, 0), (0, 0, -1), (0, 0, -2), (0, 0, -3)], k=[0, 0, 0, 1, 1, 1] )

		pm.select( clear=True )
		pm.select( curve, add=True )
		pm.mel.OxAddGuidesFromCurves()

		guidesShape = pm.ls( selection = True )[0]

		pm.parent( guidesShape, plane[0], shape = True )
		#pm.parent( curve, plane[0] )
		pm.select( clear=True )
		pm.select( plane, add=True )
		pm.cmds.rotate( -90, 0, 0 )

		pointCount = pm.mel.OxGetStrandPointCount( guidesShape, 0 )
		lastVertexBeforeGrounding = pm.mel.OxGetStrandPointInObjectCoordinates( guidesShape, 0, pointCount - 1 )
		self.assertEqual( 3.0, lastVertexBeforeGrounding[1] )

		pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GroundStrandsNodeName )
		groundStrandsNode = pm.ls( selection = True )[0]
		plane[0].outMesh >> groundStrandsNode.distributionMesh

		groundStrandsNode.detachRoots.set( 0 )
		lastVertexAfterGrounding = pm.mel.OxGetStrandPointInObjectCoordinates( guidesShape, 0, pointCount - 1 )
		self.assertEqual( lastVertexBeforeGrounding[2], lastVertexAfterGrounding[2] )

	def test_GroundWithMovedObjects( self ):
		# Create a plane and offset it 1 unit along Z coordinate
		plane = pm.polyPlane( w = 10, h = 10, subdivisionsWidth = 1, subdivisionsHeight = 1 )
		pm.select( clear=True )
		pm.select( plane, add=True )
		pm.move( 0, 0, 1, relative = True )

		# Create a curve
		curve1 = pm.curve( p=[(2, 0, 2), (3, 1, 3), (1, 2, 1), (4, 3, 4)], k=[0, 0, 0, 1, 1, 1] )
		pm.select( clear=True )
		pm.select( curve1, add=True )
		pm.move( 1, 0, 0, relative = True )

		# Create guides from curve and offset it 1 unit along X axis
		guidesShape = pm.PyNode( pm.mel.OxAddGuidesFromCurves() )

		guidesShapeTransform = pm.listRelatives( guidesShape, parent=True, fullPath=True )[0]

		# Record points before grounding
		xformBeforeGrounding = pm.xform( guidesShapeTransform, query = True, worldSpace = True, matrix = True )
		verticesBeforeGrounding = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Ground strands to the plane
		pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GroundStrandsNodeName )
		groundStrandsNode = pm.ls( selection = True )[0]
		plane[0].outMesh >> groundStrandsNode.distributionMesh

		# Ground strands
		groundStrandsNode.detachRoots.set( 0 )

		# Points after grounding should be the same as before grounding, in world space
		verticesAfterGrounding = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		xformAfterGrounding = pm.xform( guidesShapeTransform, query = True, worldSpace = True, matrix = True )

		# Compare the vertices before and after ground in world coordinates
		for vertexIndex in range( 0, len( verticesBeforeGrounding ) ):
			# NOTE: Make sure to use VectorN to get homogenized vectors, otherwise matrix multiplication won't work:
			# http://radks.blogspot.com/2014/04/vector-matrix-multiplication-in-maya.html
			vertexBeforeGroundingInWorldSpace = dt.VectorN( verticesBeforeGrounding[vertexIndex][0], verticesBeforeGrounding[vertexIndex][1], verticesBeforeGrounding[vertexIndex][2], 1 ) * dt.Matrix( xformBeforeGrounding )
			vertexAfterGroundingInWorldSpace = dt.VectorN( verticesAfterGrounding[vertexIndex][0], verticesAfterGrounding[vertexIndex][1], verticesAfterGrounding[vertexIndex][2], 1 ) * dt.Matrix( xformAfterGrounding )
			self.assertLess( ( vertexBeforeGroundingInWorldSpace - vertexAfterGroundingInWorldSpace ).length(), 0.01 )

	def test_ChangeCurvePointAfterGrounding( self ):
		size = 5
		halfSize = size / 2
		plane = pm.polyPlane( w = size, h = size, subdivisionsWidth = 1, subdivisionsHeight = 1 )

		# Create four curves which are facing "outwards"
		curve1 = pm.curve( p=[(halfSize, 0, halfSize), (halfSize, 1, halfSize), (size, 2, size), (size * 2, 3, size * 2)], k=[0, 0, 0, 1, 1, 1] )
		curve2 = pm.curve( p=[(-halfSize, 0, halfSize), (-halfSize, 1, halfSize), (-size, 2, size), (-size * 2, 3, size * 2)], k=[0, 0, 0, 1, 1, 1] )
		curve3 = pm.curve( p=[(-halfSize, 0, -halfSize), (-halfSize, 1, -halfSize), (-size, 2, -size), (-size * 2, 3, -size * 2)], k=[0, 0, 0, 1, 1, 1] )
		curve4 = pm.curve( p=[(halfSize, 0, -halfSize), (halfSize, 1, -halfSize), (size, 2, -size), (size * 2, 3, -size * 2)], k=[0, 0, 0, 1, 1, 1] )

		# Create ungrounded guides from 4 curves at the edges of the plane
		pm.select( clear=True )
		pm.select( curve1, add=True )
		pm.select( curve2, add=True )
		pm.select( curve3, add=True )
		pm.select( curve4, add=True )
		guidesShape = pm.mel.OxAddGuidesFromCurves()
		pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GroundStrandsNodeName )
		groundStrandsNode = pm.ls( selection = True )[0]
		plane[0].outMesh >> groundStrandsNode.distributionMesh

		# Ground strands
		groundStrandsNode.detachRoots.set( 0 )

		# Add hair from guides operator
		hfg = pm.PyNode( pm.mel.OxAddStrandOperator( groundStrandsNode, TestUtilities.HairFromGuidesNodeName ) )
		# For easier debugging, but fails the 'maximumXCoordinate - minimumXCoordinate' check below, comment that out if enabling this one
		#hfg.viewportCount.set( 5 )
		self.assertEqual( 1, len( pm.ls( type = TestUtilities.HairShapeName ) ) )
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		verticesBeforeChangingCurve = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Vertices should be facing outward
		minimumXCoordinate = 1000000
		maximumXCoordinate = -1000000
		for vertexIndex in range( 0, len( verticesBeforeChangingCurve ) ):
			if verticesBeforeChangingCurve[vertexIndex][0] < minimumXCoordinate:
				minimumXCoordinate = verticesBeforeChangingCurve[vertexIndex][0]
			if verticesBeforeChangingCurve[vertexIndex][0] > maximumXCoordinate:
				maximumXCoordinate = verticesBeforeChangingCurve[vertexIndex][0]

		self.assertGreaterEqual( maximumXCoordinate - minimumXCoordinate, size * 4 - 0.001 )

		pm.select( curve1 + ".cv[3]", r=True )
		pm.move( 0, 0, 0.0001, relative = True )

		verticesAfterChangingCurve = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# We didn't modify the curve so much so the vertices should be pretty much the same
		self.assertEqual( len( verticesBeforeChangingCurve ), len( verticesAfterChangingCurve ) )
		for vertexIndex in range( 0, len( verticesBeforeChangingCurve ) ):
			self.assertAlmostEqual( verticesBeforeChangingCurve[vertexIndex][0], verticesAfterChangingCurve[vertexIndex][0], places = 3 )
			self.assertAlmostEqual( verticesBeforeChangingCurve[vertexIndex][1], verticesAfterChangingCurve[vertexIndex][1], places = 3 )
			self.assertAlmostEqual( verticesBeforeChangingCurve[vertexIndex][2], verticesAfterChangingCurve[vertexIndex][2], places = 3 )

	def test_PositionBlendingCurve( self ):
		size = 5
		halfSize = size / 2
		verticalOffset = 1
		plane = pm.polyPlane( w = size, h = size, subdivisionsWidth = 1, subdivisionsHeight = 1 )
		curve1 = pm.curve( p=[(0, verticalOffset, 0), (0, verticalOffset + 1, 0), (0, verticalOffset + 2, 0), (0, verticalOffset + 3, 0)], k=[0, 0, 0, 1, 1, 1] )

		# Create ungrounded guides from curve at the edge of the plane
		pm.select( clear=True )
		pm.select( curve1, add=True )
		pm.mel.OxAddGuidesFromCurves()

		guidesShape = pm.ls( selection = True )[0]

		# Record the positions of hair strand vertices before grounding them
		verticesBeforeGrounding = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Add ground strands node on top of the guides generated from the curves
		groundStrandsNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GroundStrandsNodeName ) )
		plane[0].outMesh >> groundStrandsNode.distributionMesh

		# Ground strands
		groundStrandsNode.detachRoots.set( 0 )

		# Disable any blending initially, which should put the strand back to its ungrounded position
		pm.setAttr( groundStrandsNode + ".positionBlendRamp[0].positionBlendRamp_FloatValue", 0.0 )
		pm.setAttr( groundStrandsNode + ".positionBlendRamp[1].positionBlendRamp_FloatValue", 0.0 )
		pm.setAttr( groundStrandsNode + ".positionBlendRamp[2].positionBlendRamp_FloatValue", 0.0 )
		verticesWithoutBlending = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeGrounding, verticesWithoutBlending )

		# Enable blending completely, which should move all points towards the mesh surface by 1 unit
		pm.setAttr( groundStrandsNode + ".positionBlendRamp[0].positionBlendRamp_FloatValue", 1.0 )
		pm.setAttr( groundStrandsNode + ".positionBlendRamp[1].positionBlendRamp_FloatValue", 1.0 )
		pm.setAttr( groundStrandsNode + ".positionBlendRamp[2].positionBlendRamp_FloatValue", 1.0 )
		verticesWithoutBlending = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsAllNotNearEqual( self, verticesBeforeGrounding, verticesWithoutBlending )

		# Reload the scene and test again
		filePath = pm.saveAs( 'temp.ma' )
		pm.openFile( filePath, force = True )

		groundStrandsNode = pm.ls( type = TestUtilities.GroundStrandsNodeName )[0]
		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		# Disable any blending initially, which should put the strand back to its ungrounded position
		pm.setAttr( groundStrandsNode + ".positionBlendRamp[0].positionBlendRamp_FloatValue", 0.0 )
		pm.setAttr( groundStrandsNode + ".positionBlendRamp[1].positionBlendRamp_FloatValue", 0.0 )
		pm.setAttr( groundStrandsNode + ".positionBlendRamp[2].positionBlendRamp_FloatValue", 0.0 )
		verticesWithoutBlending = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeGrounding, verticesWithoutBlending )

		# Enable blending completely, which should move all points towards the mesh surface by 1 unit
		pm.setAttr( groundStrandsNode + ".positionBlendRamp[0].positionBlendRamp_FloatValue", 1.0 )
		pm.setAttr( groundStrandsNode + ".positionBlendRamp[1].positionBlendRamp_FloatValue", 1.0 )
		pm.setAttr( groundStrandsNode + ".positionBlendRamp[2].positionBlendRamp_FloatValue", 1.0 )
		verticesWithoutBlending = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsAllNotNearEqual( self, verticesBeforeGrounding, verticesWithoutBlending )

	# Tests that when we use the Multiplier operator to clone strands, the extra added strands are grounded in correct locations
	def test_GroundMultipliedStrands( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod = 6, length = 0.5 )
		multiplierNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.MultiplierNodeName ) )
		multiplierNode.copies.set( 2 )
		pm.setAttr( multiplierNode + ".fluffRamp[0].fluffRamp_FloatValue", 1.0 )
		multiplierNode.fluffRoot.set( 0.5 )
		multiplierNode.fluffTip.set( 0.5 )
		multiplierNode.spreadTip.set( 0 )
		groundStrandsNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GroundStrandsNodeName ) )

		verticesBeforeGrounding = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Ground strands
		groundStrandsNode.detachRoots.set( 0 )

		verticesAfterGrounding = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Since the multiplied strand is already near the surface the resulting vertices shouldn't change much
		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeGrounding, verticesAfterGrounding, epsilon = 0.05 )

	def test_GroundHairFromGuidesAndReload( self ):
		guidesShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 2, strandCount = 10 )
		groundStrandsNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GroundStrandsNodeName ) )
		groundStrandsNode.detachRoots.set( 0 )
		editGuides = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.EditGuidesShapeName ) )

		# Select some hairs and create a set
		pm.select( editGuides + ".f[0:5]" )
		pm.sets( name = "set1" )

		filePath = pm.saveAs( 'temp.mb' )
		pm.openFile( filePath, force = True )

		guidesShape = pm.ls( type = TestUtilities.EditGuidesShapeName )[0]
		TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# No assertions should happen
