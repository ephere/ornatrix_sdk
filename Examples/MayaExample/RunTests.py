import os

import maya.cmds as cmds
import maya.mel as mel
import maya.standalone

if __name__ == '__main__':
	maya.standalone.initialize( name='python' )
	cmds.loadPlugin( 'Ornatrix' )
	cmds.loadPlugin( 'OxMayaExample' )

	plane = cmds.polyPlane()
	transform = plane[0]

	guidesFromMesh = cmds.createNode( "GuidesFromMeshNode", name = "GuidesFromMesh#" )
	cmds.connectAttr( transform + '.outMesh', guidesFromMesh + '.inputMesh' )
	cmds.setAttr( guidesFromMesh + ".length", 5 )

	hairFromGuides = mel.eval( 'OxAddStrandOperator( "' + guidesFromMesh + '", "HairFromGuidesNode" )' )

	expectedStrandCount = 1000
	strandCount = cmds.OxPrintStrandCount( "HairShape1" )
	exitCode = 0
	if strandCount != expectedStrandCount:
		print( 'Strand count mismatch, expected {}, got {}'.format( expectedStrandCount, strandCount ) )
		exitCode = 1

	verticesBefore = cmds.OxGetVertices( "HairShape1" )

	scaleOperator = mel.eval( 'OxAddStrandOperator( "' + guidesFromMesh + '", "ScaleHairOperatorNode" )' )
	factor = 2
	cmds.setAttr( scaleOperator + '.factor', factor )

	verticesAfter = cmds.OxGetVertices( "HairShape1" )
	if abs(verticesBefore[5] * factor - verticesAfter[5]) > 0.0001:
		print( 'Coordinate mismatch, expected {}, got {}'.format( verticesBefore[5] * factor, verticesAfter[5] ) )
		exitCode = 1

	# cmds.evalDeferred( 'cmds.quit( abort = True, exitCode = {} )'.format( exitCode ) )
	os._exit( exitCode )
