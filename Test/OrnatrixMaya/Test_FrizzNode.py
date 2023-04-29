import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.system as pmsys
import pymel.core.datatypes as dt

class Test_FrizzNode( MayaTest.OxTestCase ):

	def test_BasicGuideFrizz( self ):
		# Check that at least some vertices are modified
		TestUtilities.TestStrandOperatorChangingGuides( self, TestUtilities.FrizzNodeName, False, 0.05 )

	def test_SaveLoadScene( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane()
		pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.FrizzNodeName )
		pmsys.renameFile( "Test_FrizzNode_SaveLoadScene.mb" )
		pmsys.saveFile()
		pmsys.openFile( "Test_FrizzNode_SaveLoadScene.mb" )
		self.assertGreater( len( pm.ls( type = TestUtilities.HairShapeName ) ), 0 )
		pmsys.newFile()

	def test_FrizzEffect( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( guideCount = 1000, rootGenerationMethod = 0 )

		tipsBefore = TestUtilities.GetTipPositions( guidesShape )
		frizzNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.FrizzNodeName ) )
		frizzNode.outlierFraction.set( 0 )
		from math import sqrt
		frizzNode.amount.set( 1 / sqrt( 2 ) ) # Now maximum tip shift is 1

		for randomSeed in [1, 123]:
			frizzNode.randomSeed.set( randomSeed )

			tipsAfter = TestUtilities.GetTipPositions( guidesShape )
			tipShifts = [dt.Vector( p1 ).distanceTo( p2 ) for p1, p2 in zip( tipsBefore, tipsAfter )]

			minTipShift = min( tipShifts )
			maxTipShift = max( tipShifts )
			avgTipShift = sum( tipShifts ) / len( tipShifts )

			# Checks for min/max/average values:

			self.assertLess( minTipShift, 0.005 )

			self.assertGreater( maxTipShift, 0.3 )
			self.assertLess( maxTipShift, 1 + 1e-6 )

			self.assertGreater( avgTipShift, 0.11 )
			self.assertLess( avgTipShift, 0.13 )

	def test_LengthDependentOptionDoesNotExplodeStrands( self ):
		guidesShape = TestUtilities.AddGuidesToNewPlane( guideCount = 1000, rootGenerationMethod = 0 )

		tipsBefore = TestUtilities.GetTipPositions( guidesShape )
		frizzNode = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.FrizzNodeName ) )
		frizzNode.outlierFraction.set( 0 )
		from math import sqrt
		frizzNode.amount.set( 1 )
		frizzNode.isLengthDependent.set( True )

		tipsAfter = TestUtilities.GetTipPositions( guidesShape )
		tipShifts = [dt.Vector( p1 ).distanceTo( p2 ) for p1, p2 in zip( tipsBefore, tipsAfter )]

		maxTipShift = max( tipShifts )
		self.assertLess( maxTipShift, 1 + 1e-6 )
