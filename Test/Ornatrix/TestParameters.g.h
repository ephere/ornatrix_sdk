// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"
#include "Ephere/Ornatrix/AIPhysics/AiModel.h"
#include "Ephere/Ornatrix/ParameterComponents.h"

namespace Ephere { namespace Geometry {
struct SurfacePosition;
} }

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
class Ramp;
struct StrandChannel;
struct TextureMapParameter;
} }

namespace Ephere { namespace Ornatrix {

struct TestParameters
{
	enum { Version = 1 };

	enum { HighestId = 37, FirstUnusedId = 38 };

	static char const* GetName() { return "TestParameters"; }

	enum class WaveformType
	{
		Sine,
		Helix,
	};

	// Defines how the strands are curled along their axis
	struct Waveform : Parameters::ParameterDescriptor<TestParameters, 1, WaveformType>
	{
		static char const* Name() { return "Waveform"; }

		static WaveformType DefaultValue() { return WaveformType::Helix; }
	};

	struct Magnitude
	{
		struct Value : Parameters::ParameterDescriptor<TestParameters, 3, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Magnitude.Value"; }

			static float DefaultValue() { return 1.0f; }
		};

		struct Channel : Parameters::ParameterDescriptor<TestParameters, 4, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Magnitude.Channel"; }
		};

		// A nested group of arrays
		struct Sinks
		{
			struct Ramps : Parameters::ParameterDescriptor<TestParameters, 6, Ephere::Ornatrix::Ramp[]>
			{
				static char const* Name() { return "Magnitude.Sinks.Ramps"; }
			};

			struct Positions : Parameters::ParameterDescriptor<TestParameters, 7, Ephere::Geometry::SurfacePosition[]>
			{
				static char const* Name() { return "Magnitude.Sinks.Positions"; }
			};
		};
	};

	struct Phase
	{
		struct Value : Parameters::ParameterDescriptor<TestParameters, 8, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "Phase.Value"; }

			static float DefaultValue() { return 2.0f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};

