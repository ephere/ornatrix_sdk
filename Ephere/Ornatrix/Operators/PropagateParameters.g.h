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

struct PropagateParameters
{
	enum { Version = 1 };

	enum { HighestId = 41, FirstUnusedId = 42 };

	static char const* GetName() { return "PropagateParameters"; }

	enum class RootGenerationMethodType
	{
		Uniform,
		Random,
		Vertex,
	};

	enum class TextureMappingMethodType
	{
		None,
		InheritFromBaseStrand,
		FlatMap,
	};

	struct TargetHair : Parameters::ParameterDescriptor<PropagateParameters, 40, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<PropagateParameters, 41, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	// Determines how roots are generated on strands
	struct RootGenerationMethod : Parameters::ParameterDescriptor<PropagateParameters, 1, RootGenerationMethodType>
	{
		static char const* Name() { return "RootGenerationMethod"; }

		static RootGenerationMethodType DefaultValue() { return RootGenerationMethodType::Random; }
	};

	// Determines the total number of new strands generated on each base strand
	struct RootCount : Parameters::ParameterDescriptor<PropagateParameters, 2, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "RootCount"; }

		static int DefaultValue() { return 50; }
	};

	// Determines how many points each strand generated generated by this operator will have
	struct VerticesPerRootCount : Parameters::ParameterDescriptor<PropagateParameters, 3, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "VerticesPerRootCount"; }

		static int DefaultValue() { return 4; }
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<PropagateParameters, 4, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};
	};

	// If checked, the viewport and render count values will be used per each source strands instead of overall for all strands
	struct CountPerStrand : Parameters::ParameterDescriptor<PropagateParameters, 5, bool>
	{
		static char const* Name() { return "CountPerStrand"; }

		static bool DefaultValue() { return true; }
	};

	struct RandomSeed : Parameters::ParameterDescriptor<PropagateParameters, 6, int, Parameters::Direction::In, Parameters::Purpose::RandomSeed, false, true>
	{
		static char const* Name() { return "RandomSeed"; }

		static int DefaultValue() { return 1; }
	};

	struct Range
	{
		// Starting position on source strands where to generate new strands
		struct Low : Parameters::ParameterDescriptor<PropagateParameters, 7, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Range.Low"; }

			static float DefaultValue() { return 0.0f; }
		};

		// Ending position on source strands where to generate new strands
		struct High : Parameters::ParameterDescriptor<PropagateParameters, 8, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Range.High"; }

			static float DefaultValue() { return 1.0f; }
		};
	};

	// If checked length values will be adjusted to the length of each strand on which new strands are generated.
	struct LengthRelativeToBaseStrand : Parameters::ParameterDescriptor<PropagateParameters, 9, bool>
	{
		static char const* Name() { return "LengthRelativeToBaseStrand"; }

		static bool DefaultValue() { return false; }
	};

	// When enabled and number of twist sides is set to 2, this will force all newly created hairs to be tangential to the underlying surface at the root. This is useful for feathers when you want all of them to be staying parallel to the surface.
	struct FaceSurface : Parameters::ParameterDescriptor<PropagateParameters, 10, bool>
	{
		static char const* Name() { return "FaceSurface"; }

		static bool DefaultValue() { return true; }
	};

	// Specifies how many unique twist sides will be propagated on. These sides are uniformly picked. For example having two sides would mean spawning one hair on one side of the strand and the next on the other (opposite of the first hair).
	struct SideCount : Parameters::ParameterDescriptor<PropagateParameters, 11, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "SideCount"; }

		static int DefaultValue() { return 2; }
	};

	struct Twist
	{
		// Controls the rotations of generated strands around the base strand
		struct Amount
		{
			struct Value : Parameters::ParameterDescriptor<PropagateParameters, 12, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
			{
				static char const* Name() { return "Twist.Amount.Value"; }

				static float DefaultValue() { return 0.0f; }
			};

			struct Ramp : Parameters::ParameterDescriptor<PropagateParameters, 13, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "Twist.Amount.Ramp"; }

				static char const* DefaultValue() { return "0 0 0.5 0.5 1 1"; }
			};
		};

		// Introduces variation in twist values by specified amount
		struct Randomness : Parameters::ParameterDescriptor<PropagateParameters, 14, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Twist.Randomness"; }

			static float DefaultValue() { return 0.1f; }
		};
	};

	struct Fanning
	{
		// Controls the rotations of generated strands along the base strand (from root to tip)
		struct Amount
		{
			struct Value : Parameters::ParameterDescriptor<PropagateParameters, 15, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
			{
				static char const* Name() { return "Fanning.Amount.Value"; }

				static float DefaultValue() { return 0.0f; }
			};

			struct Ramp : Parameters::ParameterDescriptor<PropagateParameters, 16, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "Fanning.Amount.Ramp"; }

				static char const* DefaultValue() { return "0 0 0.5 0.5 1 1"; }
			};
		};

		// Introduces variation in fanning values by specified amount
		struct Randomness : Parameters::ParameterDescriptor<PropagateParameters, 17, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Fanning.Randomness"; }

			static float DefaultValue() { return 0.0f; }
		};
	};

	struct Length
	{
		// Length of generated strands
		struct Amount
		{
			struct Value : Parameters::ParameterDescriptor<PropagateParameters, 18, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
			{
				static char const* Name() { return "Length.Amount.Value"; }

				static float DefaultValue() { return 10.0f; }

				static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
			};

			struct Ramp : Parameters::ParameterDescriptor<PropagateParameters, 19, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "Length.Amount.Ramp"; }

				static char const* DefaultValue() { return "0 1 0.5 1 1 1"; }
			};
		};

		// Introduces variation in generated strand length by specified amount
		struct Randomness : Parameters::ParameterDescriptor<PropagateParameters, 20, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Length.Randomness"; }

			static float DefaultValue() { return 0.2f; }
		};
	};

	// Controls the number of generated strand along each base strand and base surface
	struct Distribution
	{
		struct Channel : Parameters::ParameterDescriptor<PropagateParameters, 21, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Distribution.Channel"; }
		};

		struct Ramp : Parameters::ParameterDescriptor<PropagateParameters, 22, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Distribution.Ramp"; }

			static char const* DefaultValue() { return "0 1 0.5 1 1 1"; }
		};

		struct Map : Parameters::ParameterDescriptor<PropagateParameters, 23, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Distribution.Map"; }
		};
	};

	// Strand group assigned to generated strands
	struct ResultStrandGroup : Parameters::ParameterDescriptor<PropagateParameters, 24, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "ResultStrandGroup"; }

		static int DefaultValue() { return 1; }
	};

	// Internal id of the operator used for enumerating generated strands uniquely
	struct OperatorId : Parameters::ParameterDescriptor<PropagateParameters, 25, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "OperatorId"; }

		static int DefaultValue() { return 0; }
	};

	// Specifies how the texture coordinates will be generated for the resulting strands
	struct TextureMappingMethod : Parameters::ParameterDescriptor<PropagateParameters, 26, TextureMappingMethodType>
	{
		static char const* Name() { return "TextureMappingMethod"; }

		static TextureMappingMethodType DefaultValue() { return TextureMappingMethodType::None; }
	};

	// When root distribution is uniform this offsets the root values by specified amount randomly along the strand
	struct UniformRandomness : Parameters::ParameterDescriptor<PropagateParameters, 27, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "UniformRandomness"; }

		static float DefaultValue() { return 0; }
	};

	// When enabled, a per-strand channel is added containing root positions in range (0 to 1) of propagated strands, relative to their base strand
	struct AddRootPositionChannel : Parameters::ParameterDescriptor<PropagateParameters, 28, bool>
	{
		static char const* Name() { return "AddRootPositionChannel"; }

		static bool DefaultValue() { return false; }
	};

	struct PerStrandSide
	{
		struct Indices : Parameters::ParameterDescriptor<PropagateParameters, 29, int[]>
		{
			static char const* Name() { return "PerStrandSide.Indices"; }
		};

		struct TwistRamps : Parameters::ParameterDescriptor<PropagateParameters, 30, Ephere::Ornatrix::Ramp[]>
		{
			static char const* Name() { return "PerStrandSide.TwistRamps"; }
		};

		struct FanningRamps : Parameters::ParameterDescriptor<PropagateParameters, 31, Ephere::Ornatrix::Ramp[]>
		{
			static char const* Name() { return "PerStrandSide.FanningRamps"; }
		};

		struct LengthRamps : Parameters::ParameterDescriptor<PropagateParameters, 32, Ephere::Ornatrix::Ramp[]>
		{
			static char const* Name() { return "PerStrandSide.LengthRamps"; }
		};

		struct DistributionRamps : Parameters::ParameterDescriptor<PropagateParameters, 33, Ephere::Ornatrix::Ramp[]>
		{
			static char const* Name() { return "PerStrandSide.DistributionRamps"; }
		};

		struct Twists : Parameters::ParameterDescriptor<PropagateParameters, 34, float[]>
		{
			static char const* Name() { return "PerStrandSide.Twists"; }
		};

		struct TwistsRandomness : Parameters::ParameterDescriptor<PropagateParameters, 35, float[]>
		{
			static char const* Name() { return "PerStrandSide.TwistsRandomness"; }
		};

		struct Fannings : Parameters::ParameterDescriptor<PropagateParameters, 36, float[]>
		{
			static char const* Name() { return "PerStrandSide.Fannings"; }
		};

		struct FanningsRandomness : Parameters::ParameterDescriptor<PropagateParameters, 37, float[]>
		{
			static char const* Name() { return "PerStrandSide.FanningsRandomness"; }
		};

		struct Lengths : Parameters::ParameterDescriptor<PropagateParameters, 38, float[]>
		{
			static char const* Name() { return "PerStrandSide.Lengths"; }
		};

		struct LengthsRandomness : Parameters::ParameterDescriptor<PropagateParameters, 39, float[]>
		{
			static char const* Name() { return "PerStrandSide.LengthsRandomness"; }
		};
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::PropagateParameters::RootGenerationMethodType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::PropagateParameters::RootGenerationMethodType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case PropagateParameters::RootGenerationMethodType::Uniform: return "Uniform";
			case PropagateParameters::RootGenerationMethodType::Random: return "Random";
			case PropagateParameters::RootGenerationMethodType::Vertex: return "Vertex";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::PropagateParameters::TextureMappingMethodType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::PropagateParameters::TextureMappingMethodType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case PropagateParameters::TextureMappingMethodType::None: return "None";
			case PropagateParameters::TextureMappingMethodType::InheritFromBaseStrand: return "InheritFromBaseStrand";
			case PropagateParameters::TextureMappingMethodType::FlatMap: return "FlatMap";
			default: return "";
		}
	}
};
}