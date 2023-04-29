import c4d
import maxon
import os
import sys
import math
import Ornatrix.Ornatrix as Ox
from RunTests import RepositoryRootPath

def IsFeatureEnabled( featureName ):
	features = os.getenv( 'OX_C4D_ENABLED_FEATURES' )
	return features is not None and featureName in features


def GetObjectsByType( c4dObjectOrDocument, objectTypeId ):
	def findObjects( source, typeId, destination ):
		while source:
			if source.CheckType( typeId ):
				destination.append( source )
			findObjects( source.GetDown(), typeId, destination )
			source = source.GetNext()
	result = []
	startObject = c4dObjectOrDocument.GetFirstObject() if isinstance( c4dObjectOrDocument, c4d.documents.BaseDocument ) else c4dObjectOrDocument
	findObjects( startObject, objectTypeId, result )
	return result


def AddGuidesToMesh( meshShape, guideCount = 10, rootGenerationMethod = 4, pointsPerStrandCount = 2, lengthRandomness = 0, length = 50, randomSeed = 1, name="Hair Object" ):
	doc = c4d.documents.GetActiveDocument()
	hairObject = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
	hairObject.SetName(name)
	doc.InsertObject( hairObject )
	guidesFromMesh = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
	guidesFromMesh[Ox.res.INPUT_MESH] = meshShape
	guidesFromMesh[Ox.res.gfm_RootGenerationMethodAttribute] = rootGenerationMethod
	guidesFromMesh[Ox.res.gfm_GuideCountAttribute] = guideCount
	guidesFromMesh[Ox.res.gfm_GuidePointCountAttribute] = pointsPerStrandCount
	guidesFromMesh[Ox.res.gfm_GuideLengthRandomnessAttribute] = lengthRandomness
	guidesFromMesh[Ox.res.gfm_GuideLengthAttribute] = length
	guidesFromMesh[Ox.res.gfm_RandomSeedAttribute] = randomSeed
	guidesFromMesh.InsertUnder( hairObject )
	return hairObject

def AddGuidesToNewPlane( guideCount = 10, rootGenerationMethod = 4, pointsPerStrandCount = 2, planeSegmentCount = 1, planeSize = 1, length = 50, lengthRandomness = 0 ):
	doc = c4d.documents.GetActiveDocument()
	plane = c4d.BaseObject( c4d.Oplane )
	plane[c4d.PRIM_PLANE_WIDTH] = planeSize
	plane[c4d.PRIM_PLANE_HEIGHT] = planeSize
	plane[c4d.PRIM_PLANE_SUBW] = planeSegmentCount
	plane[c4d.PRIM_PLANE_SUBH] = planeSegmentCount
	doc.InsertObject( plane )
	return AddGuidesToMesh( plane, guideCount, rootGenerationMethod, pointsPerStrandCount, lengthRandomness, length )


def GetDataArrayAsList( hair, dataId ):
	container = Ox.GetHairOrGuidesUserData( hair, dataId, c4d.DTYPE_SUBCONTAINER )
	if container is None:
		return None
	result = [value for index, value in container]
	return result

def GetStrandIds( hair ):
	return GetDataArrayAsList( hair, Ox.res.hd_StrandIds )

def GetRotations( hair ):
	return GetDataArrayAsList( hair, Ox.res.hd_StrandRotations )

def GetVerticesInObjectCoordinates( hair ):
	return GetDataArrayAsList( hair, Ox.res.hd_Vertices )

def C4dVectorSort( v ):
	return v.x + v.y + v.z

def GetVerticesInObjectCoordinatesSorted( hair ):
	result = GetDataArrayAsList( hair, Ox.res.hd_Vertices )
	result.sort( key=C4dVectorSort )
	return result

def GetStrandChannels( hair ):
	container = Ox.GetHairOrGuidesUserData( hair, Ox.res.hd_ChannelData, c4d.DTYPE_SUBCONTAINER )
	return container


