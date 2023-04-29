from pickle import TRUE
import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt
import os
import unittest
import random
import math

class Test_HairFromGuidesNode( MayaTest.OxTestCase ):

	""" We must register Ornatrix MEL pre and post render callbacks into scene's defaultRenderGlobals node so that dense hair
	will be rendered during render and viewport hair in interactive viewport """
	def test_PrePostMelCallbacksRegisteredOnNewScene( self ):
		# Callback should be installed when a new hair gets created, but since installation happens in on idle handler,
		# we need to manually install them here and not count on that!
		pm.newFile( force = 1 )

		pm.mel.OxInstallRenderCallbacks( True )

		self.assertIn( 'OxSetIsRendering(true)', pm.getAttr('defaultRenderGlobals.preMel') )
		self.assertIn( 'OxSetIsRendering(false)', pm.getAttr('defaultRenderGlobals.postMel') )

		pm.mel.OxInstallRenderCallbacks( False )

		self.assertEqual( '', pm.getAttr('defaultRenderGlobals.preMel') )
		self.assertEqual( '', pm.getAttr('defaultRenderGlobals.postMel') )

	def test_UniformDistribution( self ):
		hairShape = TestUtilities.AddHairToMesh( pm.polyPlane( sx = 1, sy = 1 )[0], rootGenerationMethod = 0, hairRenderCount = 10, hairViewportCount = 10 )

		strandCount = pm.mel.OxGetStrandCount( hairShape )
		self.assertGreaterEqual( strandCount, 9 )

	def test_HairFromGuidesParting( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		guidesFromMesh.distribution.set( 4 )
		hairFromGuides = TestUtilities.GetNodeByType( TestUtilities.HairFromGuidesNodeName )
		hairFromGuides.distribution.set( 0 )

		pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.EditGuidesShapeName )
		pm.select( "EditGuides1.ep[0:1]" )
		pm.move( 0, 0, 40, relative = True )
		tipPositionsBeforeParting = TestUtilities.GetTipPositions( hairShape, useObjectCoordinates = True )

		pm.select( "HairFromGuides1" )

		# Add two points to define a parting strip
		pm.mel.eval( "OxEditHairFromGuidesParting -a -1 -p 0 0.500893 0.489374;" )
		pm.mel.eval( "OxEditHairFromGuidesParting -a 0 -p 0 1.0 0.489374;" )
		tipPositionsAfterParting = TestUtilities.GetTipPositions( hairShape, useObjectCoordinates = True )

		# Hair tips should be different because of the parting
		self.assertEqual( False, ( tipPositionsBeforeParting == tipPositionsAfterParting ) )

	def test_UsesSameFacesAsInputGuides( self ):
		plane = pm.polyPlane( sx = 10, sy = 10 )
		pm.select( str(plane[0]) + ".f[0:40]" )
		pm.mel.OxQuickHair()

		# Delete then re-add HairFromGuidesNode
		pm.mel.OxDeleteStrandOperator( "HairFromGuides1" )
		pm.mel.OxAddStrandOperator( "EditGuides1", "HairFromGuidesNode" )

		# GuidesFromMesh and HairFromGuides should be using same selected faces
		self.assertEqual( pm.getAttr( "GuidesFromMesh1.inputFaceIndices" ),
			pm.getAttr( "HairFromGuides1.inputFaceIndices" ) )

		self.assertEqual( pm.getAttr( "GuidesFromMesh1.useFaceInclude" ),
			pm.getAttr( "HairFromGuides1.useFaceInclude" ) )

	def test_UsesSameDistributionMeshAsInputGuides( self ):
		plane1 = pm.polyPlane( sx = 1, sy = 1 )

		# Create a 2x2 face plane and delete 3 of the 4 faces
		plane2 = pm.polyPlane( sx = 2, sy = 2 )[0]

		pm.select( plane2 + ".e[1]" )
		curve1 = pm.polyToCurve( form = 2, degree = 3 )

		pm.select( plane2 + ".e[2]" )
		curve2 = pm.polyToCurve( form = 2, degree = 3 )

		pm.select( plane2 + ".e[3]" )
		curve3 = pm.polyToCurve( form = 2, degree = 3 )

		pm.select( clear = True )
		pm.select( curve1, add = True )
		pm.select( curve2, add = True )
		pm.select( curve3, add = True )

		pm.mel.OxAddGuidesFromCurves()
		pm.mel.OxAddStrandOperator( "", TestUtilities.GroundStrandsNodeName )
		groundStrandsNode = pm.ls( selection = True )[0]
		plane1[0].outMesh >> groundStrandsNode.distributionMesh
		groundStrandsNode.detachRoots.set( 0 )

		pm.mel.OxAddStrandOperator( groundStrandsNode, TestUtilities.HairFromGuidesNodeName )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]

		# Set per-vertex hair distribution
		hairFromGuides.distribution.set( 4 )

		# Set 'NClosestGuides' guide area calculation method as it doesn't remove any roots.
		hairFromGuides.guideArea.set( 0 )

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		# If hair is generated on plane1, as expected, there should only be four strands, one per each vertex
		self.assertEqual( 4, pm.mel.OxGetStrandCount( hairShape ) )

	def test_MinorGuideChangeCausesMinorInterpolationChanges( self ):
		plane = pm.polyPlane( w = 5, h = 5, subdivisionsWidth = 1, subdivisionsHeight = 1 )

		hairShape = pm.PyNode( pm.mel.OxQuickHair() )
		hairFromGuides = hairShape.inputHair.inputs()[0]
		hairFromGuides.viewportCountFraction.set( 20.0 / hairFromGuides.renderCount.get() )
		editGuidesShape = hairFromGuides.inputStrands.inputs()[0]
		guidesFromMesh = editGuidesShape.inputStrands.inputs()[0]
		guidesFromMesh.lengthRandomness.set( 0 )

		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		pm.select( editGuidesShape + ".ep[0]" )
		pm.move( 0, 0, 0.0001, relative = True )

		verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# We didn't modify the guides so much so the vertices should be pretty much the same
		self.assertEqual( len( verticesBefore ), len( verticesAfter ) )
		for vertexIndex in range( 0, len( verticesBefore ) ):
			vertexBefore =  verticesBefore[vertexIndex]
			vertexAfter = verticesAfter[vertexIndex]
			self.assertAlmostEqual( vertexBefore[0], vertexAfter[0], places = 3, msg = "index: {0}, old x: {1}, new x: {2}".format( vertexIndex, vertexBefore[0], vertexAfter[0] ) )
			self.assertAlmostEqual( vertexBefore[1], vertexAfter[1], places = 3, msg = "index: {0}, old y: {1}, new y: {2}".format( vertexIndex, vertexBefore[1], vertexAfter[1] ) )
			self.assertAlmostEqual( vertexBefore[2], vertexAfter[2], places = 3, msg = "index: {0}, old z: {1}, new z: {2}".format( vertexIndex, vertexBefore[2], vertexAfter[2] ) )

	# Tests that guide positions are updated when time changes if the distribution geometry is animated
	def test_HairUpdateOnAnimatedMesh( self ):
		plane = pm.polyPlane( sx = 1, sy = 1, w = 1, h = 1 )
		pm.select( plane )
		polyPlane = plane[1]

		# Keyframe plane between first and second frames
		pm.currentTime( 1 )
		pm.setKeyframe( polyPlane + ".width" )
		pm.setKeyframe( polyPlane + ".height" )

		pm.currentTime( 5 )
		pm.setAttr( polyPlane + ".width", 2 )
		pm.setAttr( polyPlane + ".height", 2 )
		pm.setKeyframe( polyPlane + ".width" )
		pm.setKeyframe( polyPlane + ".height" )

		pm.currentTime( 1 )
		hairShape = TestUtilities.AddHairToMesh( plane[0] )

		# Set 'NClosestGuides' guide area calculation method as it doesn't recompute roots
		# on mesh change as roots will be arbtrary reordered after recompution.
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.guideArea.set( 0 )

		tipPositionsAtFrame1 = TestUtilities.GetTipPositions( hairShape, True )

		# Hairs should move along with the deforming mesh
		pm.currentTime( 2 )
		tipPositionsAtFrame2 = TestUtilities.GetTipPositions( hairShape, True )
		TestUtilities.CheckPointsNotAllNearEqual( self, tipPositionsAtFrame1, tipPositionsAtFrame2 )

		# Hairs should move along with the deforming mesh
		pm.currentTime( 3 )
		tipPositionsAtFrame3 = TestUtilities.GetTipPositions( hairShape, True )
		TestUtilities.CheckPointsNotAllNearEqual( self, tipPositionsAtFrame2, tipPositionsAtFrame3 )

		pm.currentTime( 1 )
		tipPositionsAtFrame1Try2 = TestUtilities.GetTipPositions( hairShape, True )
		TestUtilities.CheckPointsAllNearEqual( self, tipPositionsAtFrame1, tipPositionsAtFrame1Try2 )

	def test_ParallelEvaluationOfTwoHairShapes( self ):
		plane1 = pm.polyPlane( sx = 10, w = 11, h = 11 )
		hairShape1 = pm.PyNode( pm.mel.OxQuickHair() )
		plane2 = pm.polyPlane( sx = 11, w = 10, h = 10 )
		hairShape2 = pm.PyNode( pm.mel.OxQuickHair() )
		pm.animation.setKeyframe( plane1[0], attribute = "tx", value = 10 )
		pm.animation.setKeyframe( plane1[0], attribute = "tx", value = 0, time = "1sec" )
		pm.animation.setKeyframe( plane2[0], attribute = "tx", value = -10 )
		pm.animation.setKeyframe( plane2[0], attribute = "tx", value = 0, time = "1sec" )
		pm.playbackOptions( maxTime = 25 )
		pm.animation.play( wait = True )
		# This should simply not crash

	def test_GuidesAsHairGeneration( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]

		guidesFromMesh.distribution.set( 4 )

		# Number of guides with normal distribution
		hairCount = pm.mel.OxGetStrandCount( hairShape )
		self.assertGreater( hairCount, 0 )

		# Set to guide as hair mode
		hairFromGuides.distribution.set( 5 )
		hairCount = pm.mel.OxGetStrandCount( hairShape )

		# We should have same number of hairs as guides
		self.assertEqual( 4, hairCount )

	def RememberRootPositionsBase( self, hairShape ):
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.renderCount.set( 100 )
		hairFromGuides.viewportCountFraction.set( 0.1 )

		# Random face area distribution
		hairFromGuides.distribution.set( 3 )

		## TODO: Uncommenting line below will cause caching to happen prematurey and twice
		#hairFromGuides.rememberRootPositions.set( 1 )
		#hairFromGuides.rememberRootPositions.set( 0 )

		# Note that we're using the remember root positions option not in render mode
		hairFromGuides.rememberRootPositions.set( 1 )

		# Put Maya into rendering mode so we get render versions of things
		pm.mel.OxSetIsRendering( True )
		renderHairCount = pm.mel.OxGetStrandCount( hairShape )
		surfaceDependencies = []
		for i in range( 0, renderHairCount ):
			surfaceDependencies.append( pm.mel.OxGetSurfaceDependency( hairShape, i ) )
		pm.mel.OxSetIsRendering( False )
		pm.mel.OxGetStrandCount( hairShape )

		# Evaluate to ensure roots are cached
		pm.mel.OxGetStrandCount( hairShape )
		# Changing distribution at this point shouldn't matter
		hairFromGuides.distribution.set( 4 )
		pm.mel.OxGetStrandCount( hairShape )

		# Save and reload
		filePath = pm.saveAs( self.addTempFile( 'temp.ma' ) )
		pm.openFile( filePath, force = True )

		try:
			hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]

			# Set rendering mode again so we get render version of the hair
			pm.mel.OxSetIsRendering( True )

			self.assertEqual( renderHairCount, pm.mel.OxGetStrandCount( hairShape ) )

			# All surface dependency info should be preserved and identical
			surfaceDependenciesAfterLoad = []
			for i in range( 0, renderHairCount ):
				surfaceDependenciesAfterLoad.append( pm.mel.OxGetSurfaceDependency( hairShape, i ) )

			self.assertEqual( surfaceDependencies, surfaceDependenciesAfterLoad )

		finally:
			pm.mel.OxSetIsRendering( False )
		return os.path.getsize( filePath )

	def test_RememberRootPositions( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		fileSize1 = self.RememberRootPositionsBase( hairShape )
		#print(fileSize1)

		# disable
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.rememberRootPositions.set( 0 )

		# try again using alembic file
		filePath = self.addTempFile( "RememberRootPositionsAbc.abc" )
		hairFromGuides.cacheFilePath.set( filePath )

		fileSize2 = self.RememberRootPositionsBase( hairShape )
		#print(fileSize2)

		# make sure also the new .ma file size is smaller
		self.assertGreater( fileSize1, fileSize2 )

	def test_RememberRootPositionsDeleteCacheFileAndReloadScene( self ):
		hairShape = TestUtilities.AddHairToNewPlane()

		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]

		cacheFilePath = self.addTempFile( "test_RememberRootPositionsDeleteCacheFileAndReloadScene.abc" )
		hairFromGuides.cacheFilePath.set( cacheFilePath )
		hairFromGuides.rememberRootPositions.set( 1 )

		# Evaluate to ensure roots are cached
		pm.mel.OxGetStrandCount( hairShape )

		# Save scene
		filePath = pm.saveAs( self.addTempFile( 'test_RememberRootPositionsDeleteCacheFileAndReloadScene.mb' ) )

		# Delete root cache file
		os.remove( cacheFilePath )

		# Reload the scene, there should be no crash
		pm.openFile( filePath, force = True )
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		pm.mel.OxGetStrandCount( hairShape )

	def test_RememberRootPositions2( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.renderCount.set( 100 )
		hairFromGuides.viewportCountFraction.set( 0.1 )

		# Random face area distribution
		hairFromGuides.distribution.set( 3 )
		hairFromGuides.rememberRootPositions.set( 1 )

		# Put Maya into rendering mode so we get render versions of things
		pm.mel.OxSetIsRendering( True )
		renderHairCount = pm.mel.OxGetStrandCount( hairShape )
		pm.mel.OxSetIsRendering( False )

		# Evaluate after rendering: hair count should be restored
		viewportHairCount = pm.mel.OxGetStrandCount( hairShape )

		self.assertNotEqual( renderHairCount, viewportHairCount )


	''' When root caching is enabled and we have cached root data modifying any distribution parameters should no longer
	re-generate roots or guide dependencies. This is true in both render and viewport preview mode since we're using the same roots in viewport, just a subset of them. '''
	def test_RememberRootPositionsDisablesRootRegeneration( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.renderCount.set( 100 )
		hairFromGuides.viewportCountFraction.set( 0.1 )

		# Set random face area distribution and turn on root caching
		hairFromGuides.distribution.set( 3 )
		hairFromGuides.rememberRootPositions.set( 1 )

		# Evaluate the hair, it should get us the cached results in viewport
		viewportHairCount = pm.mel.OxGetStrandCount( hairShape )
		viewportStrandVertices = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Grab the render output results as well
		pm.mel.OxSetIsRendering( True )
		renderHairCount = pm.mel.OxGetStrandCount( hairShape )
		renderStrandVertices = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		pm.mel.OxSetIsRendering( False )

		# Now modify the distribution parameters. This should not change the hairs or vertices in either viewport or render.
		hairFromGuides.distribution.set( 4 )
		hairFromGuides.renderCount.set( 200 )
		self.assertEqual( viewportHairCount, pm.mel.OxGetStrandCount( hairShape ) )
		TestUtilities.CheckPointsAllNearEqual( self, viewportStrandVertices, TestUtilities.GetVerticesInObjectCoordinates( hairShape ) )

		pm.mel.OxSetIsRendering( True )
		self.assertEqual( renderHairCount, pm.mel.OxGetStrandCount( hairShape ), )
		TestUtilities.CheckPointsAllNearEqual( self, renderStrandVertices, TestUtilities.GetVerticesInObjectCoordinates( hairShape ) )
		pm.mel.OxSetIsRendering( False )

	def test_RememberRootPositionsReloadWithDeformedMesh( self ):
		hairShape = TestUtilities.AddHairToNewPlane( planeSize = 10
											  # Note: Uncommenting the line below will result in test failing, this reproduces overall design issue #4877
											  #, guideRootGenerationMethod = 7, guideCount = 10
											  )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.renderCount.set( 10 )
		hairFromGuides.viewportCountFraction.set( 0.1 )
		# Random face area distribution
		hairFromGuides.distribution.set( 7 )

		# Remember roots and force evaluation
		hairFromGuides.rememberRootPositions.set( 1 )
		pm.mel.OxGetStrandCount( hairShape )

		# Move two of the base mesh vertices
		planeShape = pm.ls( type = "mesh" )[0]
		pm.select( planeShape + ".vtx[0:2]" )
		pm.move( 0, 0, 10, r = True )

		verticesBeforeReload = TestUtilities.GetVerticesInObjectCoordinatesSortedByStrandId( hairShape )

		TestUtilities.SaveAndReloadScene( self, 'test_RememberRootPositionsReloadWithDeformedMesh', True )

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		verticesAfterReload = TestUtilities.GetVerticesInObjectCoordinatesSortedByStrandId( hairShape )

		# Hair vertices should be preserved
		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeReload, verticesAfterReload )

	def test_UseEmptyVertexColorSetForHairDistribution( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		renderSettings = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.RenderSettingsNodeName ) )

		# Paint vertex color info on the two vertices of the plane
		plane = pm.ls( type = "mesh" )[0]
		pm.select( plane + '.vtx[0:3]' )
		pm.polyColorPerVertex( rgb = ( 0, 0, 0 ), cdo = True )

		meshFromStrands = pm.PyNode( pm.mel.OxAddStrandOperator( renderSettings, TestUtilities.MeshFromStrandsNodeName ) )
		hairMesh = pm.ls( type = "mesh" )[0]

		meshVerticesBeforeVertexColors = TestUtilities.GetMeshVertices( hairMesh )

		# Assign the vertex color channel to the length attribute
		hairFromGuides.distributionChannel.set( 2001 )

		#meshVerticesAfterVertexColors = TestUtilities.GetMeshVertices( hairMesh )

		self.assertEqual( 4 * 4, len( meshVerticesBeforeVertexColors ) )
		self.assertFalse( pm.attributeQuery( 'vtx', node = hairMesh, exists = True ) )

	def test_GuideFacesWithMultipleIterations( self ):
		# Use non-vertex root generation so we get some guides scattered on the plane
		hairShape = TestUtilities.AddHairToNewPlane( 20, 1, 2, 10, guideRootGenerationMethod = 1, guideCount = 4 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]

		guideCountWithoutGuideFaces = pm.mel.OxGetStrandCount( hairShape )
		hairFromGuides.useGuideFaces.set( True )

		# Set guide face iterations to value larger than one to cause iterative lookup of guides
		hairFromGuides.guideFacesIterationCount.set( 2 )
		guideCountWithGuideFaces = pm.mel.OxGetStrandCount( hairShape )

		# With guide faces on there should be fewer hairs
		self.assertLess( guideCountWithGuideFaces, guideCountWithoutGuideFaces )

	# When hair is created from guides it should inherit the channel values from the closest guides to each strand, including the strand group values
	def test_StrandGroupValuesInheritedFromGuides( self ):
		# Create some guides and assign strand group 1 to half of them
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane( pointsPerStrandCount = 2 )
		editGuidesShape.useStrandGroups.set( True )
		pm.select( editGuidesShape + ".ep[2:3]" )
		pm.mel.OxAssignStrandGroup( editGuidesShape, "1" )

		# Add hair from guides and make sure that there are more hairs than guides
		hairFromGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( editGuidesShape, TestUtilities.HairFromGuidesNodeName ) )
		hairFromGuidesNode.distribution.set( 0 )
		hairFromGuidesNode.viewportCountFraction.set( 20.0 / hairFromGuidesNode.renderCount.get() )

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		verticesBeforeLength = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Add strand length and set it to only apply to group 1
		lengthNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairFromGuidesNode, TestUtilities.LengthNodeName ) )
		lengthNode.value.set( 2 )
		lengthNode.sgp.set( "1" )

		verticesAfterLength = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# If all went well some hairs should have become longer than others
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBeforeLength, verticesAfterLength )

	# Same as test_StrandGroupValuesInheritedFromGuides but using guides as hair distribution
	def test_StrandGroupValuesInheritedFromGuidesWithGuidesAsHairDistribution( self ):
		self.test_StrandGroupValuesInheritedFromGuides()

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		# Switch to guides as hair distribution
		hairFromGuidesNode = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuidesNode.distribution.set( 5 )

		# Get the hair vertices with length operator turned off
		lengthNode = pm.ls( type = TestUtilities.LengthNodeName )[0]
		lengthNode.sgp.set( "2" )
		verticesWithoutLengthOperator = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		lengthNode.sgp.set( "1" )
		verticesWithLengthOperator = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Length should modify the hair differenly with and without strand groups
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithoutLengthOperator, verticesWithLengthOperator )

	# Reproduces a scenario where a user cached hair roots with 2 guide interpolation and saved the scene.
	# Reloading the scene should restore the correct interpolation settings (by default 3 guide interpolation is assigned)
	def test_CorrectGuideInterpolationAfterReloadingSceneWithCachedRoots( self ):
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 4 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]

		# Set interpolation to 2 guides using Affine method
		hairFromGuides.interpolationGuideCount.set( 2 )
		hairFromGuides.interpolation.set( 1 )

		# Make sure all hairs are displayed in the viewport and cache the roots
		hairFromGuides.viewportCountFraction.set( 1 )
		hairFromGuides.rememberRootPositions.set( 1 )

		# Evaluate the hair, it should get us the cached results in viewport
		viewportStrandVerticesBeforeSaving = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		TestUtilities.SaveAndReloadScene( self, 'test_CorrectGuideInterpolationAfterReloadingSceneWithCachedRoots', True )

		self.assertEqual( 1, len( pm.ls( type = TestUtilities.HairShapeName ) ) )

		# If everything opened correctly we should get the same hair as before saving
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		viewportStrandVerticesAfterLoading = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		TestUtilities.CheckPointsAllNearEqual( self, viewportStrandVerticesBeforeSaving, viewportStrandVerticesAfterLoading )

	def TestPartingMap( self, color1, color2 ):
		plane = pm.polyPlane( sx = 2, sy = 1, width = 10, height = 1 )
		pm.select( plane )
		meshShape = plane[0]

		# Add hair to plane, make sure we're using the vertex distribution for the hair and face center distribution for guides, so we'll have only two guides
		hairShape = TestUtilities.AddHairToMesh( meshShape, rootGenerationMethod = 4 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.interpolationGuideCount.set( 2 )
		hairFromGuides.interpolation.set( 1 )
		hairFromGuides.useGuideProximity.set( 0 )
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		guidesFromMesh.length.set( 2 )
		guidesFromMesh.lengthRandomness.set( 0 )
		guidesFromMesh.distribution.set( 6 )

		# Add surface comb node without randomness to spread the two guides away from one another
		surfaceComb = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.SurfaceCombNodeName ) )
		surfaceComb.chaos.set( 0 )
		surfaceComb.algorithm.set( 0 )

		# Make surface comb not completely flatten the guides, but leave them raised a little
		pm.setAttr( surfaceComb + ".slopeRamp[2].slopeRamp_FloatValue", 0.7 )

		# Create a ramp texture which will define the two partings. Leave the parting not exactly down the middle but a bit to the side so that hairs in the middle will belong to guides on the bigger side.
		rampTexture = pm.createNode( "ramp" )
		pm.setAttr( rampTexture + ".colorEntryList[0].position", 0.0 )
		pm.setAttr( rampTexture + ".colorEntryList[0].color", color1[0], color1[1], color1[2], type = "double3" )
		pm.setAttr( rampTexture + ".colorEntryList[1].position", 0.3 )
		pm.setAttr( rampTexture + ".colorEntryList[1].color", color2[0], color2[1], color2[2], type = "double3" )

		# Make ramp to be in step mode and set it to be in U direction
		rampTexture.interpolation.set( 0 )
		pm.setAttr( rampTexture + ".type", 1 )
		pm.mel.connectNodeToAttrOverride( rampTexture, hairFromGuides + ".partingGroupsMap" )
		pm.connectAttr( rampTexture + ".outColor", hairFromGuides + ".partingGroupsMap" )

		expectedTipPositions = {
			0: [-6.782012939453125, 0.9079810380935669, 0.5],
			1: [-1.782012939453125, 0.9079810380935669, 0.5],
			2: [6.782012939453125, 0.9079810380935669, 0.5],
			3: [-6.782012939453125, 0.9079810380935669, -0.5],
			4: [-1.782012939453125, 0.9079810380935669, -0.5],
			5: [6.782012939453125, 0.9079810380935669, -0.5]};

		tipPositions = TestUtilities.MakeDictionary( pm.mel.OxGetStrandIds( hairShape ), TestUtilities.GetTipPositions( hairShape, True ) )

		# If everything is correct all hairs will be interpolated from their closest guide in their parting and thus will have the shape of one of the two guides
		TestUtilities.AssertPointsAlmostEqualByKeys( self, expectedTipPositions, tipPositions )

		# Test again without using UV based partings
		hairFromGuides.interpolateGuidesInUvSpace.set( 0 )
		tipPositions = TestUtilities.MakeDictionary( pm.mel.OxGetStrandIds( hairShape ), TestUtilities.GetTipPositions( hairShape, True ) )
		TestUtilities.AssertPointsAlmostEqualByKeys( self, expectedTipPositions, tipPositions )

	def test_PartingMap( self ):
		self.TestPartingMap( [0, 0, 0], [1, 1, 1] )

	# Tests that parting map can have color values, not just greyscale
	def test_ColorPartingMap( self ):
		self.TestPartingMap( [1, 0, 0], [0, 1, 0] )

	def test_OneHairFromOneGuideUsingFaceCenterDistribution( self ):
		plane = pm.polyPlane( sx = 1, sy = 1, width = 1, height = 1 )
		hairShape = TestUtilities.AddHairToMesh( plane[0], rootGenerationMethod = 6, length = 10 )
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		guidesFromMesh.distribution.set( 6 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.guideArea.set( 0 ) # Set 'NClosestGuides' guide area as it allows one guide.
		hairFromGuides.interpolationGuideCount.set( 1 )

		# We are expecting to see one hair in the middle of the face
		vertices = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		self.assertEqual( 2, len( vertices ) )
		self.assertAlmostEqual( 0.0, dt.Vector( [0.0, 0.0, 0.0] ).distanceTo( vertices[0] ) )
		self.assertAlmostEqual( 0.0, dt.Vector( [0.0, 10.0, 0.0] ).distanceTo( vertices[1] ) )

	def test_GuideWeightsWithSegmentInterpolation( self ):
		# Create some guides with a Weights per-root channel and assign value of 1 for the last two guides
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane( pointsPerStrandCount = 2, guideLength = 1, lengthRandomness = 0 )
		pm.select( editGuidesShape )
		pm.mel.eval( TestUtilities.EditGuidesCommandName + " -crc 1 Weights;" )
		pm.mel.eval( TestUtilities.EditGuidesCommandName + " -rcd 2 4 0 0 0 0 0 0 1 1;" )

		# Add hair from guides and set the weights to be controlled by the Weights channel
		hairFromGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( editGuidesShape, TestUtilities.HairFromGuidesNodeName ) )
		hairFromGuidesNode.distribution.set( 4 )
		hairFromGuidesNode.viewportCountFraction.set( 1 )
		hairFromGuidesNode.guideWeightsChannel.set( 2 )
		hairFromGuidesNode.interpolationGuideCount.set( 2 )

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		# We shoul have all strands present and pointing up, despite two of the hairs being forced to be attracted to other guies
		vertices = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		self.assertAlmostEqual( 1, vertices[1][1] )
		self.assertAlmostEqual( 1, vertices[3][1] )
		self.assertAlmostEqual( 1, vertices[5][1] )
		self.assertAlmostEqual( 1, vertices[7][1] )

	def test_GuideWeightsWithSegmentInterpolation2( self ):
		# Create some guides with a Weights per-root channel and assign value of 1 for the last guide (ignoring others)
		guideLength = 1
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane( pointsPerStrandCount = 2, guideLength = guideLength, lengthRandomness = 0 )
		pm.select( editGuidesShape )
		pm.mel.eval( TestUtilities.EditGuidesCommandName + " -crc 1 Weights;" )
		pm.mel.eval( TestUtilities.EditGuidesCommandName + " -rcd 2 4 0 0 0 0 0 0 0 1;" )

		# Add surface comb operator to spread the hairs apart
		surfaceCombNode = pm.PyNode( pm.mel.OxAddStrandOperator( editGuidesShape, TestUtilities.SurfaceCombNodeName ) )
		surfaceCombNode.algorithm.set( 0 )

		# Add hair from guides and set the weights to be controlled by the Weights channel
		hairFromGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( surfaceCombNode, TestUtilities.HairFromGuidesNodeName ) )
		hairFromGuidesNode.distribution.set( 2 )
		hairFromGuidesNode.viewportCountFraction.set( 0.05 )
		hairFromGuidesNode.guideWeightsChannel.set( 2 )
		hairFromGuidesNode.interpolationGuideCount.set( 3 )
		hairFromGuidesNode.interpolation.set( 3 )

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		# No hair should be much longer than the source guides if interpolation is performed correctly. This will force each hair to only use one guide for interpolation.
		vertices = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		acceptableLengthError = 0.0001
		for i in range( 0, len( vertices ) // 2 ):
			self.assertLess( abs( dt.Vector( vertices[i * 2 + 1] ).distanceTo( vertices[i * 2] ) - guideLength ), acceptableLengthError )

	def test_StrandGroupsAreCorrectAferChangingDistributionType( self ):
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane( pointsPerStrandCount = 2, guideLength = 1, lengthRandomness = 0 )

		# Assigne strand group 1 to two guides
		pm.select( editGuidesShape )
		editGuidesShape.useStrandGroups.set( True )
		pm.select( editGuidesShape + ".ep[0]" )
		pm.select( editGuidesShape + ".ep[3]", add = True )
		pm.mel.OxAssignStrandGroup( editGuidesShape, "1" )

		hairFromGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( editGuidesShape, TestUtilities.HairFromGuidesNodeName ) )
		hairFromGuidesNode.distribution.set( 2 )
		hairFromGuidesNode.renderCount.set( 5 )
		hairFromGuidesNode.viewportCountFraction.set( 1 )

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		curler = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.CurlNodeName ) )
		curler.strandGroupPattern.set( "1" )
		curler.magnitude.set( 7 )

		# Evaluate hair at this time to force calculations
		TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Change hair distribution to something else
		hairFromGuidesNode.distribution.set( 3 )

		# Evaluate with and without strand group assigned. If strand groups were preserved then we should get different results.
		verticesWithStrandGroup = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		curler.strandGroupPattern.set( "2" )
		verticesWithoutStrandGroup = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithStrandGroup, verticesWithoutStrandGroup )

	def test_AddHairToMeshWithoutUvs( self ):
		# Turn off UVs on the plane mesh
		plane = pm.polyPlane( createUVs = False, sx = 1, sy = 1 )
		pm.select( plane )
		meshShape = plane[0]

		hairShape = TestUtilities.AddHairToMesh( meshShape, rootGenerationMethod = 4 )

		# Ensure hair is generated fine without UV
		self.assertEqual( 8, pm.mel.OxGetVertexCount( hairShape ) )

		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.interpolateGuidesInUvSpace.set( True )

		# Evaluating hair should not produce any errors even if UV space is used
		self.assertEqual( 0, pm.mel.OxGetVertexCount( hairShape ) )

		# Ensure that mesh generation works without errors too
		meshFromStrands = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.MeshFromStrandsNodeName ) )
		#self.assertEqual( 0, len( TestUtilities.GetMeshVertices( pm.ls( type = "mesh" )[0] ) ) )

	def test_RandomDistributionHairWithEvenDistributionGuides( self ):
		plane = pm.polyPlane()[0]

		# Initially set per-vertex guides distribution
		hairShape = TestUtilities.AddHairToMesh( plane, guideRootGenerationMethod = 4, rootGenerationMethod = 3 )
		pm.mel.OxGetVertexCount( hairShape )

		# Set guides distribution to even, this should not crash Maya
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		guidesFromMesh.distribution.set( 7 )
		pm.mel.OxGetVertexCount( hairShape )
		#pm.select( guidesFromMesh )

		#hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		#pm.mel.OxEnableOperator( hairFromGuides, False )
		#guidesShape = pm.ls( type = TestUtilities.GuidesShapeName )[0]
		#pm.mel.OxGetVertexCount( guidesShape )

		#pm.mel.OxEnableOperator( hairFromGuides, True )
		#hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		#pm.mel.OxGetVertexCount( hairShape )

	#def test_ViewportRootPositionsDontChangeOnRememberRoots( self ):
	#	plane = pm.polyPlane()[0]

	#	hairShape = TestUtilities.AddHairToMesh( plane, guideRootGenerationMethod = 4, rootGenerationMethod = 3 )
	#	pm.mel.OxGetVertexCount( hairShape )

	#	rootsBeforeRememberRoots = TestUtilities.GetRootPositions( hairShape )
	#	hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
	#	hairFromGuides.rememberRootPositions.set( 1 )

	#	rootsAfterRememberRoots = TestUtilities.GetRootPositions( hairShape )
	#	TestUtilities.CheckPointsAllNearEqual( self, rootsBeforeRememberRoots, rootsAfterRememberRoots )

	def SetupComputePerformance( self ):
		self.hairShape = TestUtilities.AddHairToNewSphere( strandCount = 50000, sphereSegmentCount = 8, pointsPerStrandCount = 10, guideLength = 1, guideCount = 1500, rootGenerationMethod = 2 )
		hfg = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		gravity = pm.PyNode( pm.mel.OxAddStrandOperator( self.hairShape, TestUtilities.GravityNodeName ) )
		gravity.strandGroupPattern.set( '123' )
		pm.currentTime( 1 )
		gravity.force.set( 1 )
		pm.setKeyframe()
		pm.currentTime( 10 )
		gravity.force.set( 0 )
		pm.setKeyframe()
		hfg = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hfg.renderCount.set( 50000 )

	def test_ComputePerformance( self ):
		self.assertPerformance( 'MayaTest.Self.SetupComputePerformance()', 'MayaTest.RunAnimation( MayaTest.Self.hairShape, frameCount = 5 )' )

	@staticmethod
	def SetRenderCountKeys( hfg, steps = 10 ):
		pm.currentTime( 1 )
		hfg.renderCount.set( 5000 )
		pm.setKeyframe( 'HairFromGuides1.renderCount' )
		pm.currentTime( steps )
		hfg.renderCount.set( 10000 )
		pm.setKeyframe( 'HairFromGuides1.renderCount' )

	def SetupRootGenerationPerformance( self ):
		self.hairShape = TestUtilities.AddHairToNewSphere( strandCount = 5000, sphereSegmentCount = 8, pointsPerStrandCount = 10, guideLength = 1, guideCount = 1500, rootGenerationMethod = 2 )
		hfg = pm.ls( type = 'HairFromGuidesNode' )[0]
		checker = pm.shadingNode( "checker", asTexture=True )
		pm.connectAttr( checker.outColor, hfg.distributionMultiplier )
		self.SetRenderCountKeys( hfg )

	def SetupEvenRootGenerationPerformance( self ):
		self.hairShape = TestUtilities.AddHairToNewPlane( planeSize = 10, planeSegmentCount = 10, strandCount = 5000, length = 1, rootGenerationMethod = 7 )
		hfg = pm.ls( type = 'HairFromGuidesNode' )[0]
		self.SetRenderCountKeys( hfg, 5 )

	def test_RootGenerationPerformance( self ):
		self.assertPerformance( 'MayaTest.Self.SetupRootGenerationPerformance()', 'MayaTest.RunAnimation( MayaTest.Self.hairShape )' )

	def test_EvenRootGenerationPerformance( self ):
		self.assertPerformance( 'MayaTest.Self.SetupEvenRootGenerationPerformance()', 'MayaTest.RunAnimation( MayaTest.Self.hairShape )' )

	def test_HairFromPropagatedGuides( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( pointsPerStrandCount = 3 )

		guideVertices = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		propagationNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.PropagationNodeName ) )
		propagationNode.verticesPerRootCount.set( 2 )

		hairFromGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.HairFromGuidesNodeName ) )
		hairFromGuidesNode.distribution.set( 4 )
		hairFromGuidesNode.viewportCountFraction.set( 1.0 )

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		self.assertEqual( 4, pm.mel.OxGetStrandCount( hairShape ) )

		hairVertices = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# If all went well guide vertices and hair vertices should be the same since propagated guide strands are skipped and not considered for interpolation
		TestUtilities.CheckPointsAllNearEqual( self, guideVertices, hairVertices )

	def test_HairFromCurveGuidesAtZeroTextureCoordinate( self ):
		plane = pm.polyPlane( w=10, h=10, subdivisionsWidth = 1, subdivisionsHeight = 1 )

		curve1 = pm.curve( p=[(10, 0, 10), (10, 2.5, 10), (10, 7.5, 10), (10, 10, 10)], k=[0, 0, 0, 1, 1, 1] )
		curve2 = pm.curve( p=[(10, 0, 10), (10, 2.5, 10), (10, 7.5, 10), (10, 10, 10)], k=[0, 0, 0, 1, 1, 1] )
		curve3 = pm.curve( p=[(10, 0, 10), (10, 2.5, 10), (10, 7.5, 10), (10, 10, 10)], k=[0, 0, 0, 1, 1, 1] )

		pm.select( clear = True )
		pm.select( curve1, add = True )
		pm.select( curve2, add = True )
		pm.select( curve3, add = True )
		guidesFromCurves = pm.mel.OxAddGuidesFromCurves()

		# Ground the guides
		groundStrandsNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromCurves, TestUtilities.GroundStrandsNodeName ) )
		plane[0].outMesh >> groundStrandsNode.distributionMesh
		groundStrandsNode.detachRoots.set( 0 )

		hairFromGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromCurves, TestUtilities.HairFromGuidesNodeName ) )
		# Set Even distribution with Affine interpolation
		hairFromGuidesNode.distribution.set( 4 )
		hairFromGuidesNode.interpolation.set( 1 )
		hairFromGuidesNode.renderCount.set( 10 )
		hairFromGuidesNode.viewportCountFraction.set( 1 )

		# Make sure that all vertices are valid (we don't have any NaN vertices)
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		vertices = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		for i in range( 0, len( vertices ) ):
			for j in range( 0, 3 ):
				self.assertGreaterEqual( vertices[i][j], -10 )
				self.assertLessEqual( vertices[i][j], 11 )

	def test_GeneratePerVertexData( self ):
		# Add guides and generate a new per-vertex data channel on them
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		generateStrandDataNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.GenerateStrandDataNodeName ) )
		generateStrandDataNode.newChannelType.set( 1 )
		generateStrandDataNode.newChannelName.set( "TestChannel" )
		generateStrandDataNode.generationMethod.set( 0 )
		generateStrandDataNode.minimumTargetValue.set( 2.0 )

		# Add hair from guides
		hairFromGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.HairFromGuidesNodeName ) )

		# By default per-vertex channels are not generated
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		self.assertEqual( None, pm.mel.OxGetVertexChannels( hairShape ) )

		# Turning on "Generate Per-Vertex Data" option should pass the guides channel data to hair
		hairFromGuidesNode.generatePerVertexData.set( True )
		hairFromGuidesNode.distribution.set( 6 )
		self.assertEqual( [u'TestChannel'], pm.mel.OxGetVertexChannels( hairShape ) )

		# All per-vertex values should be 2.0 in hair. We don't test whether they were interpolated correctly, but maybe that should be tested separately.
		self.assertEqual( [2.0, 2.0], pm.mel.OxGetVertexValuesForChannel( guidesShape, 0 ) )

	def test_DisplayHairWithoutUvCoordinates( self ):
		hairShape = TestUtilities.AddHairToNewPlane()

		# Turn off base UV generation
		plane = pm.ls( type = "polyPlane" )[0]
		plane.createUVs.set( 0 )

		# Make sure we have enough guides so that their ids are sufficiently large
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		guidesFromMesh.distribution.set( 2 )
		pm.setAttr( "%s.%s" % ( guidesFromMesh.name(), "count" ), 100 )

		# Turn off interpolation by UVs
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.interpolateGuidesInUvSpace.set( 0 )
		hairFromGuides.guideAreaCircleThroughNearest.set( 0 )

		# Add mesh from strands to force evaluation of texture coordinates
		pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.MeshFromStrandsNodeName ) )
		TestUtilities.GetMeshVertices( pm.ls( type = "mesh" )[0] )

	def test_GuideProximity( self ):
		hairShape = TestUtilities.AddHairToNewSphere( radius = 50, rootGenerationMethod = 2, strandCount = 20 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]

		hairFromGuides.useGuideProximity.set( 1 )
		hairFromGuides.guideProximityDistance.set( 7 )

		# If everything is properly recalculated then vertices should be sticking out along the mesh normals
		vertices = pm.mel.OxGetVertices( hairShape )

		for i in range( 0, pm.mel.OxGetStrandCount( hairShape ) ):
			directionVector = dt.Vector( vertices[i * 6 + 3], vertices[i * 6 + 4], vertices[i * 6 + 5] ) - dt.Vector( vertices[i * 6], vertices[i * 6 + 1], vertices[i * 6 + 2] )
			directionVector.normalize()
			error = 1 - directionVector.dot( [0, 0, 1] )
			self.assertLessEqual( error, 0.1 )

	def test_IncreasePointCountAfterPlantingGuides( self ):
		testScenePath = os.path.dirname( os.path.realpath( __file__ ) ) + "\\test_IncreasePointCountAfterPlantingGuides.ma"
		pm.openFile( testScenePath, force = True )
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]

		# This should not cause a crash
		pm.setAttr( "%s.%s" % ( guidesFromMesh.name(), "pointCount" ), 40 )

		# Dummy evaluator
		pm.mel.OxGetStrandCount( pm.ls( type = TestUtilities.HairShapeName )[0] )

	def test_WidthWithHFGAndRememberRoots( self ):
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 0 )
		changeWidth = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ChangeWidthNodeName ) )
		checkerTexture = pm.createNode( "checker" )
		pm.mel.connectNodeToAttrOverride( checkerTexture, changeWidth + ".widthMultiplier" )
		pm.connectAttr( checkerTexture + ".outColor", changeWidth + ".widthMultiplier" )

		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.rememberRootPositions.set( 1 )

		widthsBeforeReload = pm.mel.OxGetWidths( hairShape )
		filePath = pm.saveAs( 'temp.mb' )
		pm.openFile( filePath, force = True )

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		widthsAfterReload = pm.mel.OxGetWidths( hairShape )

		self.assertSequenceAlmostEqual( widthsBeforeReload, widthsAfterReload )

	def test_ZeroHairTriangle( self ):
		segmentCount = 10
		hairShape = TestUtilities.AddHairToNewPlane(
			strandCount = 400,
			rootGenerationMethod = 2,
			pointsPerStrandCount = 2,
			planeSegmentCount = segmentCount,
			guideRootGenerationMethod = 4,
			guideCount = 10,
			planeSize = 1 )

		renderHairCount = pm.mel.OxGetStrandCount( hairShape )
		hairedFaceIndices = []
		for i in range( 0, renderHairCount ):
			hairedFaceIndices.append( int(pm.mel.OxGetSurfaceDependency( hairShape, i )[0]) )

		polygonCount = segmentCount * segmentCount
		hairedPolygonCount = len( set( hairedFaceIndices ) )

		self.assertEqual( polygonCount, hairedPolygonCount )

	def test_GuidesAsHairProducesTexCoordinates( self ):
		# distribution 5 is guides as hair
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 5 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]

		textureCoordinates = pm.mel.OxGetTextureCoordinates( hairShape, 0, type=2 )

		expectedTextureCoordinates = TestUtilities.MakeDictionaryWithIncrementingKeys( [[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [1.0, 1.0, 0.0]] )
		actualTextureCoordinates = TestUtilities.MakeDictionary( pm.mel.OxGetStrandIds( hairShape ), TestUtilities.MakeVector3Array( pm.mel.OxGetTextureCoordinates( hairShape, 0, type=2 ) ) )
		TestUtilities.AssertEqualByKeys( self, expectedTextureCoordinates, actualTextureCoordinates )

	def test_GuideProximityTextureCoordinates( self ):
		hairShape = TestUtilities.AddHairToNewPlane( strandCount = 100, rootGenerationMethod = 2, pointsPerStrandCount = 2, planeSegmentCount = 1, guideRootGenerationMethod = 4, length = 1, planeSize = 1 )

		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.interpolation.set( 1 )
		hairFromGuides.useGuideProximity.set( 1 )
		hairFromGuides.guideProximityDistance.set( 0.11 )
		textureCoordinates = pm.mel.OxGetTextureCoordinates( hairShape, 0, type = 2 )
		rootPositions = TestUtilities.GetRootPositions( hairShape )

		# The guides should be close enough to each corner of the place to have coordinates of that corner
		for i in range( 0, len( textureCoordinates ) // 3 ):
			offsetPosition = [rootPositions[i][0] + 0.5, -( rootPositions[i][2] - 0.5 ), 0]
			textureVector = [textureCoordinates[i * 3], textureCoordinates[i * 3 + 1], textureCoordinates[i * 3 + 2]]
			self.assertLess( dt.Vector( offsetPosition ).distanceTo( textureVector ), 0.00005 )

	@unittest.skipIf( MayaTest.OsIsMac, "Crashes on the Mac for an unknown reason (build# 7710). TODO: need to figure it out somehow, users may run into bugs" )
	def test_MultiplyDivideTextureUsedAsMultiplierMap( self ):
		hairShape = TestUtilities.AddHairToNewSphere( strandCount = 5000, sphereSegmentCount = 8, pointsPerStrandCount = 10, guideLength = 1, guideCount = 1500, rootGenerationMethod = 2 )
		hfg = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		checker = pm.shadingNode( "checker", asTexture=True )
		multiplydivider = pm.createNode( 'multiplyDivide' )
		pm.connectAttr( checker.outColor, multiplydivider + '.input1' )
		pm.setAttr( multiplydivider + '.input2', 1.0, 10.0, 1.0, type = "double3" )
		pm.connectAttr( multiplydivider + '.output', hfg.distributionMultiplier )
		strandCount = pm.mel.OxGetStrandCount( hairShape )
		self.assertTrue( 2450 < strandCount and strandCount < 2550, 'strandCount = ' + str( strandCount ) )

	def test_GuidesAsHairModeOutputsHairData( self ):
		hairShape = TestUtilities.AddHairToMesh( pm.polyPlane( sx = 1, sy = 1 )[0], rootGenerationMethod = 5 )
		self.assertEqual( 'HairData', hairShape.inputHair.get( type = True ) )

	def test_GuidesWithDistributionChannel( self ):
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane( pointsPerStrandCount = 2 )
		pm.select( editGuidesShape + ".ep[2]" )
		activeGuideId = pm.mel.OxGetSelectedStrandIds( editGuidesShape )[0]
		activeGuideLength = 23.5
		pm.mel.OxEditGuides( activeGuideId, 2, 0.0, 0.0, 0.0, 0.0, 0.0, activeGuideLength - 10.0, eg = True )

		hairFromGuides = pm.PyNode( pm.mel.OxAddStrandOperator( editGuidesShape, TestUtilities.HairFromGuidesNodeName ) )
		hairFromGuides.distribution.set( 3 ) # Random face distribution
		hairFromGuides.guideArea.set( 0 ) # N closest guides
		hairFromGuides.interpolationGuideCount.set( 1 )
		hairFromGuides.distributionChannel.set( 1 ) # Selection

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		strandCount = pm.mel.OxGetStrandCount( hairShape )
		for strandIndex in range( strandCount ):
			strandPoints = pm.mel.OxGetStrandPoints( hairShape, strandIndex )
			self.assertEqual( strandPoints[:-1], [0.0] * 5 )
			strandLength = strandPoints[-1]
			self.assertEqual( strandLength, activeGuideLength )

	def test_IsolateHairByStrandIds( self ):
		hairShape = TestUtilities.AddHairToNewPlane(
			strandCount = 1000,
			rootGenerationMethod = 2
			)
		hfg = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		# get ids for full render hair
		strandIds = pm.mel.OxGetStrandIds( hairShape )
		# force reduced viewport geometry
		hfg.viewportCountFraction.set( 0.1 )
		viewportCnt = pm.mel.OxGetStrandCount( hairShape )

		# test that viewport geometry is generated before setting isolated ids
		self.assertFalse( hfg.isolatedStrandIds.get() )
		self.assertFalse( hfg.evalRenderGeometry.get() )

		random.shuffle( strandIds )
		isolatedCnt = int( len( strandIds ) * 0.33 )
		isolatedIds = strandIds[:isolatedCnt]
		hfg.isolatedStrandIds.set( isolatedIds, type = "Int32Array" )

		# test that only isolated ids are generated
		strandIds = pm.mel.OxGetStrandIds( hairShape )
		self.assertTrue( len( strandIds ) == isolatedCnt )
		self.assertTrue( set( strandIds ) == set( isolatedIds ) )
		self.assertTrue( hfg.evalRenderGeometry.get() )

		# test that clearing isolated ids restores viewport geometry
		hfg.isolatedStrandIds.set( [], type = "Int32Array" )
		strandCnt = pm.mel.OxGetStrandCount( hairShape )

		self.assertFalse( hfg.isolatedStrandIds.get() )
		self.assertEqual( viewportCnt, strandCnt )
		self.assertFalse( hfg.evalRenderGeometry.get() )

	def test_IsolateHairIsolatedVisibilityRate( self ):
		hairShape = TestUtilities.AddHairToNewPlane(
			strandCount = 1000,
			rootGenerationMethod = 2
			)
		hfg = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		# test default value
		self.assertAlmostEqual( hfg.isolatedVisibilityRate.get(), 1.0 )

		# get ids for full render hair
		strandIds = pm.mel.OxGetStrandIds( hairShape )
		# force reduced viewport geometry
		hfg.viewportCountFraction.set( 0.1 )
		viewportCnt = pm.mel.OxGetStrandCount( hairShape )

		self.assertFalse( hfg.evalRenderGeometry.get() )

		random.shuffle( strandIds )
		isolatedCnt = int( len( strandIds ) * 0.33 )
		isolatedIds = strandIds[:isolatedCnt]
		hfg.isolatedStrandIds.set( isolatedIds, type = "Int32Array" )

		# test that only specified portion of isolated hair is generated
		isolatedIds = set( isolatedIds )
		prevIvr = hfg.isolatedVisibilityRate.get()
		prevStrandIds = pm.mel.OxGetStrandIds( hairShape )
		ivrValues = ( 0.66, 0.33, 0.0, 0.4, 0.7, 1.0, 0.7 )
		for ivr in ivrValues:
			hfg.isolatedVisibilityRate.set( ivr )
			strandIds = pm.mel.OxGetStrandIds( hairShape )
			self.assertTrue( ivr != 0 or strandIds is None )
			if ivr != 0.0:
				self.assertTrue( isolatedIds.issuperset( strandIds ) )
				self.assertAlmostEqual( len( strandIds ), math.floor( round( ivr * isolatedCnt, 6 ) ) )
				self.assertTrue( hfg.evalRenderGeometry.get() )
				# test that increasing/decreasing ivr only adds/subtracts ids to the set from previous step
				if prevIvr >= ivr:
					# increasing ivr should keep visible isolated strands that were already visible in viewport
					self.assertTrue( set( prevStrandIds ).issuperset( strandIds ) )
				else:
					# decreasing ivr should only subtract from previously visible isolated strands
					self.assertTrue( prevStrandIds is None or set( prevStrandIds ).issubset( strandIds ) )
			prevIvr = ivr
			prevStrandIds = strandIds

		# test that clearing isolated ids restores viewport geometry
		hfg.isolatedStrandIds.set( [], type = "Int32Array" )
		strandCnt = pm.mel.OxGetStrandCount( hairShape )

		self.assertFalse( hfg.isolatedStrandIds.get() )
		self.assertEqual( viewportCnt, strandCnt )
		self.assertFalse( hfg.evalRenderGeometry.get() )

	def test_IsolateHairCulledVisibilityRate( self ):
		hairShape = TestUtilities.AddHairToNewPlane(
			strandCount = 1000,
			rootGenerationMethod = 2
			)
		hfg = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		# test default value
		self.assertAlmostEqual( hfg.culledVisibilityRate.get(), 0.0 )

		# get ids for full render hair
		strandIds = pm.mel.OxGetStrandIds( hairShape )
		renderCnt = pm.mel.OxGetStrandCount( hairShape )

		# force reduced viewport geometry
		hfg.viewportCountFraction.set( 0.1 )
		viewportCnt = pm.mel.OxGetStrandCount( hairShape )

		self.assertFalse( hfg.evalRenderGeometry.get() )

		random.shuffle( strandIds )
		isolatedCnt = int( len( strandIds ) * 0.33 )
		isolatedIds = strandIds[:isolatedCnt]
		hfg.isolatedStrandIds.set( isolatedIds, type = "Int32Array" )

		# test that only specified portion of culled hair is generated
		culledCnt = renderCnt - isolatedCnt
		prevCvr = hfg.culledVisibilityRate.get()
		prevStrandIds = pm.mel.OxGetStrandIds( hairShape )
		cvrValues = ( 0.33, 0.66, 1.0, 0.7, 0.4, 0.0, 0.4 )
		for cvr in cvrValues:
			hfg.culledVisibilityRate.set( cvr )
			strandIds = pm.mel.OxGetStrandIds( hairShape )
			self.assertTrue( set( strandIds ).issuperset( isolatedIds ) )
			self.assertAlmostEqual( len( strandIds ) - isolatedCnt, math.floor( round( cvr * culledCnt, 6 ) ) )
			self.assertTrue( hfg.evalRenderGeometry.get() )
			# test that increasing/decreasing cvr only adds/subtracts ids to the set from previous step
			if prevCvr >= cvr:
				# increasing cvr should keep visible culled strands that were already visible in viewport
				self.assertTrue( set( prevStrandIds ).issuperset( strandIds ) )
			else:
				# decreasing cvr should only subtract from previously visible culled strands
				self.assertTrue( prevStrandIds is None or set( prevStrandIds ).issubset( strandIds ) )
			prevCvr = cvr
			prevStrandIds = strandIds

		# test that clearing isolated ids restores viewport geometry
		hfg.isolatedStrandIds.set( [], type = "Int32Array" )
		strandCnt = pm.mel.OxGetStrandCount( hairShape )

		self.assertFalse( hfg.isolatedStrandIds.get() )
		self.assertEqual( viewportCnt, strandCnt )
		self.assertFalse( hfg.evalRenderGeometry.get() )

	def TestRootsAreNotRegeneratedWhenMeshDeforms( self, hairShape ):
		planeShape = pm.ls( type = "mesh" )[0]

		# Animate all plane vertices
		pm.currentTime( 1 )
		pm.select( planeShape + ".vtx[0:3]" )
		pm.setKeyframe()
		pm.currentTime( 2 )
		pm.select( planeShape + ".vtx[0:1]" )
		pm.move( 0, 0, 10, r = True )
		pm.setKeyframe()
		pm.select( planeShape + ".vtx[2:3]" )
		pm.move( 0, 0, -10, r = True )
		pm.setKeyframe()

		pm.currentTime( 1 )
		strandCountAtTime0 = pm.mel.OxGetStrandCount( hairShape )
		pm.currentTime( 2 )
		strandCountAtTime1 = pm.mel.OxGetStrandCount( hairShape )

		# Animated mesh should not cause root count to change
		self.assertEqual( strandCountAtTime0, strandCountAtTime1 )

	def test_RootsAreNotRegeneratedWhenMeshDeformsCircle( self ):
		hairShape = TestUtilities.AddHairToNewPlane( strandCount = 100, rootGenerationMethod = 7, planeSize = 10, guideArea = 2 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]

		self.TestRootsAreNotRegeneratedWhenMeshDeforms( hairShape )

	def test_RootsAreNotRegeneratedWhenMeshDeformsBarycentric( self ):
		hairShape = TestUtilities.AddHairToNewPlane( strandCount = 100, rootGenerationMethod = 7, planeSize = 10, guideArea = 1 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]

		self.TestRootsAreNotRegeneratedWhenMeshDeforms( hairShape )

	def test_ViewportStrandIdsAreSubsetOfRenderIds( self ):
		renderCount = 16
		viewportCountFraction = 0.5
		# This problem appears only in Even distribution, maybe test all?
		hairShape = TestUtilities.AddHairToNewPlane( strandCount = renderCount, rootGenerationMethod = 7 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.textureGenerationMethod.set( 2 )

		self.assertEqual( renderCount, pm.mel.OxGetStrandCount( hairShape ) )
		hairFromGuides.viewportCountFraction.set( 1 )
		renderStrandIds = pm.mel.OxGetStrandIds( hairShape )

		hairFromGuides.viewportCountFraction.set( viewportCountFraction )
		viewportCount = pm.mel.OxGetStrandCount( hairShape )
		viewportStrandIds = pm.mel.OxGetStrandIds( hairShape )

		for strandId in viewportStrandIds:
			self.assertTrue( strandId in renderStrandIds )

	def test_FlatMapTexCoordinates( self ):
		strandCount = 4
		hairShape = TestUtilities.AddHairToNewPlane( strandCount = strandCount, rootGenerationMethod = 7 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.textureGenerationMethod.set( 2 )

		self.assertEqual( strandCount, pm.mel.OxGetStrandCount( hairShape ) )
		textureCoordinatesByStrandId = TestUtilities.GetTextureCoordinatesByStrandId( self, hairShape, channel=0, type=2 )
		sortedTextureCoordinates = []
		for strandId in sorted( textureCoordinatesByStrandId.keys() ):
			sortedTextureCoordinates.extend( textureCoordinatesByStrandId[strandId] )

		# Test render coordinates which must be evenly distributed; will not work for viewport fraction < 1
		referenceTextureCoordinates = [[0.0, 0.0, 0.0], [0.0, 0.5, 0.0], [0.5, 0.0, 0.0], [0.5, 0.5, 0.0], [0.0, 0.5, 0.0], [0.0, 1.0, 0.0], [0.5, 0.5, 0.0], [0.5, 1.0, 0.0]]
		self.assertSequenceEqual( sortedTextureCoordinates, referenceTextureCoordinates )

	def test_ZeroViewportCountFraction( self ):
		# This problem appears only in Even distribution, maybe test all random ones? (2, 3, 7)
		hairShape = TestUtilities.AddHairToNewPlane( strandCount = 16, rootGenerationMethod = 7 )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		hairFromGuides.viewportCountFraction.set( 1 )
		renderCount = pm.mel.OxGetStrandCount( hairShape )
		self.assertGreater( renderCount, 0 )

		hairFromGuides.viewportCountFraction.set( 0 )
		zeroFractionCount = pm.mel.OxGetStrandCount( hairShape )
		# Replace with the less stringent assert if viewport hair is required to have some strands
		#self.assertLess( zeroFractionCount, renderCount )
		self.assertEqual( zeroFractionCount, 0 )

	def test_NonExistentFileTextureAsDistributionMap( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		hairFromGuides = TestUtilities.GetNodeByType( TestUtilities.HairFromGuidesNodeName )

		fileTexture = pm.shadingNode( "file", asTexture = True, isColorManaged = True )
		place2dTexture = pm.shadingNode( "place2dTexture", asUtility = True )
		pm.connectAttr( place2dTexture.outUV, fileTexture.uv )
		pm.connectAttr( place2dTexture.outUvFilterSize, fileTexture.uvFilterSize )
		pm.connectAttr( fileTexture.outColor, hairFromGuides.distributionMultiplier )

		pm.setAttr( fileTexture + ".fileTextureName", "nonExistentTexture.png", type = "string" )

		self.assertEqual( 0, pm.mel.OxGetStrandCount( hairShape ) )

	def test_HairFromGuidesWithHairInput( self ):
		hairShape = TestUtilities.AddHairToMesh( pm.polyPlane( sx = 2, sy = 2 )[0], rootGenerationMethod = 5 )
		self.assertEqual( pm.mel.OxGetStrandCount( hairShape ), 9 )
		hfg2 = pm.mel.OxAddStrandOperator( "HairFromGuides1", "HairFromGuidesNode" )
		self.assertGreaterEqual( pm.mel.OxGetStrandCount( hairShape ), 700 )

	def test_UDIMDistributionMap( self ):
		plane = pm.polyPlane( sx = 1, sy = 1 )
		pm.polyEditUV( "pPlane1.map[1]", scaleU = 2.0, scaleV = 1.0 )
		hairShape = TestUtilities.AddHairToMesh( plane[0], rootGenerationMethod = 0, hairRenderCount = 10, hairViewportCount = 10 )
		oldStrandCount = pm.mel.OxGetStrandCount( hairShape )
		hairFromGuides = TestUtilities.GetNodeByType( TestUtilities.HairFromGuidesNodeName )

		fileTexture = pm.shadingNode( "file", asTexture = True, isColorManaged = True )
		place2dTexture = pm.shadingNode( "place2dTexture", asUtility = True )
		pm.connectAttr( place2dTexture.outUV, fileTexture.uv )
		pm.connectAttr( place2dTexture.outUvFilterSize, fileTexture.uvFilterSize )
		pm.connectAttr( fileTexture.outColor, hairFromGuides.distributionMultiplier )

		# Modify UVs and load image as UDIM.
		udimImage = self.findTestFile( "Ox_UDIM_Test_Texture_1001.tif" )
		pm.setAttr( fileTexture + ".fileTextureName", udimImage, type = "string" )
		pm.setAttr( fileTexture + ".uvTilingMode", 3 )

		newStrandCount = pm.mel.OxGetStrandCount( hairShape )
		self.assertNotEqual( oldStrandCount, newStrandCount )
