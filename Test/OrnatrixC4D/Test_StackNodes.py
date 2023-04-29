#----------------
import os
import sys
import c4d
sys.path.insert( 0, os.path.dirname( __file__ ) )
import Ornatrix.Ornatrix as Ox
#reload(Ox)
import TestUtilities
sys.path.pop( 0 )
#----------------
from unittest import TestCase

#
# TestCase
class Test_StackNodes( TestCase ):
	doc = None
	hair = None
	plane = None

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		# Create Plane
		self.plane = c4d.BaseObject( c4d.Oplane )
		self.doc.InsertObject( self.plane )
		# Create Ornatrix Hair object
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		self.doc.InsertObject( self.hair )
		Ox.ComputeStack( self.doc )

	def test_StackNodes_GuidesFromMesh( self ):
		gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		gfm[Ox.res.INPUT_MESH] = self.plane
		self.doc.InsertObject( gfm, self.hair )
		Ox.ComputeStack( self.doc )
		# test
		guideNode = self.hair.GetParameter( Ox.res.ho_CurrentGuidesObject, c4d.DESCFLAGS_GET_NONE )
		hairNode = self.hair.GetParameter( Ox.res.ho_CurrentHairObject, c4d.DESCFLAGS_GET_NONE )
		outputNode = self.hair.GetParameter( Ox.res.ho_CurrentOutputStrandsObject, c4d.DESCFLAGS_GET_NONE )
		self.assertEqual( guideNode, gfm )
		self.assertEqual( hairNode, None )
		self.assertEqual( outputNode, gfm )
		# Disable GFM
		gfm.SetDeformMode( False )
		Ox.ComputeStack( self.doc )
		guideNode = self.hair.GetParameter( Ox.res.ho_CurrentGuidesObject, c4d.DESCFLAGS_GET_NONE )
		hairNode = self.hair.GetParameter( Ox.res.ho_CurrentHairObject, c4d.DESCFLAGS_GET_NONE )
		outputNode = self.hair.GetParameter( Ox.res.ho_CurrentOutputStrandsObject, c4d.DESCFLAGS_GET_NONE )
		self.assertEqual( guideNode, None )
		self.assertEqual( hairNode, None )
		self.assertEqual( outputNode, None )

	def test_StackNodes_HairFromGuides( self ):
		gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		gfm[Ox.res.INPUT_MESH] = self.plane
		hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		self.doc.InsertObject( hfg, self.hair )
		self.doc.InsertObject( gfm, self.hair )
		Ox.ComputeStack( self.doc )
		# test
		guideNode = self.hair.GetParameter( Ox.res.ho_CurrentGuidesObject, c4d.DESCFLAGS_GET_NONE )
		hairNode = self.hair.GetParameter( Ox.res.ho_CurrentHairObject, c4d.DESCFLAGS_GET_NONE )
		outputNode = self.hair.GetParameter( Ox.res.ho_CurrentOutputStrandsObject, c4d.DESCFLAGS_GET_NONE )
		self.assertEqual( guideNode, gfm )
		self.assertEqual( hairNode, hfg )
		self.assertEqual( outputNode, hfg )
		# Disable GFM only
		gfm.SetDeformMode( False )
		Ox.ComputeStack( self.doc )
		guideNode = self.hair.GetParameter( Ox.res.ho_CurrentGuidesObject, c4d.DESCFLAGS_GET_NONE )
		hairNode = self.hair.GetParameter( Ox.res.ho_CurrentHairObject, c4d.DESCFLAGS_GET_NONE )
		outputNode = self.hair.GetParameter( Ox.res.ho_CurrentOutputStrandsObject, c4d.DESCFLAGS_GET_NONE )
		self.assertEqual( guideNode, None )
		self.assertEqual( hairNode, None )
		self.assertEqual( outputNode, None )
		# Disable HFG only
		gfm.SetDeformMode( True )
		hfg.SetDeformMode( False )
		Ox.ComputeStack( self.doc )
		guideNode = self.hair.GetParameter( Ox.res.ho_CurrentGuidesObject, c4d.DESCFLAGS_GET_NONE )
		hairNode = self.hair.GetParameter( Ox.res.ho_CurrentHairObject, c4d.DESCFLAGS_GET_NONE )
		outputNode = self.hair.GetParameter( Ox.res.ho_CurrentOutputStrandsObject, c4d.DESCFLAGS_GET_NONE )
		self.assertEqual( guideNode, gfm )
		self.assertEqual( hairNode, None )
		self.assertEqual( outputNode, gfm )

	def test_StackNodes_Braids( self ):
		braids = c4d.BaseObject( Ox.res.ID_OX_BRAID_GUIDES_MOD )
		hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		self.doc.InsertObject( hfg, self.hair )
		self.doc.InsertObject( braids, self.hair )
		Ox.ComputeStack( self.doc )
		# test
		guideNode = self.hair.GetParameter( Ox.res.ho_CurrentGuidesObject, c4d.DESCFLAGS_GET_NONE )
		hairNode = self.hair.GetParameter( Ox.res.ho_CurrentHairObject, c4d.DESCFLAGS_GET_NONE )
		outputNode = self.hair.GetParameter( Ox.res.ho_CurrentOutputStrandsObject, c4d.DESCFLAGS_GET_NONE )
		self.assertEqual( guideNode, braids )
		self.assertEqual( hairNode, hfg )
		self.assertEqual( outputNode, hfg )

	def test_StackNodes_GuidesFromCurves( self ):
		arc = c4d.BaseObject( c4d.Osplinearc )
		self.doc.InsertObject( arc )
		curves = c4d.InExcludeData()
		curves.InsertObject( arc, 1 )
		gfc = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_CURVES_MOD )
		gfc[Ox.res.gfc_InputCurves] = curves
		hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		self.doc.InsertObject( hfg, self.hair )
		self.doc.InsertObject( gfc, self.hair )
		Ox.ComputeStack( self.doc )
		# test
		guideNode = self.hair.GetParameter( Ox.res.ho_CurrentGuidesObject, c4d.DESCFLAGS_GET_NONE )
		hairNode = self.hair.GetParameter( Ox.res.ho_CurrentHairObject, c4d.DESCFLAGS_GET_NONE )
		outputNode = self.hair.GetParameter( Ox.res.ho_CurrentOutputStrandsObject, c4d.DESCFLAGS_GET_NONE )
		self.assertEqual( guideNode, gfc )
		self.assertEqual( hairNode, hfg )
		self.assertEqual( outputNode, hfg )

	def test_StackNodes_BakedGuides( self ):
		gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		gfm[Ox.res.INPUT_MESH] = self.plane
		hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		self.doc.InsertObject( hfg, self.hair )
		self.doc.InsertObject( gfm, self.hair )
		Ox.ComputeStack( self.doc )
		# bake
		TestUtilities.SendDescriptionCommand( gfm, Ox.res.bake_Colapse_Button )
		Ox.ComputeStack( self.doc )
		bg = self.doc.SearchObject( "Baked Guides" )
		self.assertNotEqual( bg, None )
		# test
		guideNode = self.hair.GetParameter( Ox.res.ho_CurrentGuidesObject, c4d.DESCFLAGS_GET_NONE )
		hairNode = self.hair.GetParameter( Ox.res.ho_CurrentHairObject, c4d.DESCFLAGS_GET_NONE )
		outputNode = self.hair.GetParameter( Ox.res.ho_CurrentOutputStrandsObject, c4d.DESCFLAGS_GET_NONE )
		self.assertEqual( guideNode, bg )
		self.assertEqual( hairNode, hfg )
		self.assertEqual( outputNode, hfg )

	def test_StackNodes_BakedHair( self ):
		gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		gfm[Ox.res.INPUT_MESH] = self.plane
		hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		self.doc.InsertObject( hfg, self.hair )
		self.doc.InsertObject( gfm, self.hair )
		Ox.ComputeStack( self.doc )
		# bake
		TestUtilities.SendDescriptionCommand( hfg, Ox.res.bake_Colapse_Button )
		Ox.ComputeStack( self.doc )
		bh = self.doc.SearchObject( "Baked Hair" )
		self.assertNotEqual( bh, None )
		# test
		guideNode = self.hair.GetParameter( Ox.res.ho_CurrentGuidesObject, c4d.DESCFLAGS_GET_NONE )
		hairNode = self.hair.GetParameter( Ox.res.ho_CurrentHairObject, c4d.DESCFLAGS_GET_NONE )
		outputNode = self.hair.GetParameter( Ox.res.ho_CurrentOutputStrandsObject, c4d.DESCFLAGS_GET_NONE )
		# cant test guides because at this point GFM still exists
		#self.assertEqual( guideNode, None )
		self.assertEqual( hairNode, bh )
		self.assertEqual( outputNode, bh )

	def test_StackNodes_BakedHair( self ):
		gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		gfm[Ox.res.INPUT_MESH] = self.plane
		hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		gfh = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_HAIR_MOD )
		self.doc.InsertObject( gfh, self.hair )
		self.doc.InsertObject( hfg, self.hair )
		self.doc.InsertObject( gfm, self.hair )
		Ox.ComputeStack( self.doc )
		# test
		guideNode = self.hair.GetParameter( Ox.res.ho_CurrentGuidesObject, c4d.DESCFLAGS_GET_NONE )
		hairNode = self.hair.GetParameter( Ox.res.ho_CurrentHairObject, c4d.DESCFLAGS_GET_NONE )
		outputNode = self.hair.GetParameter( Ox.res.ho_CurrentOutputStrandsObject, c4d.DESCFLAGS_GET_NONE )
		self.assertEqual( guideNode, gfh )
		self.assertEqual( hairNode, None )
		self.assertEqual( outputNode, gfh )

	def test_StackNodes_HairFromMeshStrips( self ):
		strips = c4d.InExcludeData()
		strips.InsertObject( self.plane, 1 )
		hfms = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_MESH_STRIPS_MOD )
		hfms[c4d.hfms_InputMeshStrips] = strips
		self.doc.InsertObject( hfms, self.hair )
		Ox.ComputeStack( self.doc )
		# test
		guideNode = self.hair.GetParameter( Ox.res.ho_CurrentGuidesObject, c4d.DESCFLAGS_GET_NONE )
		hairNode = self.hair.GetParameter( Ox.res.ho_CurrentHairObject, c4d.DESCFLAGS_GET_NONE )
		outputNode = self.hair.GetParameter( Ox.res.ho_CurrentOutputStrandsObject, c4d.DESCFLAGS_GET_NONE )
		self.assertEqual( guideNode, None )
		self.assertEqual( hairNode, hfms )
		self.assertEqual( outputNode, hfms )




#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_StackNodes )
