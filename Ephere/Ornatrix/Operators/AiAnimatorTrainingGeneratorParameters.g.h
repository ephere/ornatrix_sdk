// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"
#include "Ephere/Ornatrix/ParameterComponents.h"


namespace Ephere { namespace Ornatrix {

struct AiAnimatorTrainingGeneratorParameters
{
	enum { Version = 1 };

	enum { HighestId = 8, FirstUnusedId = 9 };

	static char const* GetName() { return "AiAnimatorTrainingGeneratorParameters"; }

	// Random seed
	struct RandomSeed : Parameters::ParameterDescriptor<AiAnimatorTrainingGeneratorParameters, 1, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "RandomSeed"; }

		static int DefaultValue() { return 1000; }
	};

	// Maximum linear velocity
	struct VelocityLimit : Parameters::ParameterDescriptor<AiAnimatorTrainingGeneratorParameters, 2, float, Parameters::Direction::In, Parameters::Purpose::Velocity, false, true>
	{
		static char const* Name() { return "VelocityLimit"; }

		static float DefaultValue() { return 10; }
	};

	// Maximum absolute displacement from the origin
	struct PositionLimits : Parameters::ParameterDescriptor<AiAnimatorTrainingGeneratorParameters, 3, Ephere::Ornatrix::Vector3, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
	{
		static char const* Name() { return "PositionLimits"; }

		static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
	};

	// Maximum angular velocity
	struct AngularVelocityLimit : Parameters::ParameterDescriptor<AiAnimatorTrainingGeneratorParameters, 4, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "AngularVelocityLimit"; }

		static float DefaultValue() { return 3; }
	};

	// Maximum absolute rotations around the axes
	struct RotationLimits : Parameters::ParameterDescriptor<AiAnimatorTrainingGeneratorParameters, 5, Ephere::Ornatrix::Vector3, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "RotationLimits"; }
	};

	// Fraction of the animation time that will be spend in rest
	struct RestProbability : Parameters::ParameterDescriptor<AiAnimatorTrainingGeneratorParameters, 6, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
	{
		static char const* Name() { return "RestProbability"; }

		static float DefaultValue() { return 0.1f; }
	};

	// Total number of frames in the generated animation sequence
	struct FrameCount : Parameters::ParameterDescriptor<AiAnimatorTrainingGeneratorParameters, 7, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "FrameCount"; }

		static int DefaultValue() { return 5000; }
	};

	// When enabled, a standard validation sequence is generated at the end of the training animation
	struct GenerateValidationSequence : Parameters::ParameterDescriptor<AiAnimatorTrainingGeneratorParameters, 8, bool>
	{
		static char const* Name() { return "GenerateValidationSequence"; }

		static bool DefaultValue() { return false; }
	};

	struct Descriptor;
	struct Container;
};

} }
