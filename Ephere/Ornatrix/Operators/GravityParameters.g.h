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

struct GravityParameters
{
	enum { Version = 1 };

	enum { HighestId = 8, FirstUnusedId = 9 };

	static char const* GetName() { return "GravityParameters"; }

	struct TargetHair : Parameters::ParameterDescriptor<GravityParameters, 7, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<GravityParameters, 8, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<GravityParameters, 1, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};
	};

	struct Force
	{
		struct Value : Parameters::ParameterDescriptor<GravityParameters, 2, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "Force.Value"; }

			static float DefaultValue() { return 1.0f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};

		struct Channel : Parameters::ParameterDescriptor<GravityParameters, 5, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Force.Channel"; }
		};

		struct Ramp : Parameters::ParameterDescriptor<GravityParameters, 3, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Force.Ramp"; }

			static char const* DefaultValue() { return "0 1 0.5 0.5 1 0"; }
		};

		struct Map : Parameters::ParameterDescriptor<GravityParameters, 4, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Force.Map"; }
		};
	};

	// In the local coordinate space of the target hair
	struct ForceDirection : Parameters::ParameterDescriptor<GravityParameters, 6, Ephere::Ornatrix::Vector3, Parameters::Direction::In, Parameters::Purpose::UnitDirection, false, true>
	{
		static char const* Name() { return "ForceDirection"; }
	};

	struct Descriptor;
	struct Container;
};

} }
