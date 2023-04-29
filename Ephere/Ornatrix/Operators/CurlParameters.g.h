// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
class Ramp;
struct StrandChannel;
struct TextureMapParameter;
} }

namespace Ephere { namespace Ornatrix {

struct CurlParameters
{
	enum { Version = 1 };

	enum { HighestId = 23, FirstUnusedId = 24 };

	static char const* GetName() { return "CurlParameters"; }

	enum class WaveformType
	{
		Sine,
		Helix,
	};

	enum class CurlAxis
	{
		X,
		Y,
		Z,
	};

	struct TargetHair : Parameters::ParameterDescriptor<CurlParameters, 19, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<CurlParameters, 20, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	// Defines how the strands are curled along their axis
	struct Waveform : Parameters::ParameterDescriptor<CurlParameters, 1, WaveformType>
	{
		static char const* Name() { return "Waveform"; }

		static WaveformType DefaultValue() { return WaveformType::Helix; }
	};

	struct RandomSeed : Parameters::ParameterDescriptor<CurlParameters, 23, int, Parameters::Direction::In, Parameters::Purpose::RandomSeed, false, true>
	{
		static char const* Name() { return "RandomSeed"; }

		static int DefaultValue() { return 1; }
	};

	struct Magnitude
	{
		struct Value : Parameters::ParameterDescriptor<CurlParameters, 2, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "Magnitude.Value"; }

			static float DefaultValue() { return 1.0f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};

		struct Channel : Parameters::ParameterDescriptor<CurlParameters, 5, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Magnitude.Channel"; }
		};

		struct Ramp : Parameters::ParameterDescriptor<CurlParameters, 3, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Magnitude.Ramp"; }

			static char const* DefaultValue() { return "0 0 0.5 0.5 1 1"; }
		};

		struct Map : Parameters::ParameterDescriptor<CurlParameters, 4, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Magnitude.Map"; }
		};

		// Adds magnitude randomness
		struct Randomness : Parameters::ParameterDescriptor<CurlParameters, 21, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Magnitude.Randomness"; }

			static float DefaultValue() { return 0.0f; }
		};
	};

	struct Phase
	{
		struct Value : Parameters::ParameterDescriptor<CurlParameters, 6, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Phase.Value"; }

			static float DefaultValue() { return 2; }
		};

		struct Channel : Parameters::ParameterDescriptor<CurlParameters, 7, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Phase.Channel"; }
		};

		struct Map : Parameters::ParameterDescriptor<CurlParameters, 8, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Phase.Map"; }
		};

		// Adds phase randomness
		struct Randomness : Parameters::ParameterDescriptor<CurlParameters, 22, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Phase.Randomness"; }

			static float DefaultValue() { return 0.0f; }
		};

		struct Offset : Parameters::ParameterDescriptor<CurlParameters, 9, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Phase.Offset"; }

			static float DefaultValue() { return 0; }
		};
	};

	struct Stretch
	{
		struct Value : Parameters::ParameterDescriptor<CurlParameters, 10, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Stretch.Value"; }

			static float DefaultValue() { return 0; }
		};

		struct Channel : Parameters::ParameterDescriptor<CurlParameters, 11, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Stretch.Channel"; }
		};

		struct Map : Parameters::ParameterDescriptor<CurlParameters, 12, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Stretch.Map"; }
		};
	};

	struct IsLengthDependent : Parameters::ParameterDescriptor<CurlParameters, 13, bool>
	{
		static char const* Name() { return "IsLengthDependent"; }
	};

	struct Noise
	{
		struct Scale : Parameters::ParameterDescriptor<CurlParameters, 14, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Noise.Scale"; }

			static float DefaultValue() { return 3; }
		};

		struct Amount : Parameters::ParameterDescriptor<CurlParameters, 15, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "Noise.Amount"; }

			static float DefaultValue() { return 1.0f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<CurlParameters, 16, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};

		struct BlendDistance : Parameters::ParameterDescriptor<CurlParameters, 18, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "StrandGroup.BlendDistance"; }

			static float DefaultValue() { return 0; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};
	};

	struct Axis : Parameters::ParameterDescriptor<CurlParameters, 17, CurlAxis>
	{
		static char const* Name() { return "Axis"; }

		static CurlAxis DefaultValue() { return CurlAxis::Z; }
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::CurlParameters::WaveformType>
{
	enum { Count = 2 };

	static char const* GetValueName( Ephere::Ornatrix::CurlParameters::WaveformType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case CurlParameters::WaveformType::Sine: return "Sine";
			case CurlParameters::WaveformType::Helix: return "Helix";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::CurlParameters::CurlAxis>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::CurlParameters::CurlAxis value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case CurlParameters::CurlAxis::X: return "X";
			case CurlParameters::CurlAxis::Y: return "Y";
			case CurlParameters::CurlAxis::Z: return "Z";
			default: return "";
		}
	}
};
}