# Gets all vertices inside specified hair or guides in object coordinates and associates each strand with its unique strand id
def GetVerticesInObjectCoordinatesByStrandId( strandsShape ):
	result = {}
	strandIds = GetStrandIds( strandsShape )
	userDataContainer = Ox.GetHairOrGuidesUserData( strandsShape, Ox.res.hd_StrandPoints, c4d.DTYPE_SUBCONTAINER )
	for strandIndex in range( 0, len( strandIds ) ):
		verticesInObjectCoordinates = []
		strandPointsContainer = userDataContainer.GetContainer( strandIndex )
		if strandPointsContainer is not None:
			for index, value in strandPointsContainer:
				verticesInObjectCoordinates.append( value )
		result[strandIds[strandIndex]] = verticesInObjectCoordinates
	return result

def GetVerticesInObjectCoordinatesByStrandId2( strandsShape, transform ):
	result = {}
	strandIds = GetStrandIds( strandsShape )
	userDataContainer = Ox.GetHairOrGuidesUserData( strandsShape, Ox.res.hd_StrandPoints, c4d.DTYPE_SUBCONTAINER )
	for strandIndex in range( 0, len( strandIds ) ):
		verticesInObjectCoordinates = []
		strandPointsContainer = userDataContainer.GetContainer( strandIndex )
		if strandPointsContainer is not None:
			for index, value in strandPointsContainer:
				verticesInObjectCoordinates.append( value * transform )
		result[strandIds[strandIndex]] = verticesInObjectCoordinates
	return result

def GetVerticesInWorldCoordinatesByStrandId( strandsShape ):
	transform = strandsShape.GetMg()
	result = {}
	strandIds = GetStrandIds( strandsShape )
	userDataContainer = Ox.GetHairOrGuidesUserData( strandsShape, Ox.res.hd_StrandPoints, c4d.DTYPE_SUBCONTAINER )
	for strandIndex in range( 0, len( strandIds ) ):
		verticesInObjectCoordinates = []
		strandPointsContainer = userDataContainer.GetContainer( strandIndex )
		if strandPointsContainer is not None:
			for index, value in strandPointsContainer:
				verticesInObjectCoordinates.append( value * transform )
		result[strandIds[strandIndex]] = verticesInObjectCoordinates
	return result

def GetVerticesInWorldCoordinatesByStrandId( hairObject ):
	result = {}
	mg = hairObject.GetMg()
	strandIds = GetStrandIds( hairObject )
	userDataContainer = Ox.GetHairOrGuidesUserData( hairObject, Ox.res.hd_StrandPoints, c4d.DTYPE_SUBCONTAINER )
	for strandIndex in range( 0, len( strandIds ) ):
		verticesInWorldCoordinates = []
		strandPointsContainer = userDataContainer.GetContainer( strandIndex )
		if strandPointsContainer is not None:
			for index, value in strandPointsContainer:
				verticesInWorldCoordinates.append( mg * value )
		result[strandIds[strandIndex]] = verticesInWorldCoordinates
	return result

# Gets all strand lengths and associates with its unique strand id
def GetStrandLengthsByStrandId( strandsShape ):
	result = {}
	strandIds = GetStrandIds( strandsShape )
	userDataContainer = Ox.GetHairOrGuidesUserData( strandsShape, Ox.res.hd_StrandPoints, c4d.DTYPE_SUBCONTAINER )
	for strandIndex in range( 0, len( strandIds ) ):
		strandLength = 0
		prevPoint= None
		strandPointsContainer = userDataContainer.GetContainer( strandIndex )
		if strandPointsContainer is not None:
			for index, point in strandPointsContainer:
				if prevPoint != None:
					strandLength += ( point - prevPoint ).GetLength()
				prevPoint = point
		result[strandIds[strandIndex]] = strandLength
	return result

def GetRotationsByStrandId( strandsShape ):
	result = {}
	strandIds = GetStrandIds( strandsShape )
	rotations = GetRotations( strandsShape )
	for strandIndex in range( 0, len( strandIds ) ):
		result[strandIds[strandIndex]] = rotations[strandIndex]
	return result


def CheckPointsNearEqual( self, point1, point2, epsilon = 0.01, extraMessage = '' ):
	error = ( point1 - point2 ).GetLength()
	self.assertGreaterEqual( epsilon, error, msg = '[' + str( point1 ) + '] is not near equal [' + str( point2 ) + '], error: ' + str( error ) + extraMessage )

