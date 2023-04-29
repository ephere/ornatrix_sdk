// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"
#include "Ephere/Ornatrix/ParameterComponents.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
struct StrandChannel;
struct TextureMapParameter;
} }

namespace Ephere { namespace Ornatrix {

struct OscillatorParameters
{
	enum { Version = 1 };

	enum { HighestId = 19, FirstUnusedId = 20 };

	static char const* GetName() { return "OscillatorParameters"; }

	struct TargetHair : Parameters::ParameterDescriptor<OscillatorParameters, 17, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<OscillatorParameters, 18, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct CurrentTime : Parameters::ParameterDescriptor<OscillatorParameters, 19, float, Parameters::Direction::In, Parameters::Purpose::CurrentTime, true, true>
	{
		static char const* Name() { return "CurrentTime"; }

		static Parameters::TimeUnits const DefaultUnits = Parameters::TimeUnits::Seconds;
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<OscillatorParameters, 1, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};
	};

	// Linearly blend operator output with original strand's positions
	struct Amount
	{
		struct Value : Parameters::ParameterDescriptor<OscillatorParameters, 2, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Amount.Value"; }

			static float DefaultValue() { return 1.0f; }
		};

		struct Channel : Parameters::ParameterDescriptor<OscillatorParameters, 4, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Amount.Channel"; }
		};

		struct Map : Parameters::ParameterDescriptor<OscillatorParameters, 3, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Amount.Map"; }
		};
	};

	// Amount of damping force applied to slow down oscillations
	struct Damping
	{
		struct Value : Parameters::ParameterDescriptor<OscillatorParameters, 5, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Damping.Value"; }

			static float DefaultValue() { return 0.3f; }
		};

		struct Channel : Parameters::ParameterDescriptor<OscillatorParameters, 7, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Damping.Channel"; }
		};

		struct Map : Parameters::ParameterDescriptor<OscillatorParameters, 6, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Damping.Map"; }
		};
	};

	// Strand stiffness
	struct Stiffness
	{
		struct Value : Parameters::ParameterDescriptor<OscillatorParameters, 8, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Stiffness.Value"; }

			static float DefaultValue() { return 1.0f; }
		};

		struct Channel : Parameters::ParameterDescriptor<OscillatorParameters, 10, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Stiffness.Channel"; }
		};

		struct Map : Parameters::ParameterDescriptor<OscillatorParameters, 9, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Stiffness.Map"; }
		};
	};

	// Strand inertia. Increase for more expressed effect.
	struct Inertia
	{
		struct Value : Parameters::ParameterDescriptor<OscillatorParameters, 11, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Inertia.Value"; }

			static float DefaultValue() { return 0.1f; }
		};

		struct Channel : Parameters::ParameterDescriptor<OscillatorParameters, 13, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Inertia.Channel"; }
		};

		struct Map : Parameters::ParameterDescriptor<OscillatorParameters, 12, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Inertia.Map"; }
		};
	};

	// Speed up or slow down process overall
	struct TimeScale : Parameters::ParameterDescriptor<OscillatorParameters, 14, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "TimeScale"; }

		static float DefaultValue() { return 1; }
	};

	// Gravity or other static forces vector
	struct Gravity : Parameters::ParameterDescriptor<OscillatorParameters, 15, Ephere::Ornatrix::Vector3, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "Gravity"; }
	};

	// Number of iterations to solve oscillation equations
	struct IterationCount : Parameters::ParameterDescriptor<OscillatorParameters, 16, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "IterationCount"; }

		static int DefaultValue() { return 4; }
	};

	struct Descriptor;
	struct Container;
};

} }
