#include "StrandsOperatorNode.h"

#include <maya/MDataBlock.h>
#if MAYA_API_VERSION >= 201600
#include <maya/MEvaluationNode.h>
#endif
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnMessageAttribute.h>

using namespace Ephere::Plugins::Ornatrix;

const char StrandsOperatorNode::InputStackAttributeName[] = "inputStack";
const char StrandsOperatorNode::OutputStackAttributeName[] = "outputStack";
const char StrandsOperatorNode::InputStrandsAttributeName[] = "inputStrands";
const char StrandsOperatorNode::OutputStrandsAttributeName[] = "outputStrands";

MStatus StrandsOperatorNode::StrandsPorts::Initialize( MFnGenericAttribute& genericAttribute, MFnMessageAttribute& messageAttribute )
{
	MStatus status;

	InputStackAttribute = messageAttribute.create( InputStackAttributeName, "ink", &status );
	messageAttribute.setHidden( true );
	addAttribute( InputStackAttribute );

	OutputStackAttribute = messageAttribute.create( OutputStackAttributeName, "outk", &status );
	messageAttribute.setHidden( true );
	addAttribute( OutputStackAttribute );

	InputStrandsAttribute = genericAttribute.create( InputStrandsAttributeName, "ins", &status );
	genericAttribute.addAccept( HairData::GetTypeId() );
	genericAttribute.addAccept( GuidesData::GetTypeId() );
	genericAttribute.setStorable( false );
	genericAttribute.setHidden( true );
	addAttribute( InputStrandsAttribute );

	OutputStrandsAttribute = genericAttribute.create( OutputStrandsAttributeName, "outs", &status );
	genericAttribute.addAccept( HairData::GetTypeId() );
	genericAttribute.addAccept( GuidesData::GetTypeId() );
	genericAttribute.setWritable( false );
	genericAttribute.setStorable( false );
	genericAttribute.setHidden( true );
	addAttribute( OutputStrandsAttribute );

	return
		attributeAffects( InputStrandsAttribute, OutputStrandsAttribute )
		&& attributeAffects( state, OutputStrandsAttribute )
		? MS::kSuccess : MS::kFailure;
}

MStatus StrandsOperatorNode::StrandsPorts::SetAffectsOutput( MObject const& inputAttribute )
{
	return attributeAffects( inputAttribute, OutputStrandsAttribute );
}

MStatus StrandsOperatorNode::setDependentsDirty( const MPlug& plugBeingDirtied, MPlugArray& affectedPlugs )
{
	PreEvaluationDirtyPlug( plugBeingDirtied );
	return MPxNode::setDependentsDirty( plugBeingDirtied, affectedPlugs );
}

#if MAYA_API_VERSION >= 201600
MStatus StrandsOperatorNode::preEvaluation( const MDGContext& context, const MEvaluationNode& evaluationNode )
{
	if( context.isNormal() )
	{
		auto dirtyPlugIter = evaluationNode.iterator();
		for( ; !dirtyPlugIter.isDone(); dirtyPlugIter.next() )
		{
			PreEvaluationDirtyPlug( dirtyPlugIter.plug() );
		}
	}

	return MPxNode::preEvaluation( context, evaluationNode );
}
#endif

// Clears the cached hair in the datablock immediately once the input gets dirty, to save memory
void StrandsOperatorNode::PreEvaluationDirtyPlug( const MPlug& plugBeingDirtied )
{
	if( plugBeingDirtied == GetPorts().InputStrandsAttribute )
	{
		auto datablock = forceCache();
		auto handle = datablock.outputValue( GetPorts().InputStrandsAttribute );
		handle.setMPxData( nullptr );
		handle = datablock.outputValue( GetPorts().OutputStrandsAttribute );
		handle.setMPxData( nullptr );
	}
}

MStatus StrandsOperatorNode::compute( const MPlug& plug, MDataBlock& data )
{
	if( plug != GetPorts().OutputStrandsAttribute )
	{
		return MS::kUnknownParameter;
	}

	MStatus status;
	auto inputHairHandle = data.inputValue( GetPorts().InputStrandsAttribute, &status );
	auto inputHairData = static_cast<HairData*>( inputHairHandle.asPluginData() );
	if( inputHairData == nullptr )
	{
		// This should happen only when the node's input is not wired, which is normal
		data.setClean( plug );
		return MS::kSuccess;
	}

	if( inputHairData->typeId() != HairData::GetTypeId() && inputHairData->typeId() != GuidesData::GetTypeId() )
	{
		// Should not happen, verify it just in case
		return MS::kInvalidParameter;
	}

	auto outputHairHandle = data.outputValue( GetPorts().OutputStrandsAttribute, &status );
	if( !status || !outputHairHandle.copy( inputHairHandle ) )
	{
		return MS::kFailure;
	}

	if( data.inputValue( state ).asInt() != NodeState_HasNoEffect )
	{
		auto outputHairData = static_cast<HairData*>( outputHairHandle.asPluginData() );
		auto oxVersion = GetOrnatrixPluginVersion();

		if( !CHECK( oxVersion.revision >= HairData::IHairSupportRevision ) )
		{
			return MS::kNotImplemented;
		}

		status = Compute( data, *inputHairData->GetInterface<IHair>(), *outputHairData->GetInterface<IHair>() );
	}

	data.setClean( plug );
	return status;
}
