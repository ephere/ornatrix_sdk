import MayaTest
import pymel.core as pm
import pymel.core.datatypes as dt
import maya.cmds as cmds

AdoptExternalGuidesNodeName = 'AdoptExternalGuidesNode'
AnimationCacheNodeName = 'AnimationCacheNode'
BakedHairNodeName = 'BakedHairNode'
BakedGuidesNodeName = 'BakedGuidesNode'
BraidGuidesNodeName = 'BraidGuidesNode'
ChangeWidthNodeName = 'ChangeWidthNode'
ClumpNodeName = 'ClumpNode'
CurlNodeName = 'CurlNode'
CurvesFromStrandsNodeName = 'CurvesFromStrandsNode'
EditGuidesCommandName = 'OxEditGuides'
DetailNodeName = "DetailNode"
EditGuidesShapeName = 'EditGuidesShape'
FilterNodeName = 'FilterNode'
FrizzNodeName = 'FrizzNode'
GenerateStrandDataNodeName = 'GenerateStrandDataNode'
GravityNodeName = 'GravityNode'
GroomNodeName = 'GroomNode'
GroundStrandsNodeName = 'GroundStrandsNode'
GuideClusterNodeName = 'GuideClusterNode'
GuidesFromCurvesNodeName = 'GuidesFromCurvesNode'
GuidesFromGuideMeshNodeName = 'GuidesFromGuideMeshNode'
GuidesFromHairNodeName = 'GuidesFromHairNode'
GuidesFromMeshNodeName = 'GuidesFromMeshNode'
GuidesShapeName = 'GuidesShape'
HairClusterNodeName = 'HairClusterNode'
HairFromGuidesNodeName = 'HairFromGuidesNode'
HairFromMeshStripsNodeName = 'HairFromMeshStripsNode'
HairShapeName = 'HairShape'
LengthNodeName = 'LengthNode'
MergeNodeName = 'MergeNode'
MeshFromStrandsNodeName = 'MeshFromStrandsNode'
MoovPhysicsNodeName = 'MoovPhysicsNode'
MoovPhysicsLocatorNodeName = 'MoovPhysicsLocatorNode'
NoiseNodeName = 'NoiseNode'
PolyPlaneShapeName = 'polyPlane'
PushAwayFromSurfaceNodeName = 'PushAwayFromSurfaceNode'
RenderSettingsNodeName = 'RenderSettingsNode'
ResolveCollisionsNodeName = 'ResolveCollisionsNode'
RotateNodeName = 'RotateNode'
ScatterNodeName = 'ScatterNode'
ShellsNodeName = 'ShellsNode'
SurfaceCombNodeName = 'SurfaceCombNode'
SymmetryNodeName = 'SymmetryNode'
MultiplierNodeName = 'MultiplierNode'
NormalizeStrandsNodeName = 'NormalizeStrandsNode'
PropagationNodeName = 'PropagationNode'
TransferNormalsNodeName = 'TransferNormalsNode'
SmoothSurfaceNodeName = 'SmoothSurfaceNode'
AiAnimatorNodeName = 'AiAnimatorNode'

def GetNodeByType( nodeTypeName ):
	return pm.PyNode( pm.ls( type = nodeTypeName )[0] )

def GetObjectsOfPlugin( pluginName ):
	return pm.ls( type = pm.pluginInfo( pluginName, q = True, dn = True ) )

def GetOrnatrixObjects():
	return GetObjectsOfPlugin( 'Ornatrix' )

def CreateNode( typeName ):
	if typeName[-4:] == 'Node':
		return pm.createNode( typeName, name = typeName[0:-4] + '#' )
	else:
		return pm.createNode( typeName )

def DumpScene( filepath, printToConsole = False ):
	result = []
	for n in sorted( cmds.ls() ):
		result.append( n + ' : ' + cmds.nodeType( n ) )
		conns = cmds.listConnections( n, connections = True, source = False, plugs = True )
		if conns is not None:
			i = 0
			while i < len( conns ):
				result.append( '    ' + conns[i].split( '.' )[1] + ' -> ' + conns[i + 1] )
				i += 2
		attrs = cmds.listAttr( n )
		for attr in sorted( attrs ):
			try:
				val = str( cmds.getAttr( n + '.' + attr ) )
				result.append( '    ' + attr + ' = ' + val )
			except:
				pass
	with open( filepath, "w" ) as dumpFile:
		for x in result:
			dumpFile.write( x + '\n' )
			if printToConsole:
				print( x )

def DumpPoints( pointsList, filepath, printToConsole = False ):
	with open( filepath, "w" ) as dumpFile:
		for x in pointsList:
			dumpFile.write( str( x ) + '\n' )
			if printToConsole:
				print( x )


def CheckTipPositionsEqual( self, strandsShape, expectedPositions, useObjectCoordinates = False ):
	# Compare the tip positions of each guide, they should be the same as before
	for i in range( 0, pm.mel.OxGetStrandCount( strandsShape ) ):
		tipPosition = []
		if( useObjectCoordinates ):
			tipPosition = pm.mel.OxGetStrandPointInObjectCoordinates( strandsShape, i, 1 );
		else:
			tipPosition = pm.mel.OxGetStrandPoint( strandsShape, i, 1 )

		self.assertLess( dt.Vector( tipPosition ).distanceTo( expectedPositions[i] ), 0.0001,
			'i: %(i)d, tipPosition: %(tipPosition)s, expected: %(expected)s' % { 'i': i, 'tipPosition': tipPosition, 'expected' : expectedPositions[i] } )

