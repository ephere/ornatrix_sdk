// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"
#include "Ephere/Ornatrix/AIPhysics/AiModel.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
} }

namespace Ephere { namespace Ornatrix {

struct AiAnimatorParameters
{
	enum { Version = 1 };

	enum { HighestId = 12, FirstUnusedId = 9 };

	static char const* GetName() { return "AiAnimatorParameters"; }

	struct TargetHair : Parameters::ParameterDescriptor<AiAnimatorParameters, 11, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<AiAnimatorParameters, 12, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct CurrentTime : Parameters::ParameterDescriptor<AiAnimatorParameters, 10, float, Parameters::Direction::In, Parameters::Purpose::CurrentTime, true, true>
	{
		static char const* Name() { return "CurrentTime"; }

		static Parameters::TimeUnits const DefaultUnits = Parameters::TimeUnits::Seconds;
	};

	// File used to save the parameters of the model
	struct FilePath : Parameters::ParameterDescriptor<AiAnimatorParameters, 1, Ephere::Parameters::String, Parameters::Direction::In, Parameters::Purpose::FilePath>
	{
		static char const* Name() { return "FilePath"; }

		static char const* DefaultValue() { return ""; }
	};

	// Number of principal components for hair vertices
	struct ComponentCount : Parameters::ParameterDescriptor<AiAnimatorParameters, 2, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "ComponentCount"; }

		static int DefaultValue() { return 10; }
	};

	// Number of principal components for predictors
	struct PredictorComponentCount : Parameters::ParameterDescriptor<AiAnimatorParameters, 3, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "PredictorComponentCount"; }

		static int DefaultValue() { return 6; }
	};

	// Type of predictive algorithm used for simulation
	struct Algorithm : Parameters::ParameterDescriptor<AiAnimatorParameters, 4, AiPhysics::AlgorithmType>
	{
		static char const* Name() { return "Algorithm"; }
	};

	// File used to save the training data
	struct TrainingDataFilePath : Parameters::ParameterDescriptor<AiAnimatorParameters, 5, Ephere::Parameters::String, Parameters::Direction::In, Parameters::Purpose::FilePath>
	{
		static char const* Name() { return "TrainingDataFilePath"; }

		static char const* DefaultValue() { return ""; }
	};

	// Turn off prediction engine, only pass input hair through PCA transform
	struct PcaOnly : Parameters::ParameterDescriptor<AiAnimatorParameters, 6, bool>
	{
		static char const* Name() { return "PcaOnly"; }

		static bool DefaultValue() { return false; }
	};

	// Starting offset of the animation sequence used to compute the validation loss during training (not computed if zero)
	struct ValidationStartOffset : Parameters::ParameterDescriptor<AiAnimatorParameters, 7, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "ValidationStartOffset"; }

		static int DefaultValue() { return 0; }
	};

	// Use object space coordinates for training; if off, world space coordinates are used
	struct UseObjectSpaceCoordinates : Parameters::ParameterDescriptor<AiAnimatorParameters, 8, bool>
	{
		static char const* Name() { return "UseObjectSpaceCoordinates"; }

		static bool DefaultValue() { return true; }
	};

	struct Descriptor;
	struct Container;
};

} }
