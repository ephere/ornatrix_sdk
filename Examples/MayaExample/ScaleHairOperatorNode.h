// Example Ornatrix hair operator

#pragma once

#include "StrandsOperatorNode.h"

class ScaleHairOperatorNode : public StrandsOperatorNode
{
public:

	static MString GetName()
	{
		return "ScaleHairOperatorNode";
	}

	static MTypeId GetMTypeId()
	{
		return MTypeId( 1 );
	}

	static void* Create()
	{
		return new ScaleHairOperatorNode();
	}

	static MObject FactorAttribute;

	static MStatus Initialize();

protected:

	static StrandsPorts Ports;

	StrandsPorts const& GetPorts() const override;

	MStatus Compute( MDataBlock&, Ephere::Ornatrix::IHair const& inputHair, Ephere::Ornatrix::IHair& outputHair ) override;
};
