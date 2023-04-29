import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt

class Test_RenderSettingsNode( MayaTest.OxTestCase ):

	def test_AddRenderSettingsToMeshFromStrandsShape( self ):
		# Create plane, add hair, add mesh from strands, and then add render settings
		plane = pm.polyPlane( sx = 1, sy = 1 )
		pm.select( plane )
		hairShape = TestUtilities.AddHairToMesh( plane[0], rootGenerationMethod = 4 )
		mfs = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.MeshFromStrandsNodeName ) )
		meshShape = mfs.outputMesh.outputs()[0]
		self.assertEqual( 16, len( meshShape.vtx ) )

		# Record the tip sizes of the mesh before adding render settings modifier
		strandTipThickness = meshShape.vtx[2].getPosition().distanceTo( meshShape.vtx[3].getPosition() )

		pm.mel.OxAddStrandOperator( pm.mel.OxGetStackBelow( mfs ), TestUtilities.RenderSettingsNodeName )

		# The tips of the mesh from strands guide mesh should be tapered after render settings has been added
		strandTipThicknessAfterRenderSettings = meshShape.vtx[2].getPosition().distanceTo( meshShape.vtx[3].getPosition() )
		self.assertGreater( abs( strandTipThickness - strandTipThicknessAfterRenderSettings ), 0.5 )

	def test_AddRenderSettingsInTheMiddleOfTheStack( self ):
		plane = pm.polyPlane()
		hairShape = pm.PyNode( pm.mel.OxQuickHair() )
		hairFromGuides = hairShape.inputHair.inputs()[0]
		renderSettings1 = hairShape.inputRenderSettings.inputs()[0]

		# Insert between HFG and RS1
		renderSettings2 = pm.PyNode( pm.mel.OxAddStrandOperator( hairFromGuides, TestUtilities.RenderSettingsNodeName ) )
		self.assertTrue( renderSettings2.exists() )
		self.assertTrue( hairFromGuides.outputStack.isConnectedTo( renderSettings2.inputStack ) )
		self.assertTrue( renderSettings2.outputStack.isConnectedTo( renderSettings1.inputStack ) )
		self.assertTrue( renderSettings2.outputRenderSettings.isConnectedTo( renderSettings1.inputRenderSettings ) )
		pm.mel.OxDeleteStrandOperator( renderSettings2 )

		# Insert between RS1 and HairShape
		renderSettings2 = pm.PyNode( pm.mel.OxAddStrandOperator( renderSettings1, TestUtilities.RenderSettingsNodeName ) )
		self.assertTrue( renderSettings2.exists() )
		self.assertTrue( renderSettings1.outputStack.isConnectedTo( renderSettings2.inputStack ) )
		self.assertTrue( renderSettings1.outputRenderSettings.isConnectedTo( renderSettings2.inputRenderSettings ) )
		self.assertTrue( renderSettings2.outputRenderSettings.isConnectedTo( hairShape.inputRenderSettings ) )
		pm.mel.OxDeleteStrandOperator( renderSettings2 )

		# Insert above MeshFromStrands
		with MayaTest.UndoChunk():
			mfs = pm.PyNode( pm.mel.OxAddStrandOperator( renderSettings1, TestUtilities.MeshFromStrandsNodeName ) )
			renderSettings2 = pm.PyNode( pm.mel.OxAddStrandOperator( mfs, TestUtilities.RenderSettingsNodeName ) )
			self.assertTrue( renderSettings2.exists() )
			self.assertTrue( mfs.outputStack.isConnectedTo( renderSettings2.inputStack ) )

		pm.undo()

		# Insert below HairShape
		renderSettings2 = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.RenderSettingsNodeName ) )
		self.assertTrue( renderSettings2.exists() )
		self.assertTrue( renderSettings1.outputStack.isConnectedTo( renderSettings2.inputStack ) )
		self.assertTrue( renderSettings1.outputRenderSettings.isConnectedTo( renderSettings2.inputRenderSettings ) )
		self.assertTrue( renderSettings2.outputRenderSettings.isConnectedTo( hairShape.inputRenderSettings ) )

	# Makes sure effects of render settings are removed if its creation is undone
	def test_UndoAddRenderSettings( self ):
		mfs = None
		with MayaTest.UndoChunk():
			# Create plane, add hair, add mesh from strands, and then add render settings
			plane = pm.polyPlane( sx = 1, sy = 1 )
			pm.select( plane )
			hairShape = TestUtilities.AddHairToMesh( plane[0], rootGenerationMethod = 4 )
			mfs = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.MeshFromStrandsNodeName ) )

		with MayaTest.UndoChunk():
			hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
			pm.mel.OxAddStrandOperator( hairFromGuides, TestUtilities.RenderSettingsNodeName )

		meshShape = pm.ls( type = "mesh" )[0]

		self.assertEqual( 16, len( meshShape.vtx ) )
		strandTipThicknessWithRenderSettings = meshShape.vtx[2].getPosition().distanceTo( meshShape.vtx[3].getPosition() )
		self.assertAlmostEqual( 0.0, strandTipThicknessWithRenderSettings )

		# Undo addition of render settings
		pm.undo()

		# NOTE: For some reason Maya doesn't call set dependents dirty when undoing the wiring of render settings in unit tests.
		# To go around this we manually change one of the parameters to trigger the output re-evaluation.
		mfs.randomSeed.set( 2 )
		self.assertEqual( 16, len( meshShape.vtx ) )
		strandTipThicknessWithoutRenderSettings = meshShape.vtx[2].getPosition().distanceTo( meshShape.vtx[3].getPosition() )
		self.assertAlmostEqual( 1.0, strandTipThicknessWithoutRenderSettings )

	def test_UseVertexColorSetToControlThickness( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		renderSettings = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.RenderSettingsNodeName ) )

		# Paint vertex color info on the two vertices of the plane
		plane = pm.ls( type = "mesh" )[0]
		pm.select( plane + '.vtx[0:1]' )
		pm.polyColorPerVertex( rgb = ( 1, 1, 1 ), cdo = True )
		pm.select( plane + '.vtx[2:3]' )
		pm.polyColorPerVertex( rgb = ( 0.1, 0.1, 0.1 ), cdo = True )

		meshFromStrands = pm.PyNode( pm.mel.OxAddStrandOperator( renderSettings, TestUtilities.MeshFromStrandsNodeName ) )
		hairMesh = pm.ls( type = "mesh" )[0]

		# Assign the vertex color channel to the length attribute
		renderSettings.radius.set( 1 )
		renderSettings.radiusChannel.set( 2001 )

		# We should now have two strands at full length and two strands at 0.05
		# Since min value is 0.01, 50 * 0.01 = 0.05
		meshVertices = TestUtilities.GetMeshVertices( hairMesh )
		self.assertEqual( 4 * 4, len( meshVertices ) )

		# First two strands should have thickness of 1 and second two should have thickness of 0
		self.assertAlmostEqual( 2, dt.Vector( meshVertices[0] ).distanceTo( meshVertices[1] ) )
		self.assertAlmostEqual( 0.2, dt.Vector( meshVertices[8] ).distanceTo( meshVertices[9] ) )
