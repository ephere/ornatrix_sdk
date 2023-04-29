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

struct FrizzParameters
{
	enum { Version = 1 };

	enum { HighestId = 17, FirstUnusedId = 18 };

	static char const* GetName() { return "FrizzParameters"; }

	struct TargetHair : Parameters::ParameterDescriptor<FrizzParameters, 16, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<FrizzParameters, 17, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<FrizzParameters, 1, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};

		struct BlendDistance : Parameters::ParameterDescriptor<FrizzParameters, 2, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "StrandGroup.BlendDistance"; }

			static float DefaultValue() { return 0; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};
	};

	// The strength of the frizz deformation
	struct Amount
	{
		struct Value : Parameters::ParameterDescriptor<FrizzParameters, 3, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "Amount.Value"; }

			static float DefaultValue() { return 20.0f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};

		struct Channel : Parameters::ParameterDescriptor<FrizzParameters, 6, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Amount.Channel"; }
		};

		struct Ramp : Parameters::ParameterDescriptor<FrizzParameters, 4, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Amount.Ramp"; }

			static char const* DefaultValue() { return "0 0 0.5 0.5 1 1"; }
		};

		struct Map : Parameters::ParameterDescriptor<FrizzParameters, 5, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Amount.Map"; }
		};
	};

	// The frequency of the deformation
	struct Scale : Parameters::ParameterDescriptor<FrizzParameters, 7, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "Scale"; }

		static float DefaultValue() { return 1; }
	};

	// What percent of the hairs are randomly chosen to be outliers
	struct OutlierPercent : Parameters::ParameterDescriptor<FrizzParameters, 8, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
	{
		static char const* Name() { return "OutlierPercent"; }

		static float DefaultValue() { return 0.1f; }
	};

	// The strength of frizzing for strands chosen to be outliers
	struct OutlierAmount : Parameters::ParameterDescriptor<FrizzParameters, 9, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
	{
		static char const* Name() { return "OutlierAmount"; }

		static float DefaultValue() { return 5.0f; }

		static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
	};

	struct RandomSeed : Parameters::ParameterDescriptor<FrizzParameters, 10, int, Parameters::Direction::In, Parameters::Purpose::RandomSeed, false, true>
	{
		static char const* Name() { return "RandomSeed"; }

		static int DefaultValue() { return 1; }
	};

	// When enabled, the noise on strands will be consistent with strand lengths. Otherwise it will always scale the same way from start to end of the strand.
	struct IsLengthDependent : Parameters::ParameterDescriptor<FrizzParameters, 11, bool>
	{
		static char const* Name() { return "IsLengthDependent"; }
	};

	// When enabled, a new per-strand channel will be created in output hair containing values of 1 for strands which are outliers and 0 for all other strands.
	struct AddOutlierChannel : Parameters::ParameterDescriptor<FrizzParameters, 12, bool>
	{
		static char const* Name() { return "AddOutlierChannel"; }
	};

	struct Group
	{
		struct Value : Parameters::ParameterDescriptor<FrizzParameters, 14, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Group.Value"; }
		};

		struct Channel : Parameters::ParameterDescriptor<FrizzParameters, 13, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Group.Channel"; }
		};
	};

	// When enabled, the lengths of input strands will be preserved in the output.
	struct PreserveStrandLength : Parameters::ParameterDescriptor<FrizzParameters, 15, bool>
	{
		static char const* Name() { return "PreserveStrandLength"; }

		static bool DefaultValue() { return false; }
	};

	struct Descriptor;
	struct Container;
};

} }
