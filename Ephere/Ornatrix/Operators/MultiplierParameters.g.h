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

struct MultiplierParameters
{
	enum { Version = 1 };

	enum { HighestId = 33, FirstUnusedId = 34 };

	static char const* GetName() { return "MultiplierParameters"; }

	struct InputHair : Parameters::ParameterDescriptor<MultiplierParameters, 31, Ephere::Ornatrix::HairParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "InputHair"; }
	};

	struct OutputHair : Parameters::ParameterDescriptor<MultiplierParameters, 32, Ephere::Ornatrix::HairParameter, Parameters::Direction::Out, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "OutputHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<MultiplierParameters, 33, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<MultiplierParameters, 1, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};
	};

	struct RandomSeed : Parameters::ParameterDescriptor<MultiplierParameters, 2, int, Parameters::Direction::In, Parameters::Purpose::RandomSeed, false, true>
	{
		static char const* Name() { return "RandomSeed"; }
	};

	// Determines the chance of a strand being multiplied or not
	struct Probability
	{
		struct Value : Parameters::ParameterDescriptor<MultiplierParameters, 3, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Probability.Value"; }

			static float DefaultValue() { return 1.0f; }
		};

		struct Channel : Parameters::ParameterDescriptor<MultiplierParameters, 5, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Probability.Channel"; }
		};

		struct Map : Parameters::ParameterDescriptor<MultiplierParameters, 4, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Probability.Map"; }
		};
	};

	struct Copies
	{
		// The fraction of the copied strands which are displayed in the viewport
		struct ViewPercent : Parameters::ParameterDescriptor<MultiplierParameters, 9, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Copies.ViewPercent"; }

			static int DefaultValue() { return 100; }
		};

		// Number of new strands generated for each strand
		struct Count : Parameters::ParameterDescriptor<MultiplierParameters, 6, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Copies.Count"; }

			static int DefaultValue() { return 3; }
		};

		// Controls the number of copies along base surface
		struct Amount
		{
			struct Channel : Parameters::ParameterDescriptor<MultiplierParameters, 8, Ephere::Ornatrix::StrandChannel>
			{
				static char const* Name() { return "Copies.Amount.Channel"; }
			};

			struct Map : Parameters::ParameterDescriptor<MultiplierParameters, 7, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "Copies.Amount.Map"; }
			};
		};
	};

	struct Spread
	{
		// Amount of spreading performed at strand roots
		struct Root : Parameters::ParameterDescriptor<MultiplierParameters, 10, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "Spread.Root"; }

			static float DefaultValue() { return 0.0f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};

		// Amount of spreading performed at strand tips
		struct Tip : Parameters::ParameterDescriptor<MultiplierParameters, 11, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "Spread.Tip"; }

			static float DefaultValue() { return 3.0f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};

		// Amount of spreading along the strand
		struct Amount
		{
			struct Channel : Parameters::ParameterDescriptor<MultiplierParameters, 14, Ephere::Ornatrix::StrandChannel>
			{
				static char const* Name() { return "Spread.Amount.Channel"; }
			};

			struct Ramp : Parameters::ParameterDescriptor<MultiplierParameters, 12, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "Spread.Amount.Ramp"; }

				static char const* DefaultValue() { return "0 0 0.5 0.5 1 1"; }
			};

			struct Map : Parameters::ParameterDescriptor<MultiplierParameters, 13, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "Spread.Amount.Map"; }
			};
		};
	};

	struct Fluff
	{
		// Amount of fluffing performed at strand roots
		struct Root : Parameters::ParameterDescriptor<MultiplierParameters, 15, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "Fluff.Root"; }

			static float DefaultValue() { return 0.0f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};

		// Amount of fluffing performed at strand tips
		struct Tip : Parameters::ParameterDescriptor<MultiplierParameters, 16, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "Fluff.Tip"; }

			static float DefaultValue() { return 3.0f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};

		// Amount of fluffing along the strand
		struct Amount
		{
			struct Channel : Parameters::ParameterDescriptor<MultiplierParameters, 19, Ephere::Ornatrix::StrandChannel>
			{
				static char const* Name() { return "Fluff.Amount.Channel"; }
			};

			struct Ramp : Parameters::ParameterDescriptor<MultiplierParameters, 17, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "Fluff.Amount.Ramp"; }

				static char const* DefaultValue() { return "0 0 0.5 0.5 1 1"; }
			};

			struct Map : Parameters::ParameterDescriptor<MultiplierParameters, 18, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "Fluff.Amount.Map"; }
			};
		};
	};

	struct Shift
	{
		// Amount of shifting performed at strand roots
		struct Root : Parameters::ParameterDescriptor<MultiplierParameters, 20, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "Shift.Root"; }

			static float DefaultValue() { return 0.0f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};

		// Amount of shifting performed at strand tips
		struct Tip : Parameters::ParameterDescriptor<MultiplierParameters, 21, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "Shift.Tip"; }

			static float DefaultValue() { return 0.0f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};

		// Amount of shifting along the strand
		struct Amount
		{
			struct Channel : Parameters::ParameterDescriptor<MultiplierParameters, 24, Ephere::Ornatrix::StrandChannel>
			{
				static char const* Name() { return "Shift.Amount.Channel"; }
			};

			struct Ramp : Parameters::ParameterDescriptor<MultiplierParameters, 22, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "Shift.Amount.Ramp"; }

				static char const* DefaultValue() { return "0 0 0.5 0.5 1 1"; }
			};

			struct Map : Parameters::ParameterDescriptor<MultiplierParameters, 23, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "Shift.Amount.Map"; }
			};
		};
	};

	// Amount of twisting along the strand
	struct Twist
	{
		struct Value : Parameters::ParameterDescriptor<MultiplierParameters, 25, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Twist.Value"; }

			static float DefaultValue() { return 0; }
		};

		struct Ramp : Parameters::ParameterDescriptor<MultiplierParameters, 26, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Twist.Ramp"; }

			static char const* DefaultValue() { return "0 0 0.5 0.5 1 1"; }
		};
	};

	// When enabled, the input hair strand lengths will not be modified
	struct PreserveStrandLengths : Parameters::ParameterDescriptor<MultiplierParameters, 27, bool>
	{
		static char const* Name() { return "PreserveStrandLengths"; }

		static bool DefaultValue() { return false; }
	};

	// When enabled, the original strand will not be modified
	struct PreserveOriginalStrand : Parameters::ParameterDescriptor<MultiplierParameters, 28, bool>
	{
		static char const* Name() { return "PreserveOriginalStrand"; }

		static bool DefaultValue() { return false; }
	};

	// When enabled, the generated strands will be attached to the surface
	struct GroundGeneratedStrands : Parameters::ParameterDescriptor<MultiplierParameters, 29, bool>
	{
		static char const* Name() { return "GroundGeneratedStrands"; }

		static bool DefaultValue() { return false; }
	};

	// Strand Group for the generated strands
	struct ResultStrandGroup : Parameters::ParameterDescriptor<MultiplierParameters, 30, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "ResultStrandGroup"; }

		static int DefaultValue() { return 0; }
	};

	struct Descriptor;
	struct Container;
};

} }