def AddHairToMesh( meshShape, hairRenderCount = 100, hairViewportCount = 10, rootGenerationMethod = 2, pointsPerStrandCount = 2, guideRootGenerationMethod = 4, guideCount = 10, lengthRandomness = 0, length = 50, guideArea = 2, reparent = False ):
	# Create nodes
	transformNode = meshShape.firstParent2()
	guidesFromMesh = CreateNode( GuidesFromMeshNodeName )
	guidesFromMesh.lengthRandomness.set( lengthRandomness )
	guidesFromMesh.length.set( length )
	guidesFromMesh.pointCount.set( pointsPerStrandCount )
	guidesFromMesh.distribution.set( guideRootGenerationMethod )
	guidesFromMesh.attr( 'count' ).set( guideCount )
	hairFromGuides = CreateNode( HairFromGuidesNodeName )
	hairShape = pm.createNode( HairShapeName, parent = transformNode )

	# Set parameters
	hairFromGuides.renderCount.set( hairRenderCount )
	if hairRenderCount > 0.0:
		hairFromGuides.viewportCountFraction.set( hairViewportCount / ( hairRenderCount * 1.0 ) )

	hairFromGuides.distribution.set( rootGenerationMethod )
	hairFromGuides.guideArea.set( guideArea )

	# Make connections
	meshShape.outMesh >> guidesFromMesh.inputMesh
	guidesFromMesh.outputGuides >> hairFromGuides.inputStrands
	guidesFromMesh.outputStack >> hairFromGuides.inputStack
	hairFromGuides.outputHair >> hairShape.inputHair
	meshShape.outMesh >> hairShape.distributionMesh
	meshShape.outMesh >> hairFromGuides.distributionMesh

	if reparent:
		pm.parent( hairShape, meshShape )

	return hairShape

def AddGuidesToMesh( meshShape, guideCount = 10, rootGenerationMethod = 4, pointsPerStrandCount = 2, lengthRandomness = 0, length = 50, randomSeed = 1 ):
	# Create nodes
	transformNode = meshShape.firstParent2()
	guidesFromMesh = CreateNode( GuidesFromMeshNodeName )
	guidesShape = pm.createNode( HairShapeName, parent = transformNode )

	# Set parameters
	guidesFromMesh.attr( 'count' ).set( guideCount )
	guidesFromMesh.distribution.set( rootGenerationMethod )
	guidesFromMesh.pointCount.set( pointsPerStrandCount )
	guidesFromMesh.lengthRandomness.set( lengthRandomness )
	guidesFromMesh.length.set( length )
	guidesFromMesh.randomSeed.set( randomSeed )

	# Make connections
	meshShape.outMesh >> guidesFromMesh.inputMesh
	guidesFromMesh.outputGuides >> guidesShape.inputHair
	meshShape.outMesh >> guidesShape.distributionMesh

	return guidesShape

def AddGuidesToNewPlane( guideCount = 10, rootGenerationMethod = 4, pointsPerStrandCount = 2, planeSegmentCount = 1, planeSize = 1, length = 50, lengthRandomness = 0 ):
	plane = pm.polyPlane( sx = planeSegmentCount, sy = planeSegmentCount, width = planeSize, height = planeSize )
	pm.select( plane )
	meshShape = plane[0]

	return AddGuidesToMesh( meshShape, guideCount, rootGenerationMethod, pointsPerStrandCount, lengthRandomness, length )

def AddBakedHair():
	# Create nodes
	plane = pm.polyPlane( sx = 1, sy = 1, width = 10, height = 10 )
	pm.select( plane )
	meshShape = plane[0]
	transformNode = meshShape.firstParent2()

	bakedHairNode = CreateNode( BakedHairNodeName )
	hairShape = pm.createNode( HairShapeName, parent = transformNode )

	# Make connections
	meshShape.outMesh >> bakedHairNode.distributionMesh
	bakedHairNode.outputHair >> hairShape.inputHair
	meshShape.outMesh >> hairShape.distributionMesh

	return hairShape, bakedHairNode

def AddBakedHairToSegmentedPlane( planeSegmentCount = 1, planeSize = 10, hairCount = 30 ):
	planeName = 'plane1'
	plane = pm.polyPlane( sx = planeSegmentCount, sy = planeSegmentCount, width = planeSize, height = planeSize, name = planeName )
	pm.select( plane )
	meshShape = plane[0]

	hairShape = AddHairToMesh( meshShape, hairCount, hairCount, reparent = True )

	hairFromGuides = pm.ls( type = HairFromGuidesNodeName )[0]
	pm.mel.OxCollapseStack( hairFromGuides )
	bakedHairNode = pm.ls( type = BakedHairNodeName )[0]
	bakedHairNode.displayFraction.set( 1.0 )
	bakedHairNode.detachRoots.set( 0 )

	return hairShape, bakedHairNode, planeName

