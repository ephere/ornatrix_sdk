// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"

namespace Ephere { namespace Ornatrix {
struct CurvesParameter;
struct HairParameter;
} }

namespace Ephere { namespace Ornatrix {

struct GuidesFromCurvesParameters
{
	enum { Version = 1 };

	enum { HighestId = 4, FirstUnusedId = 5 };

	static char const* GetName() { return "GuidesFromCurvesParameters"; }

	struct OutputGuides : Parameters::ParameterDescriptor<GuidesFromCurvesParameters, 1, Ephere::Ornatrix::HairParameter, Parameters::Direction::Out, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "OutputGuides"; }
	};

	struct InputCurves : Parameters::ParameterDescriptor<GuidesFromCurvesParameters, 2, Ephere::Ornatrix::CurvesParameter[]>
	{
		static char const* Name() { return "InputCurves"; }
	};

	struct PointCount : Parameters::ParameterDescriptor<GuidesFromCurvesParameters, 3, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "PointCount"; }

		static int DefaultValue() { return 10; }
	};

	struct UseNormalizedCurveCoordinates : Parameters::ParameterDescriptor<GuidesFromCurvesParameters, 4, bool>
	{
		static char const* Name() { return "UseNormalizedCurveCoordinates"; }
	};

	struct Descriptor;
	struct Container;
};

} }
