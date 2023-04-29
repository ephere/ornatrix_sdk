// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
class Ramp;
} }

namespace Ephere { namespace Ornatrix {

struct ResolveCollisionsParameters
{
	enum { Version = 1 };

	enum { HighestId = 19, FirstUnusedId = 20 };

	static char const* GetName() { return "ResolveCollisionsParameters"; }

	enum class ResolveModeType
	{
		Stiff,
		Bend,
		Soft,
	};

	struct TargetHair : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 15, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 16, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 1, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};
	};

	// Collision resolve mode, in Stiff mode a colliding strand is rotated at the root, in Bend mode all segments of a colliding strand is rotated to form bend, in Soft mode segments can be rotated at different directions and angles
	struct ResolveMode : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 2, ResolveModeType>
	{
		static char const* Name() { return "ResolveMode"; }

		static ResolveModeType DefaultValue() { return ResolveModeType::Stiff; }
	};

	struct Angle
	{
		// Specifies precision used to resolve collision
		struct Step : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 3, float, Parameters::Direction::In, Parameters::Purpose::Angle, false, true>
		{
			static char const* Name() { return "Angle.Step"; }

			static float DefaultValue() { return 0.25f; }

			static Parameters::AngleUnits const DefaultUnits = Parameters::AngleUnits::Degrees;
		};

		// Limit maximum angle used to resolve collision in stiff and bend modes
		struct Maximum : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 4, float, Parameters::Direction::In, Parameters::Purpose::Angle, false, true>
		{
			static char const* Name() { return "Angle.Maximum"; }

			static float DefaultValue() { return 180; }

			static Parameters::AngleUnits const DefaultUnits = Parameters::AngleUnits::Degrees;
		};

		// Limit maximum angle change that could be used to rotate a colliding strand in backward direction. Set to 0 to disable feature.
		struct MaximumBackward : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 5, float, Parameters::Direction::In, Parameters::Purpose::Angle, false, true>
		{
			static char const* Name() { return "Angle.MaximumBackward"; }

			static float DefaultValue() { return 15; }

			static Parameters::AngleUnits const DefaultUnits = Parameters::AngleUnits::Degrees;
		};

		// Limit maximum angle used to rotate segments of a colliding strand at single step
		struct SegmentMaximum : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 6, float, Parameters::Direction::In, Parameters::Purpose::Angle, false, true>
		{
			static char const* Name() { return "Angle.SegmentMaximum"; }

			static float DefaultValue() { return 20; }

			static Parameters::AngleUnits const DefaultUnits = Parameters::AngleUnits::Degrees;
		};
	};

	// Set plasticity to leave an imprint of the collision object. The strand is fully recovers if plasticity is 0%. The strand is kept in its state of maximum rotation if plasticity is 100%.
	struct Plasticity : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 7, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
	{
		static char const* Name() { return "Plasticity"; }

		static float DefaultValue() { return 0.0f; }
	};

	struct Recovery
	{
		// Amount of recovery allowed in one step. Decrease for slower recovery.
		struct Rate : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 8, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Recovery.Rate"; }

			static float DefaultValue() { return 0.02f; }
		};

		// Recovery along strand length
		struct Ramp : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 14, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Recovery.Ramp"; }

			static char const* DefaultValue() { return "0 0 0.5 0.5 1 1"; }
		};
	};

	struct Affected
	{
		// Segment count that a colliding segment affects further down the strand
		struct SegmentCount : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 9, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Affected.SegmentCount"; }

			static int DefaultValue() { return 4; }
		};

		// Affected segment's rotation angle falloff
		struct Falloff : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 10, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Affected.Falloff"; }

			static float DefaultValue() { return 0.75f; }
		};
	};

	struct Scale
	{
		// Specify minimum scale to scale down a colliding strand at target scale angle. If rotation angle is more than scale angle then this value is used as scale, otherwise the strand is scaled linearly from its original size. Inactive if set to 100%
		struct Minimum : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 11, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Scale.Minimum"; }

			static float DefaultValue() { return 1.0f; }
		};

		// Specify target angle to scale down a colliding strand. Could be used to shrink the colliding strand along their length as it rotates. Could be used to reduce look of fur cutting through one another.
		struct Angle : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 12, float, Parameters::Direction::In, Parameters::Purpose::Angle, false, true>
		{
			static char const* Name() { return "Scale.Angle"; }

			static float DefaultValue() { return 90; }

			static Parameters::AngleUnits const DefaultUnits = Parameters::AngleUnits::Degrees;
		};
	};

	// Preserve state from previous steps for smooth collision resolution
	struct PreserveRotations : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 13, bool>
	{
		static char const* Name() { return "PreserveRotations"; }

		static bool DefaultValue() { return true; }
	};

	struct TargetCollisionObjects : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 17, Ephere::Ornatrix::PolygonMeshParameter[]>
	{
		static char const* Name() { return "TargetCollisionObjects"; }
	};

	struct CollideWithDistributionMesh : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 18, bool>
	{
		static char const* Name() { return "CollideWithDistributionMesh"; }

		static bool DefaultValue() { return true; }
	};

	struct CurrentTime : Parameters::ParameterDescriptor<ResolveCollisionsParameters, 19, float, Parameters::Direction::In, Parameters::Purpose::CurrentTime, true, true>
	{
		static char const* Name() { return "CurrentTime"; }

		static Parameters::TimeUnits const DefaultUnits = Parameters::TimeUnits::Seconds;
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::ResolveCollisionsParameters::ResolveModeType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::ResolveCollisionsParameters::ResolveModeType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case ResolveCollisionsParameters::ResolveModeType::Stiff: return "Stiff";
			case ResolveCollisionsParameters::ResolveModeType::Bend: return "Bend";
			case ResolveCollisionsParameters::ResolveModeType::Soft: return "Soft";
			default: return "";
		}
	}
};
}
