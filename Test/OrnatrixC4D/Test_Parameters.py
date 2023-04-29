#----------------
import os
import os.path
import sys
import urllib
import c4d
sys.path.insert( 0, os.path.dirname( __file__ ) )
import Ornatrix.Ornatrix as Ox
import TestUtilities
#import importlib
#importlib.reload(TestUtilities)
sys.path.pop( 0 )
#importlib.reload(Ox)
#----------------
from unittest import TestCase
from unittest import skipIf

#
# TestCase
class Test_Parameters( TestCase ):
	testObjectA = None

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		# Create Sphere
		self.testObjectA = c4d.BaseObject( Ox.res.ID_OX_TEST_OBJECT )
		self.testObjectA.SetName("TestObjectA")
		self.doc.InsertObject( self.testObjectA )
		self.doc.SetActiveObject( self.testObjectA )

	def test_Initialization( self ):
		# default values from TextParameters.xml
		self.assertAlmostEqual( 1, self.testObjectA[Ox.res.test_ParameterIntExample] )
		self.assertAlmostEqual( 0.1, self.testObjectA[Ox.res.test_ParameterFractionExample] )
		self.assertAlmostEqual( True, self.testObjectA[Ox.res.test_ParameterBoolExample] )
		self.assertAlmostEqual( 1, self.testObjectA[Ox.res.test_ParameterExternalEnumParameter] )
		TestUtilities.CheckC4dVectorsNearEqual( self, c4d.Vector(0.3,0.3,0), self.testObjectA[Ox.res.test_ParameterVectorDistanceExample] )
		TestUtilities.CheckC4dVectorsNearEqual( self, c4d.Vector(0,0,1), self.testObjectA[Ox.res.test_ParameterVectorDirectionExample] )
		# open spline
		splineData = self.testObjectA[Ox.res.test_ParameterRampExample]
		self.assertEqual( 2, splineData.GetKnotCount() )
		TestUtilities.CheckC4dVectorsNearEqual(self, c4d.Vector(0,0,0) , splineData.GetKnots()[0]['vPos'] )
		TestUtilities.CheckC4dVectorsNearEqual(self, c4d.Vector(1,1,0) , splineData.GetKnots()[1]['vPos'] )

	@skipIf( not TestUtilities.IsFeatureEnabled( 'OxC4D_UsdGroom' ), 'UsdGroom feature not enabled' )
	def test_Serialize( self ):
		cylinder = c4d.BaseObject( c4d.Ocylinder )
		sphere = c4d.BaseObject( c4d.Osphere )
		plane = c4d.BaseObject( c4d.Oplane )
		self.doc.InsertObject( cylinder )
		self.doc.InsertObject( sphere )
		self.doc.InsertObject( plane )
		cylinderPolygon = TestUtilities.CachePrimitiveIntoNewObject( cylinder, self.doc )
		spherePolygon = TestUtilities.CachePrimitiveIntoNewObject( sphere, self.doc )
		planePolygon = TestUtilities.CachePrimitiveIntoNewObject( plane, self.doc )
		cylinder.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( 100, 0, 100 ) ) )
		sphere.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( -100, 0, -100 ) ) )
		# Set parameters
		self.testObjectA[Ox.res.test_ParameterIntExample] = 22
		self.testObjectA[Ox.res.test_ParameterFractionExample] = 0.22
		self.testObjectA[Ox.res.test_ParameterBoolExample] = True
		self.testObjectA[Ox.res.test_ParameterStringExample] = "Some Text"
		self.testObjectA[Ox.res.test_ParameterExternalEnumParameter] = 2
		self.testObjectA[Ox.res.test_ParameterVectorDistanceExample] = c4d.Vector(0.11,0.22,0.33)
		rampPoints = [c4d.Vector(0.1,0,0), c4d.Vector(0.5,0.5,0), c4d.Vector(0.9,1,0)]
		self.testObjectA[Ox.res.test_ParameterRampExample] = TestUtilities.MakeSplineData(rampPoints)
		self.testObjectA[Ox.res.test_ParameterMeshExample] = cylinder
		meshList = self.testObjectA[Ox.res.test_ParameterArrayMeshesExample]
		meshList.InsertObject(plane,0)
		meshList.InsertObject(sphere,0)
		meshList.InsertObject(cylinder,0)
		self.testObjectA[Ox.res.test_ParameterArrayMeshesExample] = meshList
		# Set arrays
		Ox.SetIntegerArrayData( self.testObjectA, Ox.res.test_ParameterArrayIntsExample, [10,20,30,40,50] )
		Ox.SetBooleanArrayData( self.testObjectA, Ox.res.test_ParameterArrayBoolsExample, [True,False,True] )
		Ox.SetVectorArrayData( self.testObjectA, Ox.res.test_ParameterArrayVectorsExample, [c4d.Vector(1,2,3),c4d.Vector(1.1,2.2,3.3)] )
		Ox.SetIntegerArrayData( self.testObjectA, Ox.res.test_ParameterArrayEnumsExample, [0,1,2] )
		Ox.SetStringArrayData( self.testObjectA, Ox.res.test_ParameterArrayStringsExample, ["123","Abc","Xyz"] )
		Ox.SetSplineArrayData( self.testObjectA, Ox.res.test_ParameterMagnitudeSinksRamps, [TestUtilities.MakeSplineData(rampPoints)] )
		# Serialize
		groomFile = TestUtilities.GetTempFilePath( "oxg.yaml" )
		#print( "serialize: [" + groomFile + "] exist ["+str(os.path.exists(groomFile))+"]" )
		groomDir = os.path.dirname(groomFile)
		os.access('my_folder', os.W_OK)
		#print( "serialize dir: [" + groomDir + "] exist ["+str(os.path.exists(groomDir))+"] R ["+str(os.access(groomDir,os.R_OK))+"] W ["+str(os.access(groomDir,os.W_OK))+"] X ["+str(os.access(groomDir,os.X_OK))+"]" )
		self.testObjectA[Ox.res.test_SerializeFilePath] = groomFile
		TestUtilities.SendDescriptionCommand( self.testObjectA, Ox.res.test_Serialize_Button )
		c4d.EventAdd()
		self.assertTrue( os.path.exists( groomFile ), msg='Not serialized to:'+groomFile )
		# Deserialize
		testObjectB = c4d.BaseObject( Ox.res.ID_OX_TEST_OBJECT )
		testObjectB.SetName("TestObjectB")
		testObjectB[Ox.res.test_SerializeFilePath] = groomFile
		self.doc.InsertObject( testObjectB )
		TestUtilities.SendDescriptionCommand( testObjectB, Ox.res.test_Deserialize_Button )
		c4d.EventAdd()
		# Compare
		self.assertAlmostEqual( self.testObjectA[Ox.res.test_ParameterIntExample], testObjectB[Ox.res.test_ParameterIntExample] )
		self.assertAlmostEqual( self.testObjectA[Ox.res.test_ParameterFractionExample], testObjectB[Ox.res.test_ParameterFractionExample] )
		self.assertAlmostEqual( self.testObjectA[Ox.res.test_ParameterBoolExample], testObjectB[Ox.res.test_ParameterBoolExample] )
		self.assertAlmostEqual( self.testObjectA[Ox.res.test_ParameterStringExample], testObjectB[Ox.res.test_ParameterStringExample] )
		self.assertAlmostEqual( self.testObjectA[Ox.res.test_ParameterExternalEnumParameter], testObjectB[Ox.res.test_ParameterExternalEnumParameter] )
		TestUtilities.CheckC4dVectorsNearEqual( self, self.testObjectA[Ox.res.test_ParameterVectorDistanceExample], testObjectB[Ox.res.test_ParameterVectorDistanceExample] )
		TestUtilities.CheckC4dSplineParametersAreEqual( self, self.testObjectA, testObjectB, Ox.res.test_ParameterRampExample )
		TestUtilities.CheckArrayParametersAreEqual( self, self.testObjectA, testObjectB, Ox.res.test_ParameterArrayIntsExample, Ox.res.ID_OX_INTEGER_ARRAY, False, "test_ParameterArrayIntsExample" )
		TestUtilities.CheckArrayParametersAreEqual( self, self.testObjectA, testObjectB, Ox.res.test_ParameterArrayBoolsExample, Ox.res.ID_OX_BOOLEAN_ARRAY, False, "test_ParameterArrayBoolsExample" )
		TestUtilities.CheckArrayParametersAreEqual( self, self.testObjectA, testObjectB, Ox.res.test_ParameterArrayVectorsExample, Ox.res.ID_OX_VECTOR_ARRAY, False, "test_ParameterArrayVectorsExample" )
		TestUtilities.CheckArrayParametersAreEqual( self, self.testObjectA, testObjectB, Ox.res.test_ParameterArrayEnumsExample, Ox.res.ID_OX_INTEGER_ARRAY, False, "test_ParameterArrayEnumsExample" )
		TestUtilities.CheckArrayParametersAreEqual( self, self.testObjectA, testObjectB, Ox.res.test_ParameterArrayStringsExample, Ox.res.ID_OX_STRING_ARRAY, False, "test_ParameterArrayStringsExample" )
		TestUtilities.CheckArrayParametersAreEqual( self, self.testObjectA, testObjectB, Ox.res.test_ParameterMagnitudeSinksRamps, Ox.res.ID_OX_SPLINE_ARRAY, False, "test_ParameterMagnitudeSinksRamps" )
		TestUtilities.CheckPolygonsAreEqual( self, cylinderPolygon, testObjectB[Ox.res.test_ParameterMeshExample], "test_ParameterMeshExample" )
		TestUtilities.CheckC4dMatrixNearEqual( self, cylinder.GetMg(), testObjectB[Ox.res.test_ParameterMeshExample].GetMg(), "test_ParameterMeshExample" )
		meshList = testObjectB[Ox.res.test_ParameterArrayMeshesExample]
		TestUtilities.CheckPolygonsAreEqual( self, planePolygon, meshList.ObjectFromIndex( self.doc, 0 ), "test_ParameterArrayMeshesExample[2]" )
		TestUtilities.CheckPolygonsAreEqual( self, spherePolygon, meshList.ObjectFromIndex( self.doc, 1 ), "test_ParameterArrayMeshesExample[1]" )
		TestUtilities.CheckPolygonsAreEqual( self, cylinderPolygon, meshList.ObjectFromIndex( self.doc, 2 ), "test_ParameterArrayMeshesExample[0]" )
		TestUtilities.CheckC4dMatrixNearEqual( self, plane.GetMg(), meshList.ObjectFromIndex( self.doc, 0 ).GetMg(), "test_ParameterArrayMeshesExample[0]" )
		TestUtilities.CheckC4dMatrixNearEqual( self, sphere.GetMg(), meshList.ObjectFromIndex( self.doc, 1 ).GetMg(), "test_ParameterArrayMeshesExample[0]" )
		TestUtilities.CheckC4dMatrixNearEqual( self, cylinder.GetMg(), meshList.ObjectFromIndex( self.doc, 2 ).GetMg(), "test_ParameterArrayMeshesExample[0]" )
		# Cleanup
		os.remove( groomFile )


#----------------------------------
# mains
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_Parameters )