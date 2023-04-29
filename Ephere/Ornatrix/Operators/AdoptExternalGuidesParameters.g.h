// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
} }

namespace Ephere { namespace Ornatrix {

struct AdoptExternalGuidesParameters
{
	enum { Version = 1 };

	enum { HighestId = 9, FirstUnusedId = 10 };

	static char const* GetName() { return "AdoptExternalGuidesParameters"; }

	struct TargetHair : Parameters::ParameterDescriptor<AdoptExternalGuidesParameters, 5, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<AdoptExternalGuidesParameters, 6, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<AdoptExternalGuidesParameters, 1, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};
	};

	// When enabled, strand filtering will be done based on strand groups in control guides. Otherwise input hair strand groups will be used.
	struct UseGuideStrandGroups : Parameters::ParameterDescriptor<AdoptExternalGuidesParameters, 9, bool>
	{
		static char const* Name() { return "UseGuideStrandGroups"; }

		static bool DefaultValue() { return false; }
	};

	struct Amount : Parameters::ParameterDescriptor<AdoptExternalGuidesParameters, 2, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
	{
		static char const* Name() { return "Amount"; }

		static float DefaultValue() { return 1.0f; }
	};

	// Control point position along strand used to nearest lookup (0% - root, 100% - tip)
	struct ControlPoint : Parameters::ParameterDescriptor<AdoptExternalGuidesParameters, 3, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
	{
		static char const* Name() { return "ControlPoint"; }

		static float DefaultValue() { return 0.0f; }
	};

	// When enabled, the control and reference strands will be paired by their closest respective roots. Otherwise they will be paired by strand ids.
	struct MatchControlStrandsByDistance : Parameters::ParameterDescriptor<AdoptExternalGuidesParameters, 4, bool>
	{
		static char const* Name() { return "MatchControlStrandsByDistance"; }

		static bool DefaultValue() { return false; }
	};

	struct Objects
	{
		struct Control : Parameters::ParameterDescriptor<AdoptExternalGuidesParameters, 7, Ephere::Ornatrix::HairParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, true>
		{
			static char const* Name() { return "Objects.Control"; }
		};

		struct Reference : Parameters::ParameterDescriptor<AdoptExternalGuidesParameters, 8, Ephere::Ornatrix::HairParameter>
		{
			static char const* Name() { return "Objects.Reference"; }
		};
	};

	struct Descriptor;
	struct Container;
};

} }
