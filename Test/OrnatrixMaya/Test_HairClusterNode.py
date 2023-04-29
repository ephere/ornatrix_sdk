import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.system as pmsys
import pymel.core.datatypes as dt

class Test_HairClusterNode( MayaTest.OxTestCase ):

	def SetupClusterNode( self, nodeName ):
		node = pm.PyNode( nodeName )
		node.clusterCount.set( 1 )

	def test_BasicHairCluster( self ):
		TestUtilities.TestStrandOperatorChangingGuides( self, TestUtilities.HairClusterNodeName, False, 0.05, isTestingOnGuides = False, operatorSetup = self.SetupClusterNode )

	# Tests that we can reduce the number of input strands to hair cluster operator and no errors will happen
	def test_HairClusterAboveReducedHairCount( self ):
		hairShape = TestUtilities.AddHairToNewPlane( pointsPerStrandCount = 2, strandCount = 40, rootGenerationMethod = 2 )
		pm.mel.OxAddStrandOperator( hairShape, TestUtilities.HairClusterNodeName )
		hairClusterNode = pm.ls( type = TestUtilities.HairClusterNodeName )[0]
		hairClusterNode.clusterCount.set( 5 )

		# Get vertices just to force the operator to validate
		vertices1 = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Now reduce the number of input guides
		hairFromGuidesNode = pm.ls( type = 'HairFromGuidesNode' )[0]
		hairFromGuidesNode.viewportCount.set( 5 )

		# Evaluate result again, no error should happen
		vertices2 = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

	def test_ToggleHairClusteringInStack( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( planeSize = 10 )
		hairFromGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.HairFromGuidesNodeName ) )
		# Stack shape changes to HairShape after adding HairFromGuides
		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		hairClusterNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairFromGuidesNode, TestUtilities.HairClusterNodeName ) )
		hairClusterNode.clusterCount.set( 2 )
		verticesWithClusterOn = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		pm.mel.OxEnableOperator( hairClusterNode, 0 )
		verticesWithClusterOff = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
		pm.mel.OxEnableOperator( hairClusterNode, 1 )
		verticesWithClusterOnAgain = TestUtilities.GetVerticesInObjectCoordinates( guidesShape )

		# Re-enabling hair clustering node should cluster the hair again
		TestUtilities.CheckPointsAllNearEqual( self, verticesWithClusterOn, verticesWithClusterOnAgain )

	# Tests for a crash regression when incoming hair becomes guides
	def test_SwitchInputHairToGuides( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( planeSize = 10 )
		hairFromGuidesNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.HairFromGuidesNodeName ) )
		# Stack shape changes to HairShape after adding HairFromGuides
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		hairClusterNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairFromGuidesNode, TestUtilities.HairClusterNodeName ) )

		# Just evaluate the cluster
		TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Turn off hair from guides operator so input becomes guides
		pm.mel.OxEnableOperator( hairFromGuidesNode, 0 )

		# Evaluate again. No crash should happen now.
		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		TestUtilities.GetVerticesInObjectCoordinates( guidesShape )
