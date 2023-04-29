import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt

import unittest
import math

class Test_SmoothSurfaceNode( MayaTest.OxTestCase ):

	def test_DistributionMeshSubdivision( self ):
		mesh = pm.polyCylinder( sa = 10, r = 10.0, h = 10.0 )
		pm.select( mesh )

		guidesShape = TestUtilities.AddGuidesToMesh( mesh[0], rootGenerationMethod = 2, guideCount = 100 )

		smoothSurface = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.SmoothSurfaceNodeName ) )

		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Test root proximity with different subdivision levels
		rootsBeforeSubdivision = TestUtilities.GetRootPositions( guidesShape )
		# Test levels 0 and 3 are far enough
		smoothSurface.subdivisionLevelRender.set( 3 )
		rootsAfterSubdivisionL3 = TestUtilities.GetRootPositions( guidesShape )
		TestUtilities.CheckPointsAllNotNearEqual( self, rootsBeforeSubdivision, rootsAfterSubdivisionL3 )
		# Test levels 3 and 4 are close enough
		smoothSurface.subdivisionLevelRender.set( 4 )
		rootsAfterSubdivisionL4 = TestUtilities.GetRootPositions( guidesShape )
		TestUtilities.CheckPointsAllNearEqual( self, rootsAfterSubdivisionL3, rootsAfterSubdivisionL4, epsilon = 0.1 )

		# Test that after returning to L0 we get the same points
		smoothSurface.subdivisionLevelRender.set( 0 )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesBefore, verticesAfter )

	def DoTestStrandTipChangeWithNormals( self, normalsMode1, normalsMode2, epsilon ):
		mesh = pm.polySphere( r = 10.0 )
		pm.select( mesh )

		editGuidesShape = TestUtilities.AddEditGuidesToMesh( mesh[0], rootGenerationMethod = 2, guideCount = 100 )

		# Move guides from default vertical position to make sure tangent rotations propagate to tips
		pm.select( editGuidesShape + ".ep[:]" )
		pm.move( 0, 0, 5, relative = True )
		pm.mel.OxEditGuides( cc = True )

		smoothSurface = pm.PyNode( pm.mel.OxAddStrandOperator( editGuidesShape, TestUtilities.SmoothSurfaceNodeName ) )
		smoothSurface.subdivisionLevelRender.set( 1 )

		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		# Set strand direction to mode 1
		smoothSurface.strandDirection.set( normalsMode1 )
		tipsOriginal = TestUtilities.GetTipPositions( guidesShape, useObjectCoordinates = True )

		# Set strand direction to mode 2
		smoothSurface.strandDirection.set( normalsMode2 )
		tipsSmooth = TestUtilities.GetTipPositions( guidesShape, useObjectCoordinates = True )

		TestUtilities.CheckPointsAllNearEqual( self, tipsOriginal, tipsSmooth, epsilon = epsilon )


	def test_SmoothNormals( self ):
		"""Test that smooth normals are oriented properly"""
		# Compare "Original" with "Smooth Normals"
		self.DoTestStrandTipChangeWithNormals( normalsMode1 = 0, normalsMode2 = 1, epsilon = 1.5 )

	def test_FaceNormals( self ):
		"""Test that face normals are oriented properly"""
		# Compare "Original" with "Face Normals"
		self.DoTestStrandTipChangeWithNormals( normalsMode1 = 0, normalsMode2 = 2, epsilon = 0.8 )
