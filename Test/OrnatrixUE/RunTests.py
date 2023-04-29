"""
Runs tests within the current folder matching the file pattern Test_*.py
Tests use Python's built-in unit testing framework
They should be written as classes derived from unittest.TestCase, with methods prefixed with 'test'
https://docs.python.org/2/library/unittest.html
"""

import os
import os.path
import sys
import unittest
import unreal

try: from importlib import reload
except ImportError: pass

import logging

# 0 = disabled, 1 = enabled, 2 = run once (as in Debug)
PerformanceTestsMode = os.getenv('PerformanceTestsMode')
PerformanceTestsMode = 1 if PerformanceTestsMode is None else int(PerformanceTestsMode)

def GetRootPath():
	return os.path.dirname( os.path.abspath( __file__ ) ) if '__file__' in globals() else os.getcwd()

RunTestsRootPath = GetRootPath()

def GetRepositoryRootPath():
	global RunTestsRootPath
	repositoryRootPath = RunTestsRootPath
	for i in range( 6 ):
		repositoryRootPath = os.path.dirname( repositoryRootPath )
	return repositoryRootPath

RepositoryRootPath = GetRepositoryRootPath()

def GetRepositoryInfo():
	global RepositoryRootPath
	repositoryChangeset = ''
	repositoryRevision = 0
	repositoryBranch = 'default'
	repoInfoFile = os.path.join( RepositoryRootPath, 'Revision' )
	if os.path.exists( repoInfoFile ):
		with open( repoInfoFile ) as f:
			repositoryChangeset = f.readline().strip()
			repositoryRevision = int( f.readline() )
			repositoryBranch = f.readline().strip()
	return repositoryChangeset, repositoryRevision, repositoryBranch

RepositoryChangeset, RepositoryRevision, RepositoryBranch = GetRepositoryInfo()

def IsTestModule( moduleName ):
	if sys.modules[moduleName] is None or not hasattr( sys.modules[moduleName], '__file__' ) or sys.modules[moduleName].__file__ is None:
		return False

	modulePath = os.path.abspath( sys.modules[moduleName].__file__ )
	if 'Ornatrix' not in modulePath:
		return False
	return moduleName.startswith( 'Test_' ) or moduleName == 'TestUtilities'

sys.path.append( os.path.join( RepositoryRootPath, 'Tools', 'Python' ) )
try:
	from teamcity import is_running_under_teamcity
	from teamcity.unittestpy import TeamcityTestRunner
except:
	is_running_under_teamcity = lambda: False

def AddModuleIfNeeded( name ):
	return name[:name.index('.')] + '.' + name if name.count('.') == 1 else name

def RandomSeeds( startOrStop, stop = None ):
	start = 1
	if stop is None:
		stop = startOrStop + 1
	else:
		start = startOrStop

	for seed in range( start, stop ):
		pm.mel.OxLog( '== RandomSeed: {}'.format( seed ) )
		pm.newFile( force = True )
		yield seed

def ReloadTestModules():
	for m in list(sys.modules.keys()):
		if IsTestModule(m):
			print( "Reloading module " + m )
			reload( sys.modules[m] )

TestTimings = []

def main( name = None, repeatCount = 1, verbosity = 1, rootPath = None, printTimings = False ):
	"""
	Runs tests within the given rootPath matching the file pattern Test_*.py
	The name parameter can be given to run a specific test or module. The test needs to be identified as 'module[.class.method]', e.g.
		RunTests.main( name = 'Test_AddHair.Test_AddHair.test_AddHairToPlane' )
	The repeatCount parameter can be used to discover flaky tests by running tests multiple times.
	"""
	# UE prints a lot of nonsense each installed plugin will spam the log 
	#os.system("cls")
	
	print( "Tests start in: " + os.getcwd() )

	rootPath = os.path.dirname( os.path.abspath( __file__ ) )

	print( "rootPath: " + str( rootPath ) )
	print( "Running under TeamCity: " + ( 'yes' if is_running_under_teamcity() else 'no' ) )
	
	ReloadTestModules()

	sys.path.append( rootPath )
	
	if name is None:
		allTests = unittest.TestLoader().discover( rootPath, pattern = 'Test_*.py' )
	elif isinstance(name, str):
		allTests = unittest.TestLoader().loadTestsFromName( AddModuleIfNeeded( name ) )
	else:
		allTests = unittest.TestLoader().loadTestsFromNames( map( AddModuleIfNeeded, name ) )


	testSuite = unittest.TestSuite()
	for i in range( repeatCount ):
		testSuite.addTests( allTests )

	global TestTimings
	TestTimings = []
	if is_running_under_teamcity():
		runner = TeamcityTestRunner()
	else:
		runner = unittest.TextTestRunner( stream = sys.stdout, verbosity = verbosity )


	result = runner.run( testSuite )

	print( "Test result: " + str( result ) )
	
	
	if printTimings:
		TestTimings.sort( None, lambda x: x[1], True )
		for pair in TestTimings:
			print( '{}\t{}'.format( pair[0], pair[1] ) )
	return len( result.errors ) + len( result.failures )

def Repeat( repeatCount, testName ):
	testSuite = unittest.TestSuite()
	testSuite.addTests( unittest.TestLoader().loadTestsFromName( AddModuleIfNeeded( testName ) ) )
	runner = unittest.TextTestRunner( stream = sys.stdout, verbosity = 0 )

	for i in range( 0, repeatCount ):
		print( "Iteration {}/{}".format( i, repeatCount ) )
		result = runner.run( testSuite )
		if len( result.errors ) + len( result.failures ) > 0:
			print( "Failed at iteration " + str( i ) )
			break

if __name__ == '__main__':
	exitCode = min( main( name = None if len( sys.argv ) < 2 else sys.argv[1:], rootPath = os.getcwd() ), 127 )
	sys.exit( exitCode )

