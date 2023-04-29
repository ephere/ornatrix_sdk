import binascii
import datetime
import inspect
import os
import sys
import unittest

# For now it seems better not to import PyMEL into the global namespace. A pm. prefix seems to be an acceptable nuisance
# from pymel.core import *
import pymel.core as pm
import RunTests

if sys.version_info > (3,):
    long = int

InBatchMode = pm.about( batch = True )
ApiVersion = pm.about( apiVersion = True )
OsIsLinux = pm.about( os = True ) == 'linux64'
OsIsMac = pm.about( os = True ) == 'mac'
OsIsWindows = pm.about( os = True ) == 'win64' or pm.about( os = True ) == 'nt'

PluginFileName = 'Ornatrix'
OrnatrixVersion = ''
OrnatrixVersionMajor = 0
ClearSceneAfterTest = True
OldGroomExtension = '.oxgroom'
NewGroomExtension = '.oxg.yaml'

DeleteTestFiles = True

# Used to pass self to OxPerformanceTest setupCode and testCode
Self = None

# Use this list to run all performance tests: RunTests.main( MayaTest.PerformanceTestsList )
PerformanceTestsList = [
	'Test_CurlNode.test_ComputePerformance',
	'Test_CurlNode.test_HairDataCopyPerformance',
	'Test_CurlNode.test_HairCopyPerformanceWidth',
	'Test_EditGuidesShape.test_CommandPerformance',
	'Test_EditGuidesShape.test_BrushPerformance',
	'Test_EditGuidesShape.test_BrushPerformanceMesh',
	'Test_HairFromGuidesNode.test_ComputePerformance',
	'Test_HairFromGuidesNode.test_RootGenerationPerformance',
	'Test_MoovPhysicsNode.test_ComputePerformance_Cosserat',
	#'Test_MoovPhysicsNode.test_ComputePerformance_AllModels',
	'Test_MoovPhysicsNode.test_ComputePerformanceBaseMeshCollisions',
	#'Test_MoovPhysicsNode.test_ComputePerformanceAttractToInitialShape',
]


def LoadOrnatrixIfNeeded():
	result = 0
	if not pm.pluginInfo( PluginFileName, query=True, loaded=True ):
		pm.loadPlugin( PluginFileName )
		pm.mel.OxProfiler( '-enableHairRegistry', True )
		result = pm.mel.OxProfiler( "-assertCount" ) + pm.mel.OxProfiler( "-errorCount" )
	global OrnatrixVersion
	global OrnatrixVersionMajor
	if OrnatrixVersion == '':
		OrnatrixVersion = pm.pluginInfo( PluginFileName, query = True, version = True )
		OrnatrixVersionMajor = int( OrnatrixVersion.split( '.' )[0] )
	return result

def GetFileCrc( filepath ):
	with open( filepath ) as f:
		b = f.read()
	return long( binascii.crc32( bytearray( b, 'utf-8' ) ) )

def GetHairMemoryUsage( code ):
	pm.mel.OxProfiler( '-enableHairRegistry', True )
	code()
	result = pm.mel.OxProfiler( '-hairMemoryUsage' )
	pm.newFile( force = True )
	return result

def RunAnimation( shapeToEvaluate, startFrame = 1, frameCount = 10 ):
	if InBatchMode:
		for t in range( startFrame, startFrame + frameCount ):
			pm.currentTime( t )
			shapeToEvaluate.boundingBox()
	else:
		for t in range( startFrame, startFrame + frameCount ):
			pm.currentTime( t )
			pm.refresh()


