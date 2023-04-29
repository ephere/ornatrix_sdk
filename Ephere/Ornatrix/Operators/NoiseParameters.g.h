// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"
#include "Ephere/Ornatrix/ParameterComponents.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
class Ramp;
struct StrandChannel;
struct TextureMapParameter;
} }

namespace Ephere { namespace Ornatrix {

struct NoiseParameters
{
	enum { Version = 1 };

	enum { HighestId = 18, FirstUnusedId = 19 };

	static char const* GetName() { return "NoiseParameters"; }

	struct TargetHair : Parameters::ParameterDescriptor<NoiseParameters, 16, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<NoiseParameters, 17, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct CurrentTime : Parameters::ParameterDescriptor<NoiseParameters, 18, float, Parameters::Direction::In, Parameters::Purpose::CurrentTime, true, true>
	{
		static char const* Name() { return "CurrentTime"; }

		static Parameters::TimeUnits const DefaultUnits = Parameters::TimeUnits::Seconds;
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<NoiseParameters, 1, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};

		struct BlendDistance : Parameters::ParameterDescriptor<NoiseParameters, 2, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "StrandGroup.BlendDistance"; }

			static float DefaultValue() { return 0; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};
	};

	struct RandomSeed : Parameters::ParameterDescriptor<NoiseParameters, 3, int, Parameters::Direction::In, Parameters::Purpose::RandomSeed, false, true>
	{
		static char const* Name() { return "RandomSeed"; }

		static int DefaultValue() { return 1; }
	};

	// The strength of the applied noise displacement
	struct Amount
	{
		struct Value : Parameters::ParameterDescriptor<NoiseParameters, 4, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Amount.Value"; }

			static float DefaultValue() { return 1.0f; }
		};

		struct Channel : Parameters::ParameterDescriptor<NoiseParameters, 6, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Amount.Channel"; }
		};

		struct Ramp : Parameters::ParameterDescriptor<NoiseParameters, 5, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Amount.Ramp"; }

			static char const* DefaultValue() { return "0 0 0.5 0.5 1 1"; }
		};

		struct Map : Parameters::ParameterDescriptor<NoiseParameters, 7, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Amount.Map"; }
		};
	};

	// Strength specified individually for each axis
	struct AmountVector : Parameters::ParameterDescriptor<NoiseParameters, 8, Ephere::Ornatrix::Vector3, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "AmountVector"; }
	};

	// Frequency of the noise pattern
	struct Scale
	{
		struct Value : Parameters::ParameterDescriptor<NoiseParameters, 9, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Scale.Value"; }

			static float DefaultValue() { return 1; }
		};
	};

	// When enabled, the directional parameters will be applied as world-space vectors.
	struct WorldSpace : Parameters::ParameterDescriptor<NoiseParameters, 10, bool>
	{
		static char const* Name() { return "WorldSpace"; }

		static bool DefaultValue() { return false; }
	};

	// When enabled, the lengths of input strands will be preserved in the output.
	struct PreserveStrandLength : Parameters::ParameterDescriptor<NoiseParameters, 11, bool>
	{
		static char const* Name() { return "PreserveStrandLength"; }

		static bool DefaultValue() { return true; }
	};

	struct Offset
	{
		// Moves the noise pattern along each axis
		struct Space : Parameters::ParameterDescriptor<NoiseParameters, 12, Ephere::Ornatrix::Vector3, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "Offset.Space"; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};

		// Moves the noise pattern back and forward in time
		struct Time : Parameters::ParameterDescriptor<NoiseParameters, 13, float, Parameters::Direction::In, Parameters::Purpose::Time, false, true>
		{
			static char const* Name() { return "Offset.Time"; }

			static float DefaultValue() { return 0.0f; }

			static Parameters::TimeUnits const DefaultUnits = Parameters::TimeUnits::Seconds;
		};
	};

	struct Movement
	{
		// Vector specifying in which direction noise pattern will move
		struct Direction : Parameters::ParameterDescriptor<NoiseParameters, 14, Ephere::Ornatrix::Vector3, Parameters::Direction::In, Parameters::Purpose::UnitDirection, false, true>
		{
			static char const* Name() { return "Movement.Direction"; }
		};

		// Specifies how fast the noise pattern will move
		struct Speed : Parameters::ParameterDescriptor<NoiseParameters, 15, float, Parameters::Direction::In, Parameters::Purpose::Velocity, false, true>
		{
			static char const* Name() { return "Movement.Speed"; }

			static float DefaultValue() { return 0.0f; }
		};
	};

	struct Descriptor;
	struct Container;
};

} }
