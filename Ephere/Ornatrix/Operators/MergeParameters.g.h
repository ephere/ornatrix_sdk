// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
} }

namespace Ephere { namespace Ornatrix {

struct MergeParameters
{
	enum { Version = 1 };

	enum { HighestId = 3, FirstUnusedId = 4 };

	static char const* GetName() { return "MergeParameters"; }

	struct TargetHair : Parameters::ParameterDescriptor<MergeParameters, 1, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<MergeParameters, 3, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct MergedStrands : Parameters::ParameterDescriptor<MergeParameters, 2, Ephere::Ornatrix::HairParameter[], Parameters::Direction::In, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "MergedStrands"; }
	};

	struct Descriptor;
	struct Container;
};

} }
