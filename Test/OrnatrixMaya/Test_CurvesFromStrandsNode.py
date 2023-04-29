import MayaTest
import TestUtilities
import pymel.core as pm

class Test_CurvesFromStrandsNode( MayaTest.OxTestCase ):

	# Test creation of CurvesFromStrandsNode
	def test_Create( self ):

		plane = pm.polyPlane( w=10, h=10 )
		guidesShape = TestUtilities.AddGuidesToMesh( plane[0], 3, 1 )
		guideCount = pm.mel.OxGetStrandCount( guidesShape )
		hairTransform = guidesShape.firstParent2()
		baseName = guidesShape.name()
		self.assertEqual( guideCount, 3 )

		strandsToCurves = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.CurvesFromStrandsNodeName ) )

		self.assertEqual( guideCount, strandsToCurves.outputCurves.evaluateNumElements() )
		self.assertEqual( 0, len( pm.ls( type = TestUtilities.HairShapeName ) ) )

		curveShapes = [ pm.PyNode( baseName ), pm.PyNode( baseName + '_1' ), pm.PyNode( baseName + '_2' ) ]
		self.assertEqual( curveShapes, pm.ls( type = "nurbsCurve" ) )
		for c in curveShapes:
			self.assertEqual( curveShapes, pm.mel.OxGetStackShapeArray( c ) )
			self.assertEqual( hairTransform, c.firstParent2() )

		pm.mel.OxDeleteStrandOperator( strandsToCurves )
		self.assertEqual( 0, len( pm.ls( type = "nurbsCurve" ) ) )

		guidesShape = pm.ls( type = TestUtilities.HairShapeName )[0]
		strandsToCurves = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.CurvesFromStrandsNodeName ) )
		pm.mel.OxDeleteStrandOperator( guidesShape )
		self.assertEqual( 0, len( pm.ls( type = "nurbsCurve" ) ) )
		self.assertEqual( 0, len( pm.ls( type = TestUtilities.GuidesFromMeshNodeName ) ) )

	def test_OxSeparateSelectedCurves( self ):

		plane = pm.polyPlane( w=10, h=10 )
		guidesShape = TestUtilities.AddGuidesToMesh( plane[0], 3, 1 )
		hairTransform = pm.PyNode( guidesShape.firstParent2() )
		baseName = guidesShape.name()

		strandsToCurves = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.CurvesFromStrandsNodeName ) )
		curveShapes = [ pm.PyNode( baseName ), pm.PyNode( baseName + '_1' ), pm.PyNode( baseName + '_2' ) ]

		self.assertEqual( curveShapes, pm.mel.OxGetStackShapeArray( curveShapes[0] ) )
		for c in curveShapes:
			self.assertEqual( hairTransform, c.firstParent2() )

		pm.mel.source( "AECurvesFromStrandsNodeTemplate.mel" )
		hairTransform.rename( "Curves", ignoreShape = True ) # Rename to avoid name duplication below
		pm.select( curveShapes[1] )
		pm.mel.OxSeparateSelectedCurves( '' )
		self.assertEqual( hairTransform, pm.PyNode( baseName ).firstParent2() )
		curve1Transform = pm.PyNode( baseName + '_1' ).firstParent2()
		self.assertNotEqual( hairTransform, curve1Transform )
		self.assertEqual( hairTransform, pm.PyNode( baseName + '_2' ).firstParent2() )

		pm.select(clear = True)
		pm.mel.OxSeparateSelectedCurves( strandsToCurves )
		self.assertNotEqual( hairTransform, pm.PyNode( baseName ).firstParent2() )
		self.assertEqual( curve1Transform, pm.PyNode( baseName + '_1' ).firstParent2() )
		self.assertNotEqual( hairTransform, pm.PyNode( baseName + '_2' ).firstParent2() )

	def test_OxCopySelectedCurves( self ):

		plane = pm.polyPlane( w=10, h=10 )
		guidesShape = TestUtilities.AddGuidesToMesh( plane[0], 3, 1 )
		hairTransform = pm.PyNode( guidesShape.firstParent2() )
		baseName = guidesShape.name()

		strandsToCurves = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.CurvesFromStrandsNodeName ) )

		pm.mel.source( "AECurvesFromStrandsNodeTemplate.mel" )
		pm.select( baseName + '_1' )
		pm.mel.OxSeparateSelectedCurves( '' )

		pm.select(clear = True)
		copies = pm.mel.OxCopySelectedCurves( strandsToCurves )
		self.assertEqual( 3, len( copies ) )

		self.assertEqual( hairTransform, pm.PyNode( copies[0] ).firstParent2() )
		self.assertEqual( hairTransform, pm.PyNode( copies[1] ).firstParent2() )
		self.assertEqual( copies[0], hairTransform.name() + 'Copy' )
		self.assertEqual( copies[1], hairTransform.name() + '_1Copy' )
		self.assertEqual( copies[2], hairTransform.name() + '_2Copy' )
		self.assertEqual( pm.mel.OxGetStackBelow( hairTransform + '|HairShape1' ), strandsToCurves )

	def test_HairFromCurvesShapeMoveSelf( self ):
		curve = pm.circle( c=(0, 0, 0), nr=(0, 1, 0) )
		pm.select( curve )

		# Guides shape should be created from curves
		guidesShape = pm.PyNode( pm.mel.OxLoadGroom( path = "Hair From Curves" ) )
		guidesFromCurves = pm.ls( type = TestUtilities.GuidesFromCurvesNodeName )[0]
		guidesFromCurves.pointCount.set( 20 )

		# Convert back to curve
		strandsToCurves = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.CurvesFromStrandsNodeName ) )

		# cache the points
		pm.select( guidesShape )

		points = []
		for i in range(0, 20):
			points.append( pm.pointPosition( guidesShape + ".cv[" + str(i) + "]", world=True ) )

		# move the generated curve shape
		pm.move( 0.2, 0.5, 1.0 )

		# reevaluate
		pm.dgdirty( guidesShape, allPlugs=True )

		# check whether we generate the same curve positions in world space
		for i in range(0, 20):
			pt = pm.pointPosition( guidesShape + ".cv[" + str(i) + "]", world=True )
			self.assertNearEqual( points[i][0], pt[0], 0.00001 )
			self.assertNearEqual( points[i][1], pt[1], 0.00001 )
			self.assertNearEqual( points[i][2], pt[2], 0.00001 )

	def test_HairFromCurvesShapeMoveSource( self ):
		curve = pm.circle( c=(0, 0, 0), nr=(0, 1, 0) )
		pm.select( curve )

		# Guides shape should be created from curves
		guidesShape = pm.PyNode( pm.mel.OxLoadGroom( path = "Hair From Curves" ) )
		guidesFromCurves = pm.ls( type = TestUtilities.GuidesFromCurvesNodeName )[0]
		guidesFromCurves.pointCount.set( 20 )

		# Convert back to curve
		strandsToCurves = pm.PyNode( pm.mel.OxAddStrandOperator( guidesShape, TestUtilities.CurvesFromStrandsNodeName ) )

		# cache the points
		pm.select( guidesShape )

		points = []
		for i in range(0, 20):
			points.append( pm.pointPosition( guidesShape + ".cv[" + str(i) + "]", world=True ) )

		# move the source curve shape
		pm.select( curve )
		pm.move( 0.2, 0.5, 1.0 )

		# check whether we moved the generated shape with the same amount
		pm.select( guidesShape )
		for i in range(0, 20):
			pt = pm.pointPosition(guidesShape + ".cv[" + str(i) + "]", world=True);
			self.assertNearEqual( points[i][0] + 0.2, pt[0], 0.00001 )
			self.assertNearEqual( points[i][1] + 0.5, pt[1], 0.00001 )
			self.assertNearEqual( points[i][2] + 1.0, pt[2], 0.00001 )
