import MayaTest
import RunTests
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt

import unittest

class Test_ClumpNode( MayaTest.OxTestCase ):

	def test_CreateRandomClumps( self ):
		# Add hair and clump node
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 2, strandCount = 20 )
		verticesBeforeClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.clumpCount.set( 1 )

		verticesAfterClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# If clumping happened then vertices should have changed
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBeforeClumping, verticesAfterClumping )

	def test_CreateDeleteAllClumps( self ):
		# Add hair and clump node
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 2, strandCount = 20 )
		verticesBeforeClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )

		# Clear all initially created clumps
		TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		pm.mel.OxEditClumps( clumpNode, d = True, sl = 0 )

		# Generate random clumps and then delete them
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 10, 1 ) )
		pm.mel.OxEditClumps( clumpNode, d = True, sl = 0 )

		verticesAfterClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Hairs should not be affected
		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeClumping, verticesAfterClumping )

	def test_DeleteSelectedClump( self ):
		# Generate 2 random clumps
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 2, strandCount = 20, planeSize = 20 )
		verticesBeforeClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )
		TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		pm.mel.OxEditClumps( clumpNode, d = True, sl = 0 )
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 2, 1 ) )

		# We should have all hairs forming two unique tips
		self.assertEqual( 2, len( self.GetUniqueTipPositions( hairShape ) ) )

		# Select first clump and delete it
		clumpNode.clumpSelection.set( [0], type = "Int32Array" )
		pm.mel.OxEditClumps( clumpNode, d = True, sl = 2 )

		# If only one clump was deleted then around half of the hairs should still be in the same tip, which means we should have overall fewer tips than half of the unique hairs
		strandCount = pm.mel.OxGetStrandCount( hairShape )
		allowedError = 5
		self.assertLess( len( self.GetUniqueTipPositions( hairShape ) ), strandCount / 2 + allowedError )

	def test_DeleteSelectedGuideClump( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( length = 5, rootGenerationMethod = 4 )
		hairFromGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.HairFromGuidesNodeName ) )
		hairFromGuidesNode.interpolation.set( 1 )
		hairFromGuidesNode.renderCount.set( 100 )
		hairFromGuidesNode.viewportCountFraction.set( 1 )
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairFromGuidesNode, TestUtilities.ClumpNodeName ) )

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		# Clear all initially created clumps
		TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		pm.mel.OxEditClumps( clumpNode, d = True, sl = 0 )

		# Set guides clumping and create clumps
		guidesClumpingMethod = 2
		clumpNode.clumpCreateMethod.set( guidesClumpingMethod )
		clumpNode.flyawayFraction.set( 0 )
		pm.mel.OxEditClumps( clumpNode, c = ( guidesClumpingMethod, 4, 1 ) )

		# We should have all hairs forming four unique tips
		self.assertEqual( 4, len( self.GetUniqueTipPositions( hairShape ) ) )

		tipsBeforeDeleting = TestUtilities.GetTipPositions( hairShape, True )

		# Select first clump and delete it
		clumpNode.clumpSelection.set( [0], type = "Int32Array" )
		pm.mel.OxEditClumps( clumpNode, d = True, sl = 2 )

		tipsAfterDeleting = TestUtilities.GetTipPositions( hairShape, True )

		# Since we deleted only one clump out of four, roughly 3/4 of all tips should remain in same positions
		strandCount = pm.mel.OxGetStrandCount( hairShape )
		differenceFromExpected = abs( strandCount * 3 / 4 - self.NearEqualElementCount( tipsBeforeDeleting, tipsAfterDeleting ) )
		allowedCountError = 5
		self.assertTrue( differenceFromExpected <= allowedCountError )

		pm.undo()
		TestUtilities.CheckPointsAllNearEqual( self, tipsBeforeDeleting, TestUtilities.GetTipPositions( hairShape, True ) )

	def NearEqualElementCount( self, list1, list2, epsilon = 0.01 ):
		result = 0
		for i in range( 0, len( list1 ) ):
			if dt.Vector( list1[i] ).distanceTo( list2[i] ) < epsilon:
				result += 1

		return result

	def test_ClumpSelectedHairs( self ):
		strandPointCount = 2
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 2, strandCount = 20, pointsPerStrandCount = strandPointCount, planeSize = 20 )
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )

		# Delete any previously existing (auto-generated) clumps
		TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		pm.mel.OxEditClumps( clumpNode, d = True, sl = 0 )

		verticesBeforeClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Select first 5 strands and create a single clump for them
		clumpNode.clumpHairSelection.set( [0, 1, 2, 3, 4], type = "Int32Array" )
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 1, 1 ) )
		verticesAfterClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		firstUnclumpedVertexIndex = 5 * strandPointCount

		# The vertices for first five strands should have been modified
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBeforeClumping, verticesAfterClumping, pointsCount = firstUnclumpedVertexIndex )

		# The vertices for the rest of the strands should not have been modified
		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeClumping, verticesAfterClumping, points1Start = firstUnclumpedVertexIndex, points2Start = firstUnclumpedVertexIndex )

	def test_UnclumpSelectedHairs( self ):
		strandPointCount = 2
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 2, strandCount = 20, pointsPerStrandCount = strandPointCount )
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )

		# Clear all initially created clumps
		TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		pm.mel.OxEditClumps( clumpNode, d = True, sl = 0 )

		verticesBeforeClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Clump all hairs into 2 random clumps
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 2, 1 ) )
		verticesAfterClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Select and unclump only first 5 hairs
		clumpNode.clumpHairSelection.set( [0, 1, 2, 3, 4], type = "Int32Array" )
		pm.mel.OxEditClumps( clumpNode, d = True, sl = 1 )
		verticesAfterDeclumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		firstClumpedVertexIndex = 5 * strandPointCount

		# First five hairs should be restored to same positions as before clumping
		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeClumping, verticesAfterDeclumping, pointsCount = firstClumpedVertexIndex )

		# The rest of the hairs should be same as after clumping
		TestUtilities.CheckPointsAllNearEqual( self, verticesAfterDeclumping, verticesAfterClumping, points1Start = firstClumpedVertexIndex, points2Start = firstClumpedVertexIndex )

	def CreateClumpedHairWithPattern( self, planeSize = 10, guideRootGenerationMethod = 4 ):
		strandPointCount = 4
		hairShape = TestUtilities.AddHairToNewPlane( planeSize = planeSize, rootGenerationMethod = 2, strandCount = 23, pointsPerStrandCount = strandPointCount, guideRootGenerationMethod = guideRootGenerationMethod )
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )

		# Clear all initially created clumps
		TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		pm.mel.OxEditClumps( clumpNode, d = True, sl = 0 )

		patternShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod = 4, pointsPerStrandCount = strandPointCount )

		# Clump all hairs into 2 random clumps
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 2, 1 ) )
		verticesAfterClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Add pattern to the clump node and add it to the list of global patterns
		pm.connectAttr( patternShape + '.worldHair', str( clumpNode ) + '.patterns[0]' )
		clumpNode.patternIndices.set( [0], type = "Int32Array" )
		verticesAfterPatternAssignment = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Check that the pattern has modified the hairs
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAfterClumping, verticesAfterPatternAssignment )

		return verticesAfterClumping

	def test_PerClumpPatterns( self ):
		verticesAfterClumping = self.CreateClumpedHairWithPattern()

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		clumpNode = pm.ls( type = TestUtilities.ClumpNodeName )[0]

		verticesAfterPatternAssignment = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Select first clump and override its settings by removing the pattern there.
		# We'll manually adjust the pattern indices override array since typically this is done from a GUI callback.
		clumpNode.clumpSelection.set( [0], type = "Int32Array" )
		clumpNode.patternIndicesOverride.set( [0], type = "Int32Array" )

		# Evaluate to update the internal clump selection
		TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		clumpNode.patternIndicesOverride.set( [], type = "Int32Array" )

		# With one clump using a pattern and another one not using it we should now get different hair
		verticesAfterFirstClumpPatternRemoving = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAfterPatternAssignment, verticesAfterFirstClumpPatternRemoving )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAfterClumping, verticesAfterFirstClumpPatternRemoving )

		# Select the second clump and adjust its amount override
		clumpNode.clumpSelection.set( [1], type = "Int32Array" )
		clumpNode.patternIndicesOverride.set( [0], type = "Int32Array" )
		TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		clumpNode.amountOverride.set( 0.5 )

		# Hairs are now in an entirely new configuration
		verticesAfterSecondClumpAmountOverride = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAfterFirstClumpPatternRemoving, verticesAfterSecondClumpAmountOverride )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAfterPatternAssignment, verticesAfterSecondClumpAmountOverride )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAfterClumping, verticesAfterSecondClumpAmountOverride )

		# Reset the second clump to get previous configuration back
		pm.mel.OxEditClumps( clumpNode, r = True )
		verticesAfterSecondClumpReset = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesAfterFirstClumpPatternRemoving, verticesAfterSecondClumpReset )

	def test_TwistingWithClumpPatterns( self ):
		self.CreateClumpedHairWithPattern()

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		clumpNode = pm.ls( type = TestUtilities.ClumpNodeName )[0]

		clumpNode.flyawayFraction.set( 0 )
		verticesAfterClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Set twisting to a small non-zero value
		clumpNode.twist.set( 0.05 )

		verticesAfterSmallTwist = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Since the twist is small we should get a small change in vertices, but not too big
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAfterClumping, verticesAfterSmallTwist, epsilon = 0.001 )
		TestUtilities.CheckPointsAllNearEqual( self, verticesAfterClumping, verticesAfterSmallTwist, epsilon = 5 )

	def test_RoundClumpsWithManuallySelectedHairsAndPatterns( self ):
		self.CreateClumpedHairWithPattern()

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		clumpNode = pm.ls( type = TestUtilities.ClumpNodeName )[0]

		clumpNode.flyawayFraction.set( 0 )

		# Delete all clumps
		pm.mel.OxEditClumps( clumpNode, d = True, sl = 0 )

		# Select some hairs and create a single clump from them
		clumpNode.clumpHairSelection.set( [0, 1, 2, 3, 4], type = "Int32Array" )
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 1, 1 ) )

		# Turn on rounded clumps
		clumpNode.roundClumps.set( 1 )
		verticesAfterClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# We should not get an assertion

	def test_ChangeClumpedHairPointCount( self ):
		verticesAfterClumping = self.CreateClumpedHairWithPattern()

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		clumpNode = pm.ls( type = TestUtilities.ClumpNodeName )[0]

		# Change incoming guide and hair point counts
		guidesFromMesh.pointCount.set( 5 )

		# We should not get any errors upon next evaluation
		verticesAfterPointCountChange = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

	def test_ChangeClumpedHairPointCount2( self ):
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 2, strandCount = 100, planeSize = 10 )
		externalGuidesShape = TestUtilities.AddGuidesToNewPlane( planeSize = 10, planeSegmentCount = 2 )

		# Set affine interpolation so we have some hairs generated from each guide
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.interpolation.set( 1 )

		# Clump by guides, we should get a clump for each of the 4 guides
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )
		clumpNode.twist.set( 0.5 )

		# Add pattern to the clump node and add it to the list of global patterns
		pm.connectAttr( externalGuidesShape + '.worldHair', str( clumpNode ) + '.patterns[0]' )

		# Connect the external guides attribute
		externalGuidesShape.outputHair >> clumpNode.externalClumpStrands
		clumpNode.clumpCreateMethod.set( 3 )

		# Value 3 means clumping to external guides
		pm.mel.OxEditClumps( clumpNode, c = ( 3, 0, 0 ) )

		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		guidesFromMesh.pointCount.set( 5 )

	def test_ChangeInputGuideCountWithPatterns( self ):
		verticesAfterClumping = self.CreateClumpedHairWithPattern( guideRootGenerationMethod = 2 )
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]

		pm.mel.OxGetStrandCount( hairShape )

		# Change incoming guide and hair point counts
		guidesFromMesh.pointCount.set( 5 )
		pm.setAttr( guidesFromMesh + ".count", 3 )

		# This should not cause a crash
		pm.mel.OxGetStrandCount( hairShape )

	# Gets strand tip positions which are located in a unique object space location. Useful for seeing how many clumps there are.
	def GetUniqueTipPositions( self, hairShape, epsilon = 0.01 ):
		result = []
		for strandIndex in range( 0, pm.mel.OxGetStrandCount( hairShape ) ):
			pointCount = pm.mel.OxGetStrandPointCount( hairShape, strandIndex )
			currentStrandTip = pm.mel.OxGetStrandPointInObjectCoordinates( hairShape, strandIndex, pointCount - 1 )

			if len( result ) == 0:
				result.append( currentStrandTip )
			else:
				hasResultAtSamePosition = False
				for resultValue in result:
					if( dt.Vector( resultValue ).distanceTo( currentStrandTip ) <= epsilon ):
						hasResultAtSamePosition = True

				if not hasResultAtSamePosition:
					result.append( currentStrandTip )

		return result

	def test_CreateClumpsFromStrandGroups( self ):
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 2, strandCount = 20, planeSize = 10 )
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]

		# Set affine interpolation so we have some hairs generated from each guide
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.interpolation.set( 1 )

		editGuidesShape = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.EditGuidesShapeName ) )
		editGuidesShape.useStrandGroups.set( True )

		# Clump by strand groups, only one big clump is created by default since we only have one group
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )

		pm.mel.OxEditClumps( clumpNode, c = ( 1, 0, 0 ) )
		self.assertEqual( 1, len( self.GetUniqueTipPositions( hairShape ) ) )

		# Create a new guide group and regenerate the clumps, we should now have two clumps, one for guide group 0 and one for 1
		pm.select( editGuidesShape + ".ep[0]" )
		pm.mel.OxAssignStrandGroup( editGuidesShape, "1" )
		pm.mel.OxEditClumps( clumpNode, c = ( 1, 0, 0 ) )
		self.assertEqual( 2, len( self.GetUniqueTipPositions( hairShape ) ) )

		# Create another guide group, to create 3 clumps
		pm.select( editGuidesShape + ".ep[1]" )
		pm.mel.OxAssignStrandGroup( editGuidesShape, "2" )
		pm.mel.OxEditClumps( clumpNode, c = ( 1, 0, 0 ) )
		self.assertEqual( 3, len( self.GetUniqueTipPositions( hairShape ) ) )

	def test_CreateClumpsFromGuides( self ):
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 2, strandCount = 20, planeSize = 10 )
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]

		# Set affine interpolation so we have some hairs generated from each guide
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.interpolation.set( 1 )

		# Clump by guides, we should get a clump for each of the 4 guides
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )

		pm.mel.OxEditClumps( clumpNode, c = ( 2, 0, 0 ) )
		self.assertEqual( 4, len( self.GetUniqueTipPositions( hairShape ) ) )

	# Guide clumping is a special mode where we need to make sure that all hairs clump to their closest guide centers
	def test_GuideClumpingMakesAllHairClumpToClosestGuides( self ):
		hairShape = TestUtilities.AddHairToNewSphere( rootGenerationMethod = 2, strandCount = 50, sphereSegmentCount = 4, guideLength = 5, pointsPerStrandCount = 5 )

		# Set affine interpolation so we have some hairs generated from each guide
		# Make sure that all hairs follow their closest guide shape
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.interpolation.set( 1 )
		hairFromGuides.autoPart.set( 1 )
		hairFromGuides.angleThreshold.set( 1 )

		# Clump by guides, we should get a clump for each of the 4 guides
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )
		clumpNode.clumpCreateMethod.set( 2 )
		pm.mel.OxEditClumps( clumpNode, c = ( 2, 0, 0 ) )

		 # If all hairs are clumped to their closest guide then their lengths should be near 5
		hairLengths = TestUtilities.GetStrandLengths( hairShape )
		for length in hairLengths:
			self.assertNearEqual( 5, length, 1 )

	# Tests for a regression of a crash when decreasing surface mesh vertex count
	def test_DecreaseDistributionMeshVertexCountAfterClumping( self ):
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 2, strandCount = 20, planeSegmentCount = 2 )
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		pm.mel.OxEditClumps( clumpNode, c = ( 2, 0, 0 ) )

		# Reduce plane segments and evaluate
		polyPlane = pm.ls( type = "polyPlane" )[0]
		polyPlane.subdivisionsWidth.set( 1 )
		pm.mel.OxGetStrandCount( hairShape )

		# If no crash happened we are good

	def test_CreateClumpsFromExternalGuides( self ):
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 2, strandCount = 100, planeSize = 10 )
		externalGuidesShape = TestUtilities.AddGuidesToNewPlane( planeSize = 10, planeSegmentCount = 2 )

		# Set affine interpolation so we have some hairs generated from each guide
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.interpolation.set( 1 )

		# Clump by guides, we should get a clump for each of the 4 guides
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )

		# Connect the external guides attribute
		externalGuidesShape.outputHair >> clumpNode.externalClumpStrands

		# Value 3 means clumping to external guides
		pm.mel.OxEditClumps( clumpNode, c = ( 3, 0, 0 ) )

		# We should have 9 unique tips because the plane which generates guides has 2 side segments and 9 vertices altogether
		self.assertEqual( 9, len( self.GetUniqueTipPositions( hairShape ) ) )

	def test_IncreaseClumpPatternStemCount( self ):
		strandPointCount = 4
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 2, strandCount = 23, pointsPerStrandCount = strandPointCount )
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )

		# Clear all initially created clumps
		TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		pm.mel.OxEditClumps( clumpNode, d = True, sl = 0 )

		patternShape = pm.PyNode( pm.mel.OxAddBraidGuides() )

		# Clump all hairs into 2 random clumps
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 2, 1 ) )
		verticesAfterClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Add pattern to the clump node and add it to the list of global patterns
		pm.connectAttr( patternShape + '.worldHair', str( clumpNode ) + '.patterns[0]' )
		clumpNode.patternIndices.set( [0], type = "Int32Array" )

		verticesAfterPatternAssignment = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Modify the braid shape by increasing stem count, by setting preset type from Pigtail (3-stem) to 5-stem
		patternNode = pm.ls( type = TestUtilities.BraidGuidesNodeName )[0]
		patternNode.presetIndex.set( 3 )
		verticesAfterStemCountChange = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# If no crash happened we are good, also vertices should be different
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAfterPatternAssignment, verticesAfterStemCountChange, epsilon = 0.001 )

	def test_ClumpAmountControllerByColorSet( self ):
		strandPointCount = 2
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 2, strandCount = 23, pointsPerStrandCount = strandPointCount )
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.clumpCount.set( 1 )

		clumpedVerticesBeforeColorSet = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Paint vertex color info on the two vertices of the plane
		plane = pm.ls( type = "mesh" )[0]
		pm.select( plane + '.vtx[2:3]' )
		pm.polyColorPerVertex( rgb = ( 0, 0, 0 ), cdo = True )

		# Assign the vertex color channel to the length attribute
		clumpNode.amountChannel.set( 2001 )

		clumpedVerticesAfterColorSet = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Assigning the vertex color channel to clumping channel should have modified the result
		TestUtilities.CheckPointsNotAllNearEqual( self, clumpedVerticesBeforeColorSet, clumpedVerticesAfterColorSet, epsilon = 0.001 )

	def AssertFirstStrandIsSmooth( self, hairShape ):
		clumpedVertices = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		firstVertexIndex = 0
		for strandIndex in range( 0, pm.mel.OxGetStrandCount( hairShape ) ):
			strand1PreviousVector = dt.Vector( clumpedVertices[firstVertexIndex + 1] ) - dt.Vector( clumpedVertices[firstVertexIndex] )
			strand1PreviousVector.normalize()

			pointCount = pm.mel.OxGetStrandPointCount( hairShape, strandIndex )
			for pointIndex in range( 1, pointCount ):
				strand1Vector = dt.Vector( clumpedVertices[firstVertexIndex + pointIndex] ) - dt.Vector( clumpedVertices[firstVertexIndex + pointIndex - 1] )
				strand1Vector.normalize()

				self.assertLess( 1 - strand1Vector.dot( strand1PreviousVector ), 0.01, msg = "Previous vector: " + str( strand1PreviousVector ) + ", current vector: " + str( strand1Vector ) + ", point index: " + str( pointIndex ) )

				strand1PreviousVector = strand1Vector

			firstVertexIndex += pointCount

	# Tests for a bug where hairs would have jagged breaks when over 22 points are used
	def test_ClumpedHairsAreSmoothAtHighPointCount( self ):
		pointCount = 23
		hairShape = TestUtilities.AddHairToNewPlane( pointsPerStrandCount = pointCount, planeSize = 20 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairFromGuides, TestUtilities.ClumpNodeName ) )

		# Clump all hairs into 1 clump
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 1, 1 ) )

		# Make sure that the resulting clumped hair is smooth by checking that all consecutive segment direction vectors are relatively the same
		self.AssertFirstStrandIsSmooth( hairShape )

		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		guidesFromMesh.pointCount.set( 26 )

		self.AssertFirstStrandIsSmooth( hairShape )

	# Tests that strand ids are properly used to reference hairs, to allow hair order changes
	@unittest.skip( "Clump positions can change if input strand positions change" )
	def test_ChangingInputStrandOrderDoesNotChangeClumpStructure( self ):
		hairShape = TestUtilities.AddHairToNewPlane( planeSize = 20 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairFromGuides, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )

		hairFromGuides.distribution.set( 2 )
		hairFromGuides.renderCount.set( 2 )

		pm.setAttr( clumpNode + ".amountRamp[0].amountRamp_FloatValue", 1.0 )
		pm.setAttr( clumpNode + ".amountRamp[1].amountRamp_FloatValue", 1.0 )
		pm.setAttr( clumpNode + ".amountRamp[3].amountRamp_FloatValue", 1.0 )

		# Clump all hairs into 1 clump
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 1, 1 ) )

		boundingBox1 = TestUtilities.GetBoundingBox( TestUtilities.GetVerticesInObjectCoordinates( hairShape ) )

		# Changing the render count should not change the clump stems
		hairFromGuides.renderCount.set( 1 )
		boundingBox2 = TestUtilities.GetBoundingBox( TestUtilities.GetVerticesInObjectCoordinates( hairShape ) )

		TestUtilities.CheckPointsAllNearEqual( self, boundingBox1, boundingBox2 )

	# Tests for a bug where clumper would crash when used on top of baked hair which had viewport preview set to 100%
	def test_FullBakedHairPreviewWithClumping( self ):
		# Bake the hair
		hairShape = TestUtilities.AddHairToNewPlane( planeSize = 10, rootGenerationMethod = 2, strandCount = 100 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		pm.mel.OxCollapseStack( hairFromGuides )
		bakedHair = pm.ls( type = TestUtilities.BakedHairNodeName )[0]
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		# Add clumping
		clump = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )

		# Increase hair display fraction to 100%
		bakedHair.displayFraction.set( 1 )
		verticesAtFullPreview = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# If no errors happened we are good

	# Tests that when Guides is used for clumping method the guides themselves are used for the clump stems
	def test_GuideClumpStemsUseGuides( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod = 2, planeSize = 20 )

		# Add frizz just to randomize the guide tip positions
		frizz = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.FrizzNodeName ) )
		frizz.amount.set( 10 )

		guideTipPositions = TestUtilities.GetTipPositions( guidesShape, True )

		hairFromGuides = pm.PyNode( pm.mel.OxAddStrandOperator( frizz, TestUtilities.HairFromGuidesNodeName ) )
		hairFromGuides.interpolation.set( 1 )
		hairFromGuides.distribution.set( 2 )
		hairFromGuides.renderCount.set( 100 )
		hairFromGuides.viewportCountFraction.set( 1 )

		# Add clump, set it to Guides mode and generate clumps
		clump = pm.PyNode( pm.mel.OxAddStrandOperator( hairFromGuides, TestUtilities.ClumpNodeName ) )
		clump.clumpCreateMethod.set( 2 )
		clump.flyawayFraction.set( 0 )
		pm.mel.OxEditClumps( clump, c = ( 2, 1, 1 ) )

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		uniqueHairTipPositions = self.GetUniqueTipPositions( hairShape )
		self.assertEqual( len( uniqueHairTipPositions ), len( guideTipPositions ) )

		# All hairs should be clustered to existing guide tips
		TestUtilities.CheckPointsMatch( self, uniqueHairTipPositions, guideTipPositions )

	def test_GuideClumpStemsUseExternalStrands( self ):
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 2, strandCount = 100, planeSize = 10 )
		externalGuidesShape = TestUtilities.AddGuidesToNewPlane( planeSize = 10, planeSegmentCount = 2, rootGenerationMethod = 2 )

		# Add frizz just to randomize the guide tip positions
		frizz = pm.PyNode( pm.mel.OxAddStrandOperator( externalGuidesShape, TestUtilities.FrizzNodeName ) )
		frizz.amount.set( 10 )

		# Set affine interpolation so we have some hairs generated from each guide
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.interpolation.set( 1 )

		# Clump by guides, we should get a clump for each of the 4 guides
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )
		clumpNode.clumpCreateMethod.set( 3 )

		# Connect the external guides attribute
		externalGuidesShape.outputHair >> clumpNode.externalClumpStrands

		# Value 3 means clumping to external guides
		pm.mel.OxEditClumps( clumpNode, c = ( 3, 0, 0 ) )

		guideTipPositions = TestUtilities.GetTipPositions( externalGuidesShape, True )
		uniqueHairTipPositions = self.GetUniqueTipPositions( hairShape )

		# We should have 9 unique tips because the plane which generates guides has 2 side segments and 9 vertices altogether
		TestUtilities.CheckPointsMatch( self, uniqueHairTipPositions, guideTipPositions )

		# Reload the scene and test again, to ensure persistence
		filePath = pm.saveAs( 'temp.mb' )
		pm.openFile( filePath, force = True )

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		uniqueHairTipPositions = self.GetUniqueTipPositions( hairShape )
		TestUtilities.CheckPointsMatch( self, uniqueHairTipPositions, guideTipPositions )

	def test_RegionMap( self ):
		planeWidth = 10
		plane = pm.polyPlane( sx = 1, sy = 1, width = planeWidth, height = 1 )
		pm.select( plane )
		meshShape = plane[0]

		# Add hair to plane, make sure we're using the vertex distribution for the hair and face center distribution for guides, so we'll have only two guides
		hairShape = TestUtilities.AddHairToMesh( meshShape, rootGenerationMethod = 2, hairViewportCount = 100, length = 2 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.interpolationGuideCount.set( 3 )
		hairFromGuides.interpolation.set( 1 )
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]

		# Add surface comb node without randomness to spread the two guides away from one another
		surfaceComb = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.SurfaceCombNodeName ) )
		surfaceComb.chaos.set( 0 )
		surfaceComb.algorithm.set( 0 )

		# Make surface comb not completely flatten the guides, but leave them raised a little
		pm.setAttr( surfaceComb + ".slopeRamp[2].slopeRamp_FloatValue", 0.7 )

		color1 = [1, 0, 0]
		color2 = [0, 1, 0]

		# Create a ramp texture which will define the two partings. Leave the parting not exactly down the middle but a bit to the side so that hairs in the middle will belong to guides on the bigger side.
		rampTexture = pm.createNode( "ramp" )
		colorDivisionPosition = 0.7 * planeWidth
		pm.setAttr( rampTexture + ".colorEntryList[0].position", 0.0 )
		pm.setAttr( rampTexture + ".colorEntryList[0].color", color1[0], color1[1], color1[2], type = "double3" )
		pm.setAttr( rampTexture + ".colorEntryList[1].position", colorDivisionPosition / planeWidth )
		pm.setAttr( rampTexture + ".colorEntryList[1].color", color2[0], color2[1], color2[2], type = "double3" )

		# Make ramp to be in step mode and set it to be in U direction
		rampTexture.interpolation.set( 0 )
		pm.setAttr( rampTexture + ".type", 1 )

		# Add clumper with random clumping and generate some clumps
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )
		clumpNode.clumpCreateMethod.set( 0 )
		clumpNode.clumpCreateMethod.set( 2 )

		pm.mel.connectNodeToAttrOverride( rampTexture, clumpNode + ".regionMultiplier" )
		pm.connectAttr( rampTexture + ".outColor", clumpNode + ".regionMultiplier" )

		# Value 3 means clumping to external guides
		pm.mel.OxEditClumps( clumpNode, c = ( 2, 4, 0 ) )

		# Make sure that all strands are clumped
		self.assertEqual( 4, len( self.GetUniqueTipPositions( hairShape ) ) )

		allowedError = 0

		# If the clumping region map worked correctly we should not have any hair segments which cross the X axis
		vertices = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		for i in range( 0, len( vertices ) // 2 ):
			rootXCoordinate = vertices[i * 2][0]
			tipXCoordinate = vertices[i * 2 + 1][0]

			self.assertTrue(
				( rootXCoordinate <= ( colorDivisionPosition + allowedError ) and tipXCoordinate <= ( colorDivisionPosition + allowedError ) ) or
				( rootXCoordinate >= ( colorDivisionPosition - allowedError ) and tipXCoordinate >= ( colorDivisionPosition - allowedError ) ),
				   "Root coord: " + str( rootXCoordinate ) + ", tip coord: " + str( tipXCoordinate ) )

	def test_SubClumping( self ):
		# TODO: change the range end from 6 to 7 to get a failure for seed 6
		for randomSeed in RunTests.RandomSeeds( 5, 6 ):
			planeWidth = 10
			plane = pm.polyPlane( sx = 1, sy = 1, width = planeWidth, height = planeWidth )
			pm.select( plane )
			meshShape = plane[0]

			# Test parameters
			hairCount = 500
			subClumpCount = 20

			# Add hair to plane, make sure we're using the vertex distribution for the hair and face center distribution for guides, so we'll have only two guides
			hairShape = TestUtilities.AddHairToMesh( meshShape, rootGenerationMethod = 2, hairRenderCount = hairCount, hairViewportCount = hairCount, length = 2 )
			hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
			hairFromGuides.interpolationGuideCount.set( 3 )
			hairFromGuides.interpolation.set( 1 )
			hairFromGuides.randomSeed.set( randomSeed )
			guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]

			# Add surface comb node without randomness to spread the two guides away from one another
			surfaceComb = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.SurfaceCombNodeName ) )
			surfaceComb.chaos.set( 0 )
			surfaceComb.algorithm.set( 0 )

			# Make surface comb not completely flatten the guides, but leave them raised a little
			pm.setAttr( surfaceComb + ".slopeRamp[2].slopeRamp_FloatValue", 0.7 )

			# Add clumper with random clumping and generate some clumps
			clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
			clumpNode.flyawayFraction.set( 0 )
			clumpNode.clumpCreateMethod.set( 2 )

			# Value 2 means clumping to guides
			pm.mel.OxEditClumps( clumpNode, c = ( 2, 4, 0 ) )

			# Add a second clumper on top, to generate the sub-clumps
			clumpNode2 = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
			clumpNode2.flyawayFraction.set( 0 )
			clumpNode2.clumpCount.set( subClumpCount )
			pm.mel.OxEditClumps( clumpNode2, c = ( 0, subClumpCount, 0 ) )

			epsilon = 0.1

			# First check sub-clumps without region channel, we should get hairs migrating between clumps at a partial clumping value
			verticesAtFullSubClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
			clumpNode2.amount.set( 0.5 )
			verticesAtPartialSubClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
			TestUtilities.CheckPointsNotAllNearEqual( self, verticesAtFullSubClumping, verticesAtPartialSubClumping, epsilon = epsilon )

			# Now use the region channel to ensure sub-clumping properly happens
			# If sub-clumps were created properly, reducing the clumping amount should not have any hairs traveling between clumps, which means their vertices will stay nearly the same
			clumpNode.addClumpIndexChannel.set( 1 )
			clumpNode2.regionChannel.set( 2 )
			pm.mel.OxEditClumps( clumpNode2, c = ( 0, subClumpCount, 0 ) )

			self.CheckSubClumpingHairsDontMove( hairShape, clumpNode2, epsilon )

			# Increase hair count and check again
			hairFromGuides.renderCount.set( hairCount * 2 )
			self.CheckSubClumpingHairsDontMove( hairShape, clumpNode2, epsilon )

	def CheckSubClumpingHairsDontMove( self, hairShape, clumpNode2, epsilon ):
		# If sub-clumps were created properly, reducing the clumping amount should not have any hairs traveling between clumps, which means their vertices will stay nearly the same
		clumpNode2.amount.set( 0.5 )
		verticesAtPartialSubClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		clumpNode2.amount.set( 1 )
		verticesAtFullSubClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesAtFullSubClumping, verticesAtPartialSubClumping, epsilon = epsilon )

	def test_PreserveStrandLengths( self ):
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 2, pointsPerStrandCount = 5, strandCount = 20, planeSize = 50, length = 50 )

		# Record the lengths of each strand
		originalStrandLengths = TestUtilities.GetStrandLengths( hairShape )

		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )

		# Clump all hairs into 1 clump
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 1, 1 ) )

		# By default clumping should have changed the lengths of strands
		strandLengthsAfterClumping = TestUtilities.GetStrandLengths( hairShape )
		hasAtLeastOneDifferingLength = False
		for i in range( 0, len( originalStrandLengths ) ):
			if( abs( originalStrandLengths[i] - strandLengthsAfterClumping[i] ) > 0.0001 ):
				hasAtLeastOneDifferingLength = True
				break

		self.assertTrue( hasAtLeastOneDifferingLength )

		# Turn on preserve strand lengths option
		clumpNode.preserveStrandLengths.set( True )

		# With above option on the strand lengths should not be modified from original
		strandLengthsWithPreservation = TestUtilities.GetStrandLengths( hairShape )
		# Allow for 0.1% difference in length (0.05 is the smallest number that gives nonzero when rounded to one decimal place)
		self.assertSequenceAlmostEqual( originalStrandLengths, strandLengthsWithPreservation, places = 1 )

	def test_RandomClumpsAreCreatedOnlyAmongSelectedStrands( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( length = 5 )
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )

		# Create 5 clumps, each strand will be unchanged since it's in its own clump
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 5, 1 ) )

		# Clump 2 of the strands into one random clump
		clumpNode.clumpHairSelection.set( [1, 3], type = "Int32Array" )
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 1, 1 ) )

		# Both selected strands will have the same tip position if they were clumped together
		tipPositions = TestUtilities.GetTipPositions( guidesShape, useObjectCoordinates = True )
		TestUtilities.CheckPointsNearEqual( self, tipPositions[1], tipPositions[3] )

	def test_AttractToClosestPointOnStem( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( length = 5, pointsPerStrandCount = 3 )
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 1, 1 ) )

		verticesWithHairClumpValues = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		clumpNode.attractToClosestStemPoint.set( 1 )
		verticesWithStemClumpValues = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# On a plane there should be no difference in point positions
		TestUtilities.CheckPointsAllNearEqual( self, verticesWithHairClumpValues, verticesWithStemClumpValues )

	def test_PreserveStrandLengthKeepsClumpShape( self ):
		pointsPerStrandCount = 3

		# Give strands enough length to touch the stem with length constrained
		guidesShape = TestUtilities.AddGuidesToNewPlane( length = 2, pointsPerStrandCount = pointsPerStrandCount )
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 1, 1 ) )

		# Modify the clump amount curve such that it clumps 100%
		pm.setAttr( clumpNode + ".amountRamp[0].amountRamp_Interp", 0 )
		pm.setAttr( clumpNode + ".amountRamp[1].amountRamp_Interp", 0 )
		pm.setAttr( clumpNode + ".amountRamp[1].amountRamp_Position", 0.2 )
		pm.setAttr( clumpNode + ".amountRamp[1].amountRamp_FloatValue", 1.0 )
		pm.setAttr( clumpNode + ".amountRamp[2].amountRamp_Interp", 0 )

		# If everything is correct then the tip of the strands should touch the strand to which everything is clumped, which should be strand 0
		clumpNode.preserveStrandLengths.set( 1 )
		verticesWithPreserve = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		strand0XZCoordinates = [verticesWithPreserve[0][0], verticesWithPreserve[0][2]]

		for i in range(1, 3):
			self.assertSequenceAlmostEqual( strand0XZCoordinates, [verticesWithPreserve[i * pointsPerStrandCount + 2][0], verticesWithPreserve[i * pointsPerStrandCount + 2][2]] )

	def test_ClumpSingleStrandWithStrandPattern( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod=2, guideCount=20 )
		patternShape = pm.PyNode( pm.mel.OxAddBraidGuides() )

		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 2, 1 ) )

		# Add pattern to the clump node and add it to the list of global patterns
		pm.connectAttr( patternShape + '.worldHair', str( clumpNode ) + '.patterns[0]' )
		clumpNode.patternIndices.set( [0], type = "Int32Array" )

		# Changing guide count to 1 and evaluating should not cause a crash
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		guidesFromMesh.distribution.set( 6 )
		pm.mel.OxGetStrandCount( guidesShape )

	def test_ClumpSingleStrandWithStrandPattern2( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod=2, guideCount=100 )
		patternShape = pm.PyNode( pm.mel.OxAddBraidGuides() )

		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.ClumpNodeName ) )
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 5, 1 ) )

		# Add pattern to the clump node and add it to the list of global patterns
		pm.connectAttr( patternShape + '.worldHair', str( clumpNode ) + '.patterns[0]' )
		clumpNode.patternIndices.set( [0], type = "Int32Array" )

		# Changing guide count to 1 and evaluating should not cause a crash
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		guidesFromMesh.distribution.set( 6 )
		pm.mel.OxGetStrandCount( guidesShape )

	def test_UndoPatternAssignment( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod=2, guideCount=20 )
		patternShape = pm.PyNode( pm.mel.OxAddBraidGuides() )

		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.ClumpNodeName ) )
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 2, 1 ) )

		with MayaTest.UndoChunk():
			pm.connectAttr( patternShape + '.worldHair', str( clumpNode ) + '.patterns[0]' )
			pm.mel.OxGetStrandCount( guidesShape )

		# Undo
		pm.undo()
		pm.mel.OxGetStrandCount( guidesShape )

	def test_AttractToStemWithBraidPattern( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod = 4, guideCount = 20, length = 10, planeSize = 10, pointsPerStrandCount = 10 )
		patternShape = pm.PyNode( pm.mel.OxAddBraidGuides() )

		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 1, 1 ) )

		# Add pattern to the clump node and add it to the list of global patterns
		pm.connectAttr( patternShape + '.worldHair', str( clumpNode ) + '.patterns[0]' )
		clumpNode.patternIndices.set( [0], type = "Int32Array" )

		verticesWithoutAttractToStem = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		clumpNode.attractToClosestStemPoint.set( 1 )
		verticesWithAttractToStem = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Attract to stem should not have much difference
		TestUtilities.CheckPointsAllNearEqual( self, verticesWithoutAttractToStem, verticesWithAttractToStem )

	def test_FlyawaysDontChangeWhenHairOrderChanges( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( length = 5, rootGenerationMethod = 2 )
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0.5 )
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 1, 1 ) )

		initialVertices = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( guidesShape )

		# Do something to force hair root re-generation
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		guidesFromMesh.distribution.set( 3 )
		TestUtilities.GetVerticesInObjectCoordinatesByStrandId( guidesShape )
		guidesFromMesh.distribution.set( 2 )

		verticesAfterRootOrderChange = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( guidesShape )

		# If Flyaway hairs didn't change all hairs should be identical
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, initialVertices, verticesAfterRootOrderChange )

	def test_RemovePattern( self ):
		self.CreateClumpedHairWithPattern()

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		clumpNode = pm.ls( type = TestUtilities.ClumpNodeName )[0]

		# Remove the clump pattern and evaluate strands
		clumpNode.patternIndices.set( [], type = "Int32Array" )
		TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# If no crash happens we are good

	def test_ClumpsOnHairFromMeshStrips( self ):
		plane1 = pm.polyPlane( w = 5, h = 10, sx = 1, sh = 10 )
		pm.select( plane1 )
		hairShape = pm.PyNode( pm.mel.OxAddHairFromMeshStrips( [ plane1 ] ) )

		verticesBeforeClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Clump by guides, we should get a clump for each of the 4 guides
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )
		clumpNode.useUvSpace.set( 0 )

		# Clump all hairs into 2 random clumps
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 2, 1 ) )
		verticesAfterClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBeforeClumping, verticesAfterClumping, epsilon = 0.001 )

	def CreatePropagatedHairsWithClumper( self, sideCount = 1, clumpCount = 2 ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod = 6 )
		propagation = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.PropagationNodeName ) )
		propagation.generationMethod.set( 0 )
		pm.setAttr( propagation + ".count", 10 )
		propagation.verticesPerRootCount.set( 2 )
		propagation.sideCount.set( sideCount )
		propagation.lowRange.set( 0.2 )

		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )
		clumpNode.clumpCount.set( clumpCount )
		pm.mel.OxEditClumps( clumpNode, c = ( 0, clumpCount, 1 ) )

		return [guidesShape, clumpNode]

	def test_ClumpsOnPropagatedHairs( self ):
		guidesShape = self.CreatePropagatedHairsWithClumper()[0]
		tipPositions = self.GetUniqueTipPositions( guidesShape )

		# There should be one tip for the stem strand and two tips for clumped propagated hairs
		self.assertEqual( 3, len( tipPositions ) )

	def test_ClumpsOnPropagatedHairsWithMultipleSides( self ):
		fixture = self.CreatePropagatedHairsWithClumper( sideCount = 2, clumpCount = 4 )
		guidesShape = fixture[0]
		clumpNode = fixture[1]
		tipPositions = self.GetUniqueTipPositions( guidesShape )

		# There should be one tip for the stem strand and two tips on each side of for clumped propagated hairs
		self.assertEqual( 5, len( tipPositions ) )

		# No strand tip should change its X sign, because strands shouldn't be clumped to a clump stem which is on a different side of their stem strand
		clumpNode.amount.set( 0 )
		verticesBeforeClumping = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		clumpNode.amount.set( 1 )
		verticesAfterClumping = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		for i in range( 0, len( verticesBeforeClumping ) ):
			self.assertEqual( int( verticesBeforeClumping[i][0] > 0 ), int( verticesAfterClumping[i][0] > 0 ) )

	def test_ClumpsOnPropagatedHairsScenePersistence( self ):
		guidesShape = self.CreatePropagatedHairsWithClumper()[0]
		verticesBeforeSaving = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Reload the scene and test again, to ensure persistence
		filePath = pm.saveAs( 'temp.mb' )
		pm.openFile( filePath, force = True )

		verticesAfterLoading = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeSaving, verticesAfterLoading )

	def test_PreserveStrandLengthAnimationJitter( self ):
		cube = pm.polyCube( w = 10, h = 10, d = 10, sx = 1, sy = 1, sz = 1 )
		meshShape = cube[0]
		animatedVertexOffset = 3
		startTime = 1
		endTime = 20

		pm.select( meshShape + ".vtx[2] ", replace = True )
		pm.select( meshShape + ".vtx[3]", add = True  )
		pm.select( meshShape + ".vtx[4]", add = True  )
		pm.select( meshShape + ".vtx[5]", add = True  )

		pm.currentTime( startTime )
		pm.move( animatedVertexOffset, 0, 0, r = True )
		pm.setKeyframe()

		pm.currentTime( endTime )
		pm.move( - 2 * animatedVertexOffset, 0, 0, r = True )
		pm.setKeyframe()

		pm.select( cube )
		strandCount = 30
		pointsPerStrandCount = 10
		guidesShape = TestUtilities.AddGuidesToMesh( meshShape, guideCount = strandCount, pointsPerStrandCount = pointsPerStrandCount, rootGenerationMethod = 2, length = 5 )
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )
		clumpNode.preserveStrandLengths.set( 1 )
		#clumpNode.amount.set( 0.3 )
		pm.mel.OxEditClumps( clumpNode, c = ( 0, 1, 1 ) )

		pm.currentTime( startTime )
		oldVertices = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		for time in range( 1, 5 ):
			pm.currentTime( startTime + time )
			newVertices = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
			self.assertGreater( 1, TestUtilities.GetMaxPointDistance( self, oldVertices, newVertices ) )
			#print( TestUtilities.GetMaxPointDistance( self, oldVertices, newVertices ) )
			oldVertices = newVertices

	def test_UvBasedClumping( self ):
		# TODO: change the range end from 21 to 22 to get a failure for seed 21
		for randomSeed in RunTests.RandomSeeds( 20, 21 ):
			plane = pm.polyPlane( w = 1, h = 1, sx = 1, sy = 2 )
			meshShape = plane[0]
			xOffset = 0.6
			yOffset = 1.2
			pm.select( meshShape + ".vtx[4:5] ", replace = True )
			pm.move( 0, yOffset, xOffset, r = True )
			pm.select( meshShape + ".vtx[0:1] ", replace = True )
			pm.move( 0, yOffset, -xOffset, r = True )

			strandLength = 0.3
			guidesShape = TestUtilities.AddGuidesToMesh( meshShape, guideCount = 200, pointsPerStrandCount = 2, rootGenerationMethod = 2, length = strandLength )
			guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
			guidesFromMesh.randomSeed.set( randomSeed )
			clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.ClumpNodeName ) )
			clumpNode.flyawayFraction.set( 0 )
			clumpNode.useUvSpace.set( 1 )

			clumpCount = 20
			clumpNode.clumpCount.set( clumpCount )
			pm.mel.OxEditClumps( clumpNode, d = True, sl = 0 )
			pm.mel.OxEditClumps( clumpNode, c = ( 0, clumpCount, 1 ) )

			hairLengths = TestUtilities.GetStrandLengths( guidesShape )

			pm.select( meshShape + ".vtx[4:5] ", replace = True )
			pm.move( 0, -yOffset, -xOffset, r = True )
			pm.select( meshShape + ".vtx[0:1] ", replace = True )
			pm.move( 0, -yOffset, xOffset, r = True )

			hairLengthsAfterUnfold = TestUtilities.GetStrandLengths( guidesShape )

			allowedError = 0.25

			for i in range( 1, len( hairLengthsAfterUnfold ) ):
				self.assertLessEqual( hairLengthsAfterUnfold[i] - strandLength, allowedError,
							"strand {}: difference {}, allowed {}".format( i, hairLengthsAfterUnfold[i] - strandLength, allowedError ) )

	def test_ClumpingRespectsHairPartings( self ):
		meshShape = pm.polyCube( w = 10, h = 10, d = 10, sx = 1, sy = 1, sz = 1 )[0]
		hairShape = TestUtilities.AddHairToMesh( meshShape, hairViewportCount = 100 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.autoPart.set( 1 )

		tangentsBeforeClumping = TestUtilities.GetStrandTangents( hairShape, True )

		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )
		clumpNode.maximumClosestRegionClumpCandidates.set( 20 )
		clumpNode.respectHairParts.set( 1 )

		clumpCount = 20
		clumpNode.clumpCount.set( clumpCount )
		pm.mel.OxEditClumps( clumpNode, d = True, sl = 0 )
		pm.mel.OxEditClumps( clumpNode, c = ( 0, clumpCount, 1 ) )

		tangentsAfterClumping = TestUtilities.GetStrandTangents( hairShape, True )

		# If partings were respected then the tips of the hairs shouldn't have moved too far from their original positions
		TestUtilities.CheckPointsAllNearEqual( self, tangentsBeforeClumping, tangentsAfterClumping, epsilon = 1.4 )

	# Reproduces issue #5266
	def test_ReassignChannelCrash( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.amountChannel.set( 1 )

		# Evaluate
		pm.mel.OxGetStrandPoint( hairShape, 0, 1 )

		clumpNode.amountChannel.set( 0 )
		pm.mel.OxGetStrandPoint( hairShape, 0, 1 )

		# If no crash the test passes

	def test_ClumpSizeMultiplier( self ):
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod=2, strandCount=30 )

		# Add clumper generate some clumps.
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.clumpCount.set( 2 )

		# Enable clump size attributes and set clump size.
		clumpNode.setClumpSize.set( True )
		clumpNode.clumpSize.set( 10.0 )
		clumpNode.setAttr( "clumpSizeRamp[0].clumpSizeRamp_FloatValue", 1 )
		clumpNode.setAttr( "clumpSizeRamp[1].clumpSizeRamp_FloatValue", 1 )
		clumpNode.setAttr( "clumpSizeRamp[2].clumpSizeRamp_FloatValue", 1 )
		verticesBeforeMultiplier = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		'''
		Create a ramp texture which will define two partings. Leave the parting not exactly down the middle but a bit to the side
		so that hairs in the middle will belong to guides on the bigger side.
		'''
		color1 = [0, 0, 0]
		color2 = [1, 1, 1]
		rampTexture = pm.createNode( "ramp" )
		colorDivisionPosition = 0.6
		pm.setAttr( rampTexture + ".colorEntryList[0].position", 0.0 )
		pm.setAttr( rampTexture + ".colorEntryList[0].color", color1[0], color1[1], color1[2], type = "double3" )
		pm.setAttr( rampTexture + ".colorEntryList[1].position", colorDivisionPosition )
		pm.setAttr( rampTexture + ".colorEntryList[1].color", color2[0], color2[1], color2[2], type = "double3" )
		rampTexture.interpolation.set( 0 )
		pm.setAttr( rampTexture + ".type", 1 )

		# Connect ramp texture to clump size multiplier.
		pm.mel.connectNodeToAttrOverride( rampTexture, clumpNode + ".clumpSizeMultiplier" )
		pm.connectAttr( rampTexture + ".outColor", clumpNode + ".clumpSizeMultiplier" )
		verticesAfterMultiplier = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Check that the multiplier has changed the vertices.
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBeforeMultiplier, verticesAfterMultiplier )

	@unittest.skip( "Fails, see #6093" )
	def test_ChangeRampValue( self ):
		# Add hair and clump node
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 4 )
		TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.clumpCount.set( 1 )

		verticesAfterClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Adjust a ramp knot
		clumpNode.setAttr( "amountRamp[2].amountRamp_FloatValue", 0.2 )

		verticesAfterChangingRamp = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# If clumping happened then vertices should have changed
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesAfterClumping, verticesAfterChangingRamp )

	def test_ExternalGuidesAfterReevaluation( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod = 2, planeSize = 20 )

		# Add Edit Guides
		editGuidesShape = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.EditGuidesShapeName ) )

		hairFromGuides = pm.PyNode( pm.mel.OxAddStrandOperator( editGuidesShape, TestUtilities.HairFromGuidesNodeName ) )
		hairFromGuides.interpolation.set( 1 )
		hairFromGuides.distribution.set( 2 )
		hairFromGuides.renderCount.set( 100 )
		hairFromGuides.viewportCountFraction.set( 1 )

		# Add clump, set it to Guides mode and generate clumps
		clump = pm.PyNode( pm.mel.OxAddStrandOperator( hairFromGuides, TestUtilities.ClumpNodeName ) )
		clump.clumpCreateMethod.set( 2 )
		clump.flyawayFraction.set( 0 )
		pm.mel.OxEditClumps( clump, c = ( 2, 1, 1 ) )

		# Evaluate once
		pm.mel.OxGetStrandPoint( guidesShape, 0, 1 )

		# Turn off Edit Guides and evaluate
		pm.mel.OxEnableOperator( editGuidesShape, 0 )
		pm.mel.OxGetStrandPoint( guidesShape, 0, 1 )

		# Turn it back on and evaluate
		pm.mel.OxEnableOperator( editGuidesShape, 1 )
		pm.mel.OxGetStrandPoint( guidesShape, 0, 1 )