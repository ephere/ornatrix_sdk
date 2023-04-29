import MayaTest
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt
import unittest
import maya.cmds as mc

VRayOxHairPluginFileName = 'VrayOxHairMaya'
VRayOxHairVersion = ''

VRayPluginFileName = 'vrayformaya'
VRayVersion = ''

def TryLoadPlugin( pluginName ):
	if not pm.pluginInfo( pluginName, query=True, loaded=True ):
		try:
			pm.loadPlugin( pluginName )
		except:
			pass

def LoadVRayOxHairIfNeeded():
	TryLoadPlugin( VRayOxHairPluginFileName )
	global VRayOxHairVersion
	if VRayOxHairVersion == '' and pm.pluginInfo( VRayOxHairPluginFileName, query=True, loaded=True ):
		VRayOxHairVersion = pm.pluginInfo( VRayOxHairPluginFileName, query = True, version = True )
	return VRayOxHairVersion != ''

def LoadVRayIfNeeded():
	TryLoadPlugin( VRayPluginFileName )
	global VRayVersion
	if VRayVersion == '' and pm.pluginInfo( VRayPluginFileName, query=True, loaded=True ):
		VRayVersion = pm.pluginInfo( VRayPluginFileName, query = True, version = True )
	return VRayVersion != ''

class Test_VRayOxHairNode( MayaTest.OxTestCase ):

	def setUp( self ):
		MayaTest.OxTestCase.setUp( self )
		LoadVRayOxHairIfNeeded()

	def tearDown( self ):
		pm.newFile( force = True )
		pm.unloadPlugin( VRayOxHairPluginFileName )
		MayaTest.OxTestCase.tearDown( self )

	@unittest.skipIf( not LoadVRayOxHairIfNeeded(), "VrayOxHairMaya won't load, maybe V-Ray is not installed" )
	@unittest.skipIf( not LoadVRayIfNeeded(), "vrayformaya won't load, maybe V-Ray is not installed" )
	def test_CreateVRayOxHairNode( self ):
		hairShape = TestUtilities.AddHairToNewPlane()

		pm.mel.vrayOxHair( '-create' )

		nodes = pm.ls( type = u'VRayOxHair' )

		self.assertEqual( len( nodes ), 1 )

	@unittest.skipIf( not LoadVRayOxHairIfNeeded(), "VrayOxHairMaya won't load, maybe V-Ray is not installed" )
	@unittest.skipIf( not LoadVRayIfNeeded(), "vrayformaya won't load, maybe V-Ray is not installed" )
	@unittest.skip( "Always fails, disabling until we figure it out so new builds can proceed" )
	def test_VRayOxHairCrashesVRay( self ):
		"""Issue #3680, https://ephere.com:3000/issues/3680"""

		LoadVRayIfNeeded()
		self.assertNotEqual( VRayVersion, '' )
		# Crashes with sphere, fails with hair 
		sphere = pm.sphere()
		#hairShape = TestUtilities.AddHairToNewPlane()
		# Camera has to be set for Maya 2019 in batch mode
		camera = mc.camera()
		mc.vrend( camera = camera[0] )
		#pm.mel.vrayOxHair( '-create' )

