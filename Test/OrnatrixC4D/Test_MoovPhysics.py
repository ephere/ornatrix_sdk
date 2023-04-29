import os
import sys
import c4d
import unittest
from unittest import TestCase

import TestUtilities
sys.path.insert(0, os.path.dirname(__file__))
import Ornatrix.Ornatrix as Ox
sys.path.pop(0)


def RunAnimation( doc, startFrame, endFrame, fps = 30 ):
	if endFrame < startFrame:
		return

	doc.SetFps( fps )
	startTime = c4d.BaseTime( startFrame, fps )
	doc.SetMinTime( startTime )
	doc.SetMaxTime( c4d.BaseTime( endFrame, fps ) )

	frameCount = endFrame - startFrame + 1
	for frame in range( frameCount ):
		currentTime = startTime + c4d.BaseTime( frame, fps )
		doc.SetTime( currentTime )
		doc.ExecutePasses( None, True, True, True, c4d.BUILDFLAGS_INTERNALRENDERER )


def GetMoovParameterId( moov, parameterName ):

	for id, description in moov.GetUserDataContainer():
		#print( 'Searching name: {}, id: {}, value: {}'.format( description[c4d.DESC_NAME], id, str( moov[id] ) ) )
		if parameterName == description[c4d.DESC_NAME]:
			#print( 'Found {} id: {}, value: {}'.format( parameterName, id, str( moov[id] ) ) )
			return id

	#description = moov.GetDescription( c4d.DESCFLAGS_DESC_NONE )
	#for container, paramId, groupId in description:
	#	print( 'Unsuccessful name: {}, paramId: {}, groupId: {}'.format( container[c4d.DESC_NAME], paramId, groupId ) )
	#	if parameterName == container[c4d.DESC_NAME]:
	#		print( 'paramId: {}, groupId: {}'.format( paramId, groupId ) )
	#		return paramId, groupId

	# Throw an exception if parameter was not found
	raise ValueError( 'MoovPhysics: parameter \'{}\' not found'.format( parameterName ) )

def ConnectMoovCollisionMesh( moov, mesh ):
	collisionMeshDescId = c4d.DescID( c4d.DescLevel( Ox.res.moov_CollisionMeshesAttribute, c4d.CUSTOMDATATYPE_INEXCLUDE_LIST, 0 ) )
	collisionMeshData = moov.GetParameter( collisionMeshDescId, c4d.DESCFLAGS_GET_NONE )
	collisionMeshData.InsertObject( mesh, 1 )
	moov.SetParameter( collisionMeshDescId, collisionMeshData, c4d.DESCFLAGS_SET_NONE )

