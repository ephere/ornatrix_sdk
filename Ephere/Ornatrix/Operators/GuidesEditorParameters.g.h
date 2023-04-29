// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"
#include "Ephere/Ornatrix/Private/GuidesDelta.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
class Ramp;
struct StrandChannel;
struct TextureMapParameter;
} }

namespace Ephere { namespace Ornatrix {

struct GuidesEditorParameters
{
	enum { Version = 1 };

	enum { HighestId = 18, FirstUnusedId = 19 };

	static char const* GetName() { return "GuidesEditorParameters"; }

	enum class InterpolationType
	{
		ClosestNeighbor,
		Triangulation,
	};

	enum class ChangeTrackingType
	{
		Surface,
		Strand,
	};

	struct TargetHair : Parameters::ParameterDescriptor<GuidesEditorParameters, 8, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<GuidesEditorParameters, 9, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	// Controls how much this delta is applied to target strands
	struct Amount
	{
		struct Value : Parameters::ParameterDescriptor<GuidesEditorParameters, 1, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Amount.Value"; }

			static float DefaultValue() { return 1.0f; }
		};

		struct Channel : Parameters::ParameterDescriptor<GuidesEditorParameters, 4, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Amount.Channel"; }
		};

		struct Ramp : Parameters::ParameterDescriptor<GuidesEditorParameters, 2, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Amount.Ramp"; }

			static char const* DefaultValue() { return "0 1 1 1"; }
		};

		struct Map : Parameters::ParameterDescriptor<GuidesEditorParameters, 3, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Amount.Map"; }
		};
	};

	struct CreatedStrand
	{
		// Length of new strands created by this operator
		struct Length : Parameters::ParameterDescriptor<GuidesEditorParameters, 5, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "CreatedStrand.Length"; }

			static float DefaultValue() { return 10.0f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};

		// Number of points in new strands created by this operator
		struct PointCount : Parameters::ParameterDescriptor<GuidesEditorParameters, 6, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "CreatedStrand.PointCount"; }

			static int DefaultValue() { return 10; }
		};
	};

	// Determines how changes made to hair are calculated, stored, and applied by this delta
	struct ChangeTracking : Parameters::ParameterDescriptor<GuidesEditorParameters, 14, ChangeTrackingType>
	{
		static char const* Name() { return "ChangeTracking"; }

		static ChangeTrackingType DefaultValue() { return ChangeTrackingType::Surface; }
	};

	struct Interpolation
	{
		// When multi-resolution mode is enabled, determines how interpolation is performed
		struct Changes : Parameters::ParameterDescriptor<GuidesEditorParameters, 10, InterpolationType>
		{
			static char const* Name() { return "Interpolation.Changes"; }

			static InterpolationType DefaultValue() { return InterpolationType::ClosestNeighbor; }
		};

		// When multi-resolution mode is enabled, determines how interpolation is performed for created strands
		struct InputRoot : Parameters::ParameterDescriptor<GuidesEditorParameters, 11, InterpolationType>
		{
			static char const* Name() { return "Interpolation.InputRoot"; }

			static InterpolationType DefaultValue() { return InterpolationType::ClosestNeighbor; }
		};
	};

	struct CollisionResolution
	{
		// Number of iterations for collision resolution
		struct IterationCount : Parameters::ParameterDescriptor<GuidesEditorParameters, 12, int>
		{
			static char const* Name() { return "CollisionResolution.IterationCount"; }

			static int DefaultValue() { return 0; }
		};

		// Damping coefficient for the collision resolution simulation
		struct Damping : Parameters::ParameterDescriptor<GuidesEditorParameters, 13, float>
		{
			static char const* Name() { return "CollisionResolution.Damping"; }

			static float DefaultValue() { return 0; }
		};

		// When enabled, the simulation is updated with only the selected strands, and iterations act as substeps for this movement
		struct EnableSubsteps : Parameters::ParameterDescriptor<GuidesEditorParameters, 15, bool>
		{
			static char const* Name() { return "CollisionResolution.EnableSubsteps"; }

			static bool DefaultValue() { return false; }
		};
	};

	struct SoftSelection
	{
		// When enabled, soft selection of control points and roots will be used
		struct Enabled : Parameters::ParameterDescriptor<GuidesEditorParameters, 16, bool>
		{
			static char const* Name() { return "SoftSelection.Enabled"; }

			static bool DefaultValue() { return false; }
		};

		// Specifies the number of neighboring guides which will be selected around user selection when "Use Soft Selection" is on
		struct NeighborCount : Parameters::ParameterDescriptor<GuidesEditorParameters, 17, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "SoftSelection.NeighborCount"; }

			static int DefaultValue() { return 5; }
		};

		// Specifies the falloff distance when "Use Soft Selection" is on
		struct Distance : Parameters::ParameterDescriptor<GuidesEditorParameters, 18, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "SoftSelection.Distance"; }

			static float DefaultValue() { return 99.0f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};
	};

	struct Delta : Parameters::ParameterDescriptor<GuidesEditorParameters, 7, SharedPtr<GuidesDelta>>
	{
		static char const* Name() { return "Delta"; }
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::GuidesEditorParameters::InterpolationType>
{
	enum { Count = 2 };

	static char const* GetValueName( Ephere::Ornatrix::GuidesEditorParameters::InterpolationType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case GuidesEditorParameters::InterpolationType::ClosestNeighbor: return "ClosestNeighbor";
			case GuidesEditorParameters::InterpolationType::Triangulation: return "Triangulation";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::GuidesEditorParameters::ChangeTrackingType>
{
	enum { Count = 2 };

	static char const* GetValueName( Ephere::Ornatrix::GuidesEditorParameters::ChangeTrackingType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case GuidesEditorParameters::ChangeTrackingType::Surface: return "Surface";
			case GuidesEditorParameters::ChangeTrackingType::Strand: return "Strand";
			default: return "";
		}
	}
};
}
