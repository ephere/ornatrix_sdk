// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
class Ramp;
struct StrandChannel;
struct TextureMapParameter;
} }

namespace Ephere { namespace Ornatrix {

struct NormalizeParameters
{
	enum { Version = 1 };

	enum { HighestId = 14, FirstUnusedId = 2 };

	static char const* GetName() { return "NormalizeParameters"; }

	struct TargetHair : Parameters::ParameterDescriptor<NormalizeParameters, 12, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<NormalizeParameters, 13, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<NormalizeParameters, 1, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};
	};

	// The strength of normalization
	struct Amount
	{
		struct Value : Parameters::ParameterDescriptor<NormalizeParameters, 5, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Amount.Value"; }

			static float DefaultValue() { return 1.0f; }
		};

		struct Channel : Parameters::ParameterDescriptor<NormalizeParameters, 6, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Amount.Channel"; }
		};

		struct Ramp : Parameters::ParameterDescriptor<NormalizeParameters, 4, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Amount.Ramp"; }

			static char const* DefaultValue() { return "0 1 0.5 1 1 1"; }
		};

		struct Map : Parameters::ParameterDescriptor<NormalizeParameters, 7, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Amount.Map"; }
		};
	};

	// The size of the volume around each point on hair used to compute the average strand directions, relative to the size of the whole hair object
	struct Radius : Parameters::ParameterDescriptor<NormalizeParameters, 9, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
	{
		static char const* Name() { return "Radius"; }

		static float DefaultValue() { return 0.1f; }
	};

	// Number of neighboring hair points to consider when averaging positions. More points will produce smoother (lower frequency) results.
	struct NearestCount : Parameters::ParameterDescriptor<NormalizeParameters, 10, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "NearestCount"; }

		static int DefaultValue() { return 20; }
	};

	// When enabled, the normalize will work per strand group.
	struct PerStrandGroup : Parameters::ParameterDescriptor<NormalizeParameters, 14, bool>
	{
		static char const* Name() { return "PerStrandGroup"; }

		static bool DefaultValue() { return false; }
	};

	struct Descriptor;
	struct Container;
};

} }
