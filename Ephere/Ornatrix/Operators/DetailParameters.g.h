// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
struct StrandChannel;
} }

namespace Ephere { namespace Ornatrix {

struct DetailParameters
{
	enum { Version = 1 };

	enum { HighestId = 17, FirstUnusedId = 18 };

	static char const* GetName() { return "DetailParameters"; }

	struct InputHair : Parameters::ParameterDescriptor<DetailParameters, 14, Ephere::Ornatrix::HairParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "InputHair"; }
	};

	struct OutputHair : Parameters::ParameterDescriptor<DetailParameters, 15, Ephere::Ornatrix::HairParameter, Parameters::Direction::Out, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "OutputHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<DetailParameters, 16, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct PointCount
	{
		struct View : Parameters::ParameterDescriptor<DetailParameters, 1, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "PointCount.View"; }

			static int DefaultValue() { return 10; }
		};

		struct Render : Parameters::ParameterDescriptor<DetailParameters, 2, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "PointCount.Render"; }

			static int DefaultValue() { return 10; }
		};

		struct Channel : Parameters::ParameterDescriptor<DetailParameters, 13, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "PointCount.Channel"; }
		};
	};

	struct SampleByThreshold
	{
		struct Enable : Parameters::ParameterDescriptor<DetailParameters, 4, bool>
		{
			static char const* Name() { return "SampleByThreshold.Enable"; }

			static bool DefaultValue() { return false; }
		};

		struct AngleThreshold : Parameters::ParameterDescriptor<DetailParameters, 3, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "SampleByThreshold.AngleThreshold"; }

			static float DefaultValue() { return 0.1f; }
		};

		struct SampleSize : Parameters::ParameterDescriptor<DetailParameters, 5, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "SampleByThreshold.SampleSize"; }

			static float DefaultValue() { return 0.01f; }
		};
	};

	struct Smoothing
	{
		// Amount of smoothing applied to the curve before re-sampling it
		struct Amount : Parameters::ParameterDescriptor<DetailParameters, 6, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Smoothing.Amount"; }

			static float DefaultValue() { return 0.4f; }
		};

		// Number of control points of the smoothing spline
		struct PointCount : Parameters::ParameterDescriptor<DetailParameters, 8, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Smoothing.PointCount"; }

			static int DefaultValue() { return 10; }
		};
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<DetailParameters, 7, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};
	};

	struct SampleByLength
	{
		// If on, sampling is done at regular intervals determined by the distance parameter
		struct Enable : Parameters::ParameterDescriptor<DetailParameters, 9, bool>
		{
			static char const* Name() { return "SampleByLength.Enable"; }

			static bool DefaultValue() { return false; }
		};

		struct Distance : Parameters::ParameterDescriptor<DetailParameters, 10, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "SampleByLength.Distance"; }

			static float DefaultValue() { return 1.0f; }
		};

		struct MinPointCount : Parameters::ParameterDescriptor<DetailParameters, 11, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "SampleByLength.MinPointCount"; }

			static int DefaultValue() { return 2; }
		};

		struct MaxPointCount : Parameters::ParameterDescriptor<DetailParameters, 12, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "SampleByLength.MaxPointCount"; }

			static int DefaultValue() { return 999; }
		};
	};

	struct IsRendering : Parameters::ParameterDescriptor<DetailParameters, 17, bool, Parameters::Direction::In, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "IsRendering"; }

		static bool DefaultValue() { return false; }
	};

	struct Descriptor;
	struct Container;
};

} }
