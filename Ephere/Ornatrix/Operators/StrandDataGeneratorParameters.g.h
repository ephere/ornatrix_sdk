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

struct StrandDataGeneratorParameters
{
	enum { Version = 1 };

	enum { HighestId = 24, FirstUnusedId = 25 };

	static char const* GetName() { return "StrandDataGeneratorParameters"; }

	enum class GenerationMethodType
	{
		Constant,
		Random,
		Index,
		StrandLength,
		StrandCurviness,
		StrandDistanceToObject,
		IsInsideObject,
		WorldSpacePosition,
		ObjectSpacePosition,
		SeExpr,
		TextureMap,
	};

	enum class TargetDataType
	{
		ExistingStrandDataChannel,
		NewStrandDataChannel,
		StrandGroups,
		StrandIds,
		Widths,
		TextureCoordinates,
		StrandRotations,
	};

	enum class NewChannelType
	{
		PerStrand,
		PerVertex,
	};

	struct TargetHair : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 22, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct CurrentTime : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 23, float, Parameters::Direction::In, Parameters::Purpose::CurrentTime, true, true>
	{
		static char const* Name() { return "CurrentTime"; }

		static Parameters::TimeUnits const DefaultUnits = Parameters::TimeUnits::Seconds;
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 1, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};
	};

	struct RandomSeed : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 2, int, Parameters::Direction::In, Parameters::Purpose::RandomSeed, false, true>
	{
		static char const* Name() { return "RandomSeed"; }
	};

	// Determines how guide data will be generated. Depending on method chosen different parameters need to be configured.
	struct GenerationMethod : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 3, GenerationMethodType>
	{
		static char const* Name() { return "GenerationMethod"; }

		static GenerationMethodType DefaultValue() { return GenerationMethodType::Constant; }
	};

	struct NewChannel
	{
		// Name assigned to channel created by this operator
		struct ChannelName : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 4, Ephere::Parameters::String>
		{
			static char const* Name() { return "NewChannel.ChannelName"; }

			static char const* DefaultValue() { return "My Channel"; }
		};

		// Type of the new created channel for this operator
		struct Type : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 11, NewChannelType>
		{
			static char const* Name() { return "NewChannel.Type"; }

			static NewChannelType DefaultValue() { return NewChannelType::PerStrand; }
		};
	};

	struct TargetValueRange
	{
		// When enabled, the generated values will be rescaled to be in this range, instead of being between 0 and 1
		struct Enabled : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 5, bool>
		{
			static char const* Name() { return "TargetValueRange.Enabled"; }

			static bool DefaultValue() { return true; }
		};

		// Smallest generated output value
		struct Minimum : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 6, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "TargetValueRange.Minimum"; }

			static float DefaultValue() { return 0; }
		};

		// Largest generated output value
		struct Maximum : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 7, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "TargetValueRange.Maximum"; }

			static float DefaultValue() { return 1; }
		};
	};

	struct SourceValueRange
	{
		// When enabled, used strand values will be clamped with minimum and maximum values
		struct Enabled : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 8, bool>
		{
			static char const* Name() { return "SourceValueRange.Enabled"; }

			static bool DefaultValue() { return false; }
		};

		// Any strand values less than this will be clamped to the minimum target value
		struct Minimum : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 9, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "SourceValueRange.Minimum"; }

			static float DefaultValue() { return 0; }
		};

		// Any strand values larger than this will be clamped to the maximum target value
		struct Maximum : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 10, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "SourceValueRange.Maximum"; }

			static float DefaultValue() { return 1; }
		};
	};

	struct StoredValues
	{
		// When this option is enabled, the current data output is remembered and always used. Use this to freeze the output of this operator. It will also be saved with the scene.
		struct Enabled : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 12, bool>
		{
			static char const* Name() { return "StoredValues.Enabled"; }

			static bool DefaultValue() { return false; }
		};

		// When StoredValues.Enabled is on, the output values of this operator are stored in this array
		struct Values : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 13, float[]>
		{
			static char const* Name() { return "StoredValues.Values"; }
		};

		// Contains strand ids for stored values
		struct StrandIds : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 14, int[]>
		{
			static char const* Name() { return "StoredValues.StrandIds"; }
		};

		// Contains target strand topology information for per-vertex data
		struct FirstVertexIndices : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 15, int[]>
		{
			static char const* Name() { return "StoredValues.FirstVertexIndices"; }
		};
	};

	// Number of channels which should be populated with data by this operator
	struct SampleValueCount : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 16, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "SampleValueCount"; }

		static int DefaultValue() { return 1; }
	};

	// Expression string used to calculate the per-element values
	struct SeExprString : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 17, Ephere::Parameters::String>
	{
		static char const* Name() { return "SeExprString"; }

		static char const* DefaultValue() { return ""; }
	};

	// Determines which part of the hair object will be altered by this operator
	struct TargetData : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 18, TargetDataType>
	{
		static char const* Name() { return "TargetData"; }

		static TargetDataType DefaultValue() { return TargetDataType::NewStrandDataChannel; }
	};

	// Specifies the index of the target texture coordinate channel
	struct TargetChannelIndex : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 19, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "TargetChannelIndex"; }

		static int DefaultValue() { return 0; }
	};

	// Channel where data will be written
	struct TargetChannel : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 20, Ephere::Ornatrix::StrandChannel>
	{
		static char const* Name() { return "TargetChannel"; }
	};

	// Texture map from where to get values. Used with GenerationMethod::TextureMap and SeExpr texture parameter.
	struct SourceTextureMap : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 21, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "SourceTextureMap"; }
	};

	struct TargetObjects : Parameters::ParameterDescriptor<StrandDataGeneratorParameters, 24, Ephere::Ornatrix::PolygonMeshParameter[]>
	{
		static char const* Name() { return "TargetObjects"; }
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::StrandDataGeneratorParameters::GenerationMethodType>
{
	enum { Count = 11 };

	static char const* GetValueName( Ephere::Ornatrix::StrandDataGeneratorParameters::GenerationMethodType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case StrandDataGeneratorParameters::GenerationMethodType::Constant: return "Constant";
			case StrandDataGeneratorParameters::GenerationMethodType::Random: return "Random";
			case StrandDataGeneratorParameters::GenerationMethodType::Index: return "Index";
			case StrandDataGeneratorParameters::GenerationMethodType::StrandLength: return "StrandLength";
			case StrandDataGeneratorParameters::GenerationMethodType::StrandCurviness: return "StrandCurviness";
			case StrandDataGeneratorParameters::GenerationMethodType::StrandDistanceToObject: return "StrandDistanceToObject";
			case StrandDataGeneratorParameters::GenerationMethodType::IsInsideObject: return "IsInsideObject";
			case StrandDataGeneratorParameters::GenerationMethodType::WorldSpacePosition: return "WorldSpacePosition";
			case StrandDataGeneratorParameters::GenerationMethodType::ObjectSpacePosition: return "ObjectSpacePosition";
			case StrandDataGeneratorParameters::GenerationMethodType::SeExpr: return "SeExpr";
			case StrandDataGeneratorParameters::GenerationMethodType::TextureMap: return "TextureMap";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::StrandDataGeneratorParameters::TargetDataType>
{
	enum { Count = 7 };

	static char const* GetValueName( Ephere::Ornatrix::StrandDataGeneratorParameters::TargetDataType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case StrandDataGeneratorParameters::TargetDataType::ExistingStrandDataChannel: return "ExistingStrandDataChannel";
			case StrandDataGeneratorParameters::TargetDataType::NewStrandDataChannel: return "NewStrandDataChannel";
			case StrandDataGeneratorParameters::TargetDataType::StrandGroups: return "StrandGroups";
			case StrandDataGeneratorParameters::TargetDataType::StrandIds: return "StrandIds";
			case StrandDataGeneratorParameters::TargetDataType::Widths: return "Widths";
			case StrandDataGeneratorParameters::TargetDataType::TextureCoordinates: return "TextureCoordinates";
			case StrandDataGeneratorParameters::TargetDataType::StrandRotations: return "StrandRotations";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::StrandDataGeneratorParameters::NewChannelType>
{
	enum { Count = 2 };

	static char const* GetValueName( Ephere::Ornatrix::StrandDataGeneratorParameters::NewChannelType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case StrandDataGeneratorParameters::NewChannelType::PerStrand: return "PerStrand";
			case StrandDataGeneratorParameters::NewChannelType::PerVertex: return "PerVertex";
			default: return "";
		}
	}
};
}
