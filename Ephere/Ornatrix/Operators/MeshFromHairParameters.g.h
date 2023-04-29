// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"
#include "Ephere/Geometry/Native/MatrixUtilities.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
class Ramp;
struct StrandChannel;
struct TextureMapParameter;
} }

namespace Ephere { namespace Ornatrix {

struct MeshFromHairParameters
{
	enum { Version = 1 };

	enum { HighestId = 206, FirstUnusedId = 21 };

	static char const* GetName() { return "MeshFromHairParameters"; }

	enum class MeshType
	{
		Cylindrical,
		Ribbon,
		ProxyMesh,
	};

	enum class ConformToSurfaceType
	{
		IgnoreSurface,
		SurfaceNormal,
	};

	enum class MappingCoordinatesMethod
	{
		PerVertex,
		PerSegment,
		PerEdge,
		PerStrand,
		GuideMesh,
		Inherit,
	};

	struct InputHair : Parameters::ParameterDescriptor<MeshFromHairParameters, 1, Ephere::Ornatrix::HairParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "InputHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<MeshFromHairParameters, 2, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct OutputMesh : Parameters::ParameterDescriptor<MeshFromHairParameters, 3, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::Out>
	{
		static char const* Name() { return "OutputMesh"; }
	};

	struct ResultGeometryType : Parameters::ParameterDescriptor<MeshFromHairParameters, 4, MeshType>
	{
		static char const* Name() { return "ResultGeometryType"; }

		static MeshType DefaultValue() { return MeshType::Ribbon; }
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<MeshFromHairParameters, 5, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};

		struct BlendDistance : Parameters::ParameterDescriptor<MeshFromHairParameters, 6, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "StrandGroup.BlendDistance"; }

			static float DefaultValue() { return 0; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};
	};

	struct RandomSeed : Parameters::ParameterDescriptor<MeshFromHairParameters, 7, int, Parameters::Direction::In, Parameters::Purpose::RandomSeed, false, true>
	{
		static char const* Name() { return "RandomSeed"; }

		static int DefaultValue() { return 1; }
	};

	struct ConformToSurface : Parameters::ParameterDescriptor<MeshFromHairParameters, 8, ConformToSurfaceType>
	{
		static char const* Name() { return "ConformToSurface"; }

		static ConformToSurfaceType DefaultValue() { return ConformToSurfaceType::IgnoreSurface; }
	};