def AddBakedGuides( planeSegmentCount = 1, planeSize = 10 ):
	# Create nodes
	plane = pm.polyPlane( sx = planeSegmentCount, sy = planeSegmentCount, width = planeSize, height = planeSize )
	pm.select( plane )
	meshShape = plane[0]
	transformNode = meshShape.firstParent2()

	bakedGuidesNode = CreateNode( BakedGuidesNodeName )
	guidesShape = pm.createNode( HairShapeName, parent = transformNode )

	# Make connections
	meshShape.outMesh >> bakedGuidesNode.distributionMesh
	bakedGuidesNode.outputGuides >> guidesShape.inputHair
	meshShape.outMesh >> guidesShape.distributionMesh

	return guidesShape, bakedGuidesNode

def AddEditGuidesToMesh( meshShape, guideCount = 10, rootGenerationMethod = 4, pointsPerStrandCount = 2, guideLength = 10.0, lengthRandomness = 0, changeTracking = 1 ):
	transformNode = meshShape.firstParent2()
	guidesFromMesh = CreateNode( GuidesFromMeshNodeName )
	guidesShape = pm.createNode( HairShapeName, parent = transformNode )
	editGuidesShape = pm.createNode( EditGuidesShapeName, parent = transformNode )
	editGuidesShape.changeTracking.set( changeTracking )

	# Set parameters
	guidesFromMesh.attr( 'count' ).set( guideCount )
	guidesFromMesh.distribution.set( rootGenerationMethod )
	guidesFromMesh.pointCount.set( pointsPerStrandCount )
	guidesFromMesh.length.set( guideLength )
	guidesFromMesh.lengthRandomness.set( lengthRandomness )

	# Make connections
	meshShape.outMesh >> guidesFromMesh.inputMesh
	guidesFromMesh.outputGuides >> editGuidesShape.inputStrands
	guidesFromMesh.outputStack >> editGuidesShape.inputStack
	meshShape.outMesh >> editGuidesShape.distributionMesh
	editGuidesShape.outputStrands >> guidesShape.inputHair
	meshShape.outMesh >> guidesShape.distributionMesh

	return editGuidesShape

def AddHairToNewPlane( strandCount = 10, rootGenerationMethod = 4, pointsPerStrandCount = 2, planeSegmentCount = 1, guideRootGenerationMethod = 4, guideCount = 10, planeSize = 1, length = 50, guideArea = 2, reparent = False ):
	plane = pm.polyPlane( sx = planeSegmentCount, sy = planeSegmentCount, width = planeSize, height = planeSize )
	pm.select( plane )
	meshShape = plane[0]

	return AddHairToMesh( meshShape, strandCount, strandCount, rootGenerationMethod, pointsPerStrandCount, guideRootGenerationMethod, guideCount, 0, length, guideArea, reparent )

def AddHairToNewSphere( radius = 1, strandCount = 10, rootGenerationMethod = 4, pointsPerStrandCount = 2, sphereSegmentCount = 1, guideCount = 10, guideLength = 50, guideRootGenerationMethod = 4 ):
	sphere = pm.polySphere( radius = radius, sx = sphereSegmentCount, sy = sphereSegmentCount )
	pm.select( sphere )
	meshShape = sphere[0]

	return AddHairToMesh( meshShape, strandCount, strandCount, rootGenerationMethod, pointsPerStrandCount, guideCount = guideCount, length = guideLength, guideRootGenerationMethod = guideRootGenerationMethod )

def AddGuidesToNewSphere( radius = 1, strandCount = 10, rootGenerationMethod = 4, pointsPerStrandCount = 2, sphereSegmentCount = 1, guideCount = 10, guideLength = 50 ):
	sphere = pm.polySphere( radius = radius, sx = sphereSegmentCount, sy = sphereSegmentCount )
	pm.select( sphere )
	meshShape = sphere[0]

	return AddGuidesToMesh( meshShape, guideCount, rootGenerationMethod, pointsPerStrandCount, 0, guideLength )

def AddEditGuidesToNewPlane( planeSegmentCount = 1, guideCount = 10, rootGenerationMethod = 4, pointsPerStrandCount = 2, guideLength = 10.0, lengthRandomness = 0, changeTracking = 1 ):
	plane = pm.polyPlane( sx = planeSegmentCount, sy = planeSegmentCount )
	pm.select( plane )
	meshShape = plane[0]

	return AddEditGuidesToMesh( meshShape, guideCount, rootGenerationMethod, pointsPerStrandCount, guideLength, lengthRandomness, changeTracking )

def AddEditGuidesAboveHairToNewPlane():
	hairShape = AddGuidesToNewPlane()
	hairFromGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, HairFromGuidesNodeName ) )
	hairFromGuidesNode.renderCount.set( 100 )
	# We need to use 100% hair preview until we also support viewport hair fraction in stand-alone HFG operator
	hairFromGuidesNode.viewportCountFraction.set( 1 )
	editGuidesShape = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, EditGuidesShapeName ) )
	pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, ChangeWidthNodeName ) )
	return editGuidesShape

def AddEditGuidesToNewSphere( radius = 1, guideCount = 10, rootGenerationMethod = 4, pointsPerStrandCount = 2, segmentCount = 5, guideLength = 10.0, changeTracking = 1 ):
	sphere = pm.polySphere( sx = segmentCount, sy = segmentCount, r = radius )
	pm.select( sphere )
	meshShape = sphere[0]

	return AddEditGuidesToMesh( meshShape, guideCount, rootGenerationMethod, pointsPerStrandCount, guideLength, changeTracking )

