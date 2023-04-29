import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt

class Test_SymmetryNode( MayaTest.OxTestCase ):

	def test_SymmetryDuplicatesVertices( self ):
		hairShape = TestUtilities.AddHairToNewPlane()

		vertexCountBeforeSymmetry = pm.mel.OxGetVertexCount( hairShape )
		verticesBeforeSymmetryInObjectCoordinates = pm.mel.OxGetVertices( hairShape, os = True )
		symmetry = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.SymmetryNodeName ) )
		symmetry.groundMirroredStrands.set( False )
		vertexCountAfterSymmetry = pm.mel.OxGetVertexCount( hairShape )
		verticesAfterSymmetryInObjectCoordinates = pm.mel.OxGetVertices( hairShape, os = True )

		# Symmetry should have doubled the hair count
		self.assertEqual( vertexCountBeforeSymmetry * 2, vertexCountAfterSymmetry )
		self.assertEqual( len( verticesBeforeSymmetryInObjectCoordinates ) * 2, len( verticesAfterSymmetryInObjectCoordinates ) )

		# The original vertices should remain the same as before
		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeSymmetryInObjectCoordinates, verticesAfterSymmetryInObjectCoordinates, pointsCount = len( verticesBeforeSymmetryInObjectCoordinates ) )

		# The new vertices should be mirrored along X axis

		# Symmetry along Y axis
		symmetry.symmetryPlaneAxis.set( 1 )
		verticesAfterSymmetryInObjectCoordinates = pm.mel.OxGetVertices( hairShape, os = True )
		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeSymmetryInObjectCoordinates, verticesAfterSymmetryInObjectCoordinates, pointsCount = len( verticesBeforeSymmetryInObjectCoordinates ) )

		TestUtilities.CheckUniqueStrandIds( self, hairShape )

	def test_ReloadSymmetryOnTopOfDeletedGuides( self ):
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane()
		pm.mel.eval( "OxEditGuides -ds 4 0 0 1 1;" )
		symmetry = pm.PyNode( pm.mel.OxAddStrandOperator( editGuidesShape, TestUtilities.SymmetryNodeName ) )
		symmetry.symmetryPlaneAxis.set( 2 )

		# Test that vertices are placed correctly
		verticesAfterSymmetry = pm.mel.OxGetVertices( editGuidesShape, os = True )
		expectedCoordinates = [-0.5, 0, 0.5, -0.5, 10, 0.5, 0.5, 0, 0.5, 0.5, 10, 0.5, -0.5, 0, -0.5, -0.5, 10, -0.5, 0.5, 0, -0.5, 0.5, 10, -0.5]
		for i in range( 0, len( verticesAfterSymmetry ) ):
			self.assertAlmostEqual( verticesAfterSymmetry[i], expectedCoordinates[i] )

		filePath = pm.saveAs( self.addTempFile( 'test_ReloadSymmetryOnTopOfDeletedGuides.ma' ) )
		pm.openFile( filePath, force = True )

		editGuidesShape = pm.ls( type = TestUtilities.EditGuidesShapeName )[0]
		verticesAfterSymmetry = pm.mel.OxGetVertices( editGuidesShape, os = True )
		for i in range( 0, len( verticesAfterSymmetry ) ):
			self.assertAlmostEqual( verticesAfterSymmetry[i], expectedCoordinates[i] )

	def test_SymmetryAlongXAxisAppliedRightAway( self ):
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane()

		# Delete two guides along the X axis
		pm.mel.eval( "OxEditGuides -ds 4 0 1 0 1;" )

		# Add symmetry, byt default it should be mirroring along X axis
		symmetry = pm.PyNode( pm.mel.OxAddStrandOperator( editGuidesShape, TestUtilities.SymmetryNodeName ) )

		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		rootsAfterSymmetry = TestUtilities.GetRootPositions( guidesShape )

		# First two roots with negative X coordinate
		TestUtilities.CheckPointsNearEqual( self, [-0.5, 0.0, 0.5], rootsAfterSymmetry[0] )
		TestUtilities.CheckPointsNearEqual( self, [-0.5, 0.0, -0.5], rootsAfterSymmetry[1] )

		# The last two roots which were mirrored should have positive X coordinate
		TestUtilities.CheckPointsNearEqual( self, [0.5, 0.0, 0.5], rootsAfterSymmetry[2] )
		TestUtilities.CheckPointsNearEqual( self, [0.5, 0.0, -0.5], rootsAfterSymmetry[3] )
		
	def test_SymmetryStrandGroups( self ):
		# Create some guides, add a root channel, and set some values to it
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane( pointsPerStrandCount = 2, guideLength = 1, lengthRandomness = 0 )
		pm.select( editGuidesShape )
		
		# Delete two guides along the X axis
		pm.mel.eval( "OxEditGuides -ds 4 0 1 0 1;" )
		
		# Assign group 5 to last guide
		editGuidesShape.useStrandGroups.set( True )
		pm.select( editGuidesShape + ".ep[0]" )
		pm.mel.OxAssignStrandGroup( editGuidesShape, "5" )
		
		# Add symmetry, by default it should be mirroring along X axis
		symmetry = pm.PyNode( pm.mel.OxAddStrandOperator( editGuidesShape, TestUtilities.SymmetryNodeName ) )
		symmetry.strandGroup.set( "5" )

		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		rootsAfterSymmetry = TestUtilities.GetRootPositions( guidesShape )
		
		# should mirror 1 root in addition to the existing 2
		self.assertEqual( 3, len( rootsAfterSymmetry ) )
		
		# First two roots with negative X coordinate
		TestUtilities.CheckPointsNearEqual( self, [-0.5, 0.0, 0.5], rootsAfterSymmetry[0] )
		TestUtilities.CheckPointsNearEqual( self, [-0.5, 0.0, -0.5], rootsAfterSymmetry[1] )

		# The last root which was mirrored should have positive X coordinate
		TestUtilities.CheckPointsNearEqual( self, [0.5, 0.0, 0.5], rootsAfterSymmetry[2] )
		
	def test_SymmetryIgnoreDistance( self ):
		# Create some guides, add a root channel, and set some values to it
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane( planeSegmentCount = 3, pointsPerStrandCount = 2, guideLength = 1, lengthRandomness = 0 )
		pm.select( editGuidesShape )
		
		# Delete all guides on the negative side of the X axis
		pm.mel.eval( "OxEditGuides -dg 8 0 8 1 9 4 12 5 13;" )
		
		# Add symmetry, by default it should be mirroring along X axis
		symmetry = pm.PyNode( pm.mel.OxAddStrandOperator( editGuidesShape, TestUtilities.SymmetryNodeName ) )
		symmetry.useIgnoreDistance.set( 1 )
		symmetry.ignoreDistance.set( 0.2 )
		
		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		rootsAfterSymmetry = TestUtilities.GetRootPositions( guidesShape )
		
		# should mirror 4 roots in addition to the existing 2 * 4
		self.assertEqual( 4 * 3, len( rootsAfterSymmetry ) )
		
		# test the last row
		TestUtilities.CheckPointsNearEqual( self, [-0.5, 0.0,  0.5], rootsAfterSymmetry[8] )
		TestUtilities.CheckPointsNearEqual( self, [-0.5, 0.0,  1.0 / 6.0], rootsAfterSymmetry[9] )
		TestUtilities.CheckPointsNearEqual( self, [-0.5, 0.0, -1.0 / 6.0], rootsAfterSymmetry[10] )
		TestUtilities.CheckPointsNearEqual( self, [-0.5, 0.0, -0.5], rootsAfterSymmetry[11] )
