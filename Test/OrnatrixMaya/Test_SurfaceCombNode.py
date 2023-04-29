import MayaTest
import TestUtilities
import os.path
import pymel.core as pm
import pymel.core.datatypes as dt

class Test_SurfaceCombNode( MayaTest.OxTestCase ):

	#def test_BasicSurfaceComb( self ):
		## Check that at least some vertices are modified
		#TestUtilities.TestStrandOperatorChangingGuides( self, TestUtilities.SurfaceCombNodeName, False, 0.1, pointsPerStrandCount = 4 )

	def test_AddSink( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()

		guidesFromMeshNode = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]

		pm.mel.OxAddStrandOperator( guidesFromMeshNode, TestUtilities.SurfaceCombNodeName )
		surfaceCombNode = pm.ls( type = TestUtilities.SurfaceCombNodeName )[0]
		surfaceCombNode.chaos.set( 0.0 )
		surfaceCombNode.algorithm.set( 0 )

		# Record guide tip positions before adding the sink
		tipPositionsBeforeSinks = TestUtilities.GetTipPositions( guidesShape, useObjectCoordinates = True )

		pm.mel.eval( 'OxEditSurfaceCombSink -a -p 0 0 0 -d 0 0 1 -f 0 -c 0.5 0.5 -typ 2' )

		tipPositionsAfterSinks = TestUtilities.GetTipPositions( guidesShape, useObjectCoordinates = True )

		sinkDirection = dt.Vector( 0.0, 0.0, 1.0 )
		# The tips of the sinks should have changed and they should be pointing in the same direction as the sink
		for i in range( 0, len( tipPositionsBeforeSinks ) ):
			positionDelta = dt.Vector( tipPositionsAfterSinks[i] ) - dt.Vector( tipPositionsBeforeSinks[i] )
			positionDeltaWithoutSlope = dt.Vector( 0, 0, positionDelta.z ).normal()
			self.assertLess( abs( dt.dot( positionDeltaWithoutSlope, sinkDirection ) - 1.0 ), 0.01, positionDelta )

	def test_AddCurvedSink( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		pm.parent( guidesShape.firstParent2(), "pPlane1" )

		polyPlane = pm.ls( type = 'polyPlane' )[0]
		polyPlane.subdivisionsHeight.set( 4 )
		polyPlane.subdivisionsWidth.set( 4 )

		guidesFromMeshNode = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]

		pm.mel.OxAddStrandOperator( guidesFromMeshNode, TestUtilities.SurfaceCombNodeName )
		surfaceCombNode = pm.ls( type = TestUtilities.SurfaceCombNodeName )[0]
		surfaceCombNode.algorithm.set( 0 )
		surfaceCombNode.chaos.set( 0.0 )
		guidesFromMeshNode.distribution.set( 6 )

		# Record guide tip positions before adding the sink
		tipPositionsBeforeSinks = TestUtilities.GetTipPositions( guidesShape, useObjectCoordinates = True )

		curve1 = pm.curve( p=[(0.05, 0, 0.5), (0.05, 0, 0.25), (0.05, 0, -0.25), (0.05, 0, -0.5)], k=[0, 0, 0, 1, 1, 1] )
		pm.connectAttr( "curveShape1.worldSpace[0]", "%s.sinkCurves[0]" % surfaceCombNode.name() )

		# The tips of the sinks should have changed and they should be pointing in the same direction as the curve
		tipPositionsAfterSinks = TestUtilities.GetTipPositions( guidesShape, useObjectCoordinates = True )
		sinkDirection = dt.Vector( 0.0, 0.0, -1.0 )

		for i in range( 0, len( tipPositionsBeforeSinks ) ):
			positionDelta = dt.Vector( tipPositionsAfterSinks[i] ) - dt.Vector( tipPositionsBeforeSinks[i] )
			positionDeltaWithoutSlope = dt.Vector( 0, 0, positionDelta.z ).normal()
			self.assertLess( abs( dt.dot( positionDeltaWithoutSlope, sinkDirection ) - 1.0 ), 0.01, positionDelta )

	def test_DetachSinks(self):
		guidesShape = self.CreatePlaneWithTwoSinks()
		guidesFromMeshNode = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		guidesFromMeshNode.setAttr( 'count', 1 )
		guidesFromMeshNode.setAttr( 'distribution', 0 )

		origTipsPositions = TestUtilities.GetTipPositions( guidesShape, useObjectCoordinates = True )

		surfaceCombNode = pm.ls( type = TestUtilities.SurfaceCombNodeName )[0]
		polyPlane = pm.ls( type = 'polyPlane' )[0]

		surfaceCombNode.detachSinks.set( 1 )
		TestUtilities.GetTipPositions( guidesShape )

		polyPlane.subdivisionsHeight.set( 5 )
		polyPlane.subdivisionsWidth.set( 5 )
		surfaceCombNode.detachSinks.set( 0 )
		TestUtilities.CheckTipPositionsEqual( self, guidesShape, origTipsPositions, useObjectCoordinates = True )

	def CreatePlaneWithTwoSinks( self ):
		# Add guides and add a surface comb to them
		guidesShape = TestUtilities.AddGuidesToNewPlane()

		polyPlane = pm.ls( type = 'polyPlane' )[0]
		polyPlane.subdivisionsHeight.set( 4 )
		polyPlane.subdivisionsWidth.set( 4 )

		guidesFromMeshNode = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		pm.mel.OxAddStrandOperator( guidesFromMeshNode, TestUtilities.SurfaceCombNodeName )
		surfaceCombNode = pm.ls( type = TestUtilities.SurfaceCombNodeName )[0]
		surfaceCombNode.algorithm.set( 0 )

		# Add a couple of sinks
		#pm.mel.eval( 'OxEditSurfaceCombSink -a -p 0 0 0 -d 0 0 1 -f 0 -c 0.1 0.1 -typ 2' )
		pm.mel.eval( 'OxEditSurfaceCombSink -a -p 0 0 0 -d 1 0 0 -f 3 -c 0.6 0.75 -typ 2' )

		return guidesShape

	def test_SaveLoad( self ):
		# Add guides and add a surface comb to them
		guidesShape = self.CreatePlaneWithTwoSinks()

		# Record vertex positions before saving
		tipPositionsBeforeSaving = TestUtilities.GetTipPositions( guidesShape )

		# Save and load the scene
		filePath = pm.saveAs( 'test_SaveLoad.ma' )
		pm.openFile( filePath, force = True )

		try:
			guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]

			# Record vertex positions after loading
			TestUtilities.CheckTipPositionsEqual( self, guidesShape, tipPositionsBeforeSaving )
		finally:
			pm.newFile( force = 1 )

	def test_SaveLoadRandom( self ):
		# Add guides and add a surface comb to them
		guidesShape = TestUtilities.AddGuidesToNewPlane( guideCount = 20, rootGenerationMethod = 2 )

		guidesFromMeshNode = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]

		# Remember root positions to make sure that tips will have the same order when reloaded
		guidesFromMeshNode.rememberRootPositions.set( 1 )
		pm.mel.OxAddStrandOperator( guidesFromMeshNode, TestUtilities.SurfaceCombNodeName )

		# Record vertex positions before saving
		tipPositionsBeforeSaving = TestUtilities.GetTipPositions( guidesShape )

		# Save and load the scene
		filePath = pm.saveAs( 'test_SaveLoad.mb' )
		pm.openFile( filePath, force = True )

		try:
			guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]

			# Record vertex positions after loading
			TestUtilities.CheckTipPositionsEqual( self, guidesShape, tipPositionsBeforeSaving )
		finally:
			pm.newFile( force = 1 )

	def test_SaveLoadAfterUndo( self ):
		guidesShape = self.CreatePlaneWithTwoSinks()

		# Undo creation of last sink
		pm.undo()
		tipPositionsBeforeSaving = TestUtilities.GetTipPositions( guidesShape )

		# Save and load the scene
		filePath = pm.saveAs( 'test_SaveLoadAfterUndo.ma' )
		pm.openFile( filePath, force = True )

		# There should not have been any assertions after load
		try:
			guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]

			# Need to call this to make sure that points are evaluated
			TestUtilities.CheckTipPositionsEqual( self, guidesShape, tipPositionsBeforeSaving )
		finally:
			pm.newFile( force = 1 )

	# Reproduces problem in https://ephere.com:3000/issues/1945:
	# The Surface Comb operator is causing the guides to "jump", rotating or moving the guides from (Presumably) one vertex to another, even if Remember Roots is ON in GuidesFromMesh.
	def test_StrandsDontJumpOnAnimatedMesh( self ):
		meshShape = pm.polySphere( r = 10 )[0]
		latticeShape = pm.lattice( dv = ( 2, 2, 2 ), oc = True )[1]

		startTime = 0
		endTime = 5

		# Animate lattice points so the sphere deforms
		pm.currentTime( startTime )
		pm.select( latticeShape + '.pt[0:1][1][0]', r = True )
		pm.select( latticeShape + '.pt[0:1][1][1]', add = True )
		pm.setKeyframe()
		pm.currentTime( endTime )

		# Add slight movement, enough to deform the sphere to make the guides jump if there is a problem
		pm.move( 0, 1, 0, r = True )
		pm.setKeyframe()

		guidesShape = TestUtilities.AddGuidesToMesh( meshShape, pointsPerStrandCount = 4, length = 10 )

		surfaceCombNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.SurfaceCombNodeName ) )
		surfaceCombNode.algorithm.set( 0 )

		# Must evaluate to copy the algorithm value change into comber
		TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Add a couple of sinks. It is very important that the sinks are placed properly to reproduce this issue. The values were taken directly from viewport manual editing
		# and might need adjustment if sink placement code or if sphere topology changes
		pm.mel.eval( 'OxEditSurfaceCombSink -a -p -3.573807 10.369201 -8.496937 -d -10.686404 1.077389 4.782318 -f 225 -c 0.399405 0.263087 -typ 2' )
		pm.mel.eval( 'OxEditSurfaceCombSink -a -p -6.912111 3.436181 -6.941308 -d 0.66401 -6.625112 -0.00432357 -f 166 -c 0.171045 0.495398 -typ 2' )
		pm.mel.eval( 'OxEditSurfaceCombSink -a -p 1.609374 6.161289 -9.684175 -d 5.893126 1.717312 1.073268 -f 183 -c 0.0319979 0.473103 -typ 2' )

		# Over time the relative positions of vertices shouldn't change too much if guides are not jumping

		pm.currentTime( startTime )
		previousPointPositions = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		for frame in range( startTime + 1, endTime + 1 ):
			pm.currentTime( frame )
			pointPositions = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
			TestUtilities.CheckPointsAllNearEqual( self,  previousPointPositions, pointPositions, 1.0 )
			previousPointPositions = pointPositions

	def test_SinksAreSavedWithGroom( self ):
		groomPath = self.addTempFile( "test_SinksAreSavedWithGroom" + MayaTest.NewGroomExtension )
		# Add guides with two sinks
		guidesShape = self.CreatePlaneWithTwoSinks()
		tipPositionsBeforeSaving = TestUtilities.GetTipPositions( guidesShape )

		pm.select( guidesShape )
		pm.mel.OxSaveGroom( path = groomPath, optional = True )

		# Load groom with no selection
		pm.newFile( force = 1 )
		guidesShape = pm.PyNode( pm.mel.OxLoadGroom( path = groomPath ) )
		tipPositionsAfterLoading = TestUtilities.GetTipPositions( guidesShape )

		# If sink positions were saved with the groom we should get identical tips
		TestUtilities.CheckPointsAllNearEqual( self,  tipPositionsBeforeSaving, tipPositionsAfterLoading )