		struct Channel : Parameters::ParameterDescriptor<TestParameters, 9, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Phase.Channel"; }
		};

		struct Map : Parameters::ParameterDescriptor<TestParameters, 10, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Phase.Map"; }
		};
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<TestParameters, 11, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};

		struct BlendDistance : Parameters::ParameterDescriptor<TestParameters, 13, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "StrandGroup.BlendDistance"; }

			static float DefaultValue() { return 0; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};
	};

	struct Examples
	{
		struct AuxExample : Parameters::ParameterDescriptor<TestParameters, 14, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Examples.AuxExample"; }
		};

		// The value should not be serialized
		struct TransientExample : Parameters::ParameterDescriptor<TestParameters, 33, int, Parameters::Direction::In, Parameters::Purpose::Undefined, true, true>
		{
			static char const* Name() { return "Examples.TransientExample"; }
		};

		struct FractionExample : Parameters::ParameterDescriptor<TestParameters, 15, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Examples.FractionExample"; }

			static float DefaultValue() { return 0.1f; }
		};

		struct VectorDistanceExample : Parameters::ParameterDescriptor<TestParameters, 2, Ephere::Ornatrix::Vector3, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "Examples.VectorDistanceExample"; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};

		struct VectorDirectionExample : Parameters::ParameterDescriptor<TestParameters, 5, Ephere::Ornatrix::Vector3, Parameters::Direction::In, Parameters::Purpose::Direction, false, true>
		{
			static char const* Name() { return "Examples.VectorDirectionExample"; }
		};

		struct XformExample : Parameters::ParameterDescriptor<TestParameters, 31, Ephere::Ornatrix::Xform3, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Examples.XformExample"; }
		};

		struct ExternalEnumParameter : Parameters::ParameterDescriptor<TestParameters, 12, Ephere::Ornatrix::AiPhysics::AlgorithmType>
		{
			static char const* Name() { return "Examples.ExternalEnumParameter"; }
		};

		struct CustomTypeExample : Parameters::ParameterDescriptor<TestParameters, 17, Groom::Operators::SomeData>
		{
			static char const* Name() { return "Examples.CustomTypeExample"; }

			static Groom::Operators::SomeData DefaultValue() { return Groom::Operators::SomeData(); }
		};

		struct CustomSharedExample : Parameters::ParameterDescriptor<TestParameters, 30, SharedPtr<Groom::Operators::SomeData>>
		{
			static char const* Name() { return "Examples.CustomSharedExample"; }

			static SharedPtr<Groom::Operators::SomeData> DefaultValue() { return SharedPtr<Groom::Operators::SomeData>::DefaultConstruct(); }
		};

		struct BoolExample : Parameters::ParameterDescriptor<TestParameters, 18, bool, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Examples.BoolExample"; }

			static bool DefaultValue() { return true; }
		};

		struct IntExample : Parameters::ParameterDescriptor<TestParameters, 26, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Examples.IntExample"; }

			static int DefaultValue() { return true; }
		};

		struct StringExample : Parameters::ParameterDescriptor<TestParameters, 27, Ephere::Parameters::String>
		{
			static char const* Name() { return "Examples.StringExample"; }

			static char const* DefaultValue() { return ""; }
		};

		struct RampExample : Parameters::ParameterDescriptor<TestParameters, 20, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Examples.RampExample"; }

			static char const* DefaultValue() { return "0 0 1 1"; }
		};

		struct MeshExample : Parameters::ParameterDescriptor<TestParameters, 22, Ephere::Ornatrix::PolygonMeshParameter>
		{
			static char const* Name() { return "Examples.MeshExample"; }
		};

		struct TargetHairExample : Parameters::ParameterDescriptor<TestParameters, 24, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
		{
			static char const* Name() { return "Examples.TargetHairExample"; }
		};
	};

	struct ArrayExamples
	{
		struct VectorsExample : Parameters::ParameterDescriptor<TestParameters, 16, Ephere::Ornatrix::Vector3[]>
		{
			static char const* Name() { return "ArrayExamples.VectorsExample"; }
		};

		struct XformsExample : Parameters::ParameterDescriptor<TestParameters, 32, Ephere::Ornatrix::Xform3[]>
		{
			static char const* Name() { return "ArrayExamples.XformsExample"; }
		};

		struct BoolsExample : Parameters::ParameterDescriptor<TestParameters, 21, bool[]>
		{
			static char const* Name() { return "ArrayExamples.BoolsExample"; }
		};

		struct EnumsExample : Parameters::ParameterDescriptor<TestParameters, 25, WaveformType[]>
		{
			static char const* Name() { return "ArrayExamples.EnumsExample"; }
		};

		struct IntsExample : Parameters::ParameterDescriptor<TestParameters, 28, int[]>
		{
			static char const* Name() { return "ArrayExamples.IntsExample"; }
		};

		struct MeshesExample : Parameters::ParameterDescriptor<TestParameters, 23, Ephere::Ornatrix::PolygonMeshParameter[]>
		{
			static char const* Name() { return "ArrayExamples.MeshesExample"; }
		};

		struct StringsExample : Parameters::ParameterDescriptor<TestParameters, 29, Ephere::Parameters::String[]>
		{
			static char const* Name() { return "ArrayExamples.StringsExample"; }
		};
	};

	struct ArrayOfStructExample
	{
		struct Real : Parameters::ParameterDescriptor<TestParameters, 34, float[]>
		{
			static char const* Name() { return "ArrayOfStructExample.Real"; }
		};

		struct Integer : Parameters::ParameterDescriptor<TestParameters, 35, int[]>
		{
			static char const* Name() { return "ArrayOfStructExample.Integer"; }
		};
	};

	struct MapExample
	{
		struct StripId : Parameters::ParameterDescriptor<TestParameters, 36, std::int64_t[]>
		{
			static char const* Name() { return "MapExample.StripId"; }
		};

		struct Volume : Parameters::ParameterDescriptor<TestParameters, 37, float[]>
		{
			static char const* Name() { return "MapExample.Volume"; }
		};
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::TestParameters::WaveformType>
{
	enum { Count = 2 };

	static char const* GetValueName( Ephere::Ornatrix::TestParameters::WaveformType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case TestParameters::WaveformType::Sine: return "Sine";
			case TestParameters::WaveformType::Helix: return "Helix";
			default: return "";
		}
	}
};
}
