import MayaTest
import TestUtilities
import pymel.core as pm

class Test_HairFromMeshStripsNode( MayaTest.OxTestCase ):

	@staticmethod
	def CreateHairFromThreePlanes():
		plane1 = pm.polyPlane( w = 1, h = 10, sx = 1, sh = 10 )
		plane1[0].translateX.set(-2)
		plane2 = pm.polyPlane( w = 1, h = 10, sx = 1, sh = 10 )
		plane3 = pm.polyPlane( w = 1, h = 10, sx = 1, sh = 10 )
		plane3[0].translateX.set(2)
		pm.select( plane1 )
		pm.select( plane2, add = True )
		pm.select( plane3, add = True )
		planeShape1 = plane1[1].output.outputs( sh = True )[0]
		planeShape2 = plane2[1].output.outputs( sh = True )[0]
		planeShape3 = plane3[1].output.outputs( sh = True )[0]

		return pm.mel.OxAddHairFromMeshStrips( [ planeShape1, planeShape2, planeShape3 ] )

	@staticmethod
	def CreateGroundedHair():
		plane1 = pm.polyPlane( w = 1, h = 10, sx = 1, sh = 10 )
		distributionSurface = pm.polyPlane( w = 1, h = 10, sx = 1, sh = 10 )
		pm.select( plane1 )
		hairShape = pm.PyNode( pm.mel.OxAddHairFromMeshStrips( [ plane1 ] ) )

		pm.select( clear=True )
		pm.select( hairShape )
		pm.mel.OxAddStrandOperator( hairShape, TestUtilities.GroundStrandsNodeName )
		groundStrandsNode = pm.ls( selection = True )[0]
		distributionSurface[0].outMesh >> groundStrandsNode.distributionMesh
		return hairShape, groundStrandsNode

	def test_NewStack( self ):
		hairFromMeshStrips = self.CreateHairFromThreePlanes()

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		hairFromMeshStrips = pm.ls( type = TestUtilities.HairFromMeshStripsNodeName )[0]

		self.assertTrue( hairShape.exists() )
		self.assertEqual( hairShape.firstParent2(), 'Hair1' )
		self.assertTrue( hairFromMeshStrips.exists() )
		self.assertTrue( hairFromMeshStrips.outputHair.isConnectedTo( hairShape.inputHair ) )
		firstVertex = TestUtilities.GetVerticesInObjectCoordinates( hairShape )[0]
		TestUtilities.CheckPointsNearEqual( self, [-2.3669912815093994, 0.0, -5.0], firstVertex )

	def test_HairFromMeshStripGroundStrands( self ):
		hairShape, groundStrandsNode = self.CreateGroundedHair()

		# No crash should happen during call below
		groundStrandsNode.detachRoots.set( 0 )

		# After grounding surface dependencies should have been generated and assigned
		self.assertTrue( pm.mel.OxKeepsSurfaceDependency( hairShape ) )

	def test_PerStripCustomGroups( self ):
		hairFromMeshStrips = self.CreateHairFromThreePlanes()

		# Set three different group ids to each strip
		hairFromMeshStrips = pm.ls( type = TestUtilities.HairFromMeshStripsNodeName )[0]
		pm.setAttr( hairFromMeshStrips + ".stripGroupOverride[0]", 1 )
		pm.setAttr( hairFromMeshStrips + ".stripGroupOverride[1]", 3 )
		pm.setAttr( hairFromMeshStrips + ".stripGroupOverride[2]", 5 )

		# Add a frizz operator which will change the strips based on their group, by default it'll change all strips equally
		frizzNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairFromMeshStrips, TestUtilities.FrizzNodeName ) )
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		verticesWithoutGroups = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Changing the group to one should only affect the first strip
		frizzNode.strandGroupPattern.set( '1' )
		verticesWithFrizzAtGroup1 = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithoutGroups, verticesWithFrizzAtGroup1 )

		# Changing to group 2 should not frizz any strips since nothing belongs to group 2
		frizzNode.strandGroupPattern.set( '2' )
		verticesWithFrizzAtGroup2 = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithoutGroups, verticesWithFrizzAtGroup2 )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithFrizzAtGroup1, verticesWithFrizzAtGroup2 )

		# Changing to group 3 should frizz the second strip
		frizzNode.strandGroupPattern.set( '3' )
		verticesWithFrizzAtGroup3 = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithoutGroups, verticesWithFrizzAtGroup3 )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithFrizzAtGroup2, verticesWithFrizzAtGroup3 )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithFrizzAtGroup1, verticesWithFrizzAtGroup3 )

		# Changing to group 4 should not frizz any hairs
		frizzNode.strandGroupPattern.set( '4' )
		verticesWithFrizzAtGroup4 = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesWithFrizzAtGroup2, verticesWithFrizzAtGroup4 )

		# Changing to group 5 should frizz the third strip
		frizzNode.strandGroupPattern.set( '5' )
		verticesWithFrizzAtGroup5 = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithoutGroups, verticesWithFrizzAtGroup5 )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithFrizzAtGroup2, verticesWithFrizzAtGroup5 )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithFrizzAtGroup1, verticesWithFrizzAtGroup5 )

	def test_PerStripCustomGroupsPartial( self ):
		self.CreateHairFromThreePlanes()
		hairFromMeshStrips = pm.ls( type = TestUtilities.HairFromMeshStripsNodeName )[0]
		hairFromMeshStrips.hairsPerStrip.set(1)
		hairFromMeshStrips.viewPercentage.set(100)

		# Assign group index only to the first strip
		pm.setAttr( hairFromMeshStrips + ".stripGroupOverride[0]", 1 )

		# Check the assigned strand groups
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		strandGroups = pm.mel.OxGetStrandGroups( hairShape )

		self.assertEqual( [1, 0, 0], strandGroups )

	def test_HairFromMeshStripsWithRenderSettingsTurnedOff( self ):
		plane1 = pm.polyPlane( w = 1, h = 10, sx = 1, sh = 10 )
		pm.select( plane1 )
		hairShape = pm.PyNode( pm.mel.OxAddHairFromMeshStrips( [ plane1 ] ) )

		pm.select( clear=True )
		pm.select( hairShape )
		renderSettingsNode = pm.mel.OxAddStrandOperator( hairShape, TestUtilities.RenderSettingsNodeName )
		pm.mel.OxAddStrandOperator( hairShape, TestUtilities.MeshFromStrandsNodeName )
		meshShape = pm.PyNode( pm.mel.OxGetStackShape( renderSettingsNode ) )

		# Evaluate with render settings
		TestUtilities.GetMeshVertices( meshShape )

		# Turn off render settings and evaluate again, this should no cause a crash.
		pm.mel.OxEnableOperator( renderSettingsNode, False )
		TestUtilities.GetMeshVertices( meshShape )

	def test_HairsDoNotChangeOnSceneReload( self ):
		plane1 = pm.polyPlane( w = 1, h = 20, sx = 1, sh = 10 )
		plane2 = pm.polyPlane( w = 1, h = 20, sx = 1, sh = 10 )
		plane3 = pm.polyPlane( w = 1, h = 20, sx = 1, sh = 10 )
		pm.select( plane1 )
		pm.select( plane2 )
		pm.select( plane3 )
		hairShape = pm.PyNode( pm.mel.OxAddHairFromMeshStrips( [ plane1, plane2, plane3 ] ) )
		verticesBeforeReload = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Save and reload the scene
		filePath = pm.saveAs( 'temp.ma' )
		pm.openFile( filePath, force = True )

		# All strands should be in exact same place
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		verticesAfterReload = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		TestUtilities.CheckPointsAllNearEqual( self, verticesBeforeReload, verticesAfterReload )

	def test_Volume( self ):
		plane1 = pm.polyPlane( w = 1, h = 20, sx = 1, sh = 10 )
		pm.select( plane1 )
		hairShape = pm.PyNode( pm.mel.OxAddHairFromMeshStrips( [ plane1 ] ) )
		hairFromMeshStrips = pm.ls( type = TestUtilities.HairFromMeshStripsNodeName )[0]
		hairFromMeshStrips.volume.set( 0.5 )

		# There should be no crash or assertions here
		vertices = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

	def test_TemporarySmoothedMeshIsDeleted( self ):
		plane1 = pm.polyPlane( w = 1, h = 20, sx = 1, sh = 10 )
		planeShape1 = plane1[1].output.outputs( sh = True )[0]
		pm.mel.setAttr( planeShape1 + ".displaySmoothMesh", 1 )
		pm.select( plane1 )

		meshesBeforeHfms = len( pm.ls( type = "mesh" ) )
		hairShape = pm.PyNode( pm.mel.OxAddHairFromMeshStrips( [ plane1 ] ) )\

		# Evaluate the hair
		TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		#pm.move( 0, 0, 5, relative = True )
		#TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		meshesAfterHfms = len( pm.ls( type = "mesh" ) )

		self.assertEqual( meshesBeforeHfms, meshesAfterHfms )

	def test_HairsDoNotChangeWhenReferenced( self ):
		plane1 = pm.polyPlane( w = 1, h = 20, sx = 1, sh = 10 )
		plane2 = pm.polyPlane( w = 1, h = 20, sx = 1, sh = 10 )
		plane3 = pm.polyPlane( w = 1, h = 20, sx = 1, sh = 10 )
		pm.select( plane1 )
		pm.select( plane2 )
		pm.select( plane3 )
		hairShape = pm.PyNode( pm.mel.OxAddHairFromMeshStrips( [ plane1, plane2, plane3 ] ) )
		originalVertices = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Save and reference the scene
		filePath = pm.saveAs( 'temp.ma' )
		pm.newFile()
		pm.createReference( filePath, defaultNamespace = True )
		# All strands should be in exact same place
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		referencedVerticesNoNamespace = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		pm.newFile( force = True )
		pm.createReference( filePath )
		# All strands should be in exact same place
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		referencedVerticesWithNamespace = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		TestUtilities.CheckPointsAllNearEqual( self, originalVertices, referencedVerticesNoNamespace )
		TestUtilities.CheckPointsAllNearEqual( self, originalVertices, referencedVerticesWithNamespace )

	def Test_HairFromMeshStripsOverMeshFromStrands( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		meshFromStrandsNode = pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.MeshFromStrandsNodeName )
		meshShape = pm.PyNode( pm.mel.OxGetStackShape( meshFromStrandsNode ) )
		pm.select( meshShape )
		hairShape = pm.PyNode( pm.mel.OxAddHairFromMeshStrips( [] ) )
		hairFromMeshStrips = pm.mel.OxGetStackBelow( hairShape )
		self.assertEqual( "", pm.mel.OxGetStackBelow( hairFromMeshStrips ) )
