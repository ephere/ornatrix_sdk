import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt

class Test_StackMaintenance( MayaTest.OxTestCase ):

	def test_BasicOperations( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		hairFromGuides = hairShape.inputHair.inputs()[0]
		guidesFromMesh = hairFromGuides.inputStrands.inputs()[0]
		curl = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.CurlNodeName ) )
		self.assertTrue( guidesFromMesh.outputGuides.isConnectedTo( curl.inputStrands ) )
		self.assertTrue( curl.outputStrands.isConnectedTo( hairFromGuides.inputStrands ) )
		self.assertTrue( guidesFromMesh.outputStack.isConnectedTo( curl.inputStack ) )
		self.assertTrue( curl.outputStack.isConnectedTo( hairFromGuides.inputStack ) )

		self.assertEqual( [u'HairShape1', u'HairFromGuides1', u'Curl1', u'GuidesFromMesh1', u'pPlaneShape1'], pm.mel.OxGetHairStackDialogNodes() )
		self.assertEqual( u'HairShape1', pm.mel.OxGetHairStackDialogNodes( index = 0 ) )
		self.assertEqual( u'HairFromGuides1', pm.mel.OxGetHairStackDialogNodes( index = 1 ) )
		self.assertEqual( u'pPlaneShape1', pm.mel.OxGetHairStackDialogNodes( index = 4 ) )
		self.assertEqual( [2, 0, 0, 0, 3], pm.mel.OxGetHairStackDialogNodes( "-itemTypes" ) )
		self.assertEqual( 2, pm.mel.OxGetHairStackDialogNodes( "-itemTypes", index = 0 ) )
		self.assertEqual( 0, pm.mel.OxGetHairStackDialogNodes( "-itemTypes", index = 1 ) )
		self.assertEqual( 3, pm.mel.OxGetHairStackDialogNodes( "-itemTypes", index = 4 ) )
		# TODO: OxGetHairStackDialogNodes -s crashes in HairStackDialog::GetSelectedRows on Mac and Linux.
		# self.assertEqual( [0, 0, 1, 0, 0], pm.mel.OxGetHairStackDialogNodes( "-s" ) )

		# Scene selection reflects in the dialog
		pm.select( hairFromGuides )
		# self.assertEqual( [0, 1, 0, 0, 0], pm.mel.OxGetHairStackDialogNodes( "-s" ) )

	def test_StrandsOperatorNodeOutputsTheSameTypeAsInput( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		curl = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.CurlNodeName ) )
		self.assertEqual( curl.outputStrands.get( type = True ), 'GuidesData' )

		pm.mel.OxEnableOperator( curl, False )
		self.assertEqual( curl.outputStrands.get( type = True ), 'GuidesData' )

	def test_RewireOnDelete( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		hairFromGuides = hairShape.inputHair.inputs()[0]
		guidesFromMesh = hairFromGuides.inputStrands.inputs()[0]
		curl = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.CurlNodeName ) )

		pm.delete( curl )
		self.assertTrue( guidesFromMesh.outputStack.isConnectedTo( hairFromGuides.inputStack ) )

		pm.undo()
		self.assertTrue( guidesFromMesh.outputStack.isConnectedTo( curl.inputStack ) )
		self.assertTrue( curl.outputStack.isConnectedTo( hairFromGuides.inputStack ) )

		pm.redo()
		self.assertTrue( guidesFromMesh.outputStack.isConnectedTo( hairFromGuides.inputStack ) )

	def test_MeshFromStrands( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		hairFromGuides = TestUtilities.GetNodeByType( TestUtilities.HairFromGuidesNodeName )

		# When adding to the hairShape the MeshFromStrands gets inserted just below it, which removes the hairShape
		with MayaTest.UndoChunk():
			mfs = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.MeshFromStrandsNodeName ) )
		self.assertFalse( hairShape.exists() )
		self.assertIsNone( pm.mel.OxGetStackAbove( mfs ) )
		self.assertEqual( pm.mel.OxGetStackBelow( mfs ), hairFromGuides )
		self.assertEqual( pm.mel.OxGetStackAbove( hairFromGuides )[0], mfs )

		pm.undo()
		self.assertTrue( hairShape.exists() )
		self.assertEqual( pm.mel.OxGetStackBelow( hairShape ), hairFromGuides )
		self.assertIsNone( pm.mel.OxGetStackAbove( hairFromGuides ) )

		# When adding to the GuidesFromMesh the MeshFromStrands gets inserted just above it, creating a break in the stack
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		with MayaTest.UndoChunk():
			mfs = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.MeshFromStrandsNodeName ) )
		self.assertFalse( hairShape.exists() )
		self.assertEqual( pm.mel.OxGetStackBelow( hairFromGuides ), mfs )
		self.assertEqual( pm.mel.OxGetStackAbove( mfs )[0], hairFromGuides )
		self.assertEqual( pm.mel.OxGetStackBelow( mfs ), guidesFromMesh )
		self.assertEqual( pm.mel.OxGetStackAbove( guidesFromMesh )[0], mfs )

		pm.undo()
		self.assertTrue( hairShape.exists() )
		self.assertEqual( pm.mel.OxGetStackBelow( hairFromGuides ), guidesFromMesh )
		self.assertEqual( pm.mel.OxGetStackAbove( guidesFromMesh )[0], hairFromGuides )

	def test_StackShapeSwitching( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		guidesShapeName = guidesShape.name()
		self.assertNotEqual( "", guidesShapeName )
		# Stack: GFM (GS)
		guidesFromMesh = guidesShape.inputHair.inputs()[0]

		# Add RenderSettings, then MeshFromStrands and delete it
		renderSettings = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.RenderSettingsNodeName ) )
		mfs = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.MeshFromStrandsNodeName ) )
		self.assertFalse( guidesShape.exists() )
		self.assertTrue( mfs.exists() )
		meshShape = mfs.outputMesh.outputs( sh = True )[0]
		# The shape's name did not change
		self.assertEqual( guidesShapeName, meshShape.name() )

		pm.mel.OxDeleteStrandOperator( mfs )
		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		# The name is still preserved
		self.assertEqual( guidesShapeName, guidesShape.name() )

		# Add hair from guides right above guides from mesh operator
		hairFromGuidesTop = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.HairFromGuidesNodeName ) )
		# Stack: GFM -> HFG (HS)
		# The GS got replaced by a HS
		self.assertEqual( 1, len( pm.ls( type = TestUtilities.HairShapeName ) ) )
		# The shape did not change
		self.assertEqual( guidesShape, TestUtilities.GetNodeByType( TestUtilities.HairShapeName ) )

		edit = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.EditGuidesShapeName ) )
		pm.mel.OxDeleteStrandOperator( hairFromGuidesTop )
		# The RenderSettings become disabled at this point
		self.assertEqual( guidesShape, TestUtilities.GetNodeByType( TestUtilities.HairShapeName ) )
		pm.mel.OxDeleteStrandOperator( edit )
		self.assertEqual( 1, len( pm.ls( type = TestUtilities.HairShapeName ) ) )

		hairFromGuidesTop = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.HairFromGuidesNodeName ) )
		self.assertEqual( guidesShape, TestUtilities.GetNodeByType( TestUtilities.HairShapeName ) )
		# Delete the HFG
		with MayaTest.UndoChunk():
			pm.mel.OxDeleteStrandOperator( hairFromGuidesTop )
		# Stack: GFM (GS)
		self.assertFalse( hairFromGuidesTop.exists() )
		self.assertEqual( guidesShape, TestUtilities.GetNodeByType( TestUtilities.HairShapeName ) )

		pm.undo()
		# Stack: GFM -> HFG (HS)
		self.assertEqual( guidesShape, TestUtilities.GetNodeByType( TestUtilities.HairShapeName ) )

		# Add a curl operator on the guides, the shape doesn't change
		curl = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.CurlNodeName ) )
		# Stack: GFM -> Curl -> HFG (HS)
		self.assertEqual( guidesShape, TestUtilities.GetNodeByType( TestUtilities.HairShapeName ) )
		self.assertTrue( curl.outputStrands.isConnectedTo( hairFromGuidesTop.inputStrands ) )
		self.assertTrue( hairFromGuidesTop.outputHair.isConnectedTo( guidesShape.inputHair ) )

		# Add another HairFromGuides below the curl. This changes the type flowing through the curl from guides to hair and the top HFG becomes a no-op but doesn't get bypassedz
		hairFromGuidesBottom = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.HairFromGuidesNodeName ) )
		# Stack: GFM -> HFG -> Curl (HS) -> HFG(no-op)
		# The HairShape gets attached to the curl, at the breaking point of the stack
		self.assertTrue( guidesShape.exists() )
		self.assertTrue( hairFromGuidesBottom.outputHair.isConnectedTo( curl.inputStrands ) )
		self.assertTrue( curl.outputStrands.isConnectedTo( hairFromGuidesTop.inputStrands ) )

		# Delete the bottom HFG, the stack should be auto-repaired
		pm.mel.OxDeleteStrandOperator( hairFromGuidesBottom )
		# Stack: GFM -> Curl -> HFG (HS)
		self.assertTrue( guidesShape.exists() )
		self.assertTrue( curl.outputStrands.isConnectedTo( hairFromGuidesTop.inputStrands ) )
		self.assertTrue( hairFromGuidesTop.outputHair.isConnectedTo( guidesShape.inputHair ) )

		# Delete the top HFG, the HairShape gets rewired
		pm.mel.OxDeleteStrandOperator( hairFromGuidesTop )
		# Stack: GFM -> Curl (GS)
		self.assertEqual( guidesShape, TestUtilities.GetNodeByType( TestUtilities.HairShapeName ) )
		self.assertTrue( curl.outputStrands.isConnectedTo( guidesShape.inputHair ) )

		# Add a HairFromGuides below the curl again, changing the type flowing through the curl from guides to hair
		hairFromGuidesBottom = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.HairFromGuidesNodeName ) )
		# Stack: GFM -> HFG -> Curl (HS)
		self.assertEqual( guidesShape, TestUtilities.GetNodeByType( TestUtilities.HairShapeName ) )
		self.assertTrue( curl.outputStrands.isConnectedTo( guidesShape.inputHair ) )

		# Delete the HFG returning to the previous state, the HairShape gets rewired
		pm.mel.OxDeleteStrandOperator( hairFromGuidesBottom )
		# Stack: GFM -> Curl (GS)
		self.assertEqual( guidesShape, TestUtilities.GetNodeByType( TestUtilities.HairShapeName ) )
		self.assertTrue( curl.outputStrands.isConnectedTo( guidesShape.inputHair ) )

		# Can't delete the root of the stack
		with self.assertRaisesRegexp( pm.MelError, "Can't delete the root of the hair stack" ):
			pm.mel.OxDeleteStrandOperator( guidesFromMesh )

		# Deleting the stack shape deletes the whole stack
		pm.mel.OxDeleteStrandOperator( guidesShape )
		self.assertFalse( guidesFromMesh.exists() )
		self.assertFalse( curl.exists() )
		self.assertFalse( guidesShape.exists() )

	def test_OxDeleteShape( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		hairFromGuides = hairShape.inputHair.inputs()[0]
		guidesFromMesh = hairFromGuides.inputStrands.inputs()[0]
		tm = hairShape.firstParent2()

		# Deleting the stack shape deletes the whole stack
		pm.mel.OxDeleteStrandOperator( hairShape )
		self.assertFalse( guidesFromMesh.exists() )
		self.assertFalse( hairFromGuides.exists() )
		self.assertFalse( hairShape.exists() )
		self.assertEqual( TestUtilities.GetOrnatrixObjects(), [] )

		pm.undo();
		curl = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.CurlNodeName ) )
		plane = pm.PyNode( pm.plane() )
		plane.setParent( tm )
		pm.mel.OxDeleteStrandOperator( hairShape )
		self.assertFalse( guidesFromMesh.exists() )
		self.assertFalse( curl.exists() )
		self.assertFalse( hairFromGuides.exists() )
		self.assertFalse( hairShape.exists() )
		self.assertTrue( tm.exists() )


	def test_OxDeletingShapeDeletesMoovLocator( self ):
		hairShape = TestUtilities.AddHairToNewPlane( pointsPerStrandCount = 4 )
		hairFromGuides = hairShape.inputHair.inputs()[0]
		guidesFromMesh = hairFromGuides.inputStrands.inputs()[0]
		moov = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.MoovPhysicsNodeName ) )
		# Use ls, does not work with exists()
		moovLocatorCount = len( pm.ls( type = TestUtilities.MoovPhysicsLocatorNodeName ) )

		# Deleting the stack shape should delete both MoovPhysics and MoovPhysicsLocator nodes
		pm.mel.OxDeleteStrandOperator( hairShape )
		self.assertFalse( moov.exists() )
		deletedLocatorCount = moovLocatorCount - len( pm.ls( type = TestUtilities.MoovPhysicsLocatorNodeName ) )
		self.assertEqual( 1, deletedLocatorCount )


	def test_OxDeleteBranchKeepingTheMesh( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		hairFromGuides = hairShape.inputHair.inputs()[0]
		guidesFromMesh = hairFromGuides.inputStrands.inputs()[0]
		tm = hairShape.firstParent2()
		mfs = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.MeshFromStrandsNodeName ) )
		hairShape = pm.PyNode( "HairShape1" )

		pm.mel.OxDeleteBranch( hairShape, 1 )
		self.assertFalse( guidesFromMesh.exists() )
		self.assertFalse( hairFromGuides.exists() )
		self.assertFalse( mfs.exists() )
		self.assertTrue( hairShape.exists() )
		self.assertTrue( tm.exists() )

	def test_OxDeleteBranchKeepingTheCurves( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		hairFromGuides = hairShape.inputHair.inputs()[0]
		guidesFromMesh = hairFromGuides.inputStrands.inputs()[0]
		tm = hairShape.firstParent2()
		cfs = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.CurvesFromStrandsNodeName ) )
		hairShape = pm.PyNode( "HairShape1" )

		pm.mel.OxDeleteBranch( cfs, 1 )
		self.assertFalse( guidesFromMesh.exists() )
		self.assertFalse( hairFromGuides.exists() )
		self.assertFalse( cfs.exists() )
		self.assertTrue( hairShape.exists() )
		self.assertTrue( tm.exists() )

	def test_OxThreeBranches( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		mfs1 = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.MeshFromStrandsNodeName ) )
		shape1 = pm.PyNode( pm.mel.OxGetStackShape( mfs1 ) )
		mfs2 = pm.PyNode( pm.mel.OxAddStrandOperator2( hairShape, TestUtilities.MeshFromStrandsNodeName, 1, 1 ) )
		shape2 = pm.PyNode( pm.mel.OxGetStackShape( mfs2 ) )
		self.assertNotEqual( shape1, shape2 )
		mfs3 = pm.PyNode( pm.mel.OxAddStrandOperator2( hairShape, TestUtilities.MeshFromStrandsNodeName, 1, 1 ) )
		shape3 = pm.PyNode( pm.mel.OxGetStackShape( mfs3 ) )
		self.assertNotEqual( shape2, shape3 )

	def test_OxUpdateStack_RestoresTheShapeIfMissing( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		hairFromGuides = hairShape.inputHair.inputs()[0]
		guidesFromMesh = hairFromGuides.inputStrands.inputs()[0]
		pm.mel.OxDisconnect( hairShape )
		pm.delete( hairShape )
		self.assertEqual( "", pm.mel.OxGetStackShape( guidesFromMesh ) )

		pm.mel.OxUpdateStack( hairFromGuides, [] )
		self.assertEqual( 1, len( pm.ls( type = TestUtilities.HairShapeName ) ) )
		self.assertEqual( "HairShape1", pm.mel.OxGetStackShape( guidesFromMesh ) )
		hairShape = pm.PyNode( "HairShape1" )
		self.assertTrue( hairFromGuides.outputHair.isConnectedTo( hairShape.inputHair ) )
		plane = pm.PyNode( "pPlaneShape1" )
		self.assertNotEqual( hairShape.firstParent(), plane.firstParent() )
		self.assertTrue( plane.firstParent().isParentOf( hairShape.firstParent() ) )

	def test_UpdateStack_WiresStackShapeDistributionMesh( self ):
		pm.polyPlane()
		hairShape = pm.PyNode( pm.mel.OxLoadGroom( path = "Fur Ball" ) )
		self.assertEqual( hairShape, "FurBallShape1" )
		plane = pm.PyNode( "pPlaneShape1" )
		hairFromGuides = TestUtilities.GetNodeByType( TestUtilities.HairFromGuidesNodeName )
		self.assertTrue( plane.outMesh.isConnectedTo( hairFromGuides.distributionMesh ) )
		self.assertTrue( plane.outMesh.isConnectedTo( hairShape.distributionMesh ) )

	def test_UpdateStack_ProperDistributionMeshAfterShellsNode( self ):
		sphere1 = pm.polySphere( r = 1 )
		sphere2 = pm.polySphere( r = 2 )
		guidesFromMesh = pm.createNode( TestUtilities.GuidesFromMeshNodeName )
		sphere1[0].outMesh >> guidesFromMesh.inputMesh
		shellsNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.ShellsNodeName ) )
		sphere2[0].outMesh >> shellsNode.shellMeshes[0]
		sphere2[0].worldMatrix >> shellsNode.shellMeshTransforms[0]

		hairFromGuides = pm.PyNode( pm.mel.OxAddStrandOperator( shellsNode, TestUtilities.HairFromGuidesNodeName ) )

		# hairFromGuides.distributionMesh is wired to the base mesh sphere, not the shell mesh
		self.assertTrue( sphere1[0].outMesh.isConnectedTo( hairFromGuides.distributionMesh ) )
		self.assertFalse( sphere2[0].outMesh.isConnectedTo( hairFromGuides.distributionMesh ) )

	def test_OxUpdateStack_KeepsSets( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		hairFromGuides = hairShape.inputHair.inputs()[0]
		# Create and assign a new shading group
		shader, shadingGroup = pm.createSurfaceShader( 'blinn' )
		pm.sets( shadingGroup, add = hairShape )

		pm.select( hairShape )
		set2 = pm.sets()

		self.assertTrue( shadingGroup in hairShape.instObjGroups[0].outputs() )
		self.assertTrue( set2 in hairShape.instObjGroups[0].outputs() )

		pm.mel.OxEnableOperator( hairFromGuides, False )
		hairShape = pm.PyNode( "HairShape1" )
		self.assertTrue( shadingGroup in hairShape.instObjGroups[0].outputs() )
		self.assertTrue( set2 in hairShape.instObjGroups[0].outputs() )

		pm.mel.OxEnableOperator( hairFromGuides, True )
		hairShape = pm.PyNode( "HairShape1" )
		self.assertTrue( shadingGroup in hairShape.instObjGroups[0].outputs() )
		self.assertTrue( set2 in hairShape.instObjGroups[0].outputs() )

		with MayaTest.UndoChunk():
			pm.mel.OxAddStrandOperator( hairShape, TestUtilities.MeshFromStrandsNodeName )
			hairShape = pm.PyNode( "HairShape1" )
			self.assertTrue( shadingGroup in hairShape.instObjGroups[0].outputs() )
			self.assertTrue( set2 in hairShape.instObjGroups[0].outputs() )

		pm.undo()
		hairShape = pm.PyNode( "HairShape1" )
		self.assertTrue( shadingGroup in hairShape.instObjGroups[0].outputs() )
		self.assertTrue( set2 in hairShape.instObjGroups[0].outputs() )

	def test_OxEnableOperator( self ):
		plane = pm.polyPlane()
		hairShape = pm.PyNode( pm.mel.OxQuickHair() )
		hairFromGuides = hairShape.inputHair.inputs()[0]
		renderSettings = pm.ls( type = TestUtilities.RenderSettingsNodeName )[0]

		with MayaTest.UndoChunk():
			pm.mel.OxEnableOperator( hairFromGuides, False )
			self.assertEqual( 1, hairFromGuides.nodeState.get() )
			self.assertTrue( hairShape.exists() )
			guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
			editGuides = pm.ls( type = TestUtilities.EditGuidesShapeName )[0]
			self.assertTrue( editGuides.outputStrands.isConnectedTo( hairFromGuides.inputStrands ) )
			pm.select( hairShape )
			pm.mel.OxUpdateHairStackDialog()
			self.assertEqual( [u'HairShape1', u'RenderSettings1', u'HairFromGuides1', u'EditGuides1', u'GuidesFromMesh1', u'pPlaneShape1'], pm.mel.OxGetHairStackDialogNodes() )
			self.assertEqual( u'HairShape1', pm.mel.OxGetHairStackDialogNodes( index = 0 ) )
			self.assertEqual( u'RenderSettings1', pm.mel.OxGetHairStackDialogNodes( index = 1 ) )
			self.assertEqual( u'pPlaneShape1', pm.mel.OxGetHairStackDialogNodes( index = 5 ) )
			self.assertEqual( [2, 0, 1, 0, 0, 3], pm.mel.OxGetHairStackDialogNodes( "-itemTypes" ) )
			self.assertEqual( 2, pm.mel.OxGetHairStackDialogNodes( "-itemTypes", index = 0 ) )
			self.assertEqual( 0, pm.mel.OxGetHairStackDialogNodes( "-itemTypes", index = 1 ) )
			self.assertEqual( 1, pm.mel.OxGetHairStackDialogNodes( "-itemTypes", index = 2 ) )
			self.assertEqual( 3, pm.mel.OxGetHairStackDialogNodes( "-itemTypes", index = 5 ) )

			pm.mel.OxEnableOperator( hairFromGuides, True )
			self.assertEqual( 0, hairFromGuides.nodeState.get() )
			self.assertTrue( hairShape.exists() )
			self.assertEqual( 1, len( pm.ls( type = TestUtilities.HairShapeName ) ) )
			pm.mel.OxUpdateHairStackDialog()
			self.assertEqual( [2, 0, 0, 0, 0, 3], pm.mel.OxGetHairStackDialogNodes( "-it" ) )

		pm.undo()
		self.assertEqual( 0, hairFromGuides.nodeState.get() )
		self.assertTrue( hairShape.exists() )

		curlNode = pm.mel.OxAddStrandOperator( hairShape, TestUtilities.CurlNodeName )
		pm.mel.OxEnableOperator( hairFromGuides, False )
		self.assertTrue( hairShape.exists() )

	def test_RenderSettings( self ):
		plane = pm.polyPlane()
		hairShape = pm.PyNode( pm.mel.OxQuickHair() )
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		renderSettings = pm.ls( type = TestUtilities.RenderSettingsNodeName )[0]
		self.assertTrue( renderSettings.outputRenderSettings.isConnectedTo( hairShape.inputRenderSettings ) )

		# Adding a MFS re-wires the RS to it
		pm.mel.OxAddStrandOperator( renderSettings, TestUtilities.MeshFromStrandsNodeName )
		mfs = pm.ls( type = TestUtilities.MeshFromStrandsNodeName )[0]
		self.assertFalse( hairShape.exists() )
		self.assertTrue( renderSettings.exists() )
		self.assertTrue( renderSettings.outputRenderSettings.isConnectedTo( mfs.inputRenderSettings ) )

		# Deleting the MFS re-wires the RS to the new HS
		pm.mel.OxDeleteStrandOperator( mfs )
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		self.assertTrue( renderSettings.exists() )
		self.assertTrue( renderSettings.outputRenderSettings.isConnectedTo( hairShape.inputRenderSettings ) )

	def test_CopyNode( self ):
		plane1 = pm.polyPlane()
		hairShape1 = pm.PyNode( pm.mel.OxQuickHair() )
		gfm1 = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		curl1 = pm.PyNode( pm.mel.OxAddStrandOperator( gfm1, TestUtilities.CurlNodeName ) )
		curl1.phase.set( 13 )
		curl2 = pm.PyNode( pm.mel.OxAddStrandOperator( curl1, TestUtilities.CurlNodeName ) )
		curl2.phase.set( 17 )

		# Test coping the connected texture maps as well
		checker = pm.shadingNode( "checker", asTexture=True )
		pm.connectAttr( checker.outColor, curl2.magnitudeMultiplier );

		plane2 = pm.polyPlane()
		hairShape2 = pm.PyNode( pm.mel.OxQuickHair() )
		gfm2 = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[1]

		newCurls = pm.mel.OxCopyNode( [ curl1, curl2 ], gfm2 )
		self.assertEqual( 2, len( newCurls ) )
		newCurls[0] = pm.PyNode( newCurls[0] )
		newCurls[1] = pm.PyNode( newCurls[1] )
		self.assertTrue( newCurls[0].exists() )
		self.assertTrue( gfm2.outputStack.isConnectedTo( newCurls[0].inputStack ) )
		self.assertEqual( newCurls[0].phase.get(), 13 )
		self.assertTrue( newCurls[1].exists() )
		self.assertTrue( newCurls[0].outputStack.isConnectedTo( newCurls[1].inputStack ) )
		self.assertEqual( newCurls[1].phase.get(), 17 )
		self.assertTrue( checker.outColor.isConnectedTo( newCurls[1].magnitudeMultiplier ) )

	def test_CopyEditGuidesShape( self ):
		plane1 = pm.polyPlane()
		hairShape1 = pm.PyNode( pm.mel.OxQuickHair() )
		gfm1 = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		egs = pm.ls( type = TestUtilities.EditGuidesShapeName )[0]

		result = pm.mel.OxCopyNode( [ egs ], gfm1 )
		self.assertEqual( [u'EditGuides2'], result )

	def test_CopyRenderSettings( self ):
		plane1 = pm.polyPlane()
		hairShape1 = pm.PyNode( pm.mel.OxQuickHair() )
		rs1 = pm.ls( type = TestUtilities.RenderSettingsNodeName )[0]
		rs1.radiusRamp[1].radiusRamp_Position.set( 0.2 )
		rs1.radiusRamp[1].radiusRamp_FloatValue.set( 0.3 )

		rs2 = pm.PyNode( pm.mel.OxCopyNode( [ rs1 ], rs1 )[0] )
		self.assertAlmostEqual( 0.2, rs2.radiusRamp[1].radiusRamp_Position.get(), places = 3 )
		self.assertAlmostEqual( 0.3, rs2.radiusRamp[1].radiusRamp_FloatValue.get(), places = 3 )

	def test_CopyMoov( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		moov = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.MoovPhysicsNodeName ) )

		moov1 = pm.PyNode( pm.mel.OxCopyNode( [ moov ], guidesFromMesh )[0] )
		self.assertTrue( moov1.currentTime.isConnected() )

	def test_ReorderStack( self ):
		plane = pm.polyPlane()
		hairShape = pm.PyNode( pm.mel.OxQuickHair() )
		hairFromGuides = hairShape.inputHair.inputs()[0]
		editGuides = hairFromGuides.inputStrands.inputs()[0]
		guidesFromMesh = editGuides.inputStrands.inputs()[0]
		curl = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.CurlNodeName ) )
		self.assertTrue( guidesFromMesh.outputStack.isConnectedTo( curl.inputStack ) )
		self.assertTrue( curl.outputStack.isConnectedTo( editGuides.inputStack ) )

		pm.mel.OxReorderStack( hairFromGuides, [ curl ] )

		# Stack attributes
		self.assertTrue( guidesFromMesh.outputStack.isConnectedTo( editGuides.inputStack ) )
		self.assertTrue( hairFromGuides.outputStack.isConnectedTo( curl.inputStack ) )

		# Data attributes
		self.assertTrue( guidesFromMesh.outputGuides.isConnectedTo( editGuides.inputStrands ) )
		self.assertTrue( hairFromGuides.outputHair.isConnectedTo( curl.inputStrands ) )
		self.assertTrue( curl.outputStrands.isConnectedTo( hairShape.inputHair ) )

	def test_ReorderStack_WithRenderSettings( self ):
		plane = pm.polyPlane()
		hairShape = pm.PyNode( pm.mel.OxQuickHair() )
		hairFromGuides = hairShape.inputHair.inputs()[0]
		renderSettings1 = hairShape.inputRenderSettings.inputs()[0]
		renderSettings2 = pm.PyNode( pm.mel.OxAddStrandOperator( renderSettings1, TestUtilities.RenderSettingsNodeName ) )
		self.assertTrue( renderSettings1.outputStack.isConnectedTo( renderSettings2.inputStack ) )
		self.assertTrue( renderSettings2.outputRenderSettings.isConnectedTo( hairShape.inputRenderSettings ) )

		pm.mel.OxReorderStack( renderSettings2, [ renderSettings1 ] )

		# Stack attributes
		self.assertTrue( hairFromGuides.outputStack.isConnectedTo( renderSettings2.inputStack ) )
		self.assertTrue( renderSettings2.outputStack.isConnectedTo( renderSettings1.inputStack ) )

		# Data attributes
		self.assertTrue( renderSettings2.inputRenderSettings.inputs() == [] )
		self.assertTrue( renderSettings2.outputRenderSettings.isConnectedTo( renderSettings1.inputRenderSettings ) )
		self.assertTrue( renderSettings1.outputRenderSettings.isConnectedTo( hairShape.inputRenderSettings ) )

	def test_CollapseStack_GuidesAtTheTop( self ):
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane()
		guidesShape = editGuidesShape.outputStrands.outputs( sh = True )[0]
		planeShape = pm.PyNode( 'pPlaneShape1' )
		initialStrandCount = pm.mel.OxGetStrandCount( guidesShape )

		bakedGuides = pm.PyNode( pm.mel.OxCollapseStack( editGuidesShape ) )

		self.assertTrue( bakedGuides.exists() )
		self.assertTrue( guidesShape.exists() )
		self.assertEqual( 'BakedGuidesNode', bakedGuides.type() )
		self.assertTrue( bakedGuides.outputGuides.isConnectedTo( guidesShape.inputHair ) )
		self.assertTrue( planeShape.outMesh.isConnectedTo( bakedGuides.distributionMesh ) )
		self.assertEqual( [u'HairShape1', u'BakedGuides1', u'pPlaneShape1'], pm.mel.OxGetHairStackDialogNodes() )
		self.assertEqual( initialStrandCount, pm.mel.OxGetStrandCount( guidesShape ) )

	def test_CollapseStack_Guides( self ):
		plane = pm.polyPlane( sx = 2, sy = 2 )
		hairShape = pm.PyNode( pm.mel.OxQuickHair() )
		hairFromGuides = hairShape.inputHair.inputs()[0]
		hairFromGuides.renderCount.set( 100 )
		hairFromGuides.viewportCountFraction.set( 1.0 )
		hairFromGuides.distribution.set( 4 )
		editGuides = hairFromGuides.inputStrands.inputs( sh = True )[0]
		guidesFromMesh = editGuides.inputStrands.inputs()[0]
		guidesFromMesh.length.set( 10 )
		guidesFromMesh.distribution.set( 4 )
		curl = pm.PyNode( pm.mel.OxAddStrandOperator( editGuides, TestUtilities.CurlNodeName ) )
		initialStrandCount = pm.mel.OxGetStrandCount( hairShape )

		bakedGuides = pm.PyNode( pm.mel.OxCollapseStack( curl ) )

		self.assertTrue( bakedGuides.exists() )
		self.assertTrue( hairShape.exists() )
		self.assertEqual( 'BakedGuidesNode', bakedGuides.type() )
		self.assertTrue( bakedGuides.outputGuides.isConnectedTo( hairFromGuides.inputStrands ) )
		self.assertTrue( plane[0].outMesh.isConnectedTo( bakedGuides.distributionMesh ) )
		self.assertEqual( [u'HairShape1', u'RenderSettings1', u'HairFromGuides1', u'BakedGuides1', u'pPlaneShape1'], pm.mel.OxGetHairStackDialogNodes() )
		self.assertEqual( initialStrandCount, pm.mel.OxGetStrandCount( hairShape ) )

		# Updating the distribution mesh gets reflected in hairFromGuides

		plane[1].subdivisionsWidth.set( 3 )
		currentStrandCount = pm.mel.OxGetStrandCount( hairShape )

		self.assertLess( initialStrandCount, currentStrandCount )

		# Save/load baked guides

		TestUtilities.SaveAndReloadScene( self, 'test_CollapseStack_Guides', True )
		hairShape = pm.PyNode( "HairShape1" )

		self.assertEqual( currentStrandCount, pm.mel.OxGetStrandCount( hairShape ) )

		TestUtilities.SaveAndReloadScene( self, 'test_CollapseStack_Guides', False )
		hairShape = pm.PyNode( "HairShape1" )

		self.assertEqual( currentStrandCount, pm.mel.OxGetStrandCount( hairShape ) )

	def test_CollapseStack_Hair( self ):
		plane = pm.polyPlane( sx = 2, sy = 2 )
		planeShape = pm.PyNode( 'pPlaneShape1' )
		polyPlane = pm.PyNode( 'polyPlane1' )
		hairShape = pm.PyNode( pm.mel.OxQuickHair() )
		hairFromGuides = hairShape.inputHair.inputs()[0]
		hairFromGuides.renderCount.set( 100 )

		displayedFraction = 0.1
		hairFromGuides.viewportCountFraction.set( displayedFraction )
		editGuides = hairFromGuides.inputStrands.inputs( sh = True )[0]
		guidesFromMesh = editGuides.inputStrands.inputs()[0]
		guidesFromMesh.length.set( 10 )
		guidesFromMesh.distribution.set( 4 )
		guidesFromMesh.pointCount.set( 2 )

		mfs = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.MeshFromStrandsNodeName ) )
		hairShape = pm.PyNode( "HairShape1" )
		initialVertexCountView = len( hairShape.vtx )
		with TestUtilities.RenderModeScope():
			initialVertexCountRender = len( hairShape.vtx )

		bakedHair = pm.PyNode( pm.mel.OxCollapseStack( hairFromGuides ) )
		bakedHair.displayFraction.set( displayedFraction )

		self.assertTrue( bakedHair.exists() )
		self.assertEqual( 'BakedHairNode', bakedHair.type() )
		self.assertTrue( bakedHair.outputHair.isConnectedTo( mfs.inputStrands ) )
		self.assertTrue( planeShape.outMesh.isConnectedTo( bakedHair.distributionMesh ) )
		self.assertEqual( [u'HairShape1', u'MeshFromStrands1', u'RenderSettings1', u'BakedHair1', u'pPlaneShape1'], pm.mel.OxGetHairStackDialogNodes() )

		# The number of vertices and strands in baked hair doesn't have to match the previous viewport count exactly, but it needs to be close
		#self.assertEqual( initialVertexCountView, len( hairShape.vtx ) )
		self.assertTrue( abs( initialVertexCountView - len( hairShape.vtx ) ) < 40 )
		#with TestUtilities.RenderModeScope():
		#	self.assertEqual( initialVertexCountRender, len( hairShape.vtx ) )

		# Updating the distribution mesh gets reflected in the final mesh shape

		polyPlane.subdivisionsWidth.set( 4 )

		# TODO:
		# self.assertLess( initialVertexCount, len( hairShape.vtx ) )

		# Save/load baked hair

		currentVertexCount = len( hairShape.vtx )

		TestUtilities.SaveAndReloadScene( self, 'test_CollapseStack_Hair', True )
		hairShape = pm.PyNode( "HairShape1" )

		self.assertEqual( currentVertexCount, len( hairShape.vtx ) )

		TestUtilities.SaveAndReloadScene( self, 'test_CollapseStack_Hair', False )
		hairShape = pm.PyNode( "HairShape1" )

		self.assertEqual( currentVertexCount, len( hairShape.vtx ) )

		mfs = TestUtilities.GetNodeByType( TestUtilities.MeshFromStrandsNodeName )
		# Create and assign a new shading group
		shader, shadingGroup = pm.createSurfaceShader( 'blinn' )
		pm.sets( 'initialShadingGroup', remove = hairShape )
		pm.sets( shadingGroup, add = hairShape )

	def test_CollapseStack_HairWithRotateOperator( self ):
		plane = pm.polyPlane( sx = 2, sy = 2 )
		planeShape = pm.PyNode( 'pPlaneShape1' )
		polyPlane = pm.PyNode( 'polyPlane1' )
		hairShape = pm.PyNode( pm.mel.OxQuickHair() )
		hairFromGuides = hairShape.inputHair.inputs()[0]
		hairFromGuides.renderCount.set( 123 )
		hairFromGuides.viewportCountFraction.set( 12.0 / hairFromGuides.renderCount.get() )
		rotateNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.RotateNodeName ) )
		hairShape = pm.PyNode( "HairShape1" )
		bakedHair = pm.PyNode( pm.mel.OxCollapseStack( rotateNode ) )

		# Saving and reloading scene should not crash Maya
		TestUtilities.SaveAndReloadScene( self, 'test_CollapseStack_HairWithRotateOperator', False )

	def test_UpdateStack_AuxiliaryInputs( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		guidesFromMesh = guidesShape.inputHair.inputs()[0]
		edit = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.EditGuidesShapeName ) )
		curl = pm.PyNode( pm.mel.OxAddStrandOperator( edit, TestUtilities.CurlNodeName ) )
		cluster = pm.PyNode( pm.mel.OxAddStrandOperator( curl, TestUtilities.GuideClusterNodeName ) )
		self.assertTrue( curl.outputStrands.isConnectedTo( cluster.inputHairGuides ) )

		pm.mel.OxReorderStack( guidesFromMesh, [ cluster ] )
		self.assertFalse( curl.outputStrands.isConnectedTo( cluster.inputHairGuides ) )
		self.assertTrue( guidesFromMesh.outputGuides.isConnectedTo( cluster.inputHairGuides ) )

	def test_HairStackDialog_MeshBase( self ):
		plane = pm.polyPlane()
		pm.mel.OxQuickHair()
		self.assertEqual( [u'HairShape1', u'RenderSettings1', u'HairFromGuides1', u'EditGuides1', u'GuidesFromMesh1', u'pPlaneShape1'], pm.mel.OxGetHairStackDialogNodes() )

		plane2 = pm.polyPlane()
		pm.select( plane2 )
		pm.mel.OxUpdateHairStackDialog()
		self.assertIsNone( pm.mel.OxGetHairStackDialogNodes() )

		# Selecting the base shape fills the dialog
		pm.select( "pPlaneShape1" )
		pm.mel.OxUpdateHairStackDialog()
		self.assertEqual( [u'HairShape1', u'RenderSettings1', u'HairFromGuides1', u'EditGuides1', u'GuidesFromMesh1', u'pPlaneShape1'], pm.mel.OxGetHairStackDialogNodes() )

		# Clearing the selection with a full dialog does not clear it
		pm.select( clear=True )
		pm.mel.OxUpdateHairStackDialog()
		self.assertEqual( [u'HairShape1', u'RenderSettings1', u'HairFromGuides1', u'EditGuides1', u'GuidesFromMesh1', u'pPlaneShape1'], pm.mel.OxGetHairStackDialogNodes() )

		# Selecting the RenderSettings fills the dialog
		pm.select( plane2 )
		pm.mel.OxUpdateHairStackDialog()
		pm.select( "RenderSettings1" )
		pm.mel.OxUpdateHairStackDialog()
		self.assertEqual( [u'HairShape1', u'RenderSettings1', u'HairFromGuides1', u'EditGuides1', u'GuidesFromMesh1', u'pPlaneShape1'], pm.mel.OxGetHairStackDialogNodes() )

	def test_HairStackDialog_UpdateOnDelete( self ):
		plane = pm.polyPlane()
		hairShape = pm.PyNode( pm.mel.OxQuickHair() )
		hairFromGuides = hairShape.inputHair.inputs()[0]
		renderSettings = hairShape.inputRenderSettings.inputs()[0]
		curl = pm.PyNode( pm.mel.OxAddStrandOperator( hairFromGuides, TestUtilities.CurlNodeName ) )

		with MayaTest.UndoChunk():
			pm.mel.OxDeleteStrandOperator( curl )
			self.assertEqual( [u'HairShape1', u'RenderSettings1', u'HairFromGuides1', u'EditGuides1', u'GuidesFromMesh1', u'pPlaneShape1'], pm.mel.OxGetHairStackDialogNodes() )
		pm.undo()

		# Deleting the operator just below the shape
		with MayaTest.UndoChunk():
			pm.mel.OxDeleteStrandOperator( renderSettings )
			self.assertEqual( [u'HairShape1', u'Curl1', u'HairFromGuides1', u'EditGuides1', u'GuidesFromMesh1', u'pPlaneShape1'], pm.mel.OxGetHairStackDialogNodes() )
			pm.mel.OxDeleteStrandOperator( curl )
			self.assertEqual( [u'HairShape1', u'HairFromGuides1', u'EditGuides1', u'GuidesFromMesh1', u'pPlaneShape1'], pm.mel.OxGetHairStackDialogNodes() )
			pm.mel.OxDeleteStrandOperator( hairFromGuides )
			self.assertEqual( [u'HairShape1', u'EditGuides1', u'GuidesFromMesh1', u'pPlaneShape1'], pm.mel.OxGetHairStackDialogNodes() )
		pm.undo()

		pm.mel.OxDeleteStrandOperator( hairShape )
		self.assertIsNone( pm.mel.OxGetHairStackDialogNodes() )

	def test_HairStackDialog_CurvesBase( self ):
		curve1 = pm.curve( p=[(1, 0, 1), (1, 1, 1.1), (1, 2, 1.2), (1, 3, 1.3)], k=[0, 0, 0, 1, 1, 1] )
		curve2 = pm.curve( p=[(-1, 0, 1), (-1, 1, 1.1), (-1, 2, 1.2), (-1, 3, 1.3)], k=[0, 0, 0, 1, 1, 1] )

		pm.select( clear = True )
		pm.select( curve1, add=True )
		pm.select( curve2, add=True )
		guidesShape = pm.mel.OxQuickHair()
		self.assertEqual( [u'HairShape1', u'GuidesFromCurves1', u'curveShape1'], pm.mel.OxGetHairStackDialogNodes() )

		plane2 = pm.polyPlane()
		pm.select( plane2 )
		pm.mel.OxUpdateHairStackDialog()
		self.assertIsNone( pm.mel.OxGetHairStackDialogNodes() )

		# Selecting the base shape fills the dialog
		pm.select( "curveShape1" )
		pm.mel.OxUpdateHairStackDialog()
		self.assertEqual( [u'HairShape1', u'GuidesFromCurves1', u'curveShape1'], pm.mel.OxGetHairStackDialogNodes() )

	def test_HairStackDialog_SupportsMeshFromStrands( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		mfs = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.MeshFromStrandsNodeName ) )

		pm.mel.OxShowHairStackDialog( "-show" )
		contents = pm.mel.OxGetHairStackDialogNodes()
		self.assertEqual( 4, len( contents ) )
		self.assertEqual( 'HairShape1', contents[0] )

		pm.select( "HairShape1", replace = True )
		self.assertEqual( 4, len( pm.mel.OxGetHairStackDialogNodes() ) )

		pm.select( clear=True )
		self.assertEqual( 4, len( pm.mel.OxGetHairStackDialogNodes() ) )

	def test_HairStackDialog_PrefersLatestSelection( self ):
		plane = pm.polyPlane()
		hairShape1 = pm.PyNode( pm.mel.OxQuickHair() )
		pm.select( plane )
		hairShape2 = pm.PyNode( pm.mel.OxQuickHair() )
		self.assertEqual( u'HairShape2', pm.mel.OxGetHairStackDialogNodes()[0] )
		pm.select( None )
		self.assertEqual( u'HairShape2', pm.mel.OxGetHairStackDialogNodes()[0] )

	def test_DeleteGuidesFromGuideMesh( self ):
		plane = pm.polyPlane()
		hairShape = pm.PyNode( pm.mel.OxQuickHair() )
		hairFromGuides = hairShape.inputHair.inputs()[0]
		mfs = pm.PyNode( pm.mel.OxAddStrandOperator( hairFromGuides, TestUtilities.MeshFromStrandsNodeName ) )
		gfgm = pm.PyNode( pm.mel.OxAddStrandOperator( mfs, TestUtilities.GuidesFromGuideMeshNodeName ) )
		pm.mel.OxDeleteStrandOperator( gfgm )
		# The MeshFromStrands node before the GuidesFromGuideMesh is not deleted
		self.assertTrue( mfs.exists() )
		# A new mesh shape gets added as the new stack shape
		self.assertEqual( 'mesh', pm.PyNode( pm.mel.OxGetStackShape( mfs ) ).type() )

	def test_NonHairNodesInTheStack( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		mfs = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.MeshFromStrandsNodeName ) )
		cloth = pm.createNode( 'nCloth' )
		pm.mel.OxAddStackAttributes( cloth )
		self.assertEqual( 1, pm.mel.OxIsOperator( cloth ) )
		mfs.outputStack >> cloth.inputStack
		tops = pm.mel.OxGetStackTops( cloth )
		self.assertEqual( 1, len( tops ) )

	def test_StateIcons( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		pm.mel.OxShowHairStackDialog( "-show" )

		pm.select( hairShape )
		pm.mel.OxUpdateHairStackDialog()

		# HairFromGuides1 should have the state icon
		self.assertTrue( pm.mel.OxGetHairStackDialogNodes( "-hasStateIcon", index = 1 ) )

	# Tests that whenever new operator is added, the output of topmost guides operator in the stack is wired as the guides input
	def test_TopmostGuidesAreWiredToAddedOperator( self ):
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane( guideLength = 1 )

		# Add a length operator to the guides, making it the new top-most operator which outputs guides
		pm.mel.OxAddStrandOperator( editGuidesShape, TestUtilities.LengthNodeName )
		lengthNode = pm.ls( type = TestUtilities.LengthNodeName )[0]

		# Add hair from guides operator. The output of length operator should be connected to its guides input
		hairFromGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( lengthNode, TestUtilities.HairFromGuidesNodeName ) )

		# Reduce strand count to speed up execution
		hairFromGuidesNode.viewportCountFraction.set( 10.0 / hairFromGuidesNode.renderCount.get() )

		# Add a guide cluster operator which will "glue" the hairs to some of the guides
		pm.mel.OxAddStrandOperator( hairFromGuidesNode, TestUtilities.GuideClusterNodeName )

		# Record the Z position of a hair tip, it will measure the length of the hair
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		tipPositionsBeforeLengthChange = TestUtilities.GetTipPositions( hairShape )

		# Change the length of the guides in the length operator
		lengthNode.value.set( 2.0 )

		# If correct guides were wired, the resulting hairs should be about twice as long since they'd be clustered to the updated guides and not the original ones
		tipPositionsAfterLengthChange = TestUtilities.GetTipPositions( hairShape )

		self.assertAlmostEqual( 2, tipPositionsAfterLengthChange[0][2] / tipPositionsBeforeLengthChange[0][2], places = 1 )

	# Tests that if multiple render settings operators are present in a hair shape stack they are properly passed to mesh from strands operator
	def test_RenderSettingsPropagation( self ):
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane( guideLength = 1 )

		# Set first two strands to be group 1 and other two to be group 2
		pm.mel.eval( TestUtilities.EditGuidesCommandName  + " -sg 4 0 1 1 1 2 2 3 2;" );

		# Add hair from guides node with per-vertex distribution
		hairFromGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( editGuidesShape, TestUtilities.HairFromGuidesNodeName ) )
		hairFromGuidesNode.distribution.set( 4 )
		hairFromGuidesNode.interpolation.set( 1 )

		# Add two render settings node, one operating on group 1 and another on group 2, each setting different hair width
		renderSettings1 = pm.PyNode( pm.mel.OxAddStrandOperator( hairFromGuidesNode, TestUtilities.RenderSettingsNodeName ) )
		renderSettings2 = pm.PyNode( pm.mel.OxAddStrandOperator( renderSettings1, TestUtilities.RenderSettingsNodeName ) )
		renderSettings1.strandGroup.set( 1 )
		renderSettings2.strandGroup.set( 2 )
		group1Radius = 2.0
		group2Radius = 3.0
		renderSettings1.radius.set( group1Radius )
		renderSettings2.radius.set( group2Radius )

		# Add mesh from strands operator at the top
		meshFromStrandsNode = pm.PyNode( pm.mel.OxAddStrandOperator( renderSettings2, TestUtilities.MeshFromStrandsNodeName ) )

		# Check the width of the resulting mesh hairs, it should use the values we specified in the render settings nodes
		meshShape = pm.ls( type = 'mesh' )[0]
		vertices = TestUtilities.GetMeshVertices( meshShape )
		self.assertEqual( 16, len( vertices ) )
		strand1Width = vertices[0].distanceTo( vertices[1] )
		strand2Width = vertices[4].distanceTo( vertices[5] )
		strand3Width = vertices[8].distanceTo( vertices[9] )
		strand4Width = vertices[12].distanceTo( vertices[13] )

		self.assertAlmostEqual( group1Radius * 2, strand1Width, places = 3 )
		self.assertAlmostEqual( group1Radius * 2, strand2Width, places = 3 )
		self.assertAlmostEqual( group2Radius * 2, strand3Width, places = 3 )
		self.assertAlmostEqual( group2Radius * 2, strand4Width, places = 3 )

	def test_Namespaces( self ):
		pm.namespace( add = 'Test' )
		pm.namespace( set = 'Test' )
		plane = pm.polyPlane()
		guidesFromMesh = pm.createNode( TestUtilities.GuidesFromMeshNodeName )
		plane[0].outMesh >> guidesFromMesh.inputMesh
		pm.mel.OxUpdateStack( guidesFromMesh, [] )
		self.assertTrue( pm.objExists( 'Test:HairShape1' ) )
		pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.MeshFromStrandsNodeName )
		self.assertTrue( pm.objExists( 'Test:HairShape1' ) )

	def CollapseStack_PreserveTextureCoordinates( self, ascii ):
		hairShape = TestUtilities.AddHairToNewPlane()

		textureCoordinatesBeforeCollapse = pm.mel.OxGetTextureCoordinates( hairShape, 0, type = 0 )

		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		bakedHair = pm.PyNode( pm.mel.OxCollapseStack( hairFromGuides ) )
		bakedHair.displayFraction.set( 1 )

		textureCoordinatesAfterCollapse = pm.mel.OxGetTextureCoordinates( hairShape, 0, type = 0 )

		TestUtilities.CheckPointsAllNearEqual( self, textureCoordinatesBeforeCollapse, textureCoordinatesAfterCollapse )

		filePath = pm.saveAs( 'temp.m' + ( 'a' if ascii else 'b' ) )
		pm.newFile( force = True )
		pm.openFile( filePath, force = True )
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		textureCoordinatesAfterReloading = pm.mel.OxGetTextureCoordinates( hairShape, 0, type = 0 )

		TestUtilities.CheckPointsAllNearEqual( self, textureCoordinatesBeforeCollapse, textureCoordinatesAfterReloading )

	def test_CollapseStack_PreserveTextureCoordinates_Binary( self ):
		self.CollapseStack_PreserveTextureCoordinates( False )

	def test_CollapseStack_PreserveTextureCoordinates_Ascii( self ):
		self.CollapseStack_PreserveTextureCoordinates( True )

	def test_GuidesNoiseAndTwoHairFrizzNodes( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		noise = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.NoiseNodeName ) )
		hairFromGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( noise, TestUtilities.HairFromGuidesNodeName ) )
		hairFromGuidesNode.renderCount.set( 10 )
		frizz1 = pm.PyNode( pm.mel.OxAddStrandOperator( hairFromGuidesNode, TestUtilities.FrizzNodeName ) )
		frizz2 = pm.PyNode( pm.mel.OxAddStrandOperator( frizz1, TestUtilities.FrizzNodeName ) )

		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]

		for i in range( 0, 10 ):
			noise.amount.set( i / 10 )
			TestUtilities.GetTipPositions( hairShape )

	def test_HasOutputGuides( self ):
		hairShape = TestUtilities.AddHairToNewPlane()
		lengthNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.LengthNodeName ) )
		self.assertFalse( pm.mel.OxHasOutputGuides( lengthNode ) )

	def test_ClumpGuidesConnectionAfterReordering( self ):
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 7, length = 5, planeSize = 20 )
		hairFromGuidesNode = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		clumpNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ClumpNodeName ) )
		clumpNode.flyawayFraction.set( 0 )
		clumpNode.clumpCreateMethod.set( 2 );
		pm.mel.OxEditClumps( clumpNode, c = ( 2, 0, 0 ) )
		verticesAfterClumping = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Add length node at the end of the stack first, make sure it not change the hair in any way
		lengthNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.LengthNodeName ) )
		verticesAfterLength = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesAfterClumping, verticesAfterLength )

		# Insert a length node between HFG and Clump nodes
		pm.mel.OxReorderStack( hairFromGuidesNode, [lengthNode] )
		#lengthNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairFromGuidesNode, TestUtilities.LengthNodeName ) )

		verticesAfterLengthReorder = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# The length node should not affect clumping
		TestUtilities.CheckPointsAllNearEqual( self, verticesAfterClumping, verticesAfterLengthReorder )
