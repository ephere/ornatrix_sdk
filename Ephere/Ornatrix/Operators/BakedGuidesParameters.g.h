// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
} }

namespace Ephere { namespace Ornatrix {

struct BakedGuidesParameters
{
	enum { Version = 1 };

	enum { HighestId = 5, FirstUnusedId = 6 };

	static char const* GetName() { return "BakedGuidesParameters"; }

	struct InputGuides : Parameters::ParameterDescriptor<BakedGuidesParameters, 1, Ephere::Ornatrix::HairParameter>
	{
		static char const* Name() { return "InputGuides"; }
	};

	struct OutputGuides : Parameters::ParameterDescriptor<BakedGuidesParameters, 2, Ephere::Ornatrix::HairParameter, Parameters::Direction::Out, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "OutputGuides"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<BakedGuidesParameters, 3, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct SourceFilePath : Parameters::ParameterDescriptor<BakedGuidesParameters, 4, Ephere::Parameters::String, Parameters::Direction::In, Parameters::Purpose::FilePath>
	{
		static char const* Name() { return "SourceFilePath"; }

		static char const* DefaultValue() { return ""; }
	};

	struct CurvesPath : Parameters::ParameterDescriptor<BakedGuidesParameters, 5, Ephere::Parameters::String>
	{
		static char const* Name() { return "CurvesPath"; }

		static char const* DefaultValue() { return ""; }
	};

	struct Descriptor;
	struct Container;
};

} }
