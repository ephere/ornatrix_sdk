import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.system as pmsys
import pymel.core.datatypes as dt

class Test_FillStrandChannel( MayaTest.OxTestCase ):

	def test_FillPerStrand( self ):
		# Create some guides with a Weights per-root channel and assign value of 1 to all of them
		guidesShape = TestUtilities.AddEditGuidesToNewPlane( pointsPerStrandCount = 3, planeSegmentCount = 2 )
		guideCount = pm.mel.OxGetStrandCount( guidesShape )
		pm.select( guidesShape )
		pm.mel.eval( TestUtilities.EditGuidesCommandName + " -crc 1 Weights;" )
		pm.select( guidesShape + '.f[0:' + str(guideCount-1) + ']' )
		pm.mel.eval( "OxFillStrandChannel -channelIndex 1 -channelValue 1.0" )
		
		values = pm.mel.OxGetRootValuesForChannel( guidesShape, 1 )
		for strandIdx in range( 0, len( values ) ):
			self.assertEqual( values[strandIdx], 1.0 )

	def test_FillPerVertex( self ):
		# Create some guides with a Weights per-vertex channel and assign value of 1 to all of them
		guidesShape = TestUtilities.AddEditGuidesToNewPlane( pointsPerStrandCount = 3, planeSegmentCount = 2 )
		guideCount = pm.mel.OxGetStrandCount( guidesShape )
		pm.select( guidesShape )
		pm.mel.eval( TestUtilities.EditGuidesCommandName + " -cvc 1 Weights;" )
		pm.select( guidesShape + '.f[0:' + str(guideCount-1) + ']' )
		pm.mel.eval( "OxFillStrandChannel -channelIndex 1000 -channelValue 1.0" )
		channelData = pm.mel.OxGetPointData( guidesShape, 0, 1, 0, type = 1 )
		
		for strandIdx in range( 0, guideCount ):
			vertexData0 = pm.mel.OxGetPointData( guidesShape, strandIdx, 0, 0, type = 1 )
			vertexData1 = pm.mel.OxGetPointData( guidesShape, strandIdx, 1, 0, type = 1 )
			vertexData2 = pm.mel.OxGetPointData( guidesShape, strandIdx, 2, 0, type = 1 )

			self.assertEqual( vertexData0, 1.0 )
			self.assertEqual( vertexData1, 1.0 )
			self.assertEqual( vertexData1, 1.0 )