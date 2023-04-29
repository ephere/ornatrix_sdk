import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.system as pmsys
import pymel.core.datatypes as dt

class Test_MergeNode( MayaTest.OxTestCase ):

	# Tests for a bug where removing input merged shapes doesn't update the output
	def test_DeletingMergedShapesUpdatesOutput( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		guidesShapeMerged = TestUtilities.AddGuidesToNewPlane()

		mergeNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.MergeNodeName ) )

		# Initially only 4 roots are present
		self.assertEqual( 4, pm.mel.OxGetStrandCount( guidesShape ) )

		# Adding a merged shape should add 4 more strands
		pm.connectAttr( guidesShapeMerged.worldHair, mergeNode.mergedStrands[0] )
		self.assertEqual( 8, pm.mel.OxGetStrandCount( guidesShape ) )

		# Disconnecting the attribute should bring the resulting strand count back to 4
		pm.disconnectAttr( guidesShapeMerged.worldHair, mergeNode.mergedStrands[0] )
		self.assertEqual( 4, pm.mel.OxGetStrandCount( guidesShape ) )