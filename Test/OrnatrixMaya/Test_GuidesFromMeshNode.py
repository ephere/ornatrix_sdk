import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt

import unittest
import math

class Test_GuidesFromMeshNode( MayaTest.OxTestCase ):

	# Tests that guide positions are updated when time changes if the distribution geometry is animated
	def test_GuidesUpdateOnAnimatedMesh( self ):
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
		guidesShape = TestUtilities.AddGuidesToMesh( plane[0] )
		tipPositionsAtFrame1 = TestUtilities.GetTipPositions( guidesShape, True )

		# Guides should move along with the deforming mesh
		pm.currentTime( 2 )
		tipPositionsAtFrame2 = TestUtilities.GetTipPositions( guidesShape, True )
		TestUtilities.CheckPointsAllNotNearEqual( self, tipPositionsAtFrame1, tipPositionsAtFrame2 )

		# Guides should move along with the deforming mesh
		pm.currentTime( 3 )
		tipPositionsAtFrame3 = TestUtilities.GetTipPositions( guidesShape, True )
		TestUtilities.CheckPointsAllNotNearEqual( self, tipPositionsAtFrame2, tipPositionsAtFrame3 )

		pm.currentTime( 1 )
		tipPositionsAtFrame1Try2 = TestUtilities.GetTipPositions( guidesShape, True )
		TestUtilities.CheckPointsAllNearEqual( self, tipPositionsAtFrame1, tipPositionsAtFrame1Try2 )

	# Tests that we can detach roots from base mesh temporarily to allow changing the surface mesh topology
	def test_DetachChangeBaseMeshTopology( self ):
		plane = pm.polyPlane( sx = 1, sy = 1, w = 1, h = 1 )
		pm.select( plane )

		# Generate some random roots
		guidesShape = TestUtilities.AddGuidesToMesh( plane[0], rootGenerationMethod = 2 )
		guidesFromMeshNode = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		vertexPositionsBeforeDetach = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# After detaching we should no longer have any vertices being output
		guidesFromMeshNode.detachRoots.set( True )
		vertexPositionsAfterDetach = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		self.assertEqual( 0, len( vertexPositionsAfterDetach ) )

		# Change the topology of the plane
		plane[0].sx.set( 2 )
		plane[0].sy.set( 2 )

		# Reattach the guide roots. The vertex positions should be same as before they were detached
		guidesFromMeshNode.detachRoots.set( False )
		vertexPositionsAfterAttach = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		TestUtilities.CheckPointsAllNearEqual( self, vertexPositionsBeforeDetach, vertexPositionsAfterAttach )

	# Tests that if root caching is turned off while roots are detached, the roots will automatically be reattached first
	def test_TurnOffRootCachingWithoutReattachingRoots( self ):
		plane = pm.polyPlane( sx = 1, sy = 1, w = 1, h = 1 )
		pm.select( plane )

		# Generate some random roots
		guidesShape = TestUtilities.AddGuidesToMesh( plane[0], rootGenerationMethod = 2 )
		vertexPositions1 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		self.assertGreater( len( vertexPositions1 ), 0 )

		guidesFromMeshNode = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		guidesFromMeshNode.detachRoots.set( True )
		vertexPositions = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		self.assertEqual( 0, len( vertexPositions ) )

		# Turn off root caching without reattaching the roots first
		guidesFromMeshNode.rememberRootPositions.set( False )

		# If all is well we should have vertices properly generated
		vertexPositions3 = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		self.assertGreater( len( vertexPositions3 ), 0 )
		TestUtilities.CheckPointsMatch( self, vertexPositions1, vertexPositions3 )

	# Tests face-center distribution method
	def test_GuidesDistribution_FaceCenter( self ):
		plane = pm.polyPlane( sx = 1, sy = 1, w = 1, h = 1 )
		pm.select( plane )

		# rootGenerationMethod 5: Face Center
		guidesShape = TestUtilities.AddGuidesToMesh( plane[0], rootGenerationMethod = 6 )
		vertexPositions = TestUtilities.GetMeshVertices( plane[0] )
		self.assertGreater( len( vertexPositions ), 0 )

		faceCenter = []
		for i in range( 0, len( vertexPositions ) ):
			faceCenter += vertexPositions[i];

		faceCenter /= len( vertexPositions );

		rootPosition = TestUtilities.GetStrandPointPositionsAtIndex( guidesShape, 0, True )
		TestUtilities.CheckPointsAllNearEqual( self, faceCenter, dt.Point(rootPosition) )

	# Tests face-center distribution method on multiple faces
	def test_GuidesDistributionFaceCenterMultipleFaces( self ):
		plane = pm.polyPlane( sx = 10, sy = 10, w = 1, h = 1 )
		pm.select( plane )

		# rootGenerationMethod 5: Face Center
		guidesShape = TestUtilities.AddGuidesToMesh( plane[0], rootGenerationMethod = 6 )

		# If no errors happened we are good
		TestUtilities.GetStrandPointPositionsAtIndex( guidesShape, 0, True )

	# Tests even distribution method
	def test_GuidesDistributionEven( self ):
		myDelta = 0.2

		def filterPoints( points, minValue, maxValue, axis ):
			pointsNew = []
			for p in points:
				if p[axis] >= minValue and p[axis] <= maxValue:
					pointsNew.append( p )
			return pointsNew

		def averageDistanceDeviation( points ):
			distanceSum = 0.0
			distances = []
			for i in range( 0, len( points ) ):
				distance = 10000.0
				for j in range( 0, len( points ) ):
					if i != j:
						distanceNew = dt.Vector( points[i] ).distanceTo( points[j] )
						if distanceNew < distance:
							distance = distanceNew
				distanceSum += distance
				distances.append( distance )
			distanceAvg = distanceSum / len( points )			

			distanceDiffSum = 0.0
			for i in range( 0, len( points ) ):
				distanceDiffSum += ( distances[i] - distanceAvg ) ** 2

			return math.sqrt( distanceDiffSum / len( points ) )

		def check( value1, value2, msg ):
			#print( "{}: {} vs {}".format( msg, value1, value2 ) )
			self.assertAlmostEqual( value1, value2, delta=myDelta )

		plane = pm.polyPlane( sx=4, sy=4, w = 1.0, h = 10.0 )
		pm.select( plane )

		guidesShape = TestUtilities.AddGuidesToMesh( plane[0], rootGenerationMethod=2, guideCount=3000 )

		guidesFromMesh = pm.PyNode( pm.ls( type=TestUtilities.GuidesFromMeshNodeName )[0] )

		ramp = pm.shadingNode( "ramp", asTexture=True )
		pm.connectAttr( ramp.outColor, guidesFromMesh.distributionMap )

		# Because distribution methods are random based and we use statistical methods this check could fail 
		# for same seeds and/or for some platforms (we use relatively small strand count to not slowdown the test much).
		# Try to use different seed in such case.
		# Also increasing guideCount should help but this also will increase test time significantly.
		# The main problem is in averageDistanceDeviation because its time complexity is O(N^2).
		guidesFromMesh.randomSeed.set( 1 )

		# Switch to Random Area distribution
		guidesFromMesh.distribution.set( 2 )
		pointsRandom = TestUtilities.GetStrandPointPositionsAtIndex( guidesShape, 0, True )

		# Switch to Even distribution
		guidesFromMesh.distribution.set( 7 )
		pointsEven = TestUtilities.GetStrandPointPositionsAtIndex( guidesShape, 0, True )

		pointsRandom1 = filterPoints( pointsRandom, -5, 0, axis=2 )
		pointsRandom2 = filterPoints( pointsRandom, 0, 5 , axis=2)
		pointsRandom3 = filterPoints( pointsRandom, -5, -4.8, axis=2 )

		pointsEven1 = filterPoints( pointsEven, -5, 0, axis=2 )
		pointsEven2 = filterPoints( pointsEven, 0, 5 , axis=2)
		pointsEven3 = filterPoints( pointsEven, -5, -4.8, axis=2 )

		pointsEven3DistanceDeviation = averageDistanceDeviation( pointsEven3 )
		pointsRandom3DistanceDeviation = averageDistanceDeviation( pointsRandom3 )

		check( float( len( pointsRandom ) ) / len( pointsEven ), 1.0, "Is near the same point count as Random Area distribution" )

		check( float( len( pointsRandom1 ) ) / len( pointsEven1 ), 1.0, "Check average density in area1" )
		check( float( len( pointsRandom2 ) ) / len( pointsEven2 ), 1.0, "Check average density in area2" )

		check( pointsEven3DistanceDeviation / pointsRandom3DistanceDeviation, 0.35, "Check average radius deviation" )

	# Tests if guides are only generated on selected faces
	def GuidesGeneratedOnSelectedFaces( self, distributionMethod ):
		plane = pm.polyPlane( sx = 10, sy = 10, w = 1, h = 1 )
		pm.select( plane )

		meshShape = plane[0]
		selectedFaces = meshShape.f[ 0 : meshShape.f.count() - 1 : 6 ]
		pm.select( selectedFaces )

		self.assertGreater( len( selectedFaces ), 0 )

		guidesShape = pm.mel.OxQuickHair();
		# Get rid of all nodes that we don't need and could affect the result
		pm.mel.OxDeleteStrandOperator( "RenderSettings1" )
		pm.mel.OxDeleteStrandOperator( "HairFromGuides1" )
		pm.mel.OxDeleteStrandOperator( "EditGuides1" )

		guidesFromMesh = pm.PyNode( pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0] )
		pm.setAttr( "%s.%s" % ( guidesFromMesh.name(), "distribution" ), distributionMethod )

		# Get the set of selected triangles (triangle indices) corresponding to selected faces on the mesh
		selectedTriangles = set()
		for face in selectedFaces:
			for i in range( 0, face.numTriangles() ):
				# Here we assume each face on the distribution mesh has equal number of triangles
				selectedTriangles.add( face.index() * face.numTriangles() + i )

		# Check if face index from the guide's SurfaceDependency info is in our selected triangle set 
		for i in range( 0, pm.mel.OxGetStrandCount( guidesShape ) ):
			sdependency = pm.mel.OxGetSurfaceDependency( guidesShape, i )
			# Triangle index should be second entry in SurfaceDependency for the strand
			self.assertTrue( int( sdependency[1] ) in selectedTriangles )

	def test_GuidesGeneratedOnSelectedFacesUniform( self ):
		self.GuidesGeneratedOnSelectedFaces( 0 )

	def test_GuidesGeneratedOnSelectedFacesRandomUv( self ):
		self.GuidesGeneratedOnSelectedFaces( 1 )

	def test_GuidesGeneratedOnSelectedFacesRandomArea( self ):
		self.GuidesGeneratedOnSelectedFaces( 2 )

	def test_GuidesGeneratedOnSelectedFacesRandomFace( self ):
		self.GuidesGeneratedOnSelectedFaces( 3 )

	def test_GuidesGeneratedOnSelectedFacesVertex( self ):
		self.GuidesGeneratedOnSelectedFaces( 4 )

	def test_GuidesGeneratedOnSelectedFacesFaceCenter( self ):
		self.GuidesGeneratedOnSelectedFaces( 6 )

	def test_GuidesGeneratedOnSelectedFacesEven( self ):
		self.GuidesGeneratedOnSelectedFaces( 7 )

	def test_VertexColorSetControllingDistribution( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]

		# Paint vertex color info on the two vertices of the plane
		plane = pm.ls( type = "mesh" )[0]
		pm.select( plane + '.vtx[2:3]' )
		pm.polyColorPerVertex( rgb = ( 0, 0, 0 ), cdo = True )

		hairCountBeforeVertexColors = pm.mel.OxGetStrandCount( guidesShape )

		# Assign the vertex color channel to the length attribute
		guidesFromMesh.distributionChannel.set( 2001 )

		hairCountAfterVertexColors = pm.mel.OxGetStrandCount( guidesShape )

		# The color set should have disabled half of the root generation
		self.assertGreater( hairCountBeforeVertexColors, hairCountAfterVertexColors )

	def test_DistributionMapDoesNotChangeGuidesOnReload( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( planeSize=10, planeSegmentCount = 10, lengthRandomness = 1, length = 5, rootGenerationMethod=2, guideCount=100 )
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]

		fractal = pm.shadingNode( "fractal", asTexture=True )
		place2dTexture = pm.shadingNode( "place2dTexture", asUtility=True )
		pm.connectAttr( place2dTexture.outUV, fractal.uv )
		pm.connectAttr( place2dTexture.outUvFilterSize, fractal.uvFilterSize )
		pm.connectAttr( fractal.outColor, guidesFromMesh.distributionMap )

		verticesBeforeReload = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( guidesShape )

		# Reload
		filePath = pm.saveAs( 'temp.mb' )

		#for i in range( 0, 10 ):
		pm.openFile( filePath, force = True )

		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		verticesAfterReload = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( guidesShape )

		# Vertices per individual strand must remain identical after reloading the scene
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, verticesBeforeReload, verticesAfterReload )

	def test_ForceSequentialStrandIds( self ):
		rootGenerationMethodCount = 8
		for rootGenerationMethod in range( 0, rootGenerationMethodCount ):
			pm.newFile( force = True )
			guidesShape = TestUtilities.AddGuidesToNewPlane( planeSegmentCount = 10, rootGenerationMethod=rootGenerationMethod, guideCount=20 )
			guidesFromMeshNode = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
			guidesFromMeshNode.forceSequentialStrandIds.set( 1 )

			# Ensure that all generated strand ids are sequential
			strandIds = pm.mel.OxGetStrandIds( guidesShape )
			hasErrors = False

			if strandIds != None:
				for i in range( 0, len( strandIds ) ):
					if strandIds[i] != i:
						hasErrors = True
						break

			if hasErrors:
				self.fail( "Strand ids for method " + str( rootGenerationMethod ) + " are not sequential: " + str( strandIds ) )

	# This test is partial, not all of the internal data is exposed by Get commands
	def test_SetGuideCountToZero( self ):
		editGuides = TestUtilities.AddEditGuidesToNewPlane( rootGenerationMethod = 2 )
		pm.select( editGuides + ".f[2:3]" )
		h = editGuides.outputStrands.outputs( sh = True )[0]
		self.assertEqual( 2, pm.mel.OxGetSelectedStrandCount( h ) )
		gfm = editGuides.inputStrands.inputs( sh = True )[0]
		gfm.c.set( 0 )
		self.assertEqual( 0, pm.mel.OxGetStrandCount( h ) )
		self.assertIsNone( pm.mel.OxGetStrandIds( h ) )
		self.assertIsNone( pm.mel.OxGetStrandGroups( h ) )
		self.assertIsNone( pm.mel.OxGetSelectedStrands( h ) )
		self.assertIsNone( pm.mel.OxGetVisibleStrands( h ) )
		# An important detail about sets of strand ID's: they may contain ID's that are no longer valid
		self.assertEqual( 0, pm.mel.OxGetSelectedStrandCount( h ) )
		self.assertIsNone( pm.mel.OxGetSelectedStrandIds( h ) )
		# But if converted to indices, these should disappear or return an index of -1
		self.assertIsNone( pm.mel.OxGetSelectedStrandIndices( h ) )

	def test_ApplyToSelectedInputFaces( self ):
		plane = pm.polyPlane( sx = 2, sy = 2 )
		pm.select( plane[0] + '.f[0:1]' )

		selectedFaces = pm.filterExpand( sm = 34 );
		guidesShape = TestUtilities.AddGuidesToMesh( plane[0], rootGenerationMethod = 4 )
		pm.mel.OxSetSelectedFaces( selectedFaces,  plane[0], pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0] )

		# We should only have 6 strands which are on the selected faces
		self.assertEqual( 6, pm.mel.OxGetStrandCount( guidesShape ) )
