// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"

namespace Ephere { namespace Ornatrix {
struct PolygonMeshParameter;
} }

namespace Ephere { namespace Ornatrix {

struct RootGeneratorParameters
{
	enum { Version = 1 };

	enum { HighestId = 10, FirstUnusedId = 11 };

	static char const* GetName() { return "RootGeneratorParameters"; }

	enum class GenerationMethodType
	{
		Uniform,
		RandomUV,
		RandomArea,
		RandomFace,
		Vertex,
		GuidesAsHair,
		FaceCenter,
		Even,
	};

	// Method used for generating roots onto a surface
	struct GenerationMethod : Parameters::ParameterDescriptor<RootGeneratorParameters, 1, GenerationMethodType>
	{
		static char const* Name() { return "GenerationMethod"; }

		static GenerationMethodType DefaultValue() { return GenerationMethodType::Even; }
	};

	struct RootCount : Parameters::ParameterDescriptor<RootGeneratorParameters, 2, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "RootCount"; }

		static int DefaultValue() { return 300; }
	};

	struct RandomSeed : Parameters::ParameterDescriptor<RootGeneratorParameters, 3, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "RandomSeed"; }

		static int DefaultValue() { return 1; }
	};

	// Determines if only faces specified in FaceInclude set are to be included
	struct UseFaceInclude : Parameters::ParameterDescriptor<RootGeneratorParameters, 4, bool>
	{
		static char const* Name() { return "UseFaceInclude"; }
	};

	// If useFaceInclude is true, contains a set of faces on which roots are to be generated
	struct FaceInclude : Parameters::ParameterDescriptor<RootGeneratorParameters, 5, bool[]>
	{
		static char const* Name() { return "FaceInclude"; }
	};

	struct UvChannel : Parameters::ParameterDescriptor<RootGeneratorParameters, 6, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "UvChannel"; }

		static int DefaultValue() { return 0; }
	};

	/* When this option is enabled, all root generation will be performed on a single thread and strand ids will be the index of the root as it is generated.
	Used for legacy scene compatibility. */
	struct UseSequentialStrandIds : Parameters::ParameterDescriptor<RootGeneratorParameters, 7, bool>
	{
		static char const* Name() { return "UseSequentialStrandIds"; }

		static bool DefaultValue() { return false; }
	};

	/* When enabled, the exact specified root count will be enforced. This might, however, result in more roots on some faces than others.
	This is useful when operations down the pipeline depend on there being an exact root count, for example, if pre-computed point cache is used on newly generated hair.
	This should only be used on guides or sparse roots as it will potentially plant a lot of roots on some faces while having fewer on others. */
	struct ForceExactRootCount : Parameters::ParameterDescriptor<RootGeneratorParameters, 8, bool>
	{
		static char const* Name() { return "ForceExactRootCount"; }

		static bool DefaultValue() { return false; }
	};

	struct UniformDistributionRandomness : Parameters::ParameterDescriptor<RootGeneratorParameters, 9, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "UniformDistributionRandomness"; }

		static float DefaultValue() { return 0; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<RootGeneratorParameters, 10, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::RootGeneratorParameters::GenerationMethodType>
{
	enum { Count = 8 };

	static char const* GetValueName( Ephere::Ornatrix::RootGeneratorParameters::GenerationMethodType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case RootGeneratorParameters::GenerationMethodType::Uniform: return "Uniform";
			case RootGeneratorParameters::GenerationMethodType::RandomUV: return "RandomUV";
			case RootGeneratorParameters::GenerationMethodType::RandomArea: return "RandomArea";
			case RootGeneratorParameters::GenerationMethodType::RandomFace: return "RandomFace";
			case RootGeneratorParameters::GenerationMethodType::Vertex: return "Vertex";
			case RootGeneratorParameters::GenerationMethodType::GuidesAsHair: return "GuidesAsHair";
			case RootGeneratorParameters::GenerationMethodType::FaceCenter: return "FaceCenter";
			case RootGeneratorParameters::GenerationMethodType::Even: return "Even";
			default: return "";
		}
	}
};
}
