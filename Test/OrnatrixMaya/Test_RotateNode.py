import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt

class Test_RotateNode( MayaTest.OxTestCase ):

	def test_RotateBillboardMesh( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()

		pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.RotateNodeName )
		rotateNode = pm.ls( type = TestUtilities.RotateNodeName )[0]

		pm.mel.OxAddStrandOperator( rotateNode, TestUtilities.MeshFromStrandsNodeName )

		# Set billboard hair type
		meshFromStrandsNode = pm.ls( type = TestUtilities.MeshFromStrandsNodeName )[0]
		meshFromStrandsNode.meshType.set( 1 )

		meshShape = pm.createNode( 'mesh' )
		meshFromStrandsNode.outputMesh >> meshShape.inMesh

		rotateNode.globalAngle.set( 0.0 )
		meshVerticesBeforeRotating = TestUtilities.GetMeshVertices( meshShape )

		rotateNode.globalAngle.set( 1.0 )
		meshVerticesAfterRotating = TestUtilities.GetMeshVertices( meshShape )

		# We expect strands to be rotated and thus the distances to be changed
		for vertexIndex in range(0, len( meshVerticesBeforeRotating )):
			distance = meshVerticesBeforeRotating[vertexIndex].distanceTo( meshVerticesAfterRotating[vertexIndex] )
			self.assertTrue( distance > 0.001 )

	def test_RotateBasedOnStrandDoesNotModifyStrandShapes( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		surfaceComb = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.SurfaceCombNodeName ) )
		surfaceComb.algorithm.set( 0 )
		pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.RotateNodeName )
		rotateNode = pm.ls( type = TestUtilities.RotateNodeName )[0]

		rotateNode.orientBasedOnStrandShape.set( False )
		verticesBeforeOrientBasedOnStrand = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		rotateNode.orientBasedOnStrandShape.set( True )
		verticesAfterOrientBasedOnStrand = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeOrientBasedOnStrand, verticesAfterOrientBasedOnStrand )

	# Tests that when rotate based on strand is used with a surface comb operator the strands will all face surface of the mesh, in the direction of their rotation
	def test_RotateBasedOnStrandGuidesAllFaceSurfaceWithPolarInterpolation( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( planeSize = 10, length = 5, pointsPerStrandCount = 3 )
		surfaceComb = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.SurfaceCombNodeName ) )
		surfaceComb.algorithm.set( 0 )
		hairFromGuides = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.HairFromGuidesNodeName ) )
		hairFromGuides.interpolation.set( 0 )
		hairFromGuides.renderCount.set( 10 )
		hairFromGuides.viewportCountFraction.set( 1 )
		hairShape = pm.ls( type = 'HairShape' )[0]
		rotateNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.RotateNodeName ) )
		rotateNode.orientBasedOnStrandShape.set( True )

		pm.mel.OxAddStrandOperator( hairShape, TestUtilities.MeshFromStrandsNodeName )
		meshShape = pm.ls( type = "mesh" )[0]

		# Each strand mesh should now face the ground. This means that each vertex should be perpendicular to the Y axis.
		yAxis = [0, 1, 0]
		meshVertices = TestUtilities.GetMeshVertices( meshShape )
		for i in range( 0, len( meshVertices ) // 2 ):
			pointVector = meshVertices[i * 2] - meshVertices[i * 2 + 1]
			self.assertAlmostEqual( 0, pointVector.dot( yAxis ), places = 4 )

	def AngleMapTest( self, shape ):
		rotateNode = pm.PyNode( pm.mel.OxAddStrandOperator( shape, TestUtilities.RotateNodeName ) )
		rotateNode.globalAngle.set( 3.142 )
		pm.mel.OxAddStrandOperator( shape, TestUtilities.MeshFromStrandsNodeName )
		meshShape = pm.ls( type = "mesh" )[0]

		meshVerticesBeforeMap = TestUtilities.GetMeshVertices( meshShape )

		# Assign a checker map to the angle
		#checker = pm.shadingNode( "checker", asTexture = True )
		#pm.connectAttr( checker.outColor, rotateNode.globalAngleMultiplier )
		color1 = [0, 0, 0]
		color2 = [1, 1, 1]
		rampTexture = pm.createNode( "ramp" )
		pm.setAttr( rampTexture + ".colorEntryList[0].position", 0.0 )
		pm.setAttr( rampTexture + ".colorEntryList[0].color", color1[0], color1[1], color1[2], type = "double3" )
		pm.setAttr( rampTexture + ".colorEntryList[1].position", 0.3 )
		pm.setAttr( rampTexture + ".colorEntryList[1].color", color2[0], color2[1], color2[2], type = "double3" )

		# Make ramp to be in step mode and set it to be in U direction
		rampTexture.interpolation.set( 0 )
		pm.setAttr( rampTexture + ".type", 0 )
		pm.mel.connectNodeToAttrOverride( rampTexture, rotateNode.globalAngleMultiplier )
		pm.connectAttr( rampTexture.outColor, rotateNode.globalAngleMultiplier )

		meshVerticesAfterMap = TestUtilities.GetMeshVertices( meshShape )

		# Only rotations of two strands should've changed
		for i in range( len( meshVerticesAfterMap ) // 2, len( meshVerticesAfterMap ) ):
			TestUtilities.CheckPointsNearEqual( self, meshVerticesBeforeMap[i], meshVerticesAfterMap[i] )

	def test_AngleMapGuides( self ):
		self.AngleMapTest( TestUtilities.AddGuidesToNewPlane( length = 1 ) )

	def test_AngleMapHair( self ):
		self.AngleMapTest( TestUtilities.AddHairToNewPlane() )