def CreateTwoCurves():
	curve1 = pm.curve( p=[(1, 0, 1), (1, 1, 1.1), (1, 2, 1.2), (1, 3, 1.3)], k=[0, 0, 0, 1, 1, 1] )
	curve2 = pm.curve( p=[(-1, 0, 1), (-1, 1, 1.1), (-1, 2, 1.2), (-1, 3, 1.3)], k=[0, 0, 0, 1, 1, 1] )

	pm.select( clear = True )
	pm.select( curve1, add = True )
	pm.select( curve2, add = True )
	return [curve1, curve2]

def AddGuidesFromCurves():
	CreateTwoCurves()
	return pm.PyNode( pm.mel.OxAddGuidesFromCurves() )

def AddGuidesFromCurvesGrounded():
	result = AddGuidesFromCurves()
	pm.mel.OxAddStrandOperator( result, GroundStrandsNodeName )
	groundStrandsNode = pm.ls( selection = True )[0]
	plane = pm.polyPlane( w=10, h=10 )
	plane[0].outMesh >> groundStrandsNode.distributionMesh
	groundStrandsNode.detachRoots.set( 0 )
	return result


# Gets all vertices inside specified hair or guides in object coordinates
def GetVerticesInObjectCoordinates( strandsShape ):
	result = []
	for strandIndex in range( 0, pm.mel.OxGetStrandCount( strandsShape ) ):
		for pointIndex in range( 0, pm.mel.OxGetStrandPointCount( strandsShape, strandIndex ) ):
			result.append( pm.mel.OxGetStrandPointInObjectCoordinates( strandsShape, strandIndex, pointIndex ) )

	return result

# Gets all vertices inside specified hair or guides in world space coordinates
def GetVerticesInWorldSpace( strandsShape ):
	verticesCoords = pm.mel.OxGetVertices( strandsShape, worldSpace = True )
	result = []
	vertexIndex = 0
	for strandIndex in range( 0, pm.mel.OxGetStrandCount( strandsShape ) ):
		for pointIndex in range( 0, pm.mel.OxGetStrandPointCount( strandsShape, strandIndex ) ):
			result.append( [ verticesCoords[vertexIndex], verticesCoords[vertexIndex + 1], verticesCoords[vertexIndex + 2] ] )
			vertexIndex += 3
	return result

# Gets all vertices inside specified hair or guides in object coordinates and associates each strand with its unique strand id
def GetVerticesInObjectCoordinatesByStrandId( strandsShape ):
	result = {}
	strandIds = pm.mel.OxGetStrandIds( strandsShape )
	for strandIndex in range( 0, pm.mel.OxGetStrandCount( strandsShape ) ):
		verticesInObjectCoordinates = []
		for pointIndex in range( 0, pm.mel.OxGetStrandPointCount( strandsShape, strandIndex ) ):
			verticesInObjectCoordinates.append( pm.mel.OxGetStrandPointInObjectCoordinates( strandsShape, strandIndex, pointIndex ) )

		result[strandIds[strandIndex]] = verticesInObjectCoordinates

	return result

def GetVerticesInObjectCoordinatesSortedByStrandId( strandsShape ):
	resultByStrandId = GetVerticesInObjectCoordinatesByStrandId( strandsShape )

	result = []
	for strandId in sorted (resultByStrandId.keys()):
		for point in resultByStrandId[strandId]:
			result.append( point )

	return result