def CheckPointsNotNearEqual( self, point1, point2, epsilon = 0.01, extraMessage = '' ):
	error = ( point1 - point2 ).GetLength()
	self.assertLessEqual( epsilon, error, msg = '[' + str( point1 ) + '] is near equal [' + str( point2 ) + '], error: ' + str( error ) + extraMessage )

def ArePointsAllNearEqual( self, points1, points2, epsilon = 0.01, points1Start = 0, points2Start = 0, pointsCount = -1 ):
	if pointsCount == -1:
		pointsCount = len( points1 ) - points1Start
	for i in range( 0, pointsCount ):
		if ( points1[i + points1Start] - points2[i + points2Start] ).GetLength() > epsilon:
			return False
	return True

def CheckPointsAllNearEqual( self, points1, points2, epsilon = 0.01, points1Start = 0, points2Start = 0, pointsCount = -1, extraMessage = '' ):
	if pointsCount == -1:
		pointsCount = len( points1 ) - points1Start
		self.assertEqual( pointsCount, len( points2 ) - points2Start )
	for i in range( 0, pointsCount ):
		CheckPointsNearEqual( self, points1[i + points1Start], points2[i + points2Start], epsilon = epsilon, extraMessage = extraMessage )

# The vertices in object space should be equal and correctly assigned to the same strand ids
def CheckPointsAllNearEqualByStrandId( self, points1, points2, epsilon = 0.01, points1Start = 0, points2Start = 0, pointsCount = -1 ):
	for strandId, strandPoints1 in iter(points1.items()):
		strandPoints2 = points2[strandId]
		CheckPointsAllNearEqual( self, strandPoints1, strandPoints2, epsilon = epsilon, points1Start = points1Start, points2Start = points2Start, pointsCount = pointsCount, extraMessage = ' at strandId ' + str( strandId ) )

def GetNearEqualPointsPercentage( self, points1, points2, epsilon = 0.01, points1Start = 0, points2Start = 0, pointsCount = -1, extraMessage = '' ):
	nearEqualPointCount = 0
	for strandId, strandPoints1 in iter(points1.items()):
		strandPoints2 = points2[strandId]
		self.assertEqual( len(strandPoints1), len( strandPoints2 ) )
		error = 0
		for i in range( 0, len(strandPoints1) ):
			error = error + ( strandPoints1[i] - strandPoints2[i] ).GetLength()
		if error <= epsilon:
			nearEqualPointCount = nearEqualPointCount + 1
	return float(nearEqualPointCount) / float(len(points1))

def GetNearEqualValuesPercentage( self, values1, values2, epsilon = 0.01):
	nearEqualValuesCount = 0
	self.assertEqual( len(values1), len(values2) )
	for strandId, value1 in iter(values1.items()):
		value2 = values2[strandId]
		error = abs( value1 - value2 )
		if error <= epsilon:
			nearEqualValuesCount = nearEqualValuesCount + 1
	return float(nearEqualValuesCount) / float(len(values1))

def CheckPercentageNearEqual( self, atualPercentage, desiredPercentage, errorMargin = 0.01, extraMessage = '' ):
	self.assertTrue( abs(atualPercentage - desiredPercentage) <= errorMargin, msg=extraMessage+' Percentage is ['+str(atualPercentage*100)+'%] should be around '+str(desiredPercentage*100)+'%' )

def CheckPointsAllNotNearEqual( self, points1, points2, epsilon = 0.01, pointsStart = 0, pointsCount = -1 ):
	self.assertEqual( len( points1 ), len( points2 ) )

	if pointsCount == -1:
		pointsCount = len( points1 ) - pointsStart

	for i in range( 0, pointsCount ):
		distance = ( points1[i + pointsStart] - points2[i + pointsStart] ).GetLength()
		self.assertLessEqual( epsilon, distance, 'Points near equal at index {}, value {}'.format( i + pointsStart, points1[i + pointsStart] ) )

def CheckPointsNotAllNearEqual( self, points1, points2, epsilon = 0.01, pointsStart = 0, pointsCount = -1, extraMessage = '' ):
	self.assertEqual( len( points1 ), len( points2 ) )
	allPointsAreEqual = True

	if pointsCount == -1:
		pointsCount = len( points1 ) - pointsStart

	for i in range( 0, pointsCount ):
		if( ( points1[pointsStart + i] - points2[pointsStart + i] ).GetLength() > epsilon ):
			allPointsAreEqual = False
			break
	self.assertFalse( allPointsAreEqual, 'All points are equal, expecting at least one not to be equal, error:' + extraMessage )

def CheckPointsNotAllNearEqualByStrandId( self, points1, points2, epsilon = 0.01, pointsStart = 0, pointsCount = -1 ):
	arePointsAllNearEqual = True
	for strandId, verticesInObjectSpace in iter(points1.items()):
		if not ArePointsAllNearEqual( self, verticesInObjectSpace, points2[strandId], epsilon = epsilon, points1Start = pointsStart, points2Start = pointsStart, pointsCount = pointsCount ):
			arePointsAllNearEqual = False
			break
	self.assertFalse( arePointsAllNearEqual, 'All points are equal, expecting at least one not to be equal' )

def CheckValuesAllNearEqualByStrandId( self, values1, values2, epsilon = 0.01, valuesStart = 0, valuesCount = -1 ):
	for strandId, value1 in iter(values1.items()):
		value2 = values2[strandId]
		diff = abs(value1 - value2)
		self.assertLessEqual( diff, epsilon, msg = '[' + str( value1 ) + '] is not near equal [' + str( value2 ) + '] at strandId: ' + str( strandId ) )


def CheckValuesNotAllNearEqualByStrandId( self, values1, values2, epsilon = 0.01, valuesStart = 0, valuesCount = -1 ):
	for strandId, value1 in iter(values1.items()):
		value2 = values2[strandId]
		diff = abs(value1 - value2)
		self.assertGreaterEqual( diff, epsilon, msg = '[' + str( value1 ) + '] is near equal [' + str( value2 ) + '] at strandId: ' + str( strandId ) )

def CheckC4dVectorsNearEqual( self, vector1, vector2, places=2, msg=None ):
	self.assertAlmostEqual( vector1[0], vector2[0], places, msg=msg )
	self.assertAlmostEqual( vector1[1], vector2[1], places, msg=msg )
	self.assertAlmostEqual( vector1[2], vector2[2], places, msg=msg )

def CheckC4dMatrixNearEqual( self, matrix1, matrix2, places=2, msg=None ):
	self.assertAlmostEqual( matrix1.off, matrix2.off, places, msg=msg )
	self.assertAlmostEqual( matrix1.v1, matrix2.v1, places, msg=msg )
	self.assertAlmostEqual( matrix1.v2, matrix2.v2, places, msg=msg )
	self.assertAlmostEqual( matrix1.v3, matrix2.v3, places, msg=msg )

def MakeSplineData(knots):
	splineData = c4d.SplineData()
	splineData.MakePointBuffer(len(knots))
	for i in range(len(knots)):
		splineData.SetKnot( i, knots[i], interpol=c4d.CustomSplineKnotInterpolationCubic )
	return splineData

def CheckC4dSplineParametersAreEqual( self, obj1, obj2, paramId ):
	CheckC4dSplineDataAreEqual( self, obj1[paramId], obj2[paramId] )

def CheckC4dSplineDataAreEqual( self, splineData1, splineData2, msg=None ):
	self.assertEqual( splineData1.GetKnotCount(), splineData2.GetKnotCount(), msg=msg )
	knots1 = splineData1.GetKnots()
	knots2 = splineData2.GetKnots()
	for i in range(splineData1.GetKnotCount()):
		CheckC4dVectorsNearEqual(self, knots1[i]['vPos'] , knots2[i]['vPos'], msg=msg )
		self.assertEqual(knots1[i]['interpol'] , knots2[i]['interpol'], msg=msg )

