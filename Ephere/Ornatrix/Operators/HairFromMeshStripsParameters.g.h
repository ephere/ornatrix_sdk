// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"
#include "Ephere/Ornatrix/ParameterComponents.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
class Ramp;
} }

namespace Ephere { namespace Ornatrix {

struct HairFromMeshStripsParameters
{
	enum { Version = 1 };

	enum { HighestId = 26, FirstUnusedId = 27 };

	static char const* GetName() { return "HairFromMeshStripsParameters"; }

	enum class DistributionMethodType
	{
		PerVertex,
		Uniform,
		Random,
	};

	struct DistributionMethod : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 1, DistributionMethodType>
	{
		static char const* Name() { return "DistributionMethod"; }

		static DistributionMethodType DefaultValue() { return DistributionMethodType::Random; }
	};

	struct HairsPerStrip : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 2, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "HairsPerStrip"; }

		static int DefaultValue() { return 100; }
	};

	struct PointsPerStrand : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 3, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "PointsPerStrand"; }

		static int DefaultValue() { return 5; }
	};

	struct InheritTextureCoordinatesFromStrips : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 4, bool>
	{
		static char const* Name() { return "InheritTextureCoordinatesFromStrips"; }

		static bool DefaultValue() { return true; }
	};

	struct Volume : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 5, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "Volume"; }

		static float DefaultValue() { return 0; }
	};

	struct VolumeNormalBias : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 6, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "VolumeNormalBias"; }

		static float DefaultValue() { return 0; }
	};

	struct RandomSeed : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 7, int, Parameters::Direction::In, Parameters::Purpose::RandomSeed, false, true>
	{
		static char const* Name() { return "RandomSeed"; }

		static int DefaultValue() { return 1; }
	};

	// When enabled, a per-strand channel containing each hair's source mesh strip id will be created
	struct AddStripIdChannel : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 8, bool>
	{
		static char const* Name() { return "AddStripIdChannel"; }

		static bool DefaultValue() { return false; }
	};

	struct IsUsingMaterialIdsAsStrandGroups : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 9, bool>
	{
		static char const* Name() { return "IsUsingMaterialIdsAsStrandGroups"; }

		static bool DefaultValue() { return false; }
	};

	struct HairsPerStripFraction : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 10, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
	{
		static char const* Name() { return "HairsPerStripFraction"; }

		static float DefaultValue() { return 1.0f; }
	};

	struct GlobalVolumeDiagram : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 11, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "GlobalVolumeDiagram"; }

		static char const* DefaultValue() { return "0 0 0.5 0.5 1 1"; }
	};

	struct OutputHair : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 25, Ephere::Ornatrix::HairParameter, Parameters::Direction::Out, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "OutputHair"; }
	};

	struct objectToWorldTransform : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 26, Ephere::Ornatrix::Xform3, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "objectToWorldTransform"; }
	};

	struct StripMeshes
	{
		struct Mesh : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 23, Ephere::Ornatrix::PolygonMeshParameter[]>
		{
			static char const* Name() { return "StripMeshes.Mesh"; }
		};

		struct UniqueId : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 24, int[]>
		{
			static char const* Name() { return "StripMeshes.UniqueId"; }
		};
	};

	struct RootBoundaryIndices : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 12, int[]>
	{
		static char const* Name() { return "RootBoundaryIndices"; }
	};

	// Strip group overrides
	struct StripsGroup
	{
		struct StripId : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 13, std::int64_t[]>
		{
			static char const* Name() { return "StripsGroup.StripId"; }
		};

		struct GroupIndex : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 14, int[]>
		{
			static char const* Name() { return "StripsGroup.GroupIndex"; }
		};
	};

	// Strip volume overrides
	struct StripsVolume
	{
		struct StripId : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 15, std::int64_t[]>
		{
			static char const* Name() { return "StripsVolume.StripId"; }
		};

		struct Volume : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 16, float[]>
		{
			static char const* Name() { return "StripsVolume.Volume"; }
		};
	};

	// Strip volume diagram overrides
	struct StripsVolumeDiagram
	{
		struct StripId : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 17, std::int64_t[]>
		{
			static char const* Name() { return "StripsVolumeDiagram.StripId"; }
		};

		struct Diagram : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 18, Ephere::Ornatrix::Ramp[]>
		{
			static char const* Name() { return "StripsVolumeDiagram.Diagram"; }
		};
	};

	// Strip hair count overrides
	struct StripsHairCount
	{
		struct StripId : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 19, std::int64_t[]>
		{
			static char const* Name() { return "StripsHairCount.StripId"; }
		};

		struct HairCount : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 20, int[]>
		{
			static char const* Name() { return "StripsHairCount.HairCount"; }
		};
	};

	// Strip distribution method overrides
	struct StripsDistributionMethod
	{
		struct StripId : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 21, std::int64_t[]>
		{
			static char const* Name() { return "StripsDistributionMethod.StripId"; }
		};

		struct DistributionMethod : Parameters::ParameterDescriptor<HairFromMeshStripsParameters, 22, DistributionMethodType[]>
		{
			static char const* Name() { return "StripsDistributionMethod.DistributionMethod"; }
		};
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::HairFromMeshStripsParameters::DistributionMethodType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::HairFromMeshStripsParameters::DistributionMethodType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case HairFromMeshStripsParameters::DistributionMethodType::PerVertex: return "PerVertex";
			case HairFromMeshStripsParameters::DistributionMethodType::Uniform: return "Uniform";
			case HairFromMeshStripsParameters::DistributionMethodType::Random: return "Random";
			default: return "";
		}
	}
};
}
