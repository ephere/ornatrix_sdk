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

struct ChangeWidthParameters
{
	enum { Version = 1 };

	enum { HighestId = 15, FirstUnusedId = 16 };

	static char const* GetName() { return "ChangeWidthParameters"; }

	struct TargetHair : Parameters::ParameterDescriptor<ChangeWidthParameters, 14, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<ChangeWidthParameters, 15, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<ChangeWidthParameters, 1, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};

		struct BlendDistance : Parameters::ParameterDescriptor<ChangeWidthParameters, 2, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "StrandGroup.BlendDistance"; }

			static float DefaultValue() { return 0; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};
	};

	// Width of the hair strands
	struct Width
	{
		struct Value : Parameters::ParameterDescriptor<ChangeWidthParameters, 3, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "Width.Value"; }

			static float DefaultValue() { return 0.05f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};

		struct Channel : Parameters::ParameterDescriptor<ChangeWidthParameters, 6, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Width.Channel"; }
		};

		struct Ramp : Parameters::ParameterDescriptor<ChangeWidthParameters, 4, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Width.Ramp"; }

			static char const* DefaultValue() { return "0 1 0.5 0.5 1 0"; }
		};

		struct Map : Parameters::ParameterDescriptor<ChangeWidthParameters, 5, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Width.Map"; }
		};
	};

	// Random variation added to hair widths
	struct Randomize
	{
		struct Value : Parameters::ParameterDescriptor<ChangeWidthParameters, 7, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Randomize.Value"; }

			static float DefaultValue() { return 0.0f; }
		};

		struct Channel : Parameters::ParameterDescriptor<ChangeWidthParameters, 9, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Randomize.Channel"; }
		};

		struct Map : Parameters::ParameterDescriptor<ChangeWidthParameters, 8, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Randomize.Map"; }
		};
	};

	struct RandomSeed : Parameters::ParameterDescriptor<ChangeWidthParameters, 13, int, Parameters::Direction::In, Parameters::Purpose::RandomSeed, false, true>
	{
		static char const* Name() { return "RandomSeed"; }

		static int DefaultValue() { return 1; }
	};

	struct UseAbsoluteLength
	{
		// When enabled, the width modification along the strand length will be done from 0 to AbsoluteLength value and not from root to tip
		struct Enabled : Parameters::ParameterDescriptor<ChangeWidthParameters, 10, bool>
		{
			static char const* Name() { return "UseAbsoluteLength.Enabled"; }

			static bool DefaultValue() { return false; }
		};

		// When enabled and UseAbsoluteLength is used, the longest strand will be used to calculate absolute length instead of the AbsoluteLength parameter
		struct UseLongestStrandLength : Parameters::ParameterDescriptor<ChangeWidthParameters, 11, bool>
		{
			static char const* Name() { return "UseAbsoluteLength.UseLongestStrandLength"; }

			static bool DefaultValue() { return false; }
		};

		// When using absolute length, this specifies the strand length along which the width curve will be applied. Shorter strands will use less of the curve.
		struct AbsoluteLength : Parameters::ParameterDescriptor<ChangeWidthParameters, 12, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "UseAbsoluteLength.AbsoluteLength"; }

			static float DefaultValue() { return 20.0f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};
	};

	struct Descriptor;
	struct Container;
};

} }
