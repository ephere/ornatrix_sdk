#----------------
import os
import sys
import c4d
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
class Test_GenerateStrandData( TestCase ):
	hair = None
	gfm = None
	gsd = None
	length = None


	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		# Create Plane
		plane = c4d.BaseObject( c4d.Oplane )
		self.doc.InsertObject( plane )
		# Create Ornatrix Hair object
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		# Create GFM modifier inside Hair Object
		self.gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		self.gfm[Ox.res.INPUT_MESH] = plane
		self.gfm[c4d.gfm_GuidePointCountAttribute] = 2
		self.gfm[c4d.gfm_GuideLengthRandomnessAttribute] = 0
		self.gsd = c4d.BaseObject( Ox.res.ID_OX_GENERATE_STRAND_DATA_MOD )
		self.gsd[c4d.gsd_MinimumTargetValueAttribute] = 2
		self.gsd[c4d.gsd_MaximumTargetValueAttribute] = 2
		self.length = c4d.BaseObject( Ox.res.ID_OX_LENGTH_MOD )
		self.doc.InsertObject( self.hair )
		self.doc.InsertObject( self.length, self.hair )
		self.doc.InsertObject( self.gsd, self.hair )
		self.doc.InsertObject( self.gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )
		# Changing channel before computing raises AttributeError('parameter set failed')
		self.length[c4d.len_ValueChannelAttribute] = 3
		Ox.ComputeStack( self.doc )

	def test_GenerateStrandData_Constant( self ):
		lengthsBefore = TestUtilities.GetStrandLengthsByStrandId( self.gfm )
		lengthsAfter = TestUtilities.GetStrandLengthsByStrandId( self.length )
		TestUtilities.CheckValuesNotAllNearEqualByStrandId( self, lengthsBefore, lengthsAfter )

	def test_GenerateStrandData_Expression( self ):
		self.gsd[c4d.gsd_GenerationMethodAttribute] = Ox.res.gsd_GenerationMethod_SeExpr
		self.gsd[c4d.gsd_MinimumTargetValueAttribute] = 1
		self.gsd[c4d.gsd_SeExprStringAttribute] = '($index%2)*1'
		Ox.ComputeStack( self.doc )
		# Compare vertices, tips of 50% should be higher, or 25% of total vertices
		lengthsBefore = TestUtilities.GetStrandLengthsByStrandId( self.gfm )
		lengthsAfter = TestUtilities.GetStrandLengthsByStrandId( self.length )
		nearEqualPercentage = TestUtilities.GetNearEqualValuesPercentage( self, lengthsBefore, lengthsAfter )
		TestUtilities.CheckPercentageNearEqual( self, nearEqualPercentage, 0.5, 0.015, 'Equal strands' )

	def test_GenerateStrandData_TextureMap( self ):
		self.gsd[c4d.gsd_GenerationMethodAttribute] = Ox.res.gsd_GenerationMethod_TextureMap
		self.gsd[c4d.gsd_MinimumTargetValueAttribute] = 1
		TestUtilities.MakeAxisAlignedBlackAndWhiteShaderX( self.gsd, c4d.gsd_TextureMapAttribute )
		Ox.ComputeStack( self.doc )
		# Compare vertices, tips of 50% should be higher, or 25% of total vertices
		lengthsBefore = TestUtilities.GetStrandLengthsByStrandId( self.gfm )
		lengthsAfter = TestUtilities.GetStrandLengthsByStrandId( self.length )
		nearEqualPercentage = TestUtilities.GetNearEqualValuesPercentage( self, lengthsBefore, lengthsAfter )
		TestUtilities.CheckPercentageNearEqual( self, nearEqualPercentage, 0.5, 0.015, 'Equal strands' )


#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_GenerateStrandData )