def CheckMatchExistsForEachPointSet( self, expectedVertices, actualVertices, pointCount = 2 ):
	for i in range( 0, len( expectedVertices ) // pointCount ):
		matchFound = False
		for j in range( 0, len( expectedVertices ) // pointCount ):
			error1 = dt.Vector( actualVertices[j * pointCount] ).distanceTo( expectedVertices[i * pointCount] )
			error2 = dt.Vector( actualVertices[j * pointCount + 1] ).distanceTo( expectedVertices[i * pointCount + 1] )
			if error1 <= 0.0001 and error2 <= 0.0001:
				matchFound = True
				break

		self.assertTrue( matchFound, 'Match not found for (strand ' + str( i ) + ') ' + str( expectedVertices[i * pointCount] ) + ', ' + str( expectedVertices[i * pointCount + 1] ) )

def GetStrandLengths( strandsShape ):
	result = []
	for strandIndex in range( 0, pm.mel.OxGetStrandCount( strandsShape ) ):
		strandLength = 0
		for pointIndex in range( 1, pm.mel.OxGetStrandPointCount( strandsShape, strandIndex ) ):
			strandLength += dt.Vector( pm.mel.OxGetStrandPointInObjectCoordinates( strandsShape, strandIndex, pointIndex ) ).distanceTo( pm.mel.OxGetStrandPointInObjectCoordinates( strandsShape, strandIndex, pointIndex - 1 ) )

		result.append( strandLength )

	return result

def GetStrandSegmentLengths( strandsShape ):
	result = []
	for strandIndex in range( 0, pm.mel.OxGetStrandCount( strandsShape ) ):
		for pointIndex in range( 1, pm.mel.OxGetStrandPointCount( strandsShape, strandIndex ) ):
			result.append( dt.Vector( pm.mel.OxGetStrandPointInObjectCoordinates( strandsShape, strandIndex, pointIndex ) ).distanceTo( pm.mel.OxGetStrandPointInObjectCoordinates( strandsShape, strandIndex, pointIndex - 1 ) ) )

	return result

def CheckPointsNearEqual( self, point1, point2, epsilon = 0.01, extraMessage = '', index = -1 ):
	error = dt.Vector( point1 ).distanceTo( point2 )
	self.assertGreaterEqual( epsilon, error, msg = '{} is not near equal {}, error: {}, index: {}  {}'.format( point1, point2, error, index, extraMessage ) )

def CheckPointsNotNearEqual( self, point1, point2, epsilon = 0.01, extraMessage = '' ):
	error = dt.Vector( point1 ).distanceTo( point2 )
	self.assertLessEqual( epsilon, error, msg = '[' + str( point1 ) + '] is near equal [' + str( point2 ) + '], error: ' + str( error ) + extraMessage )

def CheckPointsAllNearEqual( self, points1, points2, epsilon = 0.01, points1Start = 0, points2Start = 0, pointsCount = -1, extraMessage = '' ):
	if pointsCount == -1:
		pointsCount = len( points1 ) - points1Start
		self.assertEqual( pointsCount, len( points2 ) - points2Start )

	for i in range( 0, pointsCount ):
		CheckPointsNearEqual( self, points1[i + points1Start], points2[i + points2Start], epsilon = epsilon, extraMessage = extraMessage, index = i )

# The vertices in object space should be equal and correctly assigned to the same strand ids
def CheckPointsAllNearEqualByStrandId( self, points1, points2, epsilon = 0.01, points1Start = 0, points2Start = 0, pointsCount = -1 ):
	for strandId, verticesInObjectSpace in points1.items():
		point2 = points2[strandId]
		CheckPointsAllNearEqual( self, verticesInObjectSpace, point2, epsilon = epsilon, points1Start = points1Start, points2Start = points2Start, pointsCount = pointsCount, extraMessage = ' at strandId ' + str( strandId ) )

def MakeDictionaryWithIncrementingKeys( values ):
	result = {}
	for i in range( 0, len( values ) ):
		result[i] = values[i]
	return result

def MakeDictionary( keys, values ):
	result = {}
	for i in range( 0, len( keys ) ):
		result[keys[i]] = values[i]
	return result

def MakeVector3Array( values ):
	result = []
	for i in range( 0, len( values ) // 3 ):
		result.append( [values[i * 3], values[i * 3 + 1], values[i * 3 + 2]] )
	return result

def AssertEqualByKeys( self, values1, values2 ):
	for key, value in values1.items():
		self.assertEqual( value, values2[key] )

def AssertAlmostEqualByKeys( self, values1, values2, places = 5 ):
	for key, value in values1.items():
		self.assertAlmostEqual( value, values2[key], places )

def AssertPointsAlmostEqualByKeys( self, values1, values2, epsilon = 0.01 ):
	for key, value in values1.items():
		CheckPointsNearEqual( self, value, values2[key], epsilon )

# Tests that each point within one specified set each has exactly one equal point in the other specified set
def CheckPointsMatch( self, points1, points2, epsilon = 0.01, pointsStart = 0, pointsCount = -1 ):
	self.assertEqual( len( points1 ), len( points2 ) )

	if pointsCount == -1:
		pointsCount = len( points1 ) - pointsStart

	wasMatched = [False] * pointsCount
	for i in range( 0, pointsCount ):
		hasMatch = False
		point = dt.Vector( points1[i + pointsStart] )
		for j in range( 0, pointsCount ):
			if( not wasMatched[j] and point.distanceTo( points2[j + pointsStart] ) <= epsilon ):
				hasMatch = True
				wasMatched[j] = True
				break

		self.assertTrue( hasMatch )

def CheckPointsAllNotNearEqual( self, points1, points2, epsilon = 0.01, pointsStart = 0, pointsCount = -1 ):
	self.assertEqual( len( points1 ), len( points2 ) )

	if pointsCount == -1:
		pointsCount = len( points1 ) - pointsStart

	for i in range( 0, pointsCount ):
		self.assertLessEqual( epsilon, dt.Vector( points1[i + pointsStart] ).distanceTo( points2[i + pointsStart] ), 'Points near equal at index {}, value {}'.format( i + pointsStart, points1[i + pointsStart] ) )

def CheckPointsNotAllNearEqual( self, points1, points2, epsilon = 0.01, pointsStart = 0, pointsCount = -1 ):
	self.assertEqual( len( points1 ), len( points2 ) )
	allPointsAreEqual = True

	if pointsCount == -1:
		pointsCount = len( points1 ) - pointsStart

	for i in range( 0, pointsCount ):
		if( dt.Vector( points1[pointsStart + i] ).distanceTo( points2[pointsStart + i] ) > epsilon ):
			allPointsAreEqual = False
			break
	self.assertFalse( allPointsAreEqual, 'All points are equal, expecting at least one not to be equal' )

def CheckUniqueStrandIds( self, hairShape ):
	strandIds = pm.mel.OxGetStrandIds( hairShape )
	idSet = set()
	for id in strandIds:
		if id in idSet:
			self.fail( "Duplicate strand ID {}".format( id ) )
		idSet.add( id )

def GetVertexNormals (meshShape):
	selection = pm.select( meshShape )
	vtxCount = pm.polyEvaluate (v= True)
	normals = []
	for vtx in range(vtxCount):
		pm.select (meshShape+".vtx[" +str(vtx) +"]")
		normal = pm.polyNormalPerVertex( query = True, xyz = True)
		normals.append(normal[0:3])

	return normals

def GetAveragePointDistance( self, points1, points2 ):
	self.assertEqual( len( points1 ), len( points2 ) )
	result = 0.0
	for i in range( 0, len( points1 ) ):
		result += dt.Vector( points1[i] ).distanceTo( points2[i] )

	result /= len( points1 )
	return result

def GetAveragePointMove( self, points1, points2 ):
	self.assertEqual( len( points1 ), len( points2 ) )
	averageMove = dt.Vector( 0, 0, 0)
	for i in range( len( points1 ) ):
		averageMove += dt.Vector( points2[i] ) - dt.Vector( points1[i] )
	averageMove /= len( points1 )
	return averageMove

def GetMaxPointDistance( self, points1, points2 ):
	self.assertEqual( len( points1 ), len( points2 ) )
	result = 0.0
	for i in range( 0, len( points1 ) ):
		distance = dt.Vector( points1[i] ).distanceTo( points2[i] )
		if distance > result:
			result = distance
	return result

def GetBoundingBox( points ):
	smallestPoint = [99999, 99999, 99999]
	biggestPoint = [-99999, -99999, -99999]

	for i in range( 0, len( points ) ):
		for j in range( 0, 3 ):
			if points[i][j] < smallestPoint[j]:
				smallestPoint[j] = points[i][j]
			if points[i][j] > biggestPoint[j]:
				biggestPoint[j] = points[i][j]

	return [smallestPoint, biggestPoint]

def GetMeshVertices( mesh ):
	result = []
	index = 0

	for vertexIndex in range( 0, len( mesh.vtx ) ):
		result.append( mesh.vtx[vertexIndex].getPosition() )
		vertexIndex += 1

	return result

def GetTipPositions( strandsShape, useObjectCoordinates = False ):
	result = []
	for strandIndex in range( 0, pm.mel.OxGetStrandCount( strandsShape ) ):
		if( useObjectCoordinates ):
			result.append( pm.mel.OxGetStrandPointInObjectCoordinates( strandsShape, strandIndex, pm.mel.OxGetStrandPointCount( strandsShape, strandIndex ) - 1 ) )
		else:
			result.append( pm.mel.OxGetStrandPoint( strandsShape, strandIndex, pm.mel.OxGetStrandPointCount( strandsShape, strandIndex ) - 1 ) )

	return result

def GetRootPositions( strandsShape, useObjectCoordinates = True ):
	result = []
	for strandIndex in range( 0, pm.mel.OxGetStrandCount( strandsShape ) ):
		if( useObjectCoordinates ):
			result.append( pm.mel.OxGetStrandPointInObjectCoordinates( strandsShape, strandIndex, 0 ) )
		else:
			result.append( pm.mel.OxGetStrandPoint( strandsShape, strandIndex, 0 ) )

	return result

def GetStrandTangents( strandsShape, useObjectCoordinates = False ):
	result = []
	roots = GetRootPositions( strandsShape, useObjectCoordinates )
	tips = GetTipPositions( strandsShape, useObjectCoordinates )

	for strandIndex in range( 0, len( roots ) ):
		result.append( ( dt.Vector( tips[strandIndex] ) - dt.Vector( roots[strandIndex] ) ).normal() )

	return result

def GetStrandPointPositionsAtIndex( strandsShape, pointIndex, useObjectCoordinates = False, sortByStrandId = True ):
	result = []
	strandIds = []
	if( sortByStrandId ):
		strandIds = pm.mel.OxGetStrandIds( strandsShape )
	for strandIndex in range( 0, pm.mel.OxGetStrandCount( strandsShape ) ):
		if( useObjectCoordinates ):
			result.append( pm.mel.OxGetStrandPointInObjectCoordinates( strandsShape, strandIndex, pointIndex ) )
		else:
			result.append( pm.mel.OxGetStrandPoint( strandsShape, strandIndex, pointIndex ) )

	if( sortByStrandId ):
		return [x for _,x in sorted(zip(strandIds,result))]
	else:
		return result

def GetStrandPoints( strandsShape, strandIndex, useObjectCoordinates = False ):
	pointGetter = ( lambda pointIndex: pm.mel.OxGetStrandPointInObjectCoordinates( strandsShape, strandIndex, pointIndex ) ) if useObjectCoordinates else \
		( lambda pointIndex: pm.mel.OxGetStrandPoint( strandsShape, strandIndex, pointIndex ) )
	return list( map( pointGetter, range( 0, pm.mel.OxGetStrandPointCount( strandsShape, strandIndex ) ) ) )

def TestStrandOperatorChangesGuideCount( self, nodeName, pointsPerStrandCount = 2, isTestingOnGuides = True, strandCount = 10, rootGenerationMethod = 4, operatorSetup = None ):
	guidesShape = AddGuidesToNewPlane( pointsPerStrandCount = pointsPerStrandCount ) if isTestingOnGuides else \
		AddHairToNewPlane( pointsPerStrandCount = pointsPerStrandCount, strandCount = strandCount, rootGenerationMethod = rootGenerationMethod )

	verticesBefore = GetVerticesInObjectCoordinates( guidesShape )
	verticesBeforeCount = len( verticesBefore )
	self.assertGreaterEqual( verticesBeforeCount, 2, 'Expected at least two vertices, instead got: %(vertexCount)s' % { 'vertexCount': verticesBeforeCount } )

	operatorNodeName = pm.mel.OxAddStrandOperator( guidesShape, nodeName )

	if operatorSetup != None:
		operatorSetup( operatorNodeName )

	verticesAfter = GetVerticesInObjectCoordinates( guidesShape )

	self.assertNotEqual( len( verticesAfter ), len( verticesBefore ), 'Vertex count was not modified by the operator' )

def TestStrandOperatorChangingGuides( self, nodeName, isCheckingAllVertices = True, epsilon = 0.1, pointsPerStrandCount = 2, isTestingOnGuides = True, strandCount = 10, rootGenerationMethod = 4, operatorSetup = None ):
	guidesShape = AddGuidesToNewPlane( pointsPerStrandCount = pointsPerStrandCount ) if isTestingOnGuides else \
		AddHairToNewPlane( pointsPerStrandCount = pointsPerStrandCount, strandCount = strandCount, rootGenerationMethod = rootGenerationMethod )

	verticesBefore = GetVerticesInObjectCoordinates( guidesShape )
	verticesBeforeCount = len( verticesBefore )
	self.assertGreaterEqual( verticesBeforeCount, 2, 'Expected at least two vertices, instead got: %(vertexCount)s' % { 'vertexCount': verticesBeforeCount } )

	operatorNodeName = pm.mel.OxAddStrandOperator( guidesShape, nodeName )

	if operatorSetup != None:
		operatorSetup( operatorNodeName )

	verticesAfter = GetVerticesInObjectCoordinates( guidesShape )

	self.assertEqual( len( verticesAfter ), len( verticesBefore ), 'Vertex count was modified by the operator' )

	wasAtLeastOneVertexDifferent = False
	for vertexIndex in range( 0, len( verticesBefore ) ):
		distance = dt.Vector( verticesAfter[vertexIndex] ).distanceTo( verticesBefore[vertexIndex] )
		if isCheckingAllVertices:
			self.assertGreater( distance, epsilon,
				'vertexIndex: %(vertexIndex)d, vertexPosition: %(tipPosition)s, expected anything but: %(expected)s' % { 'vertexIndex': vertexIndex, 'tipPosition': verticesAfter[vertexIndex], 'expected': verticesBefore[vertexIndex] } )
		elif not wasAtLeastOneVertexDifferent:
			wasAtLeastOneVertexDifferent = distance > epsilon

	if not isCheckingAllVertices:
		self.assertTrue( wasAtLeastOneVertexDifferent, 'Operator did not change any vertices' )

class RenderModeScope:
	"""
	A "context manager" to be used with Python's 'with' statement. See https://docs.python.org/2/reference/datamodel.html#context-managers
	Wraps the code within the 'with' statement in OxSetIsRendering(true) / OxSetIsRendering(false) calls
	Typical usage:
		\code
		with TestUtilities.RenderModeScope():
			# some code that will run with Ornatrix set in render mode
		\endcode
	"""
	def __enter__( self ):
		pm.mel.OxSetIsRendering(True)

	def __exit__( self, type, value, traceback ):
		pm.mel.OxSetIsRendering(False)


def CreateSingleHairWithPropagator( count = 4, length = 2, sideCount = 2 ):
	resultShape = AddGuidesToNewPlane( guideCount = 1, pointsPerStrandCount = 3, rootGenerationMethod = 2, planeSegmentCount = 1, length = 5 )
	propagatorNode = pm.PyNode( pm.mel.OxAddStrandOperator( resultShape, PropagationNodeName ) )
	propagatorNode.setAttr( 'count', count )
	propagatorNode.verticesPerRootCount.set( 4 )
	propagatorNode.textureMappingMethod.set( 2 )
	propagatorNode.generationMethod.set( 0 )
	propagatorNode.length.set( length )
	propagatorNode.lengthRandomness.set( 0 )
	propagatorNode.sideCount.set( sideCount )
	propagatorNode.setAttr( "lengthRamp[0].lengthRamp_FloatValue", 1 )
	propagatorNode.setAttr( "lengthRamp[1].lengthRamp_FloatValue", 1 )
	propagatorNode.setAttr( "lengthRamp[2].lengthRamp_FloatValue", 1 )
	propagatorNode.setAttr( "distributionRamp[0].distributionRamp_FloatValue", 1 )
	propagatorNode.setAttr( "distributionRamp[1].distributionRamp_FloatValue", 1 )
	propagatorNode.setAttr( "distributionRamp[2].distributionRamp_FloatValue", 1 )
	return resultShape

def GetTextureCoordinates( strandsShape, channel, type ):
	result = []
	for strandIndex in range( 0, pm.mel.OxGetStrandCount( strandsShape ) ):
		for pointIndex in range( 0, pm.mel.OxGetStrandPointCount( strandsShape, strandIndex ) ):
			result.append( pm.mel.OxGetTextureCoordinate( strandsShape, strandIndex, pointIndex, channel, type = type ) )
	return result

def GetTextureCoordinatesByStrandId( self, strandsShape, channel, type ):
	result = {}
	strandIds = pm.mel.OxGetStrandIds( strandsShape )
	for strandIndex in range( 0, pm.mel.OxGetStrandCount( strandsShape ) ):
		strandTextureCoordinates = []
		for pointIndex in range( 0, pm.mel.OxGetStrandPointCount( strandsShape, strandIndex ) ):
			strandTextureCoordinates.append( pm.mel.OxGetTextureCoordinate( strandsShape, strandIndex, pointIndex, channel, type = type ) )

		result[strandIds[strandIndex]] = strandTextureCoordinates

	return result

def GetReferencePropagatorTextureCoordinates( stemVertexCount, branchVertexCount, branchesPerStemCount, sideCount = 2, islandCount = 1, repetitions = 1 ):
	"""Obtains reference coordinates for a texture atlas with a uniform flat-map propagator."""
	stemVerticesV = [ float( index ) / ( stemVertexCount - 1 ) for index in range( stemVertexCount ) ]
	branchVerticesU = [ float( index ) / ( branchVertexCount - 1 ) / islandCount / sideCount for index in range( branchVertexCount ) ]
	result = []
	stemOffsetU = 1.0 / ( sideCount * islandCount )
	for islandIndex in range( islandCount ):
		islandResult = []
		uOffset = float( islandIndex ) / islandCount
		for vertexV in stemVerticesV:
			islandResult.append( dt.Vector( stemOffsetU + uOffset, vertexV, 0.0 ) )
		for branchIndex in range( branchesPerStemCount ):
			vOffset = float( branchIndex ) / ( branchesPerStemCount - 1 )
			sideIndex = branchIndex % sideCount
			uOffsetBranch = stemOffsetU * ( ( sideIndex // 2 ) * 2 + 1 ) + uOffset
			direction = -1 if sideIndex % 2 == 0 else 1
			for vertexU in branchVerticesU:
				islandResult.append( dt.Vector( uOffsetBranch + vertexU * direction, vOffset, 0.0 ) )
		for _ in range( repetitions ):
			result.extend( islandResult )
	return result

def CreateBoxedPlaneWithOperators():
	hairShape = AddHairToNewPlane()
	curlNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, CurlNodeName ) )
	frizzNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, FrizzNodeName ) )
	lengthNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, LengthNodeName ) )
	return [hairShape, curlNode, frizzNode, lengthNode]

def BoxNodesWithHFG( self, boxGuidesFromMesh = False ):
	hairShape = AddGuidesToNewPlane()
	editGuidesShape = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, EditGuidesShapeName ) )
	hairFromGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, HairFromGuidesNodeName ) )
	hairFromGuidesNode.renderCount.set( 100 )
	# We need to use 100% hair preview until we also support viewport hair fraction in stand-alone HFG operator
	hairFromGuidesNode.viewportCountFraction.set( 1 )
	changeWidthNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, ChangeWidthNodeName ) )

	verticesBeforeBoxing = pm.mel.OxGetVertices( hairShape, os = True )
	if boxGuidesFromMesh:
		pm.mel.OxBoxNodes( [changeWidthNode, hairFromGuidesNode, editGuidesShape, pm.mel.ls( type = GuidesFromMeshNodeName )[0]] )
	else:
		pm.mel.OxBoxNodes( [changeWidthNode, hairFromGuidesNode, editGuidesShape] )

	return [verticesBeforeBoxing, hairShape]

def CreateStrand( face, barycentricCoordinate, length, pointCount ):
	commandString = EditGuidesCommandName  + ' -cg 1 ' + str( face ) + ' 0 ' + str( pointCount ) + ' '
	commandString += str( barycentricCoordinate[0] ) + ' ' + str( barycentricCoordinate[1] ) + ' -cv ' + str( pointCount ) + ' '

	for i in range( 0, pointCount ):
		commandString += '0 0 ' + str( length * float( i ) / float( pointCount - 1 ) ) + ' '

	commandString += ';'

	pm.mel.eval( commandString )

def SaveAndReloadScene( self, fileName = 'testScene', binary = False ):
	fileExtension = 'mb' if binary else 'ma'
	filePath = pm.saveAs( self.addTempFile( fileName + '.' + fileExtension ) )
	pm.newFile( force = True )
	pm.openFile( filePath, force = True )

def SetBitmapAttribute( mapInput, imageFilePath ):
	fileTexture = pm.shadingNode( "file", asTexture = True, isColorManaged = True )
	place2dTexture = pm.shadingNode( "place2dTexture", asUtility = True )
	pm.connectAttr( place2dTexture.outUV, fileTexture.uv )
	pm.connectAttr( place2dTexture.outUvFilterSize, fileTexture.uvFilterSize )
	pm.connectAttr( fileTexture.outColor, mapInput )

	# Modify UVs and load image as UDIM.
	pm.setAttr( fileTexture + ".fileTextureName", imageFilePath, type = "string" )
	#pm.setAttr( fileTexture + ".uvTilingMode", 3 )