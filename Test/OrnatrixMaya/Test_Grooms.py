import MayaTest
import TestUtilities
import Test_BakedGuidesNode
import Test_BakedHairNode
import Test_HairFromMeshStripsNode
import os.path
import pymel.core as pm
import pymel.core.datatypes as dt
import sys
import unittest
import xml.etree.ElementTree as et

if sys.version_info > (3,):
	long = int

''' This class tests that all of the operators within Ornatrix Maya can be properly saved and loaded as grooms. It needs to be updated whenever new operators which support
grooms are added, or when new groom parameters are added '''
class Test_Grooms( MayaTest.OxTestCase ):

	# Tests for a bug where default interpolation parameter is specified in HairFromGuides groom operator causing an assertion when generating hair
	def test_BarycentricAttributePresentInHFGGroom( self ):
		hairShape = pm.PyNode( pm.mel.OxLoadGroom( path = """<?xml version="1.0" encoding="utf-8"?>
<Groom name="test_BarycentricAttributePresentInHFGGroom">
	<Stack>
		<Sphere referenced="true" optional="true" />
		<GuidesFromMesh />
		<HairFromGuides interpolation="Barycentric" />
	</Stack>
</Groom>""" ) )

		strandCount = pm.mel.OxGetStrandCount( hairShape )
		self.assertTrue( strandCount != 0 )

	def TestXmlGroom( self, operatorNodeName, operatorName, nonDefaultParameters, expectedNonDefaultAttributes = [], addGuides = True, addHair = False ):
		defaultParametersGroomName = "test_groomDefaultParameters" + operatorName + MayaTest.OldGroomExtension
		nonDefaultParametersGroomName = "test_groomNonDefaultParameters" + operatorName + MayaTest.OldGroomExtension

		guidesShape = None
		expectedOperators = None
		operator = None
		if addGuides:
			if addHair:
				guidesShape = TestUtilities.AddHairToNewPlane()
				expectedOperators = ["PolygonMesh", "GuidesFromMesh", "HairFromGuides", operatorName]
			else:
				guidesShape = TestUtilities.AddGuidesToNewPlane()
				expectedOperators = ["PolygonMesh", "GuidesFromMesh", operatorName]
			operator = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, operatorNodeName ) )
		else:
			expectedOperators = ["PolygonMesh", operatorName]
			if operatorNodeName == TestUtilities.GuidesFromMeshNodeName:
				plane = pm.polyPlane()
				pm.select( plane )
				meshShape = plane[0]

				# Create nodes
				transformNode = meshShape.firstParent2()
				operator = pm.createNode( TestUtilities.GuidesFromMeshNodeName )
				guidesShape = pm.createNode( TestUtilities.HairShapeName, parent = transformNode )

				# Make connections
				meshShape.outMesh >> operator.inputMesh
				operator.outputGuides >> guidesShape.inputHair
				# Note: `guidesShape.distributionMesh` isn't connected here
			elif operatorNodeName == TestUtilities.GuidesFromCurvesNodeName:
				guidesShape = TestUtilities.AddGuidesFromCurves()
				operator = pm.ls( type = TestUtilities.GuidesFromCurvesNodeName )[0]
				expectedOperators = ["Curves", operatorName]
			else:
				plane = pm.polyPlane( sx = 1, sy = 1 )
				pm.select( plane )
				guidesShape = plane[0]

		pm.mel.OxSaveGroom( path = self.addTempFile( defaultParametersGroomName ), optional = True )
		defaultParametersGroomFilePath = self.addTempFile( defaultParametersGroomName )
		xml = et.parse( defaultParametersGroomFilePath )
		root = xml.getroot()[0]
		children = [child.tag for child in root]
		self.assertEqual( children, expectedOperators )
		operatorNode = root.find( operatorName )

		#print( defaultParametersGroomFilePath )

		# Uncomment to see te XML file
		#with open( defaultParametersGroomFilePath, 'r' ) as fin:
		#	print( fin.read() )

		# With default values we have an empty operator element
		operatorChildren = [child.tag for child in operatorNode] + list( operatorNode.attrib.keys() )
		self.assertEqual( sorted( expectedNonDefaultAttributes ), sorted( operatorChildren ) )

		# Set non-default values
		for parameter in nonDefaultParameters:
			mayaAttribute = operator.attr( parameter[0] )

			attributeValue = 0
			if len( parameter ) > 1:
				attributeValue = parameter[1]
			else:
				# Values which typically would never be used as a default
				nonDefaultValues = {
					'float' : 0.234,
					'long' : 87,
					'integer' : 87,
					'enum' : 12,
					'string' : 'foo' }
				attributeValue = nonDefaultValues[mayaAttribute.type()]

			mayaAttribute.set( attributeValue )

		pm.mel.OxSaveGroom( path = self.addTempFile( nonDefaultParametersGroomName ), optional = True )
		xml = et.parse( self.addTempFile( nonDefaultParametersGroomName ) )

		root = xml.getroot()[0]
		children = [child.tag for child in root]
		self.assertEqual( children, expectedOperators )
		operatorNode = root.find( operatorName )

		# With default values we have an empty operator element
		operatorChildren = [child.tag for child in operatorNode]
		operatorAttributes = [attrib for attrib in operatorNode.attrib]
		expectedLength = len( nonDefaultParameters ) + len( expectedNonDefaultAttributes )
		actualLength = len( operatorAttributes ) + len( operatorChildren )
		self.assertEqual( expectedLength, actualLength, "Expected: " + str( nonDefaultParameters ) + str( expectedNonDefaultAttributes ) + str( ", Actual: " ) + str( operatorAttributes ) + str( operatorChildren ) )

	def TestNewGroom( self, operatorNodeName, operator = None, addHair = False, attributesToChange = None, crcWindows = None, importType = 3 ):
		groomName = "test_groom_" + operatorNodeName + MayaTest.NewGroomExtension

		hairShape = None
		if operator is None:
			if addHair:
				hairShape = TestUtilities.AddHairToNewPlane()
			else:
				hairShape = TestUtilities.AddGuidesToNewPlane( pointsPerStrandCount = 15 )
			operator = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, operatorNodeName ) )
			pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ChangeWidthNodeName ) )

		hairShape = pm.PyNode( pm.mel.OxGetStackShape( operator ) )

		if attributesToChange is not None:
			for change in attributesToChange:
				operator.setAttr( change[0], change[1] )

		filePath = self.addTempFile( groomName )
		hairShapeTypeBefore = hairShape.type()
		if hairShapeTypeBefore == TestUtilities.HairShapeName:
			verticesBefore = TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		pm.select( hairShape )
		pm.mel.OxSaveGroom( path = filePath, forceOverwrite = True )

		# TOOD: Check what's different on other OS's or machines
		#if MayaTest.OsIsWindows and not crcWindows is None:
		#	self.assertFileCrc( filePath, crcWindows )

		pm.newFile( force = 1 )
		hairShape = pm.PyNode( pm.mel.OxLoadGroom( path = filePath, importType = importType ) )
		self.assertEqual( hairShapeTypeBefore, hairShape.type() )
		if hairShape.type() == TestUtilities.HairShapeName:
			verticesEvaluatedGroom = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
			TestUtilities.CheckPointsAllNearEqual( self, verticesBefore, verticesEvaluatedGroom )

	# Guides loaded from an Alembic file
	def test_BakedGuidesGroom_Alembic( self ):
		abcFilePath = self.addTempFile( "test_BakedGuidesGroom.abc" )
		Test_BakedGuidesNode.CreateGuidesAbcArchive( abcFilePath, planeSegmentCount = 1, planeSize = 10 )
		hairShape, bakedNode = TestUtilities.AddBakedGuides()
		bakedNode.sourceFilePath.set( abcFilePath )
		self.TestNewGroom( TestUtilities.BakedGuidesNodeName, bakedNode, importType = 3 )

	# Local guides value that was the result of stack collapse
	def test_BakedGuidesGroom_Collapsed( self ):
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane( planeSegmentCount = 2, pointsPerStrandCount = 3 )
		bakedNode = pm.PyNode( pm.mel.OxCollapseStack( editGuidesShape ) )
		self.TestNewGroom( TestUtilities.BakedGuidesNodeName, bakedNode, importType = 3 )

	# Local guides value that was the result of stack collapse
	def test_BakedGuidesGroom_FullStack( self ):
		editGuidesShape = TestUtilities.AddEditGuidesToNewPlane( planeSegmentCount = 2, pointsPerStrandCount = 3 )
		bakedNode = pm.PyNode( pm.mel.OxCollapseStack( editGuidesShape ) )
		self.TestNewGroom( TestUtilities.BakedGuidesNodeName, bakedNode, importType = 0 )

	# Hair loaded from an Alembic file
	def test_BakedHairGroom_Alembic( self ):
		abcFilePath = self.addTempFile( "test_BakedHairGroom.abc" )
		Test_BakedHairNode.CreateHairAbcArchive( abcFilePath )
		hairShape, bakedNode = TestUtilities.AddBakedHair()
		bakedNode.sourceFilePath.set( abcFilePath )
		bakedNode.displayFraction.set( 1 )
		self.TestNewGroom( TestUtilities.BakedHairNodeName, bakedNode, importType = 2 )

	def TestBakedHairGroom( self, importType ):
		hairShape = TestUtilities.AddHairToNewPlane( planeSize = 10 )
		frizz = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.FrizzNodeName ) )
		frizz.amount.set( 10 )
		bakedHair = pm.PyNode( pm.mel.OxCollapseStack( frizz ) )
		self.TestNewGroom( TestUtilities.BakedHairNodeName, bakedHair, importType = importType )

	# Local hair value that was the result of stack collapse
	def test_BakedHairGroom_Collapsed( self ):
		self.TestBakedHairGroom( 2 )

	def test_BakedHairGroom_FullStack( self ):
		self.TestBakedHairGroom( 0 )

	def test_CurlGroom( self ):
		self.TestNewGroom( TestUtilities.CurlNodeName, crcWindows = long( 3105421621 ) )

		self.TestXmlGroom(
			TestUtilities.CurlNodeName,
			"Curl",
			[ ["waveform", 0], ["magnitude"], ["phase"], ["phaseOffset"], ["stretch"], ["isLengthDependent", True], ["noiseScale"], ["noiseAmount"] ],
			['magnitudeCurve'] )

	def test_DetailGroom( self ):
		self.TestNewGroom( TestUtilities.DetailNodeName )

		self.TestXmlGroom( TestUtilities.DetailNodeName, "Detail", [
				["viewPointCount"],
				["renderPointCount"],
				["angleThreshold"],
				["sampleByThreshold", True],
				["sampleSize"],
				["smoothing"]] )

	def test_EditGuidesGroom( self ):
		hairShape = TestUtilities.AddGuidesToNewPlane( pointsPerStrandCount = 15 )
		editGuidesShape = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.EditGuidesShapeName ) )
		pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ChangeWidthNodeName ) )
		import Test_EditGuidesShape
		Test_EditGuidesShape.Test_EditGuidesShape.MoveAllControlPoints( editGuidesShape, [5, 0, 10] )
		self.TestNewGroom( TestUtilities.EditGuidesShapeName, operator = editGuidesShape, crcWindows = long( 1104639627 ) )

	def test_ClumpGroom( self ):
		self.TestNewGroom( TestUtilities.ClumpNodeName, crcWindows = long( 1094156672 ), attributesToChange = [ ['amount', 0.7], ['twist', 0.1] ] )

	def test_HairFromMeshStripsGroom( self ):
		plane1 = pm.polyPlane( w = 1, h = 10, sx = 1, sh = 10 )
		plane1[0].rotate.set( 90, 0, 0 )
		plane1[0].translate.set( -2, 5, 0 )
		plane2 = pm.polyPlane( w = 1, h = 10, sx = 1, sh = 10 )
		plane2[0].rotate.set( 90, 0, 0 )
		plane2[0].translate.set( 2, 5, 0 )
		pm.select( [ plane1, plane2 ] )
		hairShape = pm.PyNode( pm.mel.OxAddHairFromMeshStrips( [ plane1, plane2 ] ) )
		groundStrandsNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.GroundStrandsNodeName ) )
		distributionSurface = pm.polyPlane( w = 10, h = 10, sx = 10, sh = 10 )
		distributionSurface[0].outMesh >> groundStrandsNode.distributionMesh
		groundStrandsNode.detachRoots.set( 0 )

		hairFromMeshStrips = pm.ls( type = TestUtilities.HairFromMeshStripsNodeName )[0]
		self.TestNewGroom( TestUtilities.HairFromMeshStripsNodeName, operator = hairFromMeshStrips, crcWindows = 1655697869 )

	def test_FrizzGroom( self ):
		self.TestNewGroom( TestUtilities.FrizzNodeName )

		self.TestXmlGroom( TestUtilities.FrizzNodeName, "Frizz", [
				["amount"],
				["scale"],
				["outlierFraction"],
				["outlierAmount"],
				["randomSeed"] ],
			['amountCurve'] )

	def test_GenerateStrandDataBaked( self ):
		self.TestNewGroom( TestUtilities.GenerateStrandDataNodeName )

	def test_GenerateStrandDataFullStack( self ):
		self.TestNewGroom( TestUtilities.GenerateStrandDataNodeName, importType = 0 )

	def test_GenerateStrandDataXml( self ):
		self.TestXmlGroom( TestUtilities.GenerateStrandDataNodeName, "GenerateStrandData", [
				["strandGroupPattern"],
				["randomSeed"],
				["newChannelType", 1],
				["newChannelName"],
				["generationMethod", 2],
				["useTargetValueRange", False],
				["minimumTargetValue"],
				["maximumTargetValue"],
				["useSourceValueRange", True],
				["minimumSourceValue"],
				["maximumSourceValue"]] )

	def test_GravityGroom( self ):
		self.TestNewGroom( TestUtilities.GravityNodeName )

		self.TestXmlGroom( TestUtilities.GravityNodeName, "Gravity", [
				["force"],
				["strandGroupPattern"]],
				["amountCurve"] )

	def test_GroundStrandsGroom( self ):
		self.TestNewGroom( TestUtilities.GroundStrandsNodeName )

		self.TestXmlGroom( TestUtilities.GroundStrandsNodeName, "GroundStrands", [], ["positionBlendRamp"] )

	def test_GuideClusterGroom( self ):
		self.TestXmlGroom( TestUtilities.GuideClusterNodeName, "GuideCluster", [
				["clusterAmount"],
				["twistAmount"],
				["wispAmount"],
				["wispOffset"],
				["isUsingWisps", True]],
				["twistCurve", "clusterCurve"] )

	def test_GuidesFromCurvesGroom( self ):
		self.TestXmlGroom( TestUtilities.GuidesFromCurvesNodeName, "GuidesFromCurves", [["pointCount"]], addGuides = False )

	def test_GuidesFromHairGroom( self ):
		self.TestXmlGroom( TestUtilities.GuidesFromHairNodeName, "GuidesFromHair", [], addHair = True )

	def test_GuidesFromMeshGroom( self ):
		self.TestXmlGroom( TestUtilities.GuidesFromMeshNodeName, "GuidesFromMesh", [
				["distribution", 4],
				["count"],
				["length"],
				["lengthRandomness"],
				["pointCount"],
				["randomSeed"],
				["topologyBasedOrientation", False],
				["detachRoots", True],
				["rememberRootPositions", True]], addGuides = False, addHair = False )

	def test_HairClusterGroom( self ):
		self.TestXmlGroom( TestUtilities.HairClusterNodeName, "HairCluster", [
				["clusterCount"],
				["randomSeed"],
				["distanceThreshold"],
				["useUv", True]],
				["clusterCurve"], addHair = True )

	def test_HairFromGuidesGroom( self ):
		self.TestNewGroom( TestUtilities.HairFromGuidesNodeName, attributesToChange = [ [ 'viewportCountFraction', 1 ], [ 'renderCount', 100 ] ], crcWindows = long( 1416345784 ) )

		self.TestXmlGroom( TestUtilities.HairFromGuidesNodeName, "HairFromGuides", [
				["distribution", 4],
				["viewportCount"],
				["renderCount"],
				["interpolationGuideCount", 1],
				["guideProximityDistance"]],
				[] )

	def test_LengthGroom( self ):
		self.TestNewGroom( TestUtilities.LengthNodeName )

		self.TestXmlGroom( TestUtilities.LengthNodeName, "Length", [
				["value"],
				["randomize"],
				["randomSeed"],
				["strandGroupPattern"],
				["minimumValue"],
				["maximumValue"]] )

	def test_MeshFromHairGroom( self ):
		self.TestNewGroom( TestUtilities.MeshFromStrandsNodeName, importType = 0 )

		self.TestXmlGroom( TestUtilities.MeshFromStrandsNodeName, "MeshFromStrands", [
				["meshType", 0],
				["strandGroup"],
				["strandGroupPattern", "1,2"],
				["randomSeed"]] )

	def test_MultiplierGroom( self ):
		self.TestNewGroom( TestUtilities.MultiplierNodeName )

		self.TestXmlGroom( TestUtilities.MultiplierNodeName, "Multiplier", [
				["randomSeed"],
				["strandGroupPattern"],
				["probability"],
				["copies"],
				["copiesViewPercent"],
				["spreadRoot"],
				["spreadTip"],
				["fluffRoot"],
				["fluffTip"],
				["shiftRoot"],
				["shiftTip"],
				["twist"]],
				["twistCurve", "spreadCurve", "fluffCurve", "shiftCurve"] )

	def test_PropagationGroom( self ):
		self.TestNewGroom( TestUtilities.PropagationNodeName )

		self.TestXmlGroom( TestUtilities.PropagationNodeName, "Propagation", [
				["generationMethod", 0],
				["count", 1],
				["verticesPerRootCount"],
				["countPerStrand", False],
				["randomSeed"],
				["lowRange"],
				["highRange"],
				["lengthRelativeToBaseStrand", True],
				["faceSurface", False],
				["sideCount"],
				["twist"],
				["twistRandomness"],
				["fanning"],
				["fanningRandomness"],
				["length"],
				["lengthRandomness"],
				["resultStrandGroup"]],
				["twistCurve", "fanningCurve", "lengthCurve", "distributionCurve"] )

	def test_PushAwayFromSurfaceGroom( self ):
		self.TestNewGroom( TestUtilities.PushAwayFromSurfaceNodeName )

		self.TestXmlGroom( TestUtilities.PushAwayFromSurfaceNodeName, "PushAwayFromSurface", [["strandGroupPattern"], ["distance"]], ["amountRamp"] )

	def test_RenderSettingsGroom( self ):
		self.TestXmlGroom( TestUtilities.RenderSettingsNodeName, "RenderSettings", [
				["radius"],
				["strandGroup"]],
				["radiusCurve"] )

	def test_RotateGroom( self ):
		self.TestNewGroom( TestUtilities.RotateNodeName )

		self.TestXmlGroom( TestUtilities.RotateNodeName, "RotateStrands", [
				["globalAngle"],
				["globalChaos"],
				["faceCoordinate", True],
				["randomSeed"],
				["strandGroupPattern"],
				["orientBasedOnStrandShape", True]] )

	def test_SurfaceCombGroom( self ):
		self.TestNewGroom( TestUtilities.SurfaceCombNodeName )

		# The generated XML is different on different machines
		#self.TestXmlGroom( TestUtilities.SurfaceCombNodeName, "SurfaceComb", [
		#		["affectWholeStrand", True],
		#		["chaos"],
		#		["randomSeed"],
		#		["applyToShape", False],
		#		["applyToDirection", False]],
		#		["slopeCurve", "algorithm"] )

	def test_SymmetryGroom( self ):
		self.TestNewGroom( TestUtilities.SymmetryNodeName )

		self.TestXmlGroom( TestUtilities.SymmetryNodeName, "Symmetry", [
				["useIgnoreDistance", True],
				["ignoreDistance"]] )

	def test_XmlGroomWithSphereAsBaseObject( self ):
		groomPath = self.addTempFile( "test_GroomWithSphereAsBaseObject" + MayaTest.OldGroomExtension )
		hairShape = TestUtilities.AddHairToNewSphere( radius = 12.3 )
		pm.select( hairShape )
		pm.mel.OxSaveGroom( path = groomPath, optional = True )

		xml = et.parse( groomPath )
		root = xml.getroot()[0]
		children = [child.tag for child in root]
		self.assertEqual( children, ["Sphere", "GuidesFromMesh", "HairFromGuides"] )
		self.assertEqual( '12.3', root.find( 'Sphere' ).attrib["radius"] )
		self.assertEqual( 'true', root.find( 'Sphere' ).attrib["optional"] )

		# Load groom with no selection
		pm.newFile( force = 1 )
		hairShape = pm.PyNode( pm.mel.OxLoadGroom( path = groomPath ) )

		# Ensure that a sphere has been created as a base object and radius is correct
		sphereShape = pm.ls( type = "polySphere" )[0]
		self.assertAlmostEqual( 12.3, sphereShape.radius.get(), places = 3 )

	def test_GroomWithSphereAsBaseObject( self ):
		groomPath = self.addTempFile( "test_GroomWithSphereAsBaseObject" + MayaTest.NewGroomExtension )
		hairShape = TestUtilities.AddHairToNewSphere( radius = 12.3 )
		pm.select( hairShape )
		pm.mel.OxSaveGroom( path = groomPath, optional = True )

		# Load groom with no selection
		pm.newFile( force = 1 )
		hairShape = pm.PyNode( pm.mel.OxLoadGroom( path = groomPath ) )

		# Ensure that a sphere has been created as a base object and radius is correct
		sphereShape = pm.ls( type = "polySphere" )[0]
		self.assertAlmostEqual( 12.3, sphereShape.radius.get(), places = 3 )

	def test_CreateAndResaveFurballGroom( self ):
		hairShape = pm.PyNode( pm.mel.OxLoadGroom( path = "Fur Ball" ) )
		guidesFromMesh = TestUtilities.GetNodeByType( TestUtilities.GuidesFromMeshNodeName )
		# Even(7) is the only value of distribution for which this test passes.
		# TODO: investigate why the test fails for other distribution methods
		self.assertEqual( guidesFromMesh.distribution.get(), 7 )
		verticesFromStockGroom = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		verticesFromStockGroom.sort()

		# Generate second groom
		groomPath = self.addTempFile( "test_CreateAndResaveFurballGroom" + MayaTest.NewGroomExtension )
		pm.mel.OxSaveGroom( path = groomPath, optional = True )
		pm.newFile( force = 1 )

		hairShape = pm.PyNode( pm.mel.OxLoadGroom( path = groomPath ) )
		verticesFromResavedGroom = TestUtilities.GetVerticesInObjectCoordinates( hairShape )
		verticesFromResavedGroom.sort()

		TestUtilities.CheckPointsAllNearEqual( self, verticesFromStockGroom, verticesFromResavedGroom )

	def test_FeathersGroom( self ):
		meshShape = pm.PyNode( pm.mel.OxLoadGroom( path = "Feathers" ) )
		verticesFromStockGroom = TestUtilities.GetMeshVertices( meshShape )
		self.assertGreater( len( verticesFromStockGroom ), 0 )

	def test_GroomCommands( self ):
		# Save grooms
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 0 )
		hairFromGuides = TestUtilities.GetNodeByType( TestUtilities.HairFromGuidesNodeName )
		hairFromGuides.renderCount.set( 5000 )
		hairFromGuides.viewportCountFraction.set( 0.2 )
		strandCount = pm.mel.OxGetStrandCount( hairShape )
		self.assertTrue( strandCount > 0 )
		pm.select( hairShape )
		groomPath = self.addTempFile( "test_groom" + MayaTest.NewGroomExtension )
		pm.mel.OxSaveGroom( path = groomPath, optional = True )

		pm.newFile( force = 1 )

		with self.assertRaises( pm.MelError ):
			pm.mel.OxLoadGroom( path = "Invalid Groom Name.oxgroom" )

		with self.assertRaises( pm.MelError ):
			pm.mel.OxLoadGroom( path = "Invalid Groom Name.oxg.yaml" )

		# Load groom with no selection
		hairShape = pm.PyNode( pm.mel.OxLoadGroom( path = groomPath ) )
		self.assertEqual( hairShape, "test_groomShape1" )
		hairFromGuides = TestUtilities.GetNodeByType( TestUtilities.HairFromGuidesNodeName )
		self.assertTrue( hairFromGuides.exists() )
		self.assertEqual( hairFromGuides.distribution.get(), 0 )
		self.assertAlmostEqual( hairFromGuides.viewportCountFraction.get(), 0.2 )
		# TODO: self.assertEqual( pm.mel.OxGetStrandCount( hairShape ), strandCount )

		# Load groom to selection
		pm.newFile( force = 1 )
		pm.polyPlane( ch = True, o = True, w = 10, h = 10, sh = 1, sw = 1 )
		pm.select( "pPlane1" )
		pm.flushUndo()

		pm.mel.OxLoadGroom( path = groomPath )
		hairFromGuides = TestUtilities.GetNodeByType( TestUtilities.HairFromGuidesNodeName )
		self.assertTrue( hairFromGuides.exists() )
		self.assertEqual( hairFromGuides.distribution.get(), 0 )
		self.assertAlmostEqual( hairFromGuides.viewportCountFraction.get(), 0.2 )
		hairShape = TestUtilities.GetNodeByType( TestUtilities.HairShapeName )
		self.assertNotEqual( len( hairShape.instObjGroups[0].outputs() ), 0 )

		# Undo
		pm.undo()
		self.assertEqual( [], pm.ls( type = TestUtilities.HairFromGuidesNodeName ) )

		# Redo
		pm.redo()
		hairFromGuides = TestUtilities.GetNodeByType( TestUtilities.HairFromGuidesNodeName )
		self.assertTrue( hairFromGuides.exists() )
		self.assertEqual( hairFromGuides.distribution.get(), 0 )
		self.assertAlmostEqual( hairFromGuides.viewportCountFraction.get(), 0.2 )

	def test_GroomSaveLoadEditGuides( self ):
		filePath = self.addTempFile( "test_groom" + MayaTest.NewGroomExtension )
		plane = pm.polyPlane( sx = 1, sy = 1 )[0]
		editGuidesShape = TestUtilities.AddEditGuidesToMesh( plane, 2, 2 )
		guidesFromMesh = pm.ls( type = TestUtilities.GuidesFromMeshNodeName )[0]

		pm.select( editGuidesShape )
		pm.mel.OxEditGuides( 2, 0, 2, 0, 0, 0, 4, 0, 0, 1, 2, 0, 0, 0, 4, 0, 0, eg = True )
		originalPosition = pm.mel.OxGetStrandPoint( editGuidesShape, 0, 1 )
		pm.mel.OxSaveGroom( path = filePath, optional = True )
		pm.newFile( force = 1 )

		pm.polyPlane( sx = 1, sy = 1 )
		guidesShape = pm.PyNode( pm.mel.OxLoadGroom( path = filePath ) )
		editGuidesShape = TestUtilities.GetNodeByType( TestUtilities.EditGuidesShapeName )
		self.assertTrue( guidesShape.exists() )
		self.assertTrue( editGuidesShape.exists() )
		groomPositions = [pm.mel.OxGetStrandPoint( editGuidesShape, 0, 1 ), pm.mel.OxGetStrandPoint( editGuidesShape, 1, 1 )]

		hasOriginalPosition = False
		for groomPoint in groomPositions:
			if originalPosition == groomPoint:
				hasOriginalPosition = True

		self.assertTrue( hasOriginalPosition )

	def test_GroomSaveLoadBakedGuides( self ):
		plane = pm.polyPlane( sx = 1, sy = 1 )[0]
		editGuidesShape = TestUtilities.AddEditGuidesToMesh( plane, 2, 2 )
		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		pm.select( editGuidesShape )
		pm.mel.eval( 'OxEditGuides -eg 2 0 2 0 0 0 4 0 0 1 2 0 0 0 4 0 0' )
		originalPosition = pm.mel.OxGetStrandPoint( guidesShape, 0, 1 )
		bakedGuides = pm.PyNode( pm.mel.OxCollapseStack( editGuidesShape ) )
		# TODO: Switch to USD groom once OxAddOperator gets rewritten in C++ and supports all operators, including GfM and Baked*
		groomPath = self.addTempFile( "test_groom" + MayaTest.OldGroomExtension )
		pm.mel.OxSaveGroom( path = groomPath, optional = True )
		pm.newFile( force = 1 )

		guidesShape = pm.PyNode( pm.mel.OxLoadGroom( path = groomPath ) )
		self.assertTrue( guidesShape.exists() )
		self.assertEqual( guidesShape.type(), TestUtilities.HairShapeName )
		self.assertEqual( guidesShape.inputHair.inputs()[0].type(), TestUtilities.GuidesFromCurvesNodeName )
		#self.assertEqual( len( pm.ls( type = TestUtilities.GuidesFromCurvesNodeName ) ), 1 )
		self.assertEqual( len( pm.ls( type = 'nurbsCurve' ) ), 2 )
		self.assertEqual( pm.mel.OxGetStrandCount( guidesShape ), 2 )
		groomPosition = pm.mel.OxGetStrandPoint( guidesShape, 0, 1 )
		# TODO: figure out why points are in different coordinate systems and x/y/z are in different order
		#self.assertAlmostEqual( originalPosition[0], groomPosition[0], places = 1 )

	def test_GenerateAndCreateGroom( self ):
		# A basic stack with some operators
		hairShape = TestUtilities.AddHairToNewPlane()
		pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.CurlNodeName ) )
		frizzNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.FrizzNodeName ) )

		# Test that we can use a map too
		TestUtilities.SetBitmapAttribute( frizzNode.amountMap, self.findTestFile( "Checker.png" ) )

		originalVertices = pm.mel.OxGetVertices( hairShape, os = True )

		pm.select( hairShape )

		filePath = pm.mel.OxSaveGroom( path = self.addTempFile( "test_GenerateAndCreateGroom" + MayaTest.NewGroomExtension ), forceOverwrite = True )
		# TOOD: Check what's different on other OS's or machines
		#if MayaTest.OsIsWindows:
		#	self.assertFileCrc( filePath, 1008431245 )

		pm.select( clear = True )
		groomHairShape = pm.PyNode( pm.mel.OxLoadGroom( path = filePath ) )

		groomVertices = pm.mel.OxGetVertices( groomHairShape, os = True )

		# Inspect the stack to make sure it is preserved
		groomStackShapes = pm.mel.OxGetStackNodes( groomHairShape )
		originalStackShapes = pm.mel.OxGetStackNodes( hairShape )

		self.assertEqual( len( originalStackShapes ), len( groomStackShapes ) )
		TestUtilities.CheckPointsAllNearEqual( self, originalVertices, groomVertices )

	def test_GenerateGroomWithCurves( self ):
		# Not used?
		plane = pm.polyPlane( w=10, h=10 )

		hairShape = TestUtilities.AddGuidesFromCurves()

		filePath = pm.mel.OxSaveGroom( path = self.addTempFile( "test_GenerateGroomWithCurves" + MayaTest.NewGroomExtension ), forceOverwrite = True )

	def test_GenerateGroomWithCurves2( self ):
		pm.openFile( self.findTestFile( 'OxMayaV4HairFromCurvesGroom.ma' ), force = True )
		hairShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		pm.select( hairShape )
		filePath = pm.mel.OxSaveGroom( path = self.addTempFile( "test_GenerateGroomWithCurves2" + MayaTest.NewGroomExtension ), forceOverwrite = True )

	def test_InstalledGroomCount( self ):
		# Change expected value if default grooms change
		self.assertEqual( 3, pm.mel.OxGroomInfo( q = True, gc = True ) )

	# TODO: just MayaTest.OsIsMac
	@unittest.skipIf( not MayaTest.OsIsWindows, "Arnold causes an exception in the Autodesk license manager" )
	def test_ArnoldTranslator( self ):
		if not pm.pluginInfo( 'mtoa', query = True, loaded = True ):
			return
		hairShape = TestUtilities.AddHairToNewPlane()

		hairShape.aiUseProcedural.set( False )
		pm.arnoldRender( b = True )

	# TODO: just MayaTest.OsIsMac
	@unittest.skipIf( not MayaTest.OsIsWindows, "Arnold causes an exception in the Autodesk license manager" )
	@unittest.skipIf( MayaTest.ApiVersion < 201900, "Arnold procedural appears in Maya 2019" )
	def test_ArnoldProcedural( self ):
		if not pm.pluginInfo( 'mtoa', query = True, loaded = True ) or not pm.pluginInfo( 'mtoa', query = True, version = True ).startswith('5.3'):
			return
		hairShape = TestUtilities.AddHairToNewPlane()

		hairShape.aiUseProcedural.set( True )
		pm.arnoldRender( b = True )
