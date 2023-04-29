// Example Ornatrix hair operator

#include "ScaleHairOperatorNode.h"

#pragma warning( push, 2 )
#include <maya/MDataHandle.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MGlobal.h>
#pragma warning( pop )

using namespace Ephere::Plugins::Ornatrix;

MObject ScaleHairOperatorNode::FactorAttribute;
ScaleHairOperatorNode::StrandsPorts ScaleHairOperatorNode::Ports;

MStatus ScaleHairOperatorNode::Initialize()
{
	MFnMessageAttribute messageAttribute;
	MFnNumericAttribute numericAttribute;
	MFnGenericAttribute genericAttribute;

	auto status = Ports.Initialize( genericAttribute, messageAttribute );

	FactorAttribute = numericAttribute.create( "factor", "f", MFnNumericData::kFloat, 1.0f, &status );
	numericAttribute.setStorable( true );
	numericAttribute.setKeyable( true );
	numericAttribute.setSoftMin( 0.0f );
	numericAttribute.setSoftMax( 10.0f );

	return status
		&& addAttribute( FactorAttribute )
		&& Ports.SetAffectsOutput( FactorAttribute )
		? MS::kSuccess : MS::kFailure;
}

ScaleHairOperatorNode::StrandsPorts const& ScaleHairOperatorNode::GetPorts() const
{
	return Ports;
}

MStatus ScaleHairOperatorNode::Compute( MDataBlock& data, Ephere::Ornatrix::IHair const& inputHair, Ephere::Ornatrix::IHair& outputHair )
{
	auto const vertexCount = inputHair.GetVertexCount();
	if( vertexCount != outputHair.GetVertexCount() )
	{
		return MS::kFailure;
	}

	auto const handle = data.inputValue( FactorAttribute );
	auto const factor = handle.asFloat();

	auto vertices = inputHair.GetVertices( IHair::Object );
	// Parallel for would be fine here
	for( auto& vertex : vertices )
	{
		vertex *= factor;
	}

	outputHair.SetVertices( 0, vertexCount, vertices.data(), IHair::Object );
	MGlobal::displayInfo( "ScaleHairOperatorNode used IHair" );
	return MS::kSuccess;
}
