// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct TextureMapParameter;
} }

namespace Ephere { namespace Ornatrix {

struct BakedHairParameters
{
	enum { Version = 1 };

	enum { HighestId = 5, FirstUnusedId = 6 };

	static char const* GetName() { return "BakedHairParameters"; }

	struct InputHair : Parameters::ParameterDescriptor<BakedHairParameters, 1, Ephere::Ornatrix::HairParameter>
	{
		static char const* Name() { return "InputHair"; }
	};

	struct SourceFilePaths : Parameters::ParameterDescriptor<BakedHairParameters, 2, Ephere::Parameters::String[], Parameters::Direction::In, Parameters::Purpose::FilePath>
	{
		static char const* Name() { return "SourceFilePaths"; }
	};

	struct CurvesPaths : Parameters::ParameterDescriptor<BakedHairParameters, 5, Ephere::Parameters::String[]>
	{
		static char const* Name() { return "CurvesPaths"; }
	};

	struct DisplacementMultiplier : Parameters::ParameterDescriptor<BakedHairParameters, 3, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "DisplacementMultiplier"; }
	};

	// Provides a fraction of hair in range [0,1] which will be generated when "is rendering" parameter is off (e.g. inside a viewport).
	struct PreviewStrandFraction : Parameters::ParameterDescriptor<BakedHairParameters, 4, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
	{
		static char const* Name() { return "PreviewStrandFraction"; }

		static float DefaultValue() { return 0.1f; }
	};

	struct Descriptor;
	struct Container;
};

} }
