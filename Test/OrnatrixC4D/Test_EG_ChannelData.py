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
class Test_EG_ChannelData( TestCase ):
	baseLength = 100.0
	hair = None
	gfm = None
	eg = None
	guidesLength = None
	hairLength = None
	strandIdsAffectedByLength = []
	perStrandChannel = -1
	perVertexChannel = -1

	def setUp( self ):
		self.doc = c4d.documents.GetActiveDocument()
		# Delete all
		Ox.DeleteAll( self.doc )
		# Create Sphere
		plane = c4d.BaseObject( c4d.Oplane )
		self.doc.InsertObject( plane )
		# Create Ornatrix Hair object
		self.hair = c4d.BaseObject( Ox.res.ID_OX_HAIR_OBJECT )
		self.doc.InsertObject( self.hair )
		# Create Ornatrix Modifiers
		self.gfm = c4d.BaseObject( Ox.res.ID_OX_GUIDES_FROM_MESH_MOD )
		self.gfm[Ox.res.INPUT_MESH] = plane
		self.gfm[Ox.res.gfm_RootGenerationMethodAttribute] = Ox.res.gfm_RootGenerationMethodAttribute_UNIFORM
		self.gfm[Ox.res.gfm_GuideCountAttribute] = 400
		self.gfm[Ox.res.gfm_GuideLengthAttribute] = self.baseLength
		self.gfm[Ox.res.gfm_GuideLengthRandomnessAttribute] = 0.0
		self.eg = c4d.BaseObject( Ox.res.ID_OX_EDIT_GUIDES_MOD )
		self.eg.SetName( "EG" )
		self.hfg = c4d.BaseObject( Ox.res.ID_OX_HAIR_FROM_GUIDES_MOD )
		self.hfg[Ox.res.hfg_GeneratePerStrandDataAttribute] = True
		self.hfg[Ox.res.hfg_GeneratePerVertexDataAttribute] = True
		self.hfg[Ox.res.hfg_GenerateRotationsAttribute] = True
		self.guidesLength = c4d.BaseObject( Ox.res.ID_OX_LENGTH_MOD )
		self.guidesLength[Ox.res.len_ValueAttribute] = 2.0
		self.guidesLength[Ox.res.len_MinimumValueAttribute] = 1.0
		self.hairLength = c4d.BaseObject( Ox.res.ID_OX_LENGTH_MOD )
		self.hairLength[Ox.res.len_ValueAttribute] = 2.0
		self.hairLength[Ox.res.len_MinimumValueAttribute] = 1.0
		self.doc.InsertObject( self.hairLength, self.hair )
		self.doc.InsertObject( self.hfg, self.hair )
		self.doc.InsertObject( self.guidesLength, self.hair )
		self.doc.InsertObject( self.eg, self.hair )
		self.doc.InsertObject( self.gfm, self.hair )
		# Build
		Ox.ComputeStack( self.doc )
		# Add Data Channels
		self.perStrandChannel = TestUtilities.CreateDataChannel( self.doc, self.eg, "PERSTRAND", 0 )
		self.perVertexChannel = TestUtilities.CreateDataChannel( self.doc, self.eg, "PERVERTEX", 1 )
		# Fill Strand Channel and use in Lenght
		#SelectStrandsByStrandIds( self.doc, self.eg, 0, 100, 400 )
		strandIds = TestUtilities.GetStrandIds( self.eg )
		self.strandIdsAffectedByLength = strandIds[0:100]
		TestUtilities.SelectStrandsByStrandIds( self.doc, self.eg, self.strandIdsAffectedByLength )
		self.eg[Ox.res.edg_CurrentChannelValueAttribute] = 1.0
		self.eg[Ox.res.edg_CurrentChannelIdAttribute] = self.perStrandChannel
		self.guidesLength[Ox.res.len_ValueChannelAttribute] = self.perStrandChannel
		self.hairLength[Ox.res.len_ValueChannelAttribute] = self.perStrandChannel
		TestUtilities.SendDescriptionCommand(self.eg, Ox.res.edg_Button_Channels_SetValue)
		# Build
		Ox.ComputeStack( self.doc )

	def test_GuidesPerStrandDataChannel( self ):
		strandLengths = TestUtilities.GetStrandLengthsByStrandId( self.guidesLength )
		for strandId, length in iter(strandLengths.items()):
			isAffected = (self.strandIdsAffectedByLength.count(strandId) > 0)
			expectedLength = (self.baseLength*2) if isAffected else self.baseLength
			self.assertAlmostEqual( length, expectedLength, 2, msg = 'strandId [' + str( strandId ) + '] guide length is [' + str( length ) + '] instead of: ' + str( expectedLength ) )

	def test_HairPerStrandDataChannel( self ):
		minLength = -1
		maxLength = -1
		strandLengths = TestUtilities.GetStrandLengthsByStrandId( self.hairLength )
		for strandId, length in iter(strandLengths.items()):
			if minLength < 0 or length < minLength:
				minLength = length
			if maxLength < 0 or length > maxLength:
				maxLength = length
		self.assertAlmostEqual( minLength, self.baseLength, 1, msg = 'Some strands should be shorter than minimum found [' + str( minLength ) + ']' )
		self.assertAlmostEqual( maxLength, self.baseLength*4.0, 1, msg = 'Some strands should be longer than maximum found [' + str( maxLength ) + ']' )

	def test_SaveLoad( self ):
		# Change channel name to ensure it is saved
		self.eg[Ox.res.edg_CurrentChannelIdAttribute] = self.perStrandChannel
		TestUtilities.RenameDataChannel( self.doc, self.eg, "CHANGEDNAME" )
		channelsBeforeSave = TestUtilities.GetStrandChannels( self.eg )
		foundNames = []
		for index, channelContainer in channelsBeforeSave:
			channelName = channelContainer[Ox.res.hd_ChannelData_Name]
			foundNames.append( channelName )
		self.assertIn( "CHANGEDNAME", foundNames, msg="Channel name not changed." )
		# Save temp doc
		tempUrl = TestUtilities.SaveTempDocument( self.doc )
		# Load temp doc
		loadedDoc = c4d.documents.LoadDocument( tempUrl, c4d.SCENEFILTER_OBJECTS, None )
		Ox.ComputeStack( loadedDoc )
		loadedEg = loadedDoc.SearchObject( "EG" )
		# Test channels
		channelsAfterLoad = TestUtilities.GetStrandChannels( loadedEg )
		TestUtilities.CheckBaseContainersAreEqual( self, channelsBeforeSave, channelsAfterLoad, False, "ChannelData" )
		# Find channel names
		foundNames = []
		for index, channelContainer in channelsAfterLoad:
			channelName = channelContainer[Ox.res.hd_ChannelData_Name]
			if channelName == "PERSTRAND" or channelName == "PERVERTEX" or channelName == "CHANGEDNAME":
				foundNames.append( channelName )
		self.assertIn( "CHANGEDNAME", foundNames, msg="Channel name change not saved." )
		self.assertEqual( len(foundNames), 2, msg="Missing channel name, loaded only: "+str(foundNames) )
		# Cleanup
		TestUtilities.RenameDataChannel( self.doc, self.eg, "PERSTRAND" )
		c4d.documents.KillDocument( loadedDoc )
		tempUrl.IoDelete( True, True )


#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_EG_ChannelData )