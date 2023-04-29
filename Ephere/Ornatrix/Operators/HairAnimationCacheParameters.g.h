// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
} }

namespace Ephere { namespace Ornatrix {

struct HairAnimationCacheParameters
{
	enum { Version = 1 };

	enum { HighestId = 15, FirstUnusedId = 16 };

	static char const* GetName() { return "HairAnimationCacheParameters"; }

	struct TargetHair : Parameters::ParameterDescriptor<HairAnimationCacheParameters, 12, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<HairAnimationCacheParameters, 13, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct CurrentTime : Parameters::ParameterDescriptor<HairAnimationCacheParameters, 14, float, Parameters::Direction::In, Parameters::Purpose::CurrentTime, true, true>
	{
		static char const* Name() { return "CurrentTime"; }

		static Parameters::TimeUnits const DefaultUnits = Parameters::TimeUnits::Seconds;
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<HairAnimationCacheParameters, 1, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};
	};

	// Path of file containing animation or properties to apply to the hair
	struct FilePath : Parameters::ParameterDescriptor<HairAnimationCacheParameters, 2, Ephere::Parameters::String, Parameters::Direction::In, Parameters::Purpose::FilePath>
	{
		static char const* Name() { return "FilePath"; }

		static char const* DefaultValue() { return ""; }
	};

	// Object path within Alembic file pointing to curves to be loaded. If empty, all curves inside the file will be loaded. If not empty, all curves located under the specified path will be loaded.
	struct CurvesPath : Parameters::ParameterDescriptor<HairAnimationCacheParameters, 15, Ephere::Parameters::String>
	{
		static char const* Name() { return "CurvesPath"; }

		static char const* DefaultValue() { return ""; }
	};

	// Determines the intensity of the effect, can be used to blend animation.
	struct ApplyAmount : Parameters::ParameterDescriptor<HairAnimationCacheParameters, 3, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
	{
		static char const* Name() { return "ApplyAmount"; }

		static float DefaultValue() { return 1.0f; }
	};

	struct Time
	{
		// Specifies the start of animation in the scene relative to the first sample. You can use these two parameters to control the delay and playback speed of the animation.
		struct Offset : Parameters::ParameterDescriptor<HairAnimationCacheParameters, 4, float, Parameters::Direction::In, Parameters::Purpose::Time, false, true>
		{
			static char const* Name() { return "Time.Offset"; }

			static float DefaultValue() { return 0; }

			static Parameters::TimeUnits const DefaultUnits = Parameters::TimeUnits::Seconds;
		};

		struct Scale : Parameters::ParameterDescriptor<HairAnimationCacheParameters, 5, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Time.Scale"; }

			static float DefaultValue() { return 1.0f; }
		};
	};

	struct Import
	{
		// When enabled, width information will be assigned to hair, if present
		struct Widths : Parameters::ParameterDescriptor<HairAnimationCacheParameters, 6, bool>
		{
			static char const* Name() { return "Import.Widths"; }

			static bool DefaultValue() { return false; }
		};

		// When enabled, texture coordinate information will be assigned to hair, if present
		struct TextureCoordinates : Parameters::ParameterDescriptor<HairAnimationCacheParameters, 7, bool>
		{
			static char const* Name() { return "Import.TextureCoordinates"; }

			static bool DefaultValue() { return false; }
		};

		// When enabled, strand channel information will be assigned to hair, if present
		struct ChannelData : Parameters::ParameterDescriptor<HairAnimationCacheParameters, 8, bool>
		{
			static char const* Name() { return "Import.ChannelData"; }

			static bool DefaultValue() { return true; }
		};

		// When enabled, strand group information will be assigned to hair, if present
		struct StrandGroups : Parameters::ParameterDescriptor<HairAnimationCacheParameters, 9, bool>
		{
			static char const* Name() { return "Import.StrandGroups"; }

			static bool DefaultValue() { return false; }
		};

		// When enabled, strand id information will be assigned to hair, if present
		struct StrandIds : Parameters::ParameterDescriptor<HairAnimationCacheParameters, 10, bool>
		{
			static char const* Name() { return "Import.StrandIds"; }

			static bool DefaultValue() { return true; }
		};
	};

	// When enabled, the loaded hair data by this operator completely overwrites any input strands making this operator only output the loaded animated hair. This is useful if you want to completely capture and cache the state of your hair stack in a particular position.
	struct OverwriteInput : Parameters::ParameterDescriptor<HairAnimationCacheParameters, 11, bool>
	{
		static char const* Name() { return "OverwriteInput"; }

		static bool DefaultValue() { return false; }
	};

	struct Descriptor;
	struct Container;
};

} }
