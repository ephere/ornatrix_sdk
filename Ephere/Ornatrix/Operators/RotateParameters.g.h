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

struct RotateParameters
{
	enum { Version = 1 };

	enum { HighestId = 15, FirstUnusedId = 16 };

	static char const* GetName() { return "RotateParameters"; }

	struct TargetHair : Parameters::ParameterDescriptor<RotateParameters, 14, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<RotateParameters, 15, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<RotateParameters, 1, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};
	};

	// Adjusts the rotation angle, this value is added on top of the pre-calculated rotation
	struct Angle
	{
		struct Value : Parameters::ParameterDescriptor<RotateParameters, 2, float, Parameters::Direction::In, Parameters::Purpose::Angle, false, true>
		{
			static char const* Name() { return "Angle.Value"; }

			static float DefaultValue() { return 0; }

			static Parameters::AngleUnits const DefaultUnits = Parameters::AngleUnits::Degrees;
		};

		struct Channel : Parameters::ParameterDescriptor<RotateParameters, 4, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Angle.Channel"; }
		};

		struct Map : Parameters::ParameterDescriptor<RotateParameters, 3, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Angle.Map"; }
		};
	};

	// Adds randomness to rotation values
	struct Chaos
	{
		struct Value : Parameters::ParameterDescriptor<RotateParameters, 5, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Chaos.Value"; }

			static float DefaultValue() { return 0.0f; }
		};

		struct Channel : Parameters::ParameterDescriptor<RotateParameters, 7, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Chaos.Channel"; }
		};

		struct Map : Parameters::ParameterDescriptor<RotateParameters, 6, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Chaos.Map"; }
		};
	};

	struct FaceCoordinate : Parameters::ParameterDescriptor<RotateParameters, 8, bool>
	{
		static char const* Name() { return "FaceCoordinate"; }

		static bool DefaultValue() { return false; }
	};

	struct RandomSeed : Parameters::ParameterDescriptor<RotateParameters, 9, int, Parameters::Direction::In, Parameters::Purpose::RandomSeed, false, true>
	{
		static char const* Name() { return "RandomSeed"; }

		static int DefaultValue() { return 1; }
	};

	// When enabled, the rotation of the strand will be calculated such that it is facing the distribution surface. Use the two parameters below to determine how the strand's direction vector is calculated.
	struct OrientBasedOnStrandShape : Parameters::ParameterDescriptor<RotateParameters, 10, bool>
	{
		static char const* Name() { return "OrientBasedOnStrandShape"; }

		static bool DefaultValue() { return false; }
	};

	// When enabled, the strand's direction vector will be determined between its first and last points
	struct UseLastPointForStrandVector : Parameters::ParameterDescriptor<RotateParameters, 11, bool>
	{
		static char const* Name() { return "UseLastPointForStrandVector"; }

		static bool DefaultValue() { return true; }
	};

	// When above option is disabled, the strand's direction vector will be determined between point at this index and the first point
	struct PointIndexForStrandVector : Parameters::ParameterDescriptor<RotateParameters, 12, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "PointIndexForStrandVector"; }

		static int DefaultValue() { return 3; }
	};

	// Specifies the angle value, relative to the global rotation, at the tip of each strand
	struct Twist : Parameters::ParameterDescriptor<RotateParameters, 13, float, Parameters::Direction::In, Parameters::Purpose::Angle, false, true>
	{
		static char const* Name() { return "Twist"; }

		static float DefaultValue() { return 0; }

		static Parameters::AngleUnits const DefaultUnits = Parameters::AngleUnits::Degrees;
	};

	struct Descriptor;
	struct Container;
};

} }
