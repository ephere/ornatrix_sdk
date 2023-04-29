import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.system as pmsys
import pymel.core.datatypes as dt

class Test_GuideClusterNode( MayaTest.OxTestCase ):

	def test_BasicGuideCluster( self ):
		# Guide and hair must be generated at different locations for cluster to have effect so set hair root generation method to 'FaceCenter' (6)
		TestUtilities.TestStrandOperatorChangingGuides( self, TestUtilities.GuideClusterNodeName, False, 0.05, isTestingOnGuides = False, rootGenerationMethod = 6 )

	def test_GuideClusterChannel( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( pointsPerStrandCount = 2, rootGenerationMethod = 2 )
		pm.select( guidesShape )

		pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.EditGuidesShapeName )
		pm.mel.OxAddStrandOperator( "", TestUtilities.HairFromGuidesNodeName )

		guidesFromMeshNode = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		hairFromGuidesNode = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]

		# Evaluate once and change the number of incoming guides to edit guides shape
		#TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		guidesFromMeshNode.distribution.set( 4 )
		hairFromGuidesNode.viewportCountFraction.set( 10.0 / hairFromGuidesNode.renderCount.get() )

		verticesWithoutClustering = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		pm.mel.OxAddStrandOperator( "", TestUtilities.GuideClusterNodeName )
		verticesWithClustering = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Create a new guides channel and set value of some guides
		editGuidesShape = pm.ls( type = TestUtilities.EditGuidesShapeName )[0]
		pm.select( editGuidesShape )
		pm.mel.eval( TestUtilities.EditGuidesCommandName  + " -crc 1 Clustering;" )
		pm.mel.eval( TestUtilities.EditGuidesCommandName  + " -rcd 2 4 0 0 0 0 0 0 1 1;" );

		# Set clustering channel
		guideClusteringNode = pm.ls( type = TestUtilities.GuideClusterNodeName )[0]
		guideClusteringNode.clusterChannel.set( 2 )
		verticesWithPartialClustering = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		# Moment of truth, all vertices should be different in three different modes
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithoutClustering, verticesWithClustering )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithPartialClustering, verticesWithClustering )
		TestUtilities.CheckPointsNotAllNearEqual( self, verticesWithPartialClustering, verticesWithoutClustering )

		# Clearing the selection should not have any effect
		pm.select( hairShape )
		verticesAfterDeselectAll = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		TestUtilities.CheckPointsAllNearEqual( self, verticesWithPartialClustering, verticesAfterDeselectAll )