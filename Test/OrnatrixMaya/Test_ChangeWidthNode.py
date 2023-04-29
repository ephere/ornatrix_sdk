import MayaTest
import RunTests
import TestUtilities
import pymel.core as pm
import pymel.core.datatypes as dt

import unittest

class Test_ChangeWidthNode( MayaTest.OxTestCase ):

	def test_WidthChannelWithGSD( self ):
		# Add hair and clump node
		hairShape = TestUtilities.AddHairToNewPlane( rootGenerationMethod = 4 )
		generateStrandDataNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.GenerateStrandDataNodeName ) )
		changeWidthNode = pm.PyNode( pm.mel.OxAddStrandOperator( hairShape, TestUtilities.ChangeWidthNodeName ) )

		verticesBeforeChannel = pm.mel.OxGetWidths( hairShape )
		generateStrandDataNode.generationMethod.set( 0 )
		generateStrandDataNode.minimumTargetValue.set( 5 )

		# Dummy evaluation
		TestUtilities.GetVerticesInObjectCoordinates( hairShape )

		changeWidthNode.widthChannel.set( 2 )
		verticesAfterChannel = pm.mel.OxGetWidths( hairShape )

		TestUtilities.CheckPointsNotAllNearEqual( self, verticesBeforeChannel, verticesAfterChannel )