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

struct ScatterParameters
{
	enum { Version = 1 };

	enum { HighestId = 32, FirstUnusedId = 29 };

	static char const* GetName() { return "ScatterParameters"; }

	enum class DistributionMethodType
	{
		Random,
		Channel,
		Map,
	};

	enum class PerStrandChannelsType
	{
		None,
		Input,
		Scattered,
		Both,
	};

	enum class PerStrandTextureCoordinatesType
	{
		Drop,
		FromBase,
		FromScattered,
		Both,
	};

	enum class TargetWidthMethodType
	{
		Reference,
		InputWidth,
		Proportional,
	};

	enum class TextureCoordinateGenerationMethodType
	{
		None,
		CopyReference,
		CopyInput,
		Atlas,
	};

	enum class TextureAtlasGenerationMethodType
	{
		PerReferenceHair,
		PerStrandGroup,
		Channel,
	};

	enum class TextureAtlasScalingMethodType
	{
		None,
		Reference,
		Input,
	};

	enum class OutputShapeMethodType
	{
		Reference,
		Input,
		AdoptInputShape,
		EnvelopAverage,
	};

	struct InputHair : Parameters::ParameterDescriptor<ScatterParameters, 23, Ephere::Ornatrix::HairParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "InputHair"; }
	};

	struct OutputHair : Parameters::ParameterDescriptor<ScatterParameters, 24, Ephere::Ornatrix::HairParameter, Parameters::Direction::Out, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "OutputHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<ScatterParameters, 25, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct ReferenceHairs : Parameters::ParameterDescriptor<ScatterParameters, 26, Ephere::Ornatrix::HairParameter[], Parameters::Direction::In, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "ReferenceHairs"; }
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<ScatterParameters, 1, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};
	};

	struct RandomSeed : Parameters::ParameterDescriptor<ScatterParameters, 2, int, Parameters::Direction::In, Parameters::Purpose::RandomSeed, false, true>
	{
		static char const* Name() { return "RandomSeed"; }

		static int DefaultValue() { return 1; }
	};

	// When enabled, the scattered strands will adopt the shape of the input base strands
	struct UseBaseStrandShape : Parameters::ParameterDescriptor<ScatterParameters, 3, bool>
	{
		static char const* Name() { return "UseBaseStrandShape"; }

		static bool DefaultValue() { return true; }
	};

	struct Distribution
	{
		// Specifies how the propagated strands will be placed on top of base strands
		struct Method : Parameters::ParameterDescriptor<ScatterParameters, 4, DistributionMethodType>
		{
			static char const* Name() { return "Distribution.Method"; }

			static DistributionMethodType DefaultValue() { return DistributionMethodType::Random; }
		};

		struct Map : Parameters::ParameterDescriptor<ScatterParameters, 20, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Distribution.Map"; }
		};

		struct Channel : Parameters::ParameterDescriptor<ScatterParameters, 21, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Distribution.Channel"; }
		};
	};

	struct IgnoredStrands
	{
		// Keep Ignored Strands Channels
		struct Channels : Parameters::ParameterDescriptor<ScatterParameters, 5, bool>
		{
			static char const* Name() { return "IgnoredStrands.Channels"; }

			static bool DefaultValue() { return true; }
		};

		// Keep ignored texture coordinates
		struct TextureCoordinates : Parameters::ParameterDescriptor<ScatterParameters, 6, bool>
		{
			static char const* Name() { return "IgnoredStrands.TextureCoordinates"; }

			static bool DefaultValue() { return true; }
		};
	};

	struct PerVertex
	{
		struct Channels : Parameters::ParameterDescriptor<ScatterParameters, 7, bool>
		{
			static char const* Name() { return "PerVertex.Channels"; }

			static bool DefaultValue() { return true; }
		};

		struct TextureCoordinates : Parameters::ParameterDescriptor<ScatterParameters, 8, bool>
		{
			static char const* Name() { return "PerVertex.TextureCoordinates"; }

			static bool DefaultValue() { return true; }
		};
	};

	struct PerStrand
	{
		struct Channels : Parameters::ParameterDescriptor<ScatterParameters, 9, PerStrandChannelsType>
		{
			static char const* Name() { return "PerStrand.Channels"; }

			static PerStrandChannelsType DefaultValue() { return PerStrandChannelsType::Both; }
		};

		struct TextureCoordinates : Parameters::ParameterDescriptor<ScatterParameters, 10, PerStrandTextureCoordinatesType>
		{
			static char const* Name() { return "PerStrand.TextureCoordinates"; }

			static PerStrandTextureCoordinatesType DefaultValue() { return PerStrandTextureCoordinatesType::Both; }
		};
	};

	struct Separate
	{
		struct PerStrandTextureCoordinates : Parameters::ParameterDescriptor<ScatterParameters, 11, bool>
		{
			static char const* Name() { return "Separate.PerStrandTextureCoordinates"; }

			static bool DefaultValue() { return false; }
		};

		struct PerVertexTextureCoordinates : Parameters::ParameterDescriptor<ScatterParameters, 12, bool>
		{
			static char const* Name() { return "Separate.PerVertexTextureCoordinates"; }

			static bool DefaultValue() { return false; }
		};
	};

	// Determines how scattered hairs widths will be calculated
	struct TargetWidthMethod : Parameters::ParameterDescriptor<ScatterParameters, 13, TargetWidthMethodType>
	{
		static char const* Name() { return "TargetWidthMethod"; }

		static TargetWidthMethodType DefaultValue() { return TargetWidthMethodType::Proportional; }
	};

	// Determines how texture coordinates will be generated for the resulting strands
	struct TextureCoordinateGenerationMethod : Parameters::ParameterDescriptor<ScatterParameters, 14, TextureCoordinateGenerationMethodType>
	{
		static char const* Name() { return "TextureCoordinateGenerationMethod"; }

		static TextureCoordinateGenerationMethodType DefaultValue() { return TextureCoordinateGenerationMethodType::None; }
	};

	struct TextureAtlas
	{
		// Determines how texture atlas will be generated
		struct GenerationMethod : Parameters::ParameterDescriptor<ScatterParameters, 15, TextureAtlasGenerationMethodType>
		{
			static char const* Name() { return "TextureAtlas.GenerationMethod"; }

			static TextureAtlasGenerationMethodType DefaultValue() { return TextureAtlasGenerationMethodType::PerReferenceHair; }
		};

		// Determines the relative scaling of texture islands in the atlas
		struct ScalingMethod : Parameters::ParameterDescriptor<ScatterParameters, 16, TextureAtlasScalingMethodType>
		{
			static char const* Name() { return "TextureAtlas.ScalingMethod"; }

			static TextureAtlasScalingMethodType DefaultValue() { return TextureAtlasScalingMethodType::Reference; }
		};

		// Number of equal sub-intervals in the 0...1 range of channel values that are assigned to different texture islands
		struct MaxIslandCount : Parameters::ParameterDescriptor<ScatterParameters, 17, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "TextureAtlas.MaxIslandCount"; }

			static int DefaultValue() { return 2; }
		};

		struct Channel : Parameters::ParameterDescriptor<ScatterParameters, 22, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "TextureAtlas.Channel"; }
		};
	};

	// When enabled, the strand groups of scattered hairs are based on input strands instead of reference strands
	struct UseInputStrandGroups : Parameters::ParameterDescriptor<ScatterParameters, 18, bool>
	{
		static char const* Name() { return "UseInputStrandGroups"; }

		static bool DefaultValue() { return false; }
	};

	// Determines how the shape of the output strands will be calculated
	struct OutputShapeMethod : Parameters::ParameterDescriptor<ScatterParameters, 19, OutputShapeMethodType>
	{
		static char const* Name() { return "OutputShapeMethod"; }

		static OutputShapeMethodType DefaultValue() { return OutputShapeMethodType::Input; }
	};

	// When enabled, the generated strands will be attached to the surface mesh
	struct GroundScatteredStrands : Parameters::ParameterDescriptor<ScatterParameters, 27, bool>
	{
		static char const* Name() { return "GroundScatteredStrands"; }

		static bool DefaultValue() { return false; }
	};

	// The fraction of strand's points being modified while grounding and pulling
	struct GroundingPointsFraction : Parameters::ParameterDescriptor<ScatterParameters, 28, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "GroundingPointsFraction"; }

		static float DefaultValue() { return 0.0f; }
	};

	// Modify the root's control point of curve being made by pulling
	struct PullingRootControlPointOffset : Parameters::ParameterDescriptor<ScatterParameters, 30, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "PullingRootControlPointOffset"; }

		static float DefaultValue() { return 0.0f; }
	};

	// Modify the tip's control point of curve being made by pulling
	struct PullingTipControlPointOffset : Parameters::ParameterDescriptor<ScatterParameters, 31, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "PullingTipControlPointOffset"; }

		static float DefaultValue() { return 0.0f; }
	};

	// Change the input strand's point that the tip of scattered hair will be pulled toward it
	struct PullingTipPointOffset : Parameters::ParameterDescriptor<ScatterParameters, 32, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "PullingTipPointOffset"; }

		static float DefaultValue() { return 1.0f; }
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::ScatterParameters::DistributionMethodType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::ScatterParameters::DistributionMethodType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case ScatterParameters::DistributionMethodType::Random: return "Random";
			case ScatterParameters::DistributionMethodType::Channel: return "Channel";
			case ScatterParameters::DistributionMethodType::Map: return "Map";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::ScatterParameters::PerStrandChannelsType>
{
	enum { Count = 4 };

	static char const* GetValueName( Ephere::Ornatrix::ScatterParameters::PerStrandChannelsType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case ScatterParameters::PerStrandChannelsType::None: return "None";
			case ScatterParameters::PerStrandChannelsType::Input: return "Input";
			case ScatterParameters::PerStrandChannelsType::Scattered: return "Scattered";
			case ScatterParameters::PerStrandChannelsType::Both: return "Both";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::ScatterParameters::PerStrandTextureCoordinatesType>
{
	enum { Count = 4 };

	static char const* GetValueName( Ephere::Ornatrix::ScatterParameters::PerStrandTextureCoordinatesType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case ScatterParameters::PerStrandTextureCoordinatesType::Drop: return "Drop";
			case ScatterParameters::PerStrandTextureCoordinatesType::FromBase: return "FromBase";
			case ScatterParameters::PerStrandTextureCoordinatesType::FromScattered: return "FromScattered";
			case ScatterParameters::PerStrandTextureCoordinatesType::Both: return "Both";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::ScatterParameters::TargetWidthMethodType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::ScatterParameters::TargetWidthMethodType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case ScatterParameters::TargetWidthMethodType::Reference: return "Reference";
			case ScatterParameters::TargetWidthMethodType::InputWidth: return "InputWidth";
			case ScatterParameters::TargetWidthMethodType::Proportional: return "Proportional";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::ScatterParameters::TextureCoordinateGenerationMethodType>
{
	enum { Count = 4 };

	static char const* GetValueName( Ephere::Ornatrix::ScatterParameters::TextureCoordinateGenerationMethodType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case ScatterParameters::TextureCoordinateGenerationMethodType::None: return "None";
			case ScatterParameters::TextureCoordinateGenerationMethodType::CopyReference: return "CopyReference";
			case ScatterParameters::TextureCoordinateGenerationMethodType::CopyInput: return "CopyInput";
			case ScatterParameters::TextureCoordinateGenerationMethodType::Atlas: return "Atlas";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::ScatterParameters::TextureAtlasGenerationMethodType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::ScatterParameters::TextureAtlasGenerationMethodType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case ScatterParameters::TextureAtlasGenerationMethodType::PerReferenceHair: return "PerReferenceHair";
			case ScatterParameters::TextureAtlasGenerationMethodType::PerStrandGroup: return "PerStrandGroup";
			case ScatterParameters::TextureAtlasGenerationMethodType::Channel: return "Channel";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::ScatterParameters::TextureAtlasScalingMethodType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::ScatterParameters::TextureAtlasScalingMethodType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case ScatterParameters::TextureAtlasScalingMethodType::None: return "None";
			case ScatterParameters::TextureAtlasScalingMethodType::Reference: return "Reference";
			case ScatterParameters::TextureAtlasScalingMethodType::Input: return "Input";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::ScatterParameters::OutputShapeMethodType>
{
	enum { Count = 4 };

	static char const* GetValueName( Ephere::Ornatrix::ScatterParameters::OutputShapeMethodType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case ScatterParameters::OutputShapeMethodType::Reference: return "Reference";
			case ScatterParameters::OutputShapeMethodType::Input: return "Input";
			case ScatterParameters::OutputShapeMethodType::AdoptInputShape: return "AdoptInputShape";
			case ScatterParameters::OutputShapeMethodType::EnvelopAverage: return "EnvelopAverage";
			default: return "";
		}
	}
};
}