	struct SideCount : Parameters::ParameterDescriptor<MeshFromHairParameters, 9, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "SideCount"; }

		static int DefaultValue() { return 1; }
	};

	struct IsGuideMesh : Parameters::ParameterDescriptor<MeshFromHairParameters, 10, bool>
	{
		static char const* Name() { return "IsGuideMesh"; }
	};

	struct UseTangentsForNormals : Parameters::ParameterDescriptor<MeshFromHairParameters, 11, bool>
	{
		static char const* Name() { return "UseTangentsForNormals"; }
	};

	struct IsCrossSectionClosed : Parameters::ParameterDescriptor<MeshFromHairParameters, 12, bool>
	{
		static char const* Name() { return "IsCrossSectionClosed"; }
	};

	struct UseStrandGroupForMaterialIds : Parameters::ParameterDescriptor<MeshFromHairParameters, 13, bool>
	{
		static char const* Name() { return "UseStrandGroupForMaterialIds"; }
	};

	struct MaterialIdCount : Parameters::ParameterDescriptor<MeshFromHairParameters, 14, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "MaterialIdCount"; }

		static int DefaultValue() { return 1; }
	};

	struct ScatterMaterialIdsRandomly : Parameters::ParameterDescriptor<MeshFromHairParameters, 15, bool>
	{
		static char const* Name() { return "ScatterMaterialIdsRandomly"; }
	};

	struct MaterialIdScatterChannel : Parameters::ParameterDescriptor<MeshFromHairParameters, 16, Ephere::Ornatrix::StrandChannel>
	{
		static char const* Name() { return "MaterialIdScatterChannel"; }
	};

	struct MaterialIdScatterMap : Parameters::ParameterDescriptor<MeshFromHairParameters, 17, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "MaterialIdScatterMap"; }
	};

	// Determines whether edges connecting to consecutive segments should be visible or not
	struct IsHidingConnectingEdges : Parameters::ParameterDescriptor<MeshFromHairParameters, 18, bool>
	{
		static char const* Name() { return "IsHidingConnectingEdges"; }

		static bool DefaultValue() { return true; }
	};

	// When enabled, meshes will not be generated for strands which are hidden in input hair
	struct IgnoreHiddenStrands : Parameters::ParameterDescriptor<MeshFromHairParameters, 19, bool>
	{
		static char const* Name() { return "IgnoreHiddenStrands"; }
	};

	struct CrossSectionShape : Parameters::ParameterDescriptor<MeshFromHairParameters, 20, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "CrossSectionShape"; }

		static char const* DefaultValue() { return "0 0.5 1 0.5"; }
	};

	struct Proxy
	{
		struct Meshes : Parameters::ParameterDescriptor<MeshFromHairParameters, 101, Ephere::Ornatrix::PolygonMeshParameter[]>
		{
			static char const* Name() { return "Proxy.Meshes"; }
		};

		// When enabled, all strand meshes should be rotated to the strand's transform
		struct InheritRotation : Parameters::ParameterDescriptor<MeshFromHairParameters, 102, bool>
		{
			static char const* Name() { return "Proxy.InheritRotation"; }

			static bool DefaultValue() { return true; }
		};

		// When enabled, all proxy strand meshes should conform to the strand's shape
		struct InheritShape : Parameters::ParameterDescriptor<MeshFromHairParameters, 103, bool>
		{
			static char const* Name() { return "Proxy.InheritShape"; }

			static bool DefaultValue() { return true; }
		};

		// When enabled, all proxy strand meshes should use the strand's width
		struct InheritWidth : Parameters::ParameterDescriptor<MeshFromHairParameters, 104, bool>
		{
			static char const* Name() { return "Proxy.InheritWidth"; }

			static bool DefaultValue() { return true; }
		};

		// When enabled, texture information from source proxy mesh will be copied into the destination mesh
		struct InheritTextureCoordinates : Parameters::ParameterDescriptor<MeshFromHairParameters, 105, bool>
		{
			static char const* Name() { return "Proxy.InheritTextureCoordinates"; }

			static bool DefaultValue() { return true; }
		};

		struct IsUniformScale : Parameters::ParameterDescriptor<MeshFromHairParameters, 106, bool>
		{
			static char const* Name() { return "Proxy.IsUniformScale"; }
		};

		// Determines which axis in the proxy mesh will be aligned with the step of the input hair strands
		struct ReferenceUpAxis : Parameters::ParameterDescriptor<MeshFromHairParameters, 107, Geometry::Axis>
		{
			static char const* Name() { return "Proxy.ReferenceUpAxis"; }
		};

		// Index of a channel with values from 0 to 1 determining the index of proxy meshes to use for each strand
		struct DistributionChannel : Parameters::ParameterDescriptor<MeshFromHairParameters, 108, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Proxy.DistributionChannel"; }
		};

		// A map with values from 0 to 1 determining the index of proxy meshes to use for each strand
		struct DistributionMap : Parameters::ParameterDescriptor<MeshFromHairParameters, 109, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Proxy.DistributionMap"; }
		};
	};

	struct MappingChannelCount : Parameters::ParameterDescriptor<MeshFromHairParameters, 200, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "MappingChannelCount"; }

		static int DefaultValue() { return 1; }
	};

	struct MappingChannels
	{
		struct Type : Parameters::ParameterDescriptor<MeshFromHairParameters, 201, MappingCoordinatesMethod[]>
		{
			static char const* Name() { return "MappingChannels.Type"; }
		};

		struct CoordinateStretching : Parameters::ParameterDescriptor<MeshFromHairParameters, 202, float[]>
		{
			static char const* Name() { return "MappingChannels.CoordinateStretching"; }
		};

		struct UsePerStrandCoordinates : Parameters::ParameterDescriptor<MeshFromHairParameters, 203, bool[]>
		{
			static char const* Name() { return "MappingChannels.UsePerStrandCoordinates"; }
		};

		// Per-mapping channel values for whether texture atlas is generated when UsePerStrandCoordinates is true or if mapping coordinates are stretched onto the whole coordinate system
		struct CreateTextureAtlas : Parameters::ParameterDescriptor<MeshFromHairParameters, 204, bool[]>
		{
			static char const* Name() { return "MappingChannels.CreateTextureAtlas"; }
		};

		// If CreateTextureAtlas is true then this determines whether every strand will have its own island or just every strand group
		struct OneTextureIslandPerStrandGroup : Parameters::ParameterDescriptor<MeshFromHairParameters, 205, bool[]>
		{
			static char const* Name() { return "MappingChannels.OneTextureIslandPerStrandGroup"; }
		};

		struct BaseChannel : Parameters::ParameterDescriptor<MeshFromHairParameters, 206, int[]>
		{
			static char const* Name() { return "MappingChannels.BaseChannel"; }
		};
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::MeshFromHairParameters::MeshType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::MeshFromHairParameters::MeshType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case MeshFromHairParameters::MeshType::Cylindrical: return "Cylindrical";
			case MeshFromHairParameters::MeshType::Ribbon: return "Ribbon";
			case MeshFromHairParameters::MeshType::ProxyMesh: return "ProxyMesh";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::MeshFromHairParameters::ConformToSurfaceType>
{
	enum { Count = 2 };

	static char const* GetValueName( Ephere::Ornatrix::MeshFromHairParameters::ConformToSurfaceType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case MeshFromHairParameters::ConformToSurfaceType::IgnoreSurface: return "IgnoreSurface";
			case MeshFromHairParameters::ConformToSurfaceType::SurfaceNormal: return "SurfaceNormal";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::MeshFromHairParameters::MappingCoordinatesMethod>
{
	enum { Count = 6 };

	static char const* GetValueName( Ephere::Ornatrix::MeshFromHairParameters::MappingCoordinatesMethod value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case MeshFromHairParameters::MappingCoordinatesMethod::PerVertex: return "PerVertex";
			case MeshFromHairParameters::MappingCoordinatesMethod::PerSegment: return "PerSegment";
			case MeshFromHairParameters::MappingCoordinatesMethod::PerEdge: return "PerEdge";
			case MeshFromHairParameters::MappingCoordinatesMethod::PerStrand: return "PerStrand";
			case MeshFromHairParameters::MappingCoordinatesMethod::GuideMesh: return "GuideMesh";
			case MeshFromHairParameters::MappingCoordinatesMethod::Inherit: return "Inherit";
			default: return "";
		}
	}
};
}
