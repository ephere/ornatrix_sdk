import os
import platform
import unittest
import subprocess
import shlex

#InBatchMode = pm.about( batch = True )
#ApiVersion = pm.about( apiVersion = True )
OsIsLinux = platform.system() == 'Linux'
OsIsMac = platform.system() == 'Darwin'
OsIsWindows = platform.system() == 'Windows'

PluginFileName = 'Ornatrix'
LucidPhysicsEngineCount = -1

# Same as in Ornatrix/Ornatrix.py (used in C4D)
OK = 'ok'
FAIL = 'fail'
ERROR = 'error'
SKIP = 'skip'

class OxTestCase( unittest.TestCase ):
	def __init__( self, testname, res, msg = '' ):
		self._result = res
		self._message = msg
		#print("+ [%s] = [%s] " % (testname,self._result))
		self.add_test( testname )
		super( OxTestCase, self ).__init__( testname )

	def setUp( self ):
		print( "==== Running " + self.id() + " ====" )

	# def tearDown( self ):
	# 	print( "==== Finished " + self.id() + " ====" )

	# Dynamic test case
	def add_test( self, testname ):
		def innertest( self ):
			if ( self._result == OK ):
				self.assertTrue( True, self._message )
			elif ( self._result == FAIL ):
				self.fail( self._message )
			elif ( self._result == SKIP ):
				self.skipTest( self._message )
			elif ( self._result == ERROR ):
				self.fail( "ERROR: " + self._message )
			else:
				raise RuntimeError
		innertest.__name__ = testname
		setattr( OxTestCase, innertest.__name__, innertest )

def RunC4DTest(rootPath, testName, C4DBinary, tempDir ):
	C4DPrefsDir = os.path.join( tempDir, 'Preferences' )
	resultsPath = os.path.join( tempDir, 'results.json' )
	scenePath = os.path.join( C4DPrefsDir, 'library', 'scripts', 'Test', 'TestScene.c4d' )
	logPath = os.path.join( tempDir, 'output.log')
	cmd = '"%s" -nogui g_licenseUsername=ephere@gmail.com g_licensePassword=%s -ornatrix -ornatrixTestScript "%s" -ornatrixTestOutput "%s" g_prefspath="%s" g_logfile="%s" g_disableConsoleOutput=false "%s"' % (C4DBinary, os.getenv( 'MaxonAccountPassword' ), testName, resultsPath, C4DPrefsDir, logPath, scenePath)
	if not OsIsWindows:
		cmd = '%s >/dev/null' % ( cmd )
		proc = subprocess.Popen( shlex.split( cmd ), stdout=subprocess.PIPE )
	else:
		#cmd = 'start /wait "%s" %s' % (testName, cmd)
		proc = subprocess.Popen( shlex.split( cmd ) )
	print( cmd )

	proc.wait()

	try:
		file = open( resultsPath, 'r' )
	except:
		print( 'Test [%s] could not open result file [%s]' % ( testName, resultsPath ) )
		return None
	jresult = file.read();
	file.close()
	print( "Results: %s" % jresult )
	return jresult