class OxTestCase( unittest.TestCase ):
	"""
	Provides a setUp method which loads Ornatrix and a tearDown method to reset the scene and unload Ornatrix
	"""

	def setUp( self ):
		if LoadOrnatrixIfNeeded() > 0:
			raise RuntimeError( 'Errors detected on plugin load' )

		# Disable soft selection as our guide editing tests don't work properly if it's enabled
		pm.softSelect( sse=0 )

		# Clear out any errors that happened before starting the test, and record the current assert count (maybe it could be cleared out too)
		pm.mel.OxProfiler( "-errorCount" )
		pm.mel.OxProfiler( "-warningCount" )
		self.assertCount = pm.mel.OxProfiler( "-assertCount" )

		pm.mel.OxLog( "==== Running " + self.id() )
		pm.newFile( force = True )
		self.tempFiles = []
		self.testTime = datetime.datetime.now()

	def tearDown( self ):
		self.testTime = ( datetime.datetime.now() - self.testTime ).total_seconds()

		# Cleanup
		global DeleteTestFiles
		if DeleteTestFiles:
			for f in self.tempFiles:
				try:
					os.remove( f )
				except:
					pass

		if ClearSceneAfterTest:
			pm.newFile( force = True )

		# Check for orphaned hair objects, we can't even unload the plugin if there are any
		hairObjectCount = pm.mel.OxProfiler( '-hairObjectCount' )
		if hairObjectCount > 0:
			pm.mel.OxProfiler( '-printHairObjects' )
			# This clears the registry so it doesn't fail all the remaining tests
			pm.mel.OxProfiler( '-enableHairRegistry', False )
			pm.mel.OxProfiler( '-enableHairRegistry', True )

		# If scene is cleared after the test make sure we have no hair remaining, otherwise there will be hair objects since they're not cleared
		if ClearSceneAfterTest:
			self.assertEqual( 0, hairObjectCount )

		# Store some counts that cannot be retrieved once the plugin is unloaded
		newAssertCount = pm.mel.OxProfiler( "-assertCount" )
		errorCount = pm.mel.OxProfiler( "-errorCount" )
		warningCount = pm.mel.OxProfiler( "-warningCount" )

		pm.mel.OxLog( "==== Finished {}: {}".format( self.id(), self.testTime ) )
		RunTests.TestTimings.append( ( self.id(), self.testTime ) )

		# Unload the plugin. There are problems with unloading the DLL on Windows, which lead to test crashes. We stop the official support for unloading on Windows.
		if not OsIsWindows:
			pm.unloadPlugin( PluginFileName )

		# The test fails if any ASSERT's failed unexpectedly, these need to be inspected
		self.assertEqual( self.assertCount, newAssertCount )

		# The test fails if there are errors in the log, these need to be inspected
		self.assertEqual( 0, errorCount )
		# TODO:? self.assertEqual( 0, warningCount )

	def findTestFile( self, filename ):
		fullpath = os.path.join( os.path.dirname( os.path.realpath( __file__ ) ), filename )
		if os.access( fullpath, os.F_OK ):
			return fullpath
		fullpath = os.path.join( RunTests.RepositoryRootPath, 'External', 'TestData', filename )
		if os.access( fullpath, os.F_OK ):
			return fullpath
		self.skipTest( 'Test file not found: ' + filename )

	def addTempFile( self, filename ):
		# gettempdir returns an obscure directory on the Mac ( /var/folders/pc/pr68x4nd4436_mpb9q__1p_h0000gp/T ), we prefer /tmp
		import tempfile
		tempDir = tempfile.gettempdir() if sys.platform == 'win32' else '/tmp'
		tempFileName = os.path.join( tempDir, filename )
		self.tempFiles.append( tempFileName )
		return tempFileName

	def assertNearEqual( self, expected, actual, epsilon = 0.0001 ):
		self.assertTrue( abs( expected - actual ) < epsilon, "Unexpected not near equal value (expected: {}, actual: {})".format( expected, actual ) )

	def assertNotNearEqual( self, expected, actual, epsilon = 0.0001 ):
		self.assertTrue( abs( expected - actual ) > epsilon, "Unexpected near equal value (expected: {}, actual: {})".format( expected, actual ) )

	def assertSequenceAlmostEqual( self, expected, actual, places = 7 ):
		self.assertEqual( len( expected ), len( actual ) )
		for i in range(0, len( expected ) ):
			self.assertAlmostEqual( expected[i], actual[i], places, "Mismatch at position {}, expected: {}, actual: {}".format( i, expected[i], actual[i] ) )

	def assertPerformance( self, setupCode, targetCode, testNameSuffix = None, profileResultsFilepathPrefix = None ):
		"""
		If profileResultsFilepathPrefix is a non-empty string, it will be passed to OxPerformanceTest -profile,
		which will enable the profiler and store the results using the provided prefix.
		If it is an empty string, -profile will be passed to OxPerformanceTest without an argument, which will
		enable the profiler and store the results into the ProfileSavePath specified in %LOCALAPPDATA%\OrnatrixMaya.ini, using the test name as a file name prefix
		"""
		LoadOrnatrixIfNeeded()

		# From https://stackoverflow.com/questions/5067604/determine-function-name-from-within-that-function-without-using-traceback
		testName = self.__class__.__name__ + '.' + inspect.stack()[1][3]
		if testNameSuffix is not None:
			testName += '.' + testNameSuffix

		global Self
		Self = self

		if setupCode != '':
			setupCode = 'import MayaTest\nimport maya.cmds\nmaya.cmds.file( new = True, force = True )\n' + setupCode
		else:
			setupCode = 'import MayaTest'

		if profileResultsFilepathPrefix == '':
			self.assertTrue( pm.mel.OxPerformanceTest( testName, '-setupPythonCode', setupCode, '-testPythonCode', targetCode, '-profile' ) )
		elif not profileResultsFilepathPrefix is None:
			self.assertTrue( pm.mel.OxPerformanceTest( testName, '-setupPythonCode', setupCode, '-testPythonCode', targetCode, '-profile',  profileResultsFilepathPrefix) )
		else:
			self.assertTrue( pm.mel.OxPerformanceTest( testName, '-setupPythonCode', setupCode, '-testPythonCode', targetCode ) )

	def assertHairMemoryUsage( self, code, expectedValue ):

		if not OsIsWindows or pm.mel.OxProfiler( '-configuration' ) == 'Debug':
			pm.mel.OxLog( 'Hair memory usage tests are disabled in Debug configuration and (for now) on Unix' )
			return

		memoryUsage = GetHairMemoryUsage( code )
		self.assertEqual( expectedValue, memoryUsage )

	def assertFileCrc( self, filepath, expectedCrc ):
		crc = GetFileCrc( filepath )
		if crc < 0:
			crc = crc + long( 0x100000000 )
		self.assertEqual( expectedCrc, crc, "Unexpected crc for file {} (expected: {}, actual: {})".format( filepath, expectedCrc, crc ) )

	def assertFileSize( self, filepath, expectedSize ):
		size = os.path.getsize( filepath )
		self.assertEqual( expectedSize, size, "Unexpected size for file {} (expected: {}, actual: {})".format( filepath, expectedSize, size ) )

	def assertAlembicFileSize( self, filepath, expectedSize ):
		size = os.path.getsize( filepath )
		self.assertAlmostEqual( expectedSize, size, delta = 1, msg = "Unexpected size for file {} (expected: {}, actual: {})".format( filepath, expectedSize, size ) )

class UndoChunk:
	"""
	A "context manager" to be used with Python's 'with' statement. See https://docs.python.org/2/reference/datamodel.html#context-managers
	Wraps the code within the 'with' statement in an undo chunk that can be later reverted with 'pm.undo()'
	Typical usage:
		\code
		with MayaTest.UndoChunk():
			# some code that creates stuff in the scene

		pm.undo()
		\endcode
	"""
	def __enter__( self ):
		pm.undoInfo( openChunk = True )
	def __exit__( self, type, value, traceback ):
		pm.undoInfo( closeChunk = True )
