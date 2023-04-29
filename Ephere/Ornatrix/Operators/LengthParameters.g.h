// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
struct StrandChannel;
struct TextureMapParameter;
} }

namespace Ephere { namespace Ornatrix {

struct LengthParameters
{
	enum { Version = 1 };

	enum { HighestId = 16, FirstUnusedId = 4 };

	static char const* GetName() { return "LengthParameters"; }

	struct TargetHair : Parameters::ParameterDescriptor<LengthParameters, 15, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<LengthParameters, 16, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<LengthParameters, 1, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};

		struct BlendDistance : Parameters::ParameterDescriptor<LengthParameters, 2, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "StrandGroup.BlendDistance"; }

			static float DefaultValue() { return 0; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};
	};

	// Global value by which to change the length of strands
	struct Length
	{
		struct Value : Parameters::ParameterDescriptor<LengthParameters, 3, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Length.Value"; }

			static float DefaultValue() { return 1.0f; }
		};

		struct Channel : Parameters::ParameterDescriptor<LengthParameters, 6, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Length.Channel"; }
		};

		struct Map : Parameters::ParameterDescriptor<LengthParameters, 5, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Length.Map"; }
		};
	};

	// Randomly varies the strand lengths along the surface
	struct Randomize
	{
		struct Value : Parameters::ParameterDescriptor<LengthParameters, 7, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Randomize.Value"; }

			static float DefaultValue() { return 0.0f; }
		};

		struct Map : Parameters::ParameterDescriptor<LengthParameters, 14, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Randomize.Map"; }
		};
	};

	struct RandomSeed : Parameters::ParameterDescriptor<LengthParameters, 8, int, Parameters::Direction::In, Parameters::Purpose::RandomSeed, false, true>
	{
		static char const* Name() { return "RandomSeed"; }

		static int DefaultValue() { return 1; }
	};

	// When enabled, the strand length will be changed by uniformly scaling all points on the strand, preserving its overall shape.
	struct UseScaling : Parameters::ParameterDescriptor<LengthParameters, 9, bool>
	{
		static char const* Name() { return "UseScaling"; }
	};

	// When enabled, set absolute strand length value in world units instead of a scaling factor.
	struct SetAbsoluteLength : Parameters::ParameterDescriptor<LengthParameters, 10, bool>
	{
		static char const* Name() { return "SetAbsoluteLength"; }
	};

	// When enabled, the width values of the hair will be adjusted based on how the strand lengths are changed.
	struct ScaleWidth : Parameters::ParameterDescriptor<LengthParameters, 11, bool>
	{
		static char const* Name() { return "ScaleWidth"; }
	};

	struct ValueLimit
	{
		// Limits the resulting strand lengths not to be smaller than this value
		struct Minimum : Parameters::ParameterDescriptor<LengthParameters, 12, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "ValueLimit.Minimum"; }

			static float DefaultValue() { return 0.001f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};

		// Limits the resulting strand lengths not to exceed this value
		struct Maximum : Parameters::ParameterDescriptor<LengthParameters, 13, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "ValueLimit.Maximum"; }

			static float DefaultValue() { return 99999.0f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};
	};

	struct Descriptor;
	struct Container;
};

} }
