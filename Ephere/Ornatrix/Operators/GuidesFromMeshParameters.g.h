// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct StrandChannel;
struct TextureMapParameter;
} }

namespace Ephere { namespace Ornatrix {

struct GuidesFromMeshParameters
{
	enum { Version = 1 };

	enum { HighestId = 8, FirstUnusedId = 9 };

	static char const* GetName() { return "GuidesFromMeshParameters"; }

	enum class SurfaceTangentComputeMethodType
	{
		ObjectSpace,
		FirstEdge,
		TextureCoordinate,
		Geodesic,
	};

	struct OutputGuides : Parameters::ParameterDescriptor<GuidesFromMeshParameters, 7, Ephere::Ornatrix::HairParameter, Parameters::Direction::Out, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "OutputGuides"; }
	};

	struct GuideLength : Parameters::ParameterDescriptor<GuidesFromMeshParameters, 1, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
	{
		static char const* Name() { return "GuideLength"; }

		static float DefaultValue() { return 50.0f; }

		static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
	};

	struct LengthRandomness : Parameters::ParameterDescriptor<GuidesFromMeshParameters, 2, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
	{
		static char const* Name() { return "LengthRandomness"; }

		static float DefaultValue() { return 0.2f; }
	};

	struct PointCount : Parameters::ParameterDescriptor<GuidesFromMeshParameters, 3, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "PointCount"; }

		static int DefaultValue() { return 10; }
	};

	struct DistributionChannel : Parameters::ParameterDescriptor<GuidesFromMeshParameters, 4, Ephere::Ornatrix::StrandChannel>
	{
		static char const* Name() { return "DistributionChannel"; }
	};

	struct DistributionMap : Parameters::ParameterDescriptor<GuidesFromMeshParameters, 5, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "DistributionMap"; }
	};

	// When enabled, roots will be generated on mesh triangles instead of polygons. This is used for backward compatibility.
	struct UseTriangularRootGeneration : Parameters::ParameterDescriptor<GuidesFromMeshParameters, 6, bool>
	{
		static char const* Name() { return "UseTriangularRootGeneration"; }

		static bool DefaultValue() { return false; }
	};

	// Determines how the per-strand transform up directions are computed on base surface
	struct SurfaceTangentComputeMethod : Parameters::ParameterDescriptor<GuidesFromMeshParameters, 8, SurfaceTangentComputeMethodType>
	{
		static char const* Name() { return "SurfaceTangentComputeMethod"; }

		static SurfaceTangentComputeMethodType DefaultValue() { return SurfaceTangentComputeMethodType::TextureCoordinate; }
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::GuidesFromMeshParameters::SurfaceTangentComputeMethodType>
{
	enum { Count = 4 };

	static char const* GetValueName( Ephere::Ornatrix::GuidesFromMeshParameters::SurfaceTangentComputeMethodType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case GuidesFromMeshParameters::SurfaceTangentComputeMethodType::ObjectSpace: return "ObjectSpace";
			case GuidesFromMeshParameters::SurfaceTangentComputeMethodType::FirstEdge: return "FirstEdge";
			case GuidesFromMeshParameters::SurfaceTangentComputeMethodType::TextureCoordinate: return "TextureCoordinate";
			case GuidesFromMeshParameters::SurfaceTangentComputeMethodType::Geodesic: return "Geodesic";
			default: return "";
		}
	}
};
}
