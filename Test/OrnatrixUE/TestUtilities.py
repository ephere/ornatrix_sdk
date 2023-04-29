import UETest
import unreal

def CreateBasicActor( name ):
	# First we load static mesh(sphere) from the engine content directory
	meshAsset = unreal.load_asset( '/Engine/BasicShapes/' + name + '.' + name )
		
	# Then we spawn an actor using EditorLevelLibrary
	actorLocation = unreal.Vector( 0.0, 0.0, 0.0 )
	actorRotation = unreal.Rotator( 0.0, 0.0, 0.0 )
	return unreal.EditorLevelLibrary.spawn_actor_from_object( meshAsset, actorLocation, actorRotation )

def CheckPointsNearEqual( self, point1, point2, epsilon = 0.01, extraMessage = '' ):
	error = point1.distance( point2 )
	self.assertGreaterEqual( epsilon, error, msg = '[' + str( point1 ) + '] is not near equal [' + str( point2 ) + '], error: ' + str( error ) + extraMessage )

def CheckPointsNotNearEqual( self, point1, point2, epsilon = 0.01, extraMessage = '' ):
	error = point1.distance( point2 )
	self.assertLessEqual( epsilon, error, msg = '[' + str( point1 ) + '] is near equal [' + str( point2 ) + '], error: ' + str( error ) + extraMessage )

def CheckPointsAllNearEqual( self, points1, points2, epsilon = 0.01, points1Start = 0, points2Start = 0, pointsCount = -1, extraMessage = '' ):
	if pointsCount == -1:
		pointsCount = len( points1 ) - points1Start
		self.assertEqual( pointsCount, len( points2 ) - points2Start )

	for i in range( 0, pointsCount ):
		CheckPointsNearEqual( self, points1[i + points1Start], points2[i + points2Start], epsilon = epsilon, extraMessage = extraMessage )

def CheckPointsNotAllNearEqual( self, points1, points2, epsilon = 0.01, points1Start = 0, points2Start = 0, pointsCount = -1, extraMessage = '' ):
	if pointsCount == -1:
		pointsCount = len( points1 ) - points1Start
		self.assertEqual( pointsCount, len( points2 ) - points2Start )

	areAllEqual = True
	for i in range( 0, pointsCount ):
		if not CheckPointsNearEqual( self, points1[i + points1Start], points2[i + points2Start], epsilon = epsilon, extraMessage = extraMessage ):
			areAllEqual = False
			break

	self.assertFalse( areAllEqual )