def CheckArrayParametersAreEqual( self, obj1, obj2, paramId, arrayDataType, allowEmpty, paramName ):
	arrayData1 = None
	arrayData2 = None
	if arrayDataType == Ox.res.ID_OX_BOOLEAN_ARRAY:
		arrayData1 = Ox.GetBooleanArrayData( obj1, paramId )
		arrayData2 = Ox.GetBooleanArrayData( obj2, paramId )
	elif arrayDataType == Ox.res.ID_OX_INTEGER_ARRAY:
		arrayData1 = Ox.GetIntegerArrayData( obj1, paramId )
		arrayData2 = Ox.GetIntegerArrayData( obj2, paramId )
	elif arrayDataType == Ox.res.ID_OX_LONG_ARRAY:
		arrayData1 = Ox.GetLongArrayData( obj1, paramId )
		arrayData2 = Ox.GetLongArrayData( obj2, paramId )
	elif arrayDataType == Ox.res.ID_OX_FLOAT_ARRAY:
		arrayData1 = Ox.GetFloatArrayData( obj1, paramId )
		arrayData2 = Ox.GetFloatArrayData( obj2, paramId )
	elif arrayDataType == Ox.res.ID_OX_VECTOR_ARRAY:
		arrayData1 = Ox.GetVectorArrayData( obj1, paramId )
		arrayData2 = Ox.GetVectorArrayData( obj2, paramId )
	elif arrayDataType == Ox.res.ID_OX_SPLINE_ARRAY:
		arrayData1 = Ox.GetSplineArrayData( obj1, paramId )
		arrayData2 = Ox.GetSplineArrayData( obj2, paramId )
	elif arrayDataType == Ox.res.ID_OX_STRING_ARRAY:
		arrayData1 = Ox.GetStringArrayData( obj1, paramId )
		arrayData2 = Ox.GetStringArrayData( obj2, paramId )
	CheckBaseContainersAreEqual( self, arrayData1, arrayData2, allowEmpty, paramName )

def CheckBaseContainersAreEqual( self, bc1, bc2, allowEmpty, paramName ):
	self.assertTrue( bc1 != None, msg=paramName+": array A is null" )
	self.assertTrue( bc2 != None, msg=paramName+": array B is null" )
	#print len(bc1)
	#print len(bc2)
	self.assertTrue( len(bc1) == len(bc2), msg=paramName+": array sizes are different" )
	if not allowEmpty:
		self.assertTrue( len(bc1) > 0, msg=paramName+": arrays are empty" )
	for i in range(len(bc1)):
		dataType1 = bc1.GetType(i)
		dataType2 = bc2.GetType(i)
		#print( str(i) + " (" + str(dataType1) + "): " + str(bc1[i]) + " == " + str(bc2[i]) )
		self.assertTrue(dataType1 == dataType2, msg=paramName+": types are different at index "+str(i) )
		if dataType1 == c4d.DA_LONG or dataType1 == c4d.DA_STRING:
			self.assertEqual( bc1[i], bc2[i], msg=paramName+": values are different at index "+str(i) )
		elif dataType1 == c4d.DA_REAL:
			self.assertAlmostEqual( bc1[i], bc2[i], msg=paramName+": values are different at index "+str(i) )
		elif dataType1 == c4d.DA_VECTOR:
			CheckC4dVectorsNearEqual( self, bc1[i], bc2[i], msg=paramName+": values are different at index "+str(i) )
		elif dataType1 == c4d.CUSTOMDATATYPE_SPLINE:
			CheckC4dSplineDataAreEqual( self, bc1[i], bc2[i], msg=paramName+": values are different at index "+str(i) )
		elif dataType1 == c4d.DA_CONTAINER:
			CheckBaseContainersAreEqual( self, bc1[i], bc2[i], allowEmpty, paramName )
		else:
			self.fail(paramName+": array data type "+str(dataType1)+" comparison not supported" )

