import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt
import unittest

class Test_CurlNode( MayaTest.OxTestCase ):

	def test_BasicGuideCurl( self ):
		# Check that at least some vertices are modified
		TestUtilities.TestStrandOperatorChangingGuides( self, TestUtilities.CurlNodeName, False, 0.1, pointsPerStrandCount = 4 )

	def test_MagnitudeChannel( self ):
		# Add hair and curl node
		hairShape = TestUtilities.AddHairToNewPlane()
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		curlName = pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.CurlNodeName )
		curl = pm.PyNode( curlName )

		# Insert guides shape node
		pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.EditGuidesShapeName )
		guidesNode = TestUtilities.GetNodeByType( TestUtilities.EditGuidesShapeName )

	def SetupComputePerformance( self ):
		self.topShape = TestUtilities.AddGuidesToNewPlane( guideCount = 20000, pointsPerStrandCount = 10, rootGenerationMethod = 2, planeSegmentCount = 2, planeSize = 100 )
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]
		curl = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.CurlNodeName ) )

		pm.currentTime( 1 )
		curl.magnitude.set( 1 )
		pm.setKeyframe()
		pm.currentTime( 10 )
		curl.magnitude.set( 10 )
		pm.setKeyframe()

		pm.currentTime( 1 )
		self.topShape.boundingBox()

	def test_ComputePerformance( self ):
		self.assertPerformance( 'MayaTest.Self.SetupComputePerformance()', 'MayaTest.RunAnimation( MayaTest.Self.topShape, frameCount = 5 )' )

	def SetupHairCopyPerformance( self, opName ):
		self.topShape = TestUtilities.AddGuidesToNewPlane( guideCount = 50000, pointsPerStrandCount = 10, rootGenerationMethod = 2, planeSegmentCount = 2, planeSize = 100 )
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]

		editGuidesShape = pm.PyNode( pm.mel.OxAddStrandOperator( guidesFromMesh, TestUtilities.EditGuidesShapeName ) )
		editGuidesShape.useStrandGroups.set( True )

		# Limit the effect of the operator to a single strand, to reduce the calculation time and emphasize copying
		pm.select( editGuidesShape + ".ep[0]" )
		pm.mel.OxAssignStrandGroup( editGuidesShape, "1" )

		operators = [editGuidesShape]
		for i in range( 0, 5 ):
			op = pm.PyNode( pm.mel.OxAddStrandOperator( operators[-1], opName ) )
			op.strandGroupPattern.set( '1' )
			operators.append( op )

		del operators[0]
		for op in operators:
			pm.select(op)
			pm.currentTime( 1 )
			if opName == TestUtilities.CurlNodeName:
				op.magnitude.set( 1 )
			else:
				op.width.set( 1 )
			pm.setKeyframe()
			pm.currentTime( 10 )
			if opName == TestUtilities.CurlNodeName:
				op.magnitude.set( 2 )
			else:
				op.width.set( 2 )
			pm.setKeyframe()

		pm.currentTime( 1 )
		self.topShape.boundingBox()

	def SetupHairDataCopyPerformance( self ):
		self.SetupHairCopyPerformance( TestUtilities.CurlNodeName )

	@unittest.skipIf( not MayaTest.InBatchMode, "For some reason produces different result in GUI" )
	def test_HairDataCopyPerformance( self ):
		self.assertHairMemoryUsage( lambda: self.SetupHairDataCopyPerformance(), 127929 )
		self.assertPerformance( 'MayaTest.Self.SetupHairDataCopyPerformance()', 'MayaTest.RunAnimation( MayaTest.Self.topShape, frameCount = 5 )' )

	def SetupHairCopyPerformanceWidth( self ):
		self.SetupHairCopyPerformance( TestUtilities.ChangeWidthNodeName )

	@unittest.skipIf( not MayaTest.InBatchMode, "For some reason produces different result in GUI" )
	def test_HairCopyPerformanceWidth( self ):
		self.assertHairMemoryUsage( lambda: self.SetupHairCopyPerformanceWidth(), 102539 )
		self.assertPerformance( 'MayaTest.Self.SetupHairCopyPerformanceWidth()', 'MayaTest.RunAnimation( MayaTest.Self.topShape, frameCount = 5 )' )

	@unittest.skipIf( not MayaTest.OsIsWindows, "Object and copy counts are not the same across OS's" )
	def test_DisabledOperatorsMemoryUsage( self ):
		pm.mel.OxProfiler( enableHairRegistry = True )
		plane = pm.polyPlane()
		hairShape = TestUtilities.AddHairToMesh( plane[0], hairRenderCount = 10000, hairViewportCount = 10 )
		for i in range( 0, 5 ):
			curl = pm.mel.OxAddStrandOperator( '', TestUtilities.CurlNodeName )
			pm.mel.OxEnableOperator( curl, False )

		pm.mel.OxGetStrandCount( hairShape )

		# HFG node now keeps one extra copy of hair object around for performance optimization when 'hfgOptimization=on',
		# so we need to account for those two different scenarios (hfgOptimization off/on)
		hairFromGuides = pm.ls( type = TestUtilities.HairFromGuidesNodeName )[0]
		if hairFromGuides.hfgOptimization.get() == False:
			self.assertEqual( 22, pm.mel.OxProfiler( '-hairObjectCount' ) )
		else:
			self.assertEqual( 18, pm.mel.OxProfiler( '-hairObjectCount' ) )

		self.assertEqual( 372, pm.mel.OxProfiler( '-hairStrandCount' ) )
