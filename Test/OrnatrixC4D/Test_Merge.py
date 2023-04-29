#----------------
import os
import sys
import c4d
import unittest
sys.path.insert( 0, os.path.dirname( __file__ ) )
import Ornatrix.Ornatrix as Ox
import TestUtilities
#import importlib
#importlib.reload(TestUtilities)
sys.path.pop( 0 )
#----------------
from unittest import TestCase

#
# TestCase
class Test_Merge( TestCase ):
	sphere = None
	hair = None
	hair1 = None
	hair2 = None
	merge = None
	strandCount = 0
	strandCount1 = 0
	strandCount2 = 0

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		# Create ground Sphere
		self.sphere = c4d.BaseObject( c4d.Osphere )
		self.sphere[c4d.PRIM_SPHERE_RAD] = 50
		self.sphere.SetName("SphereGround")
		self.doc.InsertObject( self.sphere )
		# Create spheres to hond merged hair
		sphere1 = c4d.BaseObject( c4d.Osphere )
		sphere1[c4d.PRIM_SPHERE_RAD] = 50
		sphere1[c4d.ID_BASEOBJECT_XRAY] = True
		sphere1.SetName("Sphere1")
		self.doc.InsertObject( sphere1 )
		sphere2 = c4d.BaseObject( c4d.Osphere )
		sphere2[c4d.PRIM_SPHERE_RAD] = 50
		sphere2[c4d.ID_BASEOBJECT_XRAY] = True
		sphere2.SetName("Sphere2")
		self.doc.InsertObject( sphere2 )
		# Create Hair objects
		self.hair = TestUtilities.AddGuidesToMesh( self.sphere, rootGenerationMethod=Ox.res.gfm_RootGenerationMethodAttribute_VERTEX, length=10, name="Ground Hair" )
		self.hair1 = TestUtilities.AddGuidesToMesh( sphere1, rootGenerationMethod=Ox.res.gfm_RootGenerationMethodAttribute_FACE_CENTER, length=30, name="Merged Hair 1" )
		self.hair2 = TestUtilities.AddGuidesToMesh( sphere2, length=30, name="Merged Hair 2" )
		self.hair1[c4d.ID_BASEOBJECT_VISIBILITY_EDITOR] = 1
		self.hair2[c4d.ID_BASEOBJECT_VISIBILITY_EDITOR] = 1
		# Build
		Ox.ComputeStack( self.doc )
		# Remember strand counts
		self.strandCount = len( TestUtilities.GetStrandIds( self.hair ) )
		self.strandCount1 = len( TestUtilities.GetStrandIds( self.hair1 ) )
		self.strandCount2 = len( TestUtilities.GetStrandIds( self.hair2 ) )
		# Create Merge and add hair1 to it
		self.merge = c4d.BaseObject( Ox.res.ID_OX_MERGE_MOD )
		self.doc.InsertObject( self.merge, self.hair, self.hair.GetDownLast() )
		hairList = c4d.InExcludeData()
		hairList.InsertObject( self.hair1, 1 )
		self.merge[Ox.res.mrg_MergedStrandsAttribute] = hairList
		# Build
		Ox.ComputeStack( self.doc )

	def test_HairCount( self ):
		newStrandCount = len( TestUtilities.GetStrandIds( self.hair ) )
		self.assertEqual( newStrandCount, self.strandCount + self.strandCount1 )
		# Add hair2
		hairList = self.merge[Ox.res.mrg_MergedStrandsAttribute]
		hairList.InsertObject( self.hair2, 1 )
		self.merge[Ox.res.mrg_MergedStrandsAttribute] = hairList
		Ox.ComputeStack( self.doc )
		newStrandCount = len( TestUtilities.GetStrandIds( self.hair ) )
		self.assertEqual( newStrandCount, self.strandCount + self.strandCount1 + self.strandCount2 )

	@unittest.skip( "Merger keeps resetting merged hair position" )
	def test_HairGrounded( self ):
		self.sphere.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector(100,0,100) ) )
		Ox.ComputeStack( self.doc )
		# Roots must be offset
		vertices = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( self.hair )
		for strandId, points in iter(vertices.items()):
			self.assertGreater( points[0][0], 0, msg='All roots should have X>0, is ' + str( points[0] )  )
			self.assertGreater( points[0][2], 0, msg='All roots should have Z>0, is ' + str( points[0] )  )



#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_Merge )