#----------------
import os
import sys
import c4d
sys.path.insert( 0, os.path.dirname( __file__ ) )
import Ornatrix.Ornatrix as Ox
from Test_GFM_FaceInclude import Test_GFM_FaceInclude
sys.path.pop( 0 )
#----------------
from unittest import TestCase

#
# TestCase
class Test_GFM_FaceIncludeTag( Test_GFM_FaceInclude ):

	def setUp( self ):
		self.useTagAsDistributionMesh = True
		self.redistributeHair = False
		super(Test_GFM_FaceIncludeTag, self).setUp()


#----------------------------------
# main
testObject = None
if __name__ == '__main__':
	Ox.runTest( doc, Test_GFM_FaceIncludeTag )