def CheckPolygonsAreEqual( self, polygon1, polygon2, paramName ):
	self.assertTrue( polygon1 != None, msg=paramName+": polygon A is null" )
	self.assertTrue( polygon2 != None, msg=paramName+": polygon B is null" )
	self.assertTrue( polygon1.IsInstanceOf( c4d.Opolygon ), msg=paramName+": polygon A is not a polygon" )
	self.assertTrue( polygon2.IsInstanceOf( c4d.Opolygon ), msg=paramName+": polygon B is not a polygon" )
	# test poly count
	polyCount1 = polygon1.GetPolygonCount()
	polyCount2 = polygon2.GetPolygonCount()
	self.assertEqual( polyCount1, polyCount2, msg=paramName+": polygon count is different" )
	# test points
	pointCount1 = polygon1.GetPointCount()
	pointCount2 = polygon2.GetPointCount()
	self.assertEqual( pointCount1, pointCount2, msg=paramName+": point count is different" )
	points1 = polygon1.GetAllPoints()
	points2 = polygon2.GetAllPoints()
	for pointIndex in range( pointCount1 ):
		CheckC4dVectorsNearEqual( self, points1[pointIndex], points2[pointIndex], msg=paramName+": point ["+str(pointIndex)+"] is different" )
	# test indices
	for polyIndex in range( polyCount1 ):
		poly1 = polygon1.GetPolygon( polyIndex )
		poly2 = polygon2.GetPolygon( polyIndex )
		self.assertEqual( poly1.IsTriangle(), poly2.IsTriangle(), msg=paramName+": polygon ["+str(polyIndex)+"] type is different" )
		self.assertEqual( poly1.a, poly2.a, msg=paramName+": polygon ["+str(polyIndex)+"] index A is different" )
		self.assertEqual( poly1.b, poly2.b, msg=paramName+": polygon ["+str(polyIndex)+"] index B is different" )
		self.assertEqual( poly1.c, poly2.c, msg=paramName+": polygon ["+str(polyIndex)+"] index C is different" )
		if not poly1.IsTriangle():
			self.assertEqual( poly1.d, poly2.d, msg=paramName+": polygon ["+str(polyIndex)+"] index D is different" )
	# Test uvs
	uvs1 = polygon1.GetTag( c4d.Tuvw )
	uvs2 = polygon2.GetTag( c4d.Tuvw )
	self.assertTrue( uvs1 != None, msg=paramName+": polygon A has no uvs" )
	self.assertTrue( uvs2 != None, msg=paramName+": polygon B has no uvs" )
	uvCount1 = uvs1.GetDataCount()
	uvCount2 = uvs2.GetDataCount()
	self.assertTrue( uvCount1 > 0, msg=paramName+": UV A count is zero" )
	self.assertTrue( uvCount2 > 0, msg=paramName+": UV B count is zero" )
	self.assertTrue( uvCount1 == uvCount2, msg=paramName+": UV count is different" )
	for uvIndex in range( uvCount1 ):
		uvw1 = uvs1.GetSlow( uvIndex )
		uvw2 = uvs2.GetSlow( uvIndex )
		CheckC4dVectorsNearEqual( self, uvw1["a"], uvw2["a"], msg=paramName+": UV ["+str(uvIndex)+"] A is different" )
		CheckC4dVectorsNearEqual( self, uvw1["b"], uvw2["b"], msg=paramName+": UV ["+str(uvIndex)+"] B is different" )
		CheckC4dVectorsNearEqual( self, uvw1["c"], uvw2["c"], msg=paramName+": UV ["+str(uvIndex)+"] C is different" )
		if not poly1.IsTriangle():
			CheckC4dVectorsNearEqual( self, uvw1["d"], uvw2["d"], msg=paramName+": UV ["+str(uvIndex)+"] D is different" )

def CheckHairBounds( self, hair, expectedCenter, expectedSize, delta ):
	if expectedCenter != None:
		boundingBoxCenter = hair.GetMp()
		#print "Center = "+str(boundingBoxCenter)
		self.assertAlmostEqual( expectedCenter.x, boundingBoxCenter.x, delta=delta, msg="Hair center X is "+str(boundingBoxCenter.x)+", expected "+str(expectedCenter.x) )
		self.assertAlmostEqual( expectedCenter.y, boundingBoxCenter.y, delta=delta, msg="Hair center Y is "+str(boundingBoxCenter.y)+", expected "+str(expectedCenter.y) )
		self.assertAlmostEqual( expectedCenter.z, boundingBoxCenter.z, delta=delta, msg="Hair center Z is "+str(boundingBoxCenter.z)+", expected "+str(expectedCenter.z) )
	if expectedSize != None:
		boundingBoxSize = hair.GetRad() * 2
		#print "Size = "+str(boundingBoxSize)
		self.assertAlmostEqual( expectedSize.x, boundingBoxSize.x, delta=delta, msg="Hair bounds X is "+str(boundingBoxSize.x)+", expected "+str(expectedSize.x) )
		self.assertAlmostEqual( expectedSize.y, boundingBoxSize.y, delta=delta, msg="Hair bounds Y is "+str(boundingBoxSize.y)+", expected "+str(expectedSize.y) )
		self.assertAlmostEqual( expectedSize.z, boundingBoxSize.z, delta=delta, msg="Hair bounds Z is "+str(boundingBoxSize.z)+", expected "+str(expectedSize.z) )