#@unittest.skip( "Currently crashes on both Mac and Windows" )
class Test_MoovPhysics( TestCase ):

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		Ox.DeleteAll( self.doc )

	def tearDown( self ):
		Ox.DeleteAll( self.doc )
		self.doc = None

	def test_BasicDynamics( self ):
		guideCount = 10
		hair = TestUtilities.AddGuidesToNewPlane( guideCount = guideCount, rootGenerationMethod = 7, pointsPerStrandCount = 10 )
		moov = c4d.BaseObject( Ox.res.ID_OX_MOOV_PHYSICS_MOD )
		moov.InsertUnderLast( hair )

		# Build
		Ox.ComputeStack( self.doc )

		self.assertEqual( Ox.GetStrandCount( moov ), guideCount )

		verticesBefore = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( moov )
		RunAnimation( self.doc, startFrame = 0, endFrame = 3 )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( moov )

		TestUtilities.CheckPointsNotAllNearEqualByStrandId( self, verticesBefore, verticesAfter )

	#@unittest.skip( "Temporary disabled, fails on the build server" )
	def test_DisplacedCollisionMesh( self ):
		displacement = c4d.Vector( 50, 0, 0 )
		hair = TestUtilities.AddGuidesToNewPlane( guideCount = 4, rootGenerationMethod = 7, pointsPerStrandCount = 10, length = 10 )
		plane = TestUtilities.GetObjectsByType( self.doc, c4d.Oplane )[0]
		plane.SetRelPos( displacement )

		collisionPlane = c4d.BaseObject( c4d.Oplane )
		collisionPlane[c4d.PRIM_PLANE_WIDTH] = 20
		collisionPlane[c4d.PRIM_PLANE_HEIGHT] = 20
		collisionPlane.SetRelPos( displacement )
		self.doc.InsertObject( collisionPlane )

		moov = c4d.BaseObject( Ox.res.ID_OX_MOOV_PHYSICS_MOD )
		moov.InsertUnderLast( hair )

		ConnectMoovCollisionMesh( moov, collisionPlane )

		# Build
		Ox.ComputeStack( self.doc )

		# Python parameters can only be modified after build
		moov[GetMoovParameterId( moov, 'StretchingStiffness' )] = 0.01
		moov[GetMoovParameterId( moov, 'BendingStiffness' )] = 0
		moov[GetMoovParameterId( moov, 'ParticleRadius' )] = 0.5
		moov[GetMoovParameterId( moov, 'CollideWithMeshes' )] = True

		RunAnimation( self.doc, startFrame = 0, endFrame = 15 )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( moov )
		# Need to have some free (non-root) vertices for meaningful test
		self.assertGreater( len( verticesAfter ), 2 )

		for point in verticesAfter:
			self.assertGreater( point[1], -0.01 )

	def test_AttachmentDynamics( self ):
		plane = c4d.BaseObject( c4d.Oplane )
		plane[c4d.PRIM_PLANE_WIDTH] = 10
		plane[c4d.PRIM_PLANE_HEIGHT] = 10
		plane[c4d.PRIM_PLANE_SUBW] = 2
		plane[c4d.PRIM_PLANE_SUBH] = 2
		self.doc.InsertObject( plane )

		## Move plane vertices to create rooftop geometry
		plane = c4d.utils.SendModelingCommand( command = c4d.MCOMMAND_MAKEEDITABLE, list = [plane], mode = c4d.MODELINGCOMMANDMODE_POINTSELECTION, doc = self.doc )[0]
		self.doc.InsertObject( plane )
		for pointIndex in [3, 4, 5]:
			position = plane.GetPoint( pointIndex )
			position[1] += 4
			plane.SetPoint( pointIndex, position )

		guides = TestUtilities.AddGuidesToMesh( plane, guideCount = 10, rootGenerationMethod = 2, pointsPerStrandCount = 5, lengthRandomness = 0, length = 10 )

		moov = c4d.BaseObject( Ox.res.ID_OX_MOOV_PHYSICS_MOD )
		moov.InsertUnderLast( guides )

		# Build
		Ox.ComputeStack( self.doc )

		# Python parameters can only be modified after build
		moov[GetMoovParameterId( moov, 'ModelType' )] = 0
		moov[GetMoovParameterId( moov, 'AttachmentDensity' )] = 1
		moov[GetMoovParameterId( moov, 'AttachmentStiffness' )] = 1

		attachmentMesh = c4d.BaseObject( c4d.Ocube )
		attachmentMesh[c4d.PRIM_CUBE_LEN, c4d.VECTOR_X] = 20
		attachmentMesh[c4d.PRIM_CUBE_LEN, c4d.VECTOR_Y] = 5
		attachmentMesh[c4d.PRIM_CUBE_LEN, c4d.VECTOR_Z] = 30
		attachmentMesh.SetRelPos( c4d.Vector( 0, 10, 0 ) )
		self.doc.InsertObject( attachmentMesh )

		# Connect attachment mesh
		attachmentMeshDescId = GetMoovParameterId( moov, 'AttachmentMeshes' )
		attachmentMeshData = moov.GetParameter( attachmentMeshDescId, c4d.DESCFLAGS_GET_NONE )
		attachmentMeshData.InsertObject( attachmentMesh, 1 )
		moov.SetParameter( attachmentMeshDescId, attachmentMeshData, c4d.DESCFLAGS_SET_NONE )

		#pm.hide( attachmentMesh[0] )
		#verticesBefore = TestUtilities.GetTipPositions( moov, useObjectCoordinates = True )
		verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( moov )
		RunAnimation( self.doc, startFrame = 0, endFrame = 5 )
		#verticesAfter = TestUtilities.GetTipPositions( moov, useObjectCoordinates = True )
		verticesAfter = TestUtilities.GetVerticesInObjectCoordinates( moov )

		# Check that hair does not fall down because it is held by the attachment
		for vertexBefore, vertexAfter in zip( verticesBefore, verticesAfter ):
			self.assertGreater( 2, vertexBefore[1] - vertexAfter[1] )


#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_MoovPhysics )