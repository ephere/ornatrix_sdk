import MayaTest
import TestUtilities
import pymel.core as pm
import os.path
import unittest
import shutil
import pymel.core.datatypes as dt

class Test_AnimationCacheNode( MayaTest.OxTestCase ):

	def CreateGuidesWithAnimatedLength( self, planeSegmentCount = 2, rootGenerationMethod = 4, guideCount = 10 ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = planeSegmentCount, rootGenerationMethod = rootGenerationMethod, guideCount = guideCount )
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		pm.animation.setKeyframe( guidesFromMesh, attribute = "length", value = 10 )
		pm.animation.setKeyframe( guidesFromMesh, attribute = "length", value = 20, time = 10 )

		return guidesShape

	def test_BasicAnimationCache( self ):
		guidesShape = self.CreateGuidesWithAnimatedLength()

		# Get the hair vertices at frame 1 and 10
		pm.currentTime( 1 )
		verticesAtFrame1 = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( guidesShape )
		pm.currentTime( 10 )
		verticesAtFrame10 = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( guidesShape )

		filePath = self.addTempFile( "BasicAnimationCache.abc" )

		# Export specific object
		pm.mel.OxAlembicExport( filePath, guidesShape, fromTime = 1, toTime = 10 )

		notAnimatedGuidesShape = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 2 )
		notAnimatedGuidesFromMesh = pm.PyNode( "GuidesFromMesh2" )
		animationCacheNode = pm.PyNode( pm.mel.OxAddStrandOperator( notAnimatedGuidesFromMesh, TestUtilities.AnimationCacheNodeName ) )
		animationCacheNode.filePath.set( filePath )

		# Get the loaded animated hair vertices at frame 1 and 10
		pm.currentTime( 1 )
		cachedVerticesAtFrame1 = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( notAnimatedGuidesShape )
		pm.currentTime( 10 )
		cachedVerticesAtFrame10 = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( notAnimatedGuidesShape )

		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesAtFrame1, cachedVerticesAtFrame1 )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesAtFrame10, cachedVerticesAtFrame10 )

	def test_OverwriteAnimationCache( self ):
		# generate animation with non standard generation method and strand count
		guidesShape = self.CreateGuidesWithAnimatedLength(rootGenerationMethod = 0, guideCount = 20)

		# Get the hair vertices at frame 1 and 10
		pm.currentTime( 1 )
		verticesAtFrame1 = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( guidesShape )
		pm.currentTime( 10 )
		verticesAtFrame10 = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( guidesShape )

		filePath = self.addTempFile( "BasicAnimationCache.abc" )

		# Export specific object
		pm.mel.OxAlembicExport( filePath, guidesShape, fromTime = 1, toTime = 10 )

		notAnimatedGuidesShape = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 2 )
		notAnimatedGuidesFromMesh = pm.PyNode( "GuidesFromMesh2" )
		animationCacheNode = pm.PyNode( pm.mel.OxAddStrandOperator( notAnimatedGuidesFromMesh, TestUtilities.AnimationCacheNodeName ) )
		animationCacheNode.filePath.set( filePath )
		animationCacheNode.overwriteInput.set( True )
		animationCacheNode.displayFraction.set( 1.0 )

		# Get the loaded animated hair vertices at frame 1 and 10
		pm.currentTime( 1 )
		cachedVerticesAtFrame1 = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( notAnimatedGuidesShape )
		pm.currentTime( 10 )
		cachedVerticesAtFrame10 = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( notAnimatedGuidesShape )

		# The data should still match
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesAtFrame1, cachedVerticesAtFrame1 )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesAtFrame10, cachedVerticesAtFrame10 )

	def test_RecordVertexAnimationToAlembic( self ):
		guidesShape = self.CreateGuidesWithAnimatedLength()
		animationCacheNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.AnimationCacheNodeName ) )

		pm.currentTime( 1 )
		verticesAtFrame1WithoutCaching = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		pm.currentTime( 10 )
		verticesAtFrame10WithoutCaching = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		pm.playbackOptions( minTime='0', maxTime='10', animationEndTime='10' )

		filePath = self.addTempFile( "RecordVertexAnimationToAlembic.abc" )

		animationCacheNode.filePath.set( filePath )
		animationCacheNode.importTextureCoordinates.set( True )

		# Recording with file path should record the simulation and create the file
		pm.mel.OxRecordAnimation( animationCacheNode )
		self.assertAlembicFileSize( filePath, 4446 )

		# If all keyframes are deleted now the animation on the guides should persist
		pm.cutKey( time = (0, 10) )
		pm.currentTime( 1 )
		verticesAtFrame1WithCaching = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		pm.currentTime( 10 )
		verticesAtFrame10WithCaching = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		TestUtilities.CheckPointsAllNearEqual( self, verticesAtFrame1WithoutCaching, verticesAtFrame1WithCaching )
		TestUtilities.CheckPointsAllNearEqual( self, verticesAtFrame10WithoutCaching, verticesAtFrame10WithCaching )

		# Reset the scene and try to load the same Alembic files into new guides
		pm.newFile( force = 1 )
		guidesShape = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 2 )
		animationCacheNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.AnimationCacheNodeName ) )
		animationCacheNode.filePath.set( filePath )
		pm.currentTime( 1 )
		verticesAtFrame1AfterLoading = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		pm.currentTime( 10 )
		verticesAtFrame10AfterLoading = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesAtFrame1WithoutCaching, verticesAtFrame1AfterLoading )
		TestUtilities.CheckPointsAllNearEqual( self, verticesAtFrame10WithoutCaching, verticesAtFrame10AfterLoading )

	def test_RecordVertexDataAnimationToAlembic( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 2 )
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		generateGuideData = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.GenerateStrandDataNodeName ) )
		generateGuideData.newChannelType.set( 1 )

		# Animate strand data generation with constant generation mode
		pm.animation.setKeyframe( generateGuideData, attribute = "minimumTargetValue", value = 0 )
		pm.animation.setKeyframe( generateGuideData, attribute = "minimumTargetValue", value = 1, time = 10 )

		# Ensure we get proper results
		pm.currentTime( 1 )
		# Arguments: strandIndex, pointIndex, channelIndex, channel type
		vertexDataAtFrame1 = pm.mel.OxGetPointData( guidesShape, 0, 1, 0, type = 1 )
		pm.currentTime( 10 )
		vertexDataAtFrame10 = pm.mel.OxGetPointData( guidesShape, 0, 1, 0, type = 1 )
		self.assertEqual( 0, vertexDataAtFrame1 )
		self.assertEqual( 1, vertexDataAtFrame10 )

		# Record the values to Alembic file
		animationCacheNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.AnimationCacheNodeName ) )
		filePath = self.addTempFile( "test_RecordVertexDataAnimationToAlembic.abc" )
		animationCacheNode.filePath.set( filePath )
		animationCacheNode.importTextureCoordinates.set( True )
		pm.mel.OxRecordAnimation( animationCacheNode )
		self.assertAlembicFileSize( filePath, 4059 )

		# Reset the scene and try to load the same Alembic files into new guides
		pm.newFile( force = 1 )
		guidesShape = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 2 )
		animationCacheNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.AnimationCacheNodeName ) )
		animationCacheNode.filePath.set( filePath )
		pm.currentTime( 1 )
		vertexDataAtFrame1AfterLoading = pm.mel.OxGetPointData( guidesShape, 0, 1, 0, type = 1 )
		pm.currentTime( 5 )
		vertexDataAtFrame5AfterLoading = pm.mel.OxGetPointData( guidesShape, 0, 1, 0, type = 1 )
		pm.currentTime( 10 )
		vertexDataAtFrame10AfterLoading = pm.mel.OxGetPointData( guidesShape, 0, 1, 0, type = 1 )
		self.assertEqual( 0, vertexDataAtFrame1AfterLoading )
		self.assertAlmostEqual( 0.417, vertexDataAtFrame5AfterLoading, places = 1 )
		self.assertAlmostEqual( 1, vertexDataAtFrame10AfterLoading, places = 4 )

		# Now try the same with different point count
		pm.newFile( force = 1 )
		guidesShape = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 2, pointsPerStrandCount = 3 )
		animationCacheNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.AnimationCacheNodeName ) )
		animationCacheNode.filePath.set( filePath )
		pm.currentTime( 5 )
		vertexDataAtFrame5Point0AfterLoading = pm.mel.OxGetPointData( guidesShape, 0, 0, 0, type = 1 )
		vertexDataAtFrame5Point1AfterLoading = pm.mel.OxGetPointData( guidesShape, 0, 1, 0, type = 1 )
		vertexDataAtFrame5Point2AfterLoading = pm.mel.OxGetPointData( guidesShape, 0, 2, 0, type = 1 )
		self.assertAlmostEqual( 0.417, vertexDataAtFrame5Point0AfterLoading, places = 1 )
		self.assertAlmostEqual( 0.417, vertexDataAtFrame5Point1AfterLoading, places = 1 )
		self.assertAlmostEqual( 0.417, vertexDataAtFrame5Point2AfterLoading, places = 1 )

	def ReloadRecordedAnimation1( self, sceneFilePath, animationFilePath, animationFilePath2 = '' ):
		guidesShape = self.CreateGuidesWithAnimatedLength( planeSegmentCount = 5, rootGenerationMethod = 3, guideCount = 10 )
		animationCacheNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.AnimationCacheNodeName ) )

		# Record the animated guides
		pm.playbackOptions( minTime='0', maxTime='10', animationEndTime='10' )
		animationCacheNode.filePath.set( animationFilePath )
		pm.mel.OxRecordAnimation( animationCacheNode )

		# Remove animation
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		pm.animation.setKeyframe( guidesFromMesh, attribute = "length", value = 0, time = 0 )
		pm.animation.setKeyframe( guidesFromMesh, attribute = "length", value = 0, time = 10 )

		resultContext = dict( verticesAtFrame1WithCaching=[], verticesAtFrame10WithCaching=[], filePath="" )

		# Capture the positions of strands
		pm.cutKey( time = (0, 10) )
		pm.currentTime( 1 )
		resultContext['verticesAtFrame1WithCaching'] = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( guidesShape )
		pm.currentTime( 10 )
		resultContext['verticesAtFrame10WithCaching'] = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( guidesShape )

		if len( animationFilePath2 ) > 0:
			animationCacheNode.filePath.set( animationFilePath2 )

		resultContext['filePath'] = pm.saveAs( sceneFilePath )

		return resultContext

	def ReloadRecordedAnimation2( self, resultContext ):
		# Reload the scene
		pm.openFile( resultContext['filePath'], force = True )

		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		pm.currentTime( 1 )
		verticesAtFrame1AfterLoading = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( guidesShape )
		pm.currentTime( 10 )
		verticesAtFrame10AfterLoading = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( guidesShape )

		# The vertices in object space should be equal and correctly assigned to the same strand ids
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, resultContext['verticesAtFrame1WithCaching'], verticesAtFrame1AfterLoading )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, resultContext['verticesAtFrame10WithCaching'], verticesAtFrame10AfterLoading )

	# Tests that reloading recorded animation works correctly on hair which could have differing strand ids due to multi-threading
	def test_ReloadRecordedAnimation( self ):
		context = self.ReloadRecordedAnimation1( 'temp.ma', self.addTempFile( 'test_ReloadRecordedAnimation.abc' ) )
		self.ReloadRecordedAnimation2( context )

	def CheckAllStrandsPointingAwayFromOrigin( self, strandsShape, allowedError = 0.6 ):
		vertices = TestUtilities.GetVerticesInObjectCoordinates( strandsShape )
		strandCount = pm.mel.OxGetStrandCount( strandsShape )
		for strandIndex in range( 0, strandCount ):
			strandDirection = dt.Vector( vertices[strandIndex * 2 + 1] ) - dt.Vector( vertices[strandIndex * 2] )
			strandDirection.normalize()

			expectedDirection = dt.Vector( vertices[strandIndex * 2] )
			expectedDirection.normalize()

			directionError = 1.0 - strandDirection.dot( expectedDirection )
			self.assertTrue( directionError <= allowedError, "Direction error: " + str( directionError ) )

	def test_RemappingOfHairRoots( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()

		# Add surface comb, temporarily, to spread the guides out
		surfaceComb = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.SurfaceCombNodeName ) )
		surfaceComb.algorithm.set( 0 )

		# Add animation cache and record just a single frame, to capture strand positions
		animationCacheNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.AnimationCacheNodeName ) )
		filePath = self.addTempFile( "test_ReloadRecordedAnimation.abc" )
		animationCacheNode.filePath.set( filePath )
		pm.mel.OxRecordAnimation( animationCacheNode, startTime = 0, endTime = 1 )

		# Remove surface comb since we want animation cache to keep the hair shape now
		pm.mel.OxDeleteStrandOperator( surfaceComb )

		# Increase guide count
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		guidesFromMesh.distribution.set( 2 )
		guidesFromMesh.c.set( 10 )

		# If everything is correct we should have all strands pointing away from the center, more or less
		self.CheckAllStrandsPointingAwayFromOrigin( guidesShape )

		# Save, reload and check again
		fileName = 'temp.mb'
		filePath = pm.saveAs( fileName )
		pm.openFile( filePath, force = True )

		guidesShape = TestUtilities.GetNodeByType( TestUtilities.HairShapeName )
		self.CheckAllStrandsPointingAwayFromOrigin( guidesShape )

	# Note: this test reproduces the bug only in GUI mode, not in batch
	def test_RecordHairWithNoChanges( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( rootGenerationMethod = 2 )

		animationCacheNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.AnimationCacheNodeName ) )
		filePath = self.addTempFile( "test_RecordHairWithNoChanges.abc" )
		animationCacheNode.filePath.set( filePath )

		pm.mel.OxGetStrandCount( guidesShape )

		# Uncommenting line below will prevent assertion
		# pm.playbackOptions( minTime='0', maxTime='5', animationEndTime='5' )

		pm.mel.OxRecordAnimation( animationCacheNode )

		# If no assertions happened we are good

		# Also test removing the operator
		pm.mel.OxDeleteStrandOperator( animationCacheNode )
		pm.mel.OxGetStrandCount( guidesShape )

	def test_FullPreviewDoesNotChangeHairs( self ):
		# Create hair and deform them a bit
		hairShape = TestUtilities.AddHairToNewPlane( planeSize = 10 )
		frizz = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.FrizzNodeName ) )
		frizz.amount.set( 10 )

		animationCacheNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.AnimationCacheNodeName ) )
		filePath = self.addTempFile( "test_FullPreviewDoesNotChangeHairs.abc" )
		animationCacheNode.filePath.set( filePath )

		pm.mel.OxRecordAnimation( animationCacheNode )

		# Test whether the vertices are the same with very small change of the display fraction
		animationCacheNode.displayFraction.set( 0.99 )
		verticesAtAlmostFullPreview = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		animationCacheNode.displayFraction.set( 1 )
		verticesAtFullPreview = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		TestUtilities.CheckPointsAllNearEqual( self, verticesAtAlmostFullPreview, verticesAtFullPreview )

	def test_RelativeFilePath( self ):
		dir1Path = self.addTempFile( 'dir1' )
		dir2Path = self.addTempFile( 'dir2' )
		if not os.path.isdir( dir1Path ):
			os.mkdir( dir1Path )
		if not os.path.isdir( dir2Path ):
			os.mkdir( dir2Path )

		filePath = self.addTempFile( 'dir1/test_RelativeFilePath.ma' )
		animationFilePath = self.addTempFile( 'dir1/test_ReloadRecordedAnimation.abc' )

		# Make sure to also change the animation file path to a relative one (to the scene)
		context = self.ReloadRecordedAnimation1( filePath, animationFilePath, 'test_ReloadRecordedAnimation.abc' )

		# Move the files to a different location
		newFilePath = self.addTempFile( "dir2/test_RelativeFilePath.ma" )
		newAnimationPath = self.addTempFile( 'dir2/test_ReloadRecordedAnimation.abc' )
		os.rename( context['filePath'], newFilePath )
		os.rename( animationFilePath, newAnimationPath )
		os.rmdir( dir1Path )

		context['filePath'] = newFilePath

		self.ReloadRecordedAnimation2( context )

		shutil.rmtree( dir2Path )