# Creates a data channel in EG node and returns its ID
def CreateDataChannel( doc, eg, channelName, channelType ):
	args = c4d.BaseContainer()
	args[0] = channelName
	args[1] = channelType
	command = c4d.BaseContainer()
	command[Ox.res.edg_Button_Channels_Add] = args
	eg.Message(c4d.MSG_BASECONTAINER, command)
	doc.ExecutePasses( None, True, True, True, c4d.BUILDFLAGS_INTERNALRENDERER )
	result = command[Ox.res.edg_Button_Channels_Add]
	return result[1000]

def RenameDataChannel( doc, eg, newChannelName ):
	args = c4d.BaseContainer()
	args[0] = newChannelName
	command = c4d.BaseContainer()
	command[Ox.res.edg_Button_Channels_Rename] = args
	eg.Message( c4d.MSG_BASECONTAINER, command )
	doc.ExecutePasses( None, True, True, True, c4d.BUILDFLAGS_INTERNALRENDERER )

def SelectStrandsByRange( doc, node, firstStrandIndex, selectedStrandCount, totalStrandCount, commandId=Ox.res.edg_Command_SelectStrandsByIndex ):
	args = c4d.BaseContainer()
	for strandIndex in range(totalStrandCount-1):
		args[strandIndex] = True if (strandIndex >= firstStrandIndex and strandIndex < firstStrandIndex+selectedStrandCount) else False
	command = c4d.BaseContainer()
	command[commandId] = args
	node.Message(c4d.MSG_BASECONTAINER, command)
	doc.ExecutePasses( None, True, True, True, c4d.BUILDFLAGS_INTERNALRENDERER )

def SelectStrandsByStrandIds( doc, node, strandIds, commandId=Ox.res.edg_Command_SelectStrandsByStrandId ):
	args = c4d.BaseContainer()
	strandIndex = 0
	for strandId in strandIds:
		# StrandIds are long and cant be stored in a BaseContainer
		args.SetString( strandIndex, str(strandId) )
		strandIndex += 1
	command = c4d.BaseContainer()
	command[commandId] = args
	node.Message(c4d.MSG_BASECONTAINER, command)
	doc.ExecutePasses( None, True, True, True, c4d.BUILDFLAGS_INTERNALRENDERER )

# Same as pressing a button on the Attributes Manager
def SendDescriptionCommand( node, commandId ):
	node.Message(c4d.MSG_DESCRIPTION_COMMAND, {"id": commandId})

def CachePrimitiveIntoNewObject(primitive, doc, name=None):
	primitive.SetDirty(c4d.DIRTYFLAGS_CACHE)
	commandResult = c4d.utils.SendModelingCommand(command=c4d.MCOMMAND_JOIN,list=[primitive],mode=c4d.MODELINGCOMMANDMODE_ALL,doc=doc)
	c4d.EventAdd()
	result = commandResult[0]
	if name:
		result.SetName(name)
	doc.InsertObject( result )
	return result

def MakeTransformMatrix( position=c4d.Vector(0,0,0), rotation=c4d.Vector(0,0,0), scale=c4d.Vector(1,1,1) ):
	m = c4d.Matrix()
	m = m * c4d.utils.MatrixMove( position )
	m = m * c4d.utils.MatrixScale( scale )
	m = m * c4d.utils.HPBToMatrix( c4d.Vector(c4d.utils.DegToRad(rotation.x), c4d.utils.DegToRad(rotation.y), c4d.utils.DegToRad(rotation.z) ), c4d.ROTATIONORDER_XYZGLOBAL )
	return m

def GetTempFilePath( suffix ):
	tempUrl = maxon.Application.GetTempUrl( maxon.Application.GetUrl( maxon.APPLICATION_URLTYPE.TEMP_DIR ) )
	tempUrl.SetSuffix( suffix )
	tempPath = maxon.Application.GetUrl( maxon.APPLICATION_URLTYPE.TEMP_DIR ).GetPath()
	if sys.platform == 'darwin' and tempPath[0] != '/':
		tempPath = "/" + tempPath
	tempPath = tempPath + "/" + tempUrl.GetName()
	return tempPath

