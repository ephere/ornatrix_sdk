// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
} }

namespace Ephere { namespace Ornatrix {

struct SmoothSurfaceParameters
{
	enum { Version = 1 };

	enum { HighestId = 9, FirstUnusedId = 10 };

	static char const* GetName() { return "SmoothSurfaceParameters"; }

	enum class StrandDirectionType
	{
		Original,
		SmoothNormal,
		FaceNormal,
	};

	enum class BoundaryInterpolationType
	{
		None,
		EdgesOnly,
		EdgesAndCorners,
	};

	enum class UvInterpolationType
	{
		Smooth,
		SharpCornersOnly,
		SharpCornersPlus1,
		SharpCornersPlus2,
		SharpBoundaries,
		LinearAll,
	};

	struct TargetHair : Parameters::ParameterDescriptor<SmoothSurfaceParameters, 9, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct SubdivisionLevel
	{
		struct Viewport : Parameters::ParameterDescriptor<SmoothSurfaceParameters, 1, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "SubdivisionLevel.Viewport"; }

			static int DefaultValue() { return 0; }
		};

		struct Render : Parameters::ParameterDescriptor<SmoothSurfaceParameters, 2, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "SubdivisionLevel.Render"; }

			static int DefaultValue() { return 0; }
		};

		struct UseRenderValue : Parameters::ParameterDescriptor<SmoothSurfaceParameters, 3, bool>
		{
			static char const* Name() { return "SubdivisionLevel.UseRenderValue"; }

			static bool DefaultValue() { return true; }
		};
	};

	// Defines how strand direction is determined after subdivision
	struct StrandDirection : Parameters::ParameterDescriptor<SmoothSurfaceParameters, 4, StrandDirectionType>
	{
		static char const* Name() { return "StrandDirection"; }

		static StrandDirectionType DefaultValue() { return StrandDirectionType::Original; }
	};

	// Defines the limit surface interpolation to boundary edges
	struct BoundaryInterpolation : Parameters::ParameterDescriptor<SmoothSurfaceParameters, 5, BoundaryInterpolationType>
	{
		static char const* Name() { return "BoundaryInterpolation"; }

		static BoundaryInterpolationType DefaultValue() { return BoundaryInterpolationType::EdgesAndCorners; }
	};

	// Defines the interpolation of face-varying data
	struct UvInterpolation : Parameters::ParameterDescriptor<SmoothSurfaceParameters, 6, UvInterpolationType>
	{
		static char const* Name() { return "UvInterpolation"; }

		static UvInterpolationType DefaultValue() { return UvInterpolationType::LinearAll; }
	};

	struct ChaikinCreases : Parameters::ParameterDescriptor<SmoothSurfaceParameters, 7, bool>
	{
		static char const* Name() { return "ChaikinCreases"; }

		static bool DefaultValue() { return false; }
	};

	struct SmoothTriangles : Parameters::ParameterDescriptor<SmoothSurfaceParameters, 8, bool>
	{
		static char const* Name() { return "SmoothTriangles"; }

		static bool DefaultValue() { return false; }
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::SmoothSurfaceParameters::StrandDirectionType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::SmoothSurfaceParameters::StrandDirectionType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case SmoothSurfaceParameters::StrandDirectionType::Original: return "Original";
			case SmoothSurfaceParameters::StrandDirectionType::SmoothNormal: return "SmoothNormal";
			case SmoothSurfaceParameters::StrandDirectionType::FaceNormal: return "FaceNormal";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::SmoothSurfaceParameters::BoundaryInterpolationType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::SmoothSurfaceParameters::BoundaryInterpolationType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case SmoothSurfaceParameters::BoundaryInterpolationType::None: return "None";
			case SmoothSurfaceParameters::BoundaryInterpolationType::EdgesOnly: return "EdgesOnly";
			case SmoothSurfaceParameters::BoundaryInterpolationType::EdgesAndCorners: return "EdgesAndCorners";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::SmoothSurfaceParameters::UvInterpolationType>
{
	enum { Count = 6 };

	static char const* GetValueName( Ephere::Ornatrix::SmoothSurfaceParameters::UvInterpolationType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case SmoothSurfaceParameters::UvInterpolationType::Smooth: return "Smooth";
			case SmoothSurfaceParameters::UvInterpolationType::SharpCornersOnly: return "SharpCornersOnly";
			case SmoothSurfaceParameters::UvInterpolationType::SharpCornersPlus1: return "SharpCornersPlus1";
			case SmoothSurfaceParameters::UvInterpolationType::SharpCornersPlus2: return "SharpCornersPlus2";
			case SmoothSurfaceParameters::UvInterpolationType::SharpBoundaries: return "SharpBoundaries";
			case SmoothSurfaceParameters::UvInterpolationType::LinearAll: return "LinearAll";
			default: return "";
		}
	}
};
}
