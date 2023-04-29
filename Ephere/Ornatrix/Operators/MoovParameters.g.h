// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
} }

namespace Ephere { namespace Ornatrix {

struct MoovParameters
{
	enum { Version = 1 };

	enum { HighestId = 113, FirstUnusedId = 10 };

	static char const* GetName() { return "MoovParameters"; }

	struct TargetHair : Parameters::ParameterDescriptor<MoovParameters, 1, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<MoovParameters, 2, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct CurrentTime : Parameters::ParameterDescriptor<MoovParameters, 3, float, Parameters::Direction::In, Parameters::Purpose::CurrentTime, true, true>
	{
		static char const* Name() { return "CurrentTime"; }

		static Parameters::TimeUnits const DefaultUnits = Parameters::TimeUnits::Seconds;
	};

	// Path to the simulator Python script
	struct PythonFileName : Parameters::ParameterDescriptor<MoovParameters, 4, Ephere::Parameters::String, Parameters::Direction::In, Parameters::Purpose::FilePath>
	{
		static char const* Name() { return "PythonFileName"; }

		static char const* DefaultValue() { return ""; }
	};

	// Cached simulator Python script
	struct PythonScript : Parameters::ParameterDescriptor<MoovParameters, 5, Ephere::Parameters::String>
	{
		static char const* Name() { return "PythonScript"; }

		static char const* DefaultValue() { return ""; }
	};

	// Path to the default Python scripts (added to system path)
	struct DefaultScriptPath : Parameters::ParameterDescriptor<MoovParameters, 6, Ephere::Parameters::String, Parameters::Direction::In, Parameters::Purpose::DirectoryPath>
	{
		static char const* Name() { return "DefaultScriptPath"; }

		static char const* DefaultValue() { return ""; }
	};

	// Path to the file saving the initial state capture
	struct CaptureFileName : Parameters::ParameterDescriptor<MoovParameters, 7, Ephere::Parameters::String, Parameters::Direction::In, Parameters::Purpose::FilePath>
	{
		static char const* Name() { return "CaptureFileName"; }

		static char const* DefaultValue() { return ""; }
	};

	// Flag showing if a capture file has been saved
	struct HasSavedCaptureFile : Parameters::ParameterDescriptor<MoovParameters, 8, bool>
	{
		static char const* Name() { return "HasSavedCaptureFile"; }

		static bool DefaultValue() { return false; }
	};

	// List of parameters with their types
	struct ParamTypesList : Parameters::ParameterDescriptor<MoovParameters, 9, Ephere::Parameters::String>
	{
		static char const* Name() { return "ParamTypesList"; }

		static char const* DefaultValue() { return ""; }
	};

	// Path to a file with simulation parameters
	struct ParametersFileName : Parameters::ParameterDescriptor<MoovParameters, 11, Ephere::Parameters::String, Parameters::Direction::In, Parameters::Purpose::FilePath>
	{
		static char const* Name() { return "ParametersFileName"; }

		static char const* DefaultValue() { return ""; }
	};

	struct UseSettleMode
	{
		struct Enabled : Parameters::ParameterDescriptor<MoovParameters, 14, bool>
		{
			static char const* Name() { return "UseSettleMode.Enabled"; }

			static bool DefaultValue() { return false; }
		};

		struct TimeStep : Parameters::ParameterDescriptor<MoovParameters, 15, float, Parameters::Direction::In, Parameters::Purpose::Time, false, true>
		{
			static char const* Name() { return "UseSettleMode.TimeStep"; }

			static float DefaultValue() { return 0.04f; }

			static Parameters::TimeUnits const DefaultUnits = Parameters::TimeUnits::Seconds;
		};

		struct MaxIterations : Parameters::ParameterDescriptor<MoovParameters, 16, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "UseSettleMode.MaxIterations"; }

			static int DefaultValue() { return 10; }
		};

		// Iterations stop when all hair vertices have velocity lower than this value
		struct VelocityThreshold : Parameters::ParameterDescriptor<MoovParameters, 17, float, Parameters::Direction::In, Parameters::Purpose::Velocity, false, true>
		{
			static char const* Name() { return "UseSettleMode.VelocityThreshold"; }

			static float DefaultValue() { return 1.f; }
		};

		// Iterations stop when the total evaluation time exceeds this value
		struct MaxEvaluationTime : Parameters::ParameterDescriptor<MoovParameters, 18, float, Parameters::Direction::In, Parameters::Purpose::Time, false, true>
		{
			static char const* Name() { return "UseSettleMode.MaxEvaluationTime"; }

			static float DefaultValue() { return 10000.f; }

			static Parameters::TimeUnits const DefaultUnits = Parameters::TimeUnits::Milliseconds;
		};
	};

	struct Visualizer
	{
		struct Enabled : Parameters::ParameterDescriptor<MoovParameters, 101, bool>
		{
			static char const* Name() { return "Visualizer.Enabled"; }

			static bool DefaultValue() { return false; }
		};

		struct ShowParticles : Parameters::ParameterDescriptor<MoovParameters, 102, bool>
		{
			static char const* Name() { return "Visualizer.ShowParticles"; }

			static bool DefaultValue() { return true; }
		};

		struct ShowMeshes : Parameters::ParameterDescriptor<MoovParameters, 103, bool>
		{
			static char const* Name() { return "Visualizer.ShowMeshes"; }

			static bool DefaultValue() { return true; }
		};

		struct ShowConstraints : Parameters::ParameterDescriptor<MoovParameters, 104, bool>
		{
			static char const* Name() { return "Visualizer.ShowConstraints"; }

			static bool DefaultValue() { return true; }
		};

		struct ShowVelocities : Parameters::ParameterDescriptor<MoovParameters, 105, bool>
		{
			static char const* Name() { return "Visualizer.ShowVelocities"; }

			static bool DefaultValue() { return true; }
		};

		struct ShowContacts : Parameters::ParameterDescriptor<MoovParameters, 106, bool>
		{
			static char const* Name() { return "Visualizer.ShowContacts"; }

			static bool DefaultValue() { return true; }
		};

		struct ShowCapsules : Parameters::ParameterDescriptor<MoovParameters, 107, bool>
		{
			static char const* Name() { return "Visualizer.ShowCapsules"; }

			static bool DefaultValue() { return true; }
		};

		// Allows scaling of all particle sizes in the visualizer
		struct ParticleScale : Parameters::ParameterDescriptor<MoovParameters, 110, float>
		{
			static char const* Name() { return "Visualizer.ParticleScale"; }

			static float DefaultValue() { return 1.f; }
		};

		// Allows scaling of velocities in the visualizer
		struct VelocityScale : Parameters::ParameterDescriptor<MoovParameters, 111, float>
		{
			static char const* Name() { return "Visualizer.VelocityScale"; }

			static float DefaultValue() { return 0.1f; }
		};

		// Affects coloring of constraints dependent on force
		struct ForceScale : Parameters::ParameterDescriptor<MoovParameters, 112, float>
		{
			static char const* Name() { return "Visualizer.ForceScale"; }

			static float DefaultValue() { return 1.f; }
		};

		// Allows scaling of contact velocities in the visualizer
		struct ContactScale : Parameters::ParameterDescriptor<MoovParameters, 113, float>
		{
			static char const* Name() { return "Visualizer.ContactScale"; }

			static float DefaultValue() { return 0.5f; }
		};
	};

	struct Descriptor;
	struct Container;
};

} }