def SaveTempDocument( doc ):
	# invalidate test unit objects to avoid looping
	clonedDoc = doc.GetClone()
	op = clonedDoc.GetFirstObject()
	while op is not None:
		if op.GetType() == 1050494:
			op[Ox.res.test_ScriptExecuteOnLoad] = False
		op = op.GetNext()
	# save temp file and return it
	tempUrl = maxon.Application.GetTempUrl( maxon.Application.GetUrl( maxon.APPLICATION_URLTYPE.TEMP_DIR ) )
	c4d.documents.SaveDocument( clonedDoc, tempUrl, c4d.SAVEDOCUMENTFLAGS_NONE, c4d.FORMAT_C4DEXPORT )
	c4d.documents.KillDocument( clonedDoc )
	return tempUrl

def MakeAxisAlignedBlackAndWhiteShaderX( node, paramId, zAxis=False ):
	# make gradient shader concentrated in x < 0
	shader = c4d.BaseShader( c4d.Xgradient )
	grad = shader[c4d.SLA_GRADIENT_GRADIENT]
	grad.SetKnot(0, c4d.Vector(0, 0, 0), 1, 0.5)
	grad.SetKnot(1, c4d.Vector(1, 1, 1), 100, 0)
	shader[c4d.SLA_GRADIENT_GRADIENT] = grad
	shader[c4d.SLA_GRADIENT_TYPE] = c4d.SLA_GRADIENT_TYPE_2D_V if zAxis else c4d.SLA_GRADIENT_TYPE_2D_U
	node.InsertShader( shader )
	node[paramId] = shader
	return shader

def MakeAxisAlignedBlackAndWhiteShaderZ( node, paramId ):
	return MakeAxisAlignedBlackAndWhiteShaderX( node, paramId, zAxis=True )

def MakeGrayscaleShader( node, paramId, value ):
	shader = c4d.BaseShader( c4d.Xcolor )
	shader[c4d.COLORSHADER_COLOR] = c4d.Vector(value, value, value)
	node.InsertShader( shader )
	node[paramId] = shader

def CheckStrandPlaneDisplacementByStrandId( self, strands, expectedDisplacement, delta=0.1 ):
	for strandId, strandPoints in iter(strands.items()):
		root = strandPoints[0]
		tip = strandPoints[len(strandPoints)-1]
		directionVector = ( c4d.Vector(tip.x, 0, tip.z) - c4d.Vector(root.x, 0, root.z))
		displacement = directionVector.GetLength()
		self.assertAlmostEqual( displacement, expectedDisplacement, delta=delta )

def CheckStrandDirectionByStrandId( self, strands, expectedAngleDegrees, delta=1 ):
	displacementSum = 0.0
	for strandId, strandPoints in iter(strands.items()):
		root = strandPoints[0]
		tip = strandPoints[len(strandPoints)-1]
		directionVector = ( c4d.Vector(tip.x, 0, tip.z) - c4d.Vector(root.x, 0, root.z))
		degrees = math.degrees(math.atan2(directionVector.x, directionVector.z))
		if degrees < 0:
			degrees = degrees + 360.0
		self.assertAlmostEqual( degrees, expectedAngleDegrees, delta=delta )

def CreateFloatKey( doc, curve, time, value ):
	# Add the key
	keyDict = curve.AddKey( time )
	if keyDict is None:
		raise MemoryError( "Failed to create a key" )
	# Retrieve the inserted key
	key = keyDict[ "key" ]
	keyIndex = keyDict[ "nidx" ]
	key.SetValue( curve, value )
	# Fill the key with default settings (mandatory)
	curve.SetKeyDefault( doc, keyIndex )
	key.SetInterpolation( curve, c4d.CINTERPOLATION_LINEAR )
	return key, keyIndex

def FindTestFile( self, filename ):
	fullpath = os.path.join( os.path.dirname( os.path.realpath( __file__ ) ), filename )
	if os.access( fullpath, os.F_OK ):
		return fullpath
	fullpath = os.path.join( RepositoryRootPath, 'External', 'TestData', filename )
	if os.access( fullpath, os.F_OK ):
		return fullpath
	self.skipTest( 'Test file not found: ' + filename )
