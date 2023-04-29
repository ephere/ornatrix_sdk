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

struct PushAwayFromSurfaceParameters
{
	enum { Version = 1 };

	enum { HighestId = 17, FirstUnusedId = 18 };

	static char const* GetName() { return "PushAwayFromSurfaceParameters"; }

	struct TargetHair : Parameters::ParameterDescriptor<PushAwayFromSurfaceParameters, 16, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<PushAwayFromSurfaceParameters, 13, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct CurrentTime : Parameters::ParameterDescriptor<PushAwayFromSurfaceParameters, 17, float, Parameters::Direction::In, Parameters::Purpose::CurrentTime, true, true>
	{
		static char const* Name() { return "CurrentTime"; }

		static Parameters::TimeUnits const DefaultUnits = Parameters::TimeUnits::Seconds;
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<PushAwayFromSurfaceParameters, 1, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};
	};

	// Distance of strands away from surface
	struct Distance : Parameters::ParameterDescriptor<PushAwayFromSurfaceParameters, 2, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
	{
		static char const* Name() { return "Distance"; }

		static float DefaultValue() { return 0.1f; }

		static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
	};

	struct Amount
	{
		struct Ramp : Parameters::ParameterDescriptor<PushAwayFromSurfaceParameters, 11, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Amount.Ramp"; }

			static char const* DefaultValue() { return "0 1 1 1"; }
		};
	};

	// Preserve state from previous steps for smooth collision resolution
	struct PreserveState : Parameters::ParameterDescriptor<PushAwayFromSurfaceParameters, 3, bool>
	{
		static char const* Name() { return "PreserveState"; }

		static bool DefaultValue() { return false; }
	};

	// Collision resolution iteration count
	struct IterationCount : Parameters::ParameterDescriptor<PushAwayFromSurfaceParameters, 4, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "IterationCount"; }

		static int DefaultValue() { return 3; }
	};

	// Apply optional smoothing
	struct Smoothing : Parameters::ParameterDescriptor<PushAwayFromSurfaceParameters, 5, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
	{
		static char const* Name() { return "Smoothing"; }

		static float DefaultValue() { return 0.4f; }
	};

	// Apply optional length correction after the smoothing step. Set to 0 to disable.
	struct LengthCorrection : Parameters::ParameterDescriptor<PushAwayFromSurfaceParameters, 6, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
	{
		static char const* Name() { return "LengthCorrection"; }

		static float DefaultValue() { return 0.00001f; }
	};

	// Enable optional step to also keep length per segment basis. Overall length correction is applied at any case.
	struct PreserveSegmentsLength : Parameters::ParameterDescriptor<PushAwayFromSurfaceParameters, 7, bool>
	{
		static char const* Name() { return "PreserveSegmentsLength"; }

		static bool DefaultValue() { return true; }
	};

	// Amount of recovery allowed in one step. Decrease for slower recovery. If set to 1.0 strands recover original form in one frame.
	struct RecoveryRate : Parameters::ParameterDescriptor<PushAwayFromSurfaceParameters, 8, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
	{
		static char const* Name() { return "RecoveryRate"; }

		static float DefaultValue() { return 0.02f; }
	};

	// Set plasticity to leave an imprint of the collision object. The strand is fully recovers if plasticity is 0.0. The strand is kept in its state of maximum bend if plasticity is 1.
	struct Plasticity : Parameters::ParameterDescriptor<PushAwayFromSurfaceParameters, 9, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
	{
		static char const* Name() { return "Plasticity"; }

		static float DefaultValue() { return 0.0f; }
	};

	// Do not keep distance to the distribution mesh
	struct ForceZeroDistanceToDistMesh : Parameters::ParameterDescriptor<PushAwayFromSurfaceParameters, 10, bool>
	{
		static char const* Name() { return "ForceZeroDistanceToDistMesh"; }

		static bool DefaultValue() { return false; }
	};

	// When enabled, only selected strands will be affected
	struct AffectSelectedOnly : Parameters::ParameterDescriptor<PushAwayFromSurfaceParameters, 12, bool>
	{
		static char const* Name() { return "AffectSelectedOnly"; }

		static bool DefaultValue() { return false; }
	};

	struct TargetCollisionObjects : Parameters::ParameterDescriptor<PushAwayFromSurfaceParameters, 14, Ephere::Ornatrix::PolygonMeshParameter[]>
	{
		static char const* Name() { return "TargetCollisionObjects"; }
	};

	struct CollideWithDistributionMesh : Parameters::ParameterDescriptor<PushAwayFromSurfaceParameters, 15, bool>
	{
		static char const* Name() { return "CollideWithDistributionMesh"; }

		static bool DefaultValue() { return true; }
	};

	struct Descriptor;
	struct Container;
};

} }
