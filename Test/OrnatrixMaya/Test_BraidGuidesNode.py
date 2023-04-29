import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt

class Test_BraidGuidesNode( MayaTest.OxTestCase ):
	# A very basic test which ensures that braid creating command is working and that guides are created
	def test_CreateBraidGuides( self ):
		# Add braid guides using the MEL command
		guidesShape = pm.PyNode( pm.mel.OxAddBraidGuides() )

		# By default we should get a pigtail braid with three guides
		self.assertEqual( 3, pm.mel.OxGetStrandCount( guidesShape ) )
