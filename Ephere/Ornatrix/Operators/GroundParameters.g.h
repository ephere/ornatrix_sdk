// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"
#include "Ephere/Ornatrix/ParameterComponents.h"

namespace Ephere { namespace Geometry {
struct SurfacePosition;
} }

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
class Ramp;
} }

namespace Ephere { namespace Ornatrix {

struct GroundParameters
{
	enum { Version = 1 };

	enum { HighestId = 17, FirstUnusedId = 18 };

	static char const* GetName() { return "GroundParameters"; }

	struct TargetHair : Parameters::ParameterDescriptor<GroundParameters, 13, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<GroundParameters, 14, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct Set
	{
		// When enabled, applying the grounding will update the locations of each strand's root according to mesh deformation
		struct Positions : Parameters::ParameterDescriptor<GroundParameters, 1, bool>
		{
			static char const* Name() { return "Set.Positions"; }

			static bool DefaultValue() { return true; }
		};

		// When enabled, applying the grounding will rotate each strand according to mesh deformation
		struct Rotations : Parameters::ParameterDescriptor<GroundParameters, 2, bool>
		{
			static char const* Name() { return "Set.Rotations"; }

			static bool DefaultValue() { return true; }
		};

		// When enabled, texture coordinates will be generated and assigned to target hair from its newly grounded surface
		struct TextureCoordinates : Parameters::ParameterDescriptor<GroundParameters, 3, bool>
		{
			static char const* Name() { return "Set.TextureCoordinates"; }

			static bool DefaultValue() { return true; }
		};
	};

	struct PositionBlendRamp : Parameters::ParameterDescriptor<GroundParameters, 4, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "PositionBlendRamp"; }

		static char const* DefaultValue() { return "0 1 0.5 0.5 1 0"; }
	};

	struct OriginalHairTransform : Parameters::ParameterDescriptor<GroundParameters, 15, Ephere::Ornatrix::Xform3, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "OriginalHairTransform"; }
	};

	struct GroundedStrands
	{
		struct SurfacePositions : Parameters::ParameterDescriptor<GroundParameters, 5, Ephere::Geometry::SurfacePosition[]>
		{
			static char const* Name() { return "GroundedStrands.SurfacePositions"; }
		};

		// Stores the array of strand ids for each grounded hair. This array will be used later to look up the correct strand to ground.
		struct StrandIds : Parameters::ParameterDescriptor<GroundParameters, 6, int[]>
		{
			static char const* Name() { return "GroundedStrands.StrandIds"; }
		};

		struct FirstSegmentDirections : Parameters::ParameterDescriptor<GroundParameters, 7, Ephere::Ornatrix::Vector3[], Parameters::Direction::In, Parameters::Purpose::UnitDirection>
		{
			static char const* Name() { return "GroundedStrands.FirstSegmentDirections"; }
		};

		struct RootUpDirections : Parameters::ParameterDescriptor<GroundParameters, 8, Ephere::Ornatrix::Vector3[], Parameters::Direction::In, Parameters::Purpose::UnitDirection>
		{
			static char const* Name() { return "GroundedStrands.RootUpDirections"; }
		};

		// Stores the offset vectors of roots of input hair relative to their grounded positions as they appeared during grounding
		struct InitialRootOffsetsInFaceSpace : Parameters::ParameterDescriptor<GroundParameters, 9, Ephere::Ornatrix::Vector3[], Parameters::Direction::In, Parameters::Purpose::Direction>
		{
			static char const* Name() { return "GroundedStrands.InitialRootOffsetsInFaceSpace"; }
		};

		// This parameter determines whether a strand should be flipped (its root becomes its tip) or not
		struct FlipStrands : Parameters::ParameterDescriptor<GroundParameters, 10, bool[]>
		{
			static char const* Name() { return "GroundedStrands.FlipStrands"; }
		};
	};

	// When enabled, any input strands which haven't been grounded before will be grounded during evaluation
	struct DynamicGrounding : Parameters::ParameterDescriptor<GroundParameters, 11, bool>
	{
		static char const* Name() { return "DynamicGrounding"; }

		static bool DefaultValue() { return true; }
	};

	// When enabled, during grounding the end which is closest to the surface will be considered the strand root
	struct GroundClosestEnd : Parameters::ParameterDescriptor<GroundParameters, 12, bool>
	{
		static char const* Name() { return "GroundClosestEnd"; }

		static bool DefaultValue() { return true; }
	};

	struct GroundTime : Parameters::ParameterDescriptor<GroundParameters, 16, float, Parameters::Direction::In, Parameters::Purpose::Time>
	{
		static char const* Name() { return "GroundTime"; }

		static float DefaultValue() { return 0.0f; }

		static Parameters::TimeUnits const DefaultUnits = Parameters::TimeUnits::Seconds;
	};

	struct IsGroundTimeSet : Parameters::ParameterDescriptor<GroundParameters, 17, bool>
	{
		static char const* Name() { return "IsGroundTimeSet"; }

		static bool DefaultValue() { return false; }
	};

	struct Descriptor;
	struct Container;
};

} }
