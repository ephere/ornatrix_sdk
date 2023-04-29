// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"


namespace Ephere { namespace Ornatrix {

struct TransferNormalsParameters
{
	enum { Version = 1 };

	enum { HighestId = 13, FirstUnusedId = 1 };

	static char const* GetName() { return "TransferNormalsParameters"; }

	// Number of nearby vertex normals to sample and average for each destination normal
	struct SampleCount : Parameters::ParameterDescriptor<TransferNormalsParameters, 13, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "SampleCount"; }

		static int DefaultValue() { return 1; }
	};

	struct Descriptor;
	struct Container;
};

} }
