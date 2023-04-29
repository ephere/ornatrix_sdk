#----------------
import os
import sys
import c4d
import maxon
sys.path.insert( 0, os.path.dirname( __file__ ) )
import Ornatrix.Ornatrix as Ox
import TestUtilities
import importlib
#importlib.reload(TestUtilities)
#importlib.reload(Ox)
sys.path.pop( 0 )
#----------------
from unittest import TestCase, skip

#
# TestCase
class Test_Alembic( TestCase ):
	hair = None
	gfm = None
	hfg = None
	cube = None
	tempUrl = None

	#@classmethod
	#def setUpClass( self ):
	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		# Create cube
		self.cube = c4d.BaseObject( c4d.Ocube )
		self.doc.InsertObject( self.cube )
		# Create Ornatrix Hair stack
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		#self.hair[c4d.ho_DebugStackOnConsole] = True
		self.gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		self.gfm[Ox.res.INPUT_MESH] = self.cube
		self.gfm[c4d.gfm_GuidePointCountAttribute] = 3
		self.hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		self.hfg[c4d.hfg_RenderCountAttribute] = 540
		self.hfg[c4d.hfg_ViewportCountFractionAttribute] = 1.0
		# Insert
		self.doc.InsertObject( self.hair )
		self.doc.InsertObject( self.hfg, self.hair )
		self.doc.InsertObject( self.gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )

	def tearDown( self ):
		if( self.tempUrl ):
			self.tempUrl.IoDelete( True, True )

	def ExportHair( self, worldCoordinates=None ):
		self.doc.SetActiveObject( self.hair )
		self.tempUrl = maxon.Application.GetTempUrl( maxon.Application.GetUrl( maxon.APPLICATION_URLTYPE.TEMP_DIR ) )
		self.tempUrl.SetSuffix("abc")
		if worldCoordinates is not None:
			saver = c4d.plugins.FindPlugin(Ox.res.ID_OX_ALEMBIC_SAVER, c4d.PLUGINTYPE_SCENESAVER)
			self.assertNotEqual( saver, None, 'Unable to find ID_OX_ALEMBIC_SAVER' )
			# Send MSG_RETRIEVEPRIVATEDATA to retriexe exporter data
			op = {}
			self.assertTrue( saver.Message(c4d.MSG_RETRIEVEPRIVATEDATA, op), 'Unable to message ID_OX_ALEMBIC_SAVER')
			bc = op['imexporter']
			previousWorldCoorcinates = bc[Ox.res.abce_UseWorldCoordinates]
			bc[Ox.res.abce_UseWorldCoordinates] = worldCoordinates
		c4d.documents.SaveDocument( self.doc, self.tempUrl, c4d.SAVEDOCUMENTFLAGS_NONE, Ox.res.ID_OX_ALEMBIC_SAVER )
		if worldCoordinates is not None:
			bc[Ox.res.abce_UseWorldCoordinates] = previousWorldCoorcinates

	def ImportHair( self ):
		c4d.documents.MergeDocument( self.doc, self.tempUrl, c4d.SCENEFILTER_OBJECTS|c4d.SCENEFILTER_MERGESCENE, None )
		importedHairObject = self.doc.SearchObject( self.tempUrl.GetName() )
		self.assertIsNotNone( importedHairObject, msg='Imported HairObject not found' )
		bakedHair = importedHairObject.GetDown()
		self.assertIsNotNone( bakedHair, msg='Imported Hair does not Have children' )
		self.assertEqual( bakedHair.GetType(), Ox.res.ID_OX_BAKED_HAIR_MOD, msg='Imported BakedHair not found' )
		self.assertEqual( bakedHair[c4d.bkh_SourceFilePath1Attribute], self.tempUrl.GetSystemPath(), msg='Imported BakedHair ['+bakedHair[c4d.bkh_SourceFilePath1Attribute]+'] does not point to exported Alembic file ['+self.tempUrl.GetSystemPath()+']' )
		bakedHair[c4d.bkh_DisplayFractionAttribute] = 1.0
		Ox.ComputeStack( self.doc )
		return [ importedHairObject, bakedHair ]

	def test_AlembicExport( self ):
		# Export and check file existence
		self.ExportHair()
		self.assertTrue( os.path.exists( self.tempUrl.GetSystemPath() ), msg='Exported file not found' )

	def test_AlembicImport( self ):
		strandCount = Ox.GetStrandCount( self.hair )
		vertexCount = Ox.GetVertexCount( self.hair )
		vertices = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.hair )
		# Export, Import and compare
		self.ExportHair()
		[ importedHairObject, bakedHair ] = self.ImportHair()
		strandCountImported = Ox.GetStrandCount( importedHairObject )
		self.assertEqual( strandCountImported, strandCount, msg='Imported strand count is wrong' )
		vertexCountImported = Ox.GetVertexCount( importedHairObject )
		self.assertEqual( vertexCountImported, vertexCount, msg='Imported vertex count is wrong' )
		verticesImported = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( importedHairObject )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, vertices, verticesImported )

	def test_AlembicImportAxis( self ):
		# Convert cube to polygon and use faces on positive axis to generate hair
		self.cube[c4d.PRIM_CUBE_SUBX] = 3
		self.cube[c4d.PRIM_CUBE_SUBY] = 3
		self.cube[c4d.PRIM_CUBE_SUBZ] = 3
		poly = TestUtilities.CachePrimitiveIntoNewObject( self.cube, self.doc )
		self.doc.SetActiveObject( poly )
		c4d.CallCommand(12187) # Polygon selection mode
		poly.GetPolygonS().Select(17)
		poly.GetPolygonS().Select(24)
		poly.GetPolygonS().Select(44)
		c4d.CallCommand(12552) # Set Selection tag
		selectionTag = poly.GetTag(c4d.Tpolygonselection)
		# Generate hair on positive faces only
		self.gfm[Ox.res.INPUT_MESH] = poly
		self.hfg[c4d.hfg_FaceIncludeSelectionTagAttribute] = selectionTag
		self.hfg[c4d.hfg_UseFaceIncludeAttribute] = True
		Ox.ComputeStack( self.doc )
		# Export, Import and compare
		self.ExportHair()
		[ importedHairObject, bakedHair ] = self.ImportHair()
		vertices = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.hair )
		verticesImported = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( importedHairObject )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, vertices, verticesImported )

	def test_AlembicImportWorldCoordinates( self ):
		# Move distribution mesh and export in object coorfinates
		self.cube.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( 100, 100, 100 ) ) )
		Ox.ComputeStack( self.doc )
		# Export, Import and compare
		self.ExportHair( worldCoordinates=True )
		[ importedHairObject, bakedHair ] = self.ImportHair()
		vertices = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( self.hair )
		verticesImported = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( importedHairObject )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, vertices, verticesImported )

	def test_zAlembicImportObjectCoordinates( self ):
		# Move distribution mesh and export in object coorfinates
		self.cube.SetMg( TestUtilities.MakeTransformMatrix( position=c4d.Vector( 100, 100, 100 ) ) )
		Ox.ComputeStack( self.doc )
		# Export, Import and compare
		self.ExportHair( worldCoordinates=False )
		[ importedHairObject, bakedHair ] = self.ImportHair()
		vertices = TestUtilities.GetVerticesInObjectCoordinatesByStrandId( self.hair )
		verticesImported = TestUtilities.GetVerticesInWorldCoordinatesByStrandId( importedHairObject )
		TestUtilities.CheckPointsAllNearEqualByStrandId( self, vertices, verticesImported )



#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_Alembic )