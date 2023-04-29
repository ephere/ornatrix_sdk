#pragma once

#define AUTODESK_MAYA
#include "Ephere/Plugins/Ornatrix/HairInterfacesMaya.h"

#pragma warning( push, 2 )
#include <maya/MPxNode.h>
#pragma warning( pop )

struct Version
{
	int major, minor, fix, revision;
	std::string version;
};

Version GetOrnatrixPluginVersion();

enum NodeState
{
	NodeState_Normal = 0,
	NodeState_HasNoEffect = 1,
	NodeState_Blocking = 2,
	NodeState_WaitingNormal = 8,
};

// Helper base class for Ornatrix operators
class StrandsOperatorNode : public MPxNode
{
public:

	static char const InputStackAttributeName[];
	static char const OutputStackAttributeName[];
	static char const InputStrandsAttributeName[];
	static char const OutputStrandsAttributeName[];

	struct StrandsPorts
	{
		MObject InputStackAttribute;
		MObject OutputStackAttribute;

		MObject InputStrandsAttribute;
		MObject OutputStrandsAttribute;

		MStatus Initialize( MFnGenericAttribute&, MFnMessageAttribute& );

		MStatus SetAffectsOutput( MObject const& inputAttribute );
	};

	// From MPxNode:

	MStatus setDependentsDirty( MPlug const& plugBeingDirtied, MPlugArray& affectedPlugs ) override;

#if MAYA_API_VERSION >= 201600
	MStatus preEvaluation( MDGContext const& context, MEvaluationNode const& evaluationNode ) override;
#endif

protected:

	virtual void PreEvaluationDirtyPlug( MPlug const& plugBeingDirtied );

	MStatus compute( MPlug const&, MDataBlock& ) override;

	// The rest to be implemented by deriving clients:

	virtual StrandsPorts const& GetPorts() const = 0;

	/* This implementation will be called first when Ornatrix is version 2.3.6 or later (which offers access to IHair).
	If it's OK for the client to support only these versions, it's enough to provide just this implementation */
	virtual MStatus Compute( MDataBlock&, Ephere::Ornatrix::IHair const& /*inputHair*/, Ephere::Ornatrix::IHair& /*outputHair*/ ) = 0;
};
