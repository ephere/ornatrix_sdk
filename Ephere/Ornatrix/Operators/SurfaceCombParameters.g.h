// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"
#include "Ephere/Ornatrix/ParameterComponents.h"

namespace Ephere { namespace Geometry {
struct SurfacePosition;
} }

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
class Ramp;
struct StrandChannel;
struct TextureMapParameter;
} }

namespace Ephere { namespace Ornatrix {

struct SurfaceCombParameters
{
	enum { Version = 1 };

	enum { HighestId = 110, FirstUnusedId = 35 };

	static char const* GetName() { return "SurfaceCombParameters"; }

	enum class SinkType
	{
		Repel,
		Attract,
		Direct,
	};

	enum class SinkOrientationType
	{
		Absolute,
		Relative,
	};

	enum class AlgorithmType
	{
		MeshVertex,
		Triangulation,
	};

	enum class SinkMirrorType
	{
		None,
		X,
		Y,
		Z,
	};

	enum class StrandDataModeType
	{
		SinkSize,
		SinkDistance,
		SinkGroup,
	};

	struct TargetHair : Parameters::ParameterDescriptor<SurfaceCombParameters, 23, Ephere::Ornatrix::HairParameter, Parameters::Direction::InOut, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "TargetHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<SurfaceCombParameters, 24, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	// Specifies different behaviors of how hairs are shaped by the sinks
	struct Algorithm : Parameters::ParameterDescriptor<SurfaceCombParameters, 13, AlgorithmType>
	{
		static char const* Name() { return "Algorithm"; }

		static AlgorithmType DefaultValue() { return AlgorithmType::MeshVertex; }
	};

	struct AffectWholeStrand : Parameters::ParameterDescriptor<SurfaceCombParameters, 1, bool>
	{
		static char const* Name() { return "AffectWholeStrand"; }

		static bool DefaultValue() { return false; }
	};

	struct Chaos
	{
		// Amount of randomness added to the strand directions along the surface
		struct Value : Parameters::ParameterDescriptor<SurfaceCombParameters, 2, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Chaos.Value"; }

			static float DefaultValue() { return 0.1f; }
		};

		// Frequency of randomness added to strand directions along the surface
		struct Scale : Parameters::ParameterDescriptor<SurfaceCombParameters, 16, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Chaos.Scale"; }

			static float DefaultValue() { return 1.0f; }
		};

		struct Map : Parameters::ParameterDescriptor<SurfaceCombParameters, 21, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Chaos.Map"; }
		};

		struct Channel : Parameters::ParameterDescriptor<SurfaceCombParameters, 22, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Chaos.Channel"; }
		};
	};

	struct RandomSeed : Parameters::ParameterDescriptor<SurfaceCombParameters, 3, int, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "RandomSeed"; }

		static int DefaultValue() { return 1; }
	};

	struct Slope
	{
		struct Channel : Parameters::ParameterDescriptor<SurfaceCombParameters, 18, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Slope.Channel"; }
		};

		struct Ramp : Parameters::ParameterDescriptor<SurfaceCombParameters, 4, Ephere::Ornatrix::Ramp, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Slope.Ramp"; }

			static char const* DefaultValue() { return "0 0 0.5 0.5 1 1"; }
		};

		struct Map : Parameters::ParameterDescriptor<SurfaceCombParameters, 17, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Slope.Map"; }
		};
	};

	struct ApplyTo
	{
		struct Shape : Parameters::ParameterDescriptor<SurfaceCombParameters, 5, bool>
		{
			static char const* Name() { return "ApplyTo.Shape"; }

			static bool DefaultValue() { return true; }
		};

		struct Direction : Parameters::ParameterDescriptor<SurfaceCombParameters, 6, bool>
		{
			static char const* Name() { return "ApplyTo.Direction"; }

			static bool DefaultValue() { return true; }
		};

		// When enabled, sinks will be used to set values to a per-strand data channel.
		struct StrandData : Parameters::ParameterDescriptor<SurfaceCombParameters, 27, bool>
		{
			static char const* Name() { return "ApplyTo.StrandData"; }

			static bool DefaultValue() { return false; }
		};
	};

	// When enabled, the strands will always follow the surface. Otherwise, they will attempt to follow sink directions.
	struct StickToSurface : Parameters::ParameterDescriptor<SurfaceCombParameters, 14, bool>
	{
		static char const* Name() { return "StickToSurface"; }

		static bool DefaultValue() { return true; }
	};

	// Determines how much strand segments are allowed to bend before smoothing is applied
	struct SmoothAmount : Parameters::ParameterDescriptor<SurfaceCombParameters, 15, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "SmoothAmount"; }

		static float DefaultValue() { return 0.9f; }
	};

	struct SinkOrientation
	{
		// Controls whether strand orientation is changed locally or globally relative to the whole object
		struct Type : Parameters::ParameterDescriptor<SurfaceCombParameters, 7, SinkOrientationType>
		{
			static char const* Name() { return "SinkOrientation.Type"; }

			static SinkOrientationType DefaultValue() { return SinkOrientationType::Relative; }
		};

		/* When this option is enabled, the values in orientation map will be interpreted as object-space vectors to direct the hair.
		Otherwise, the values will be interpreted as rotation angles in radians relative to input strand's surface rotation. */
		struct UseVectorOrientationMap : Parameters::ParameterDescriptor<SurfaceCombParameters, 11, bool>
		{
			static char const* Name() { return "SinkOrientation.UseVectorOrientationMap"; }

			static bool DefaultValue() { return false; }
		};

		struct Map : Parameters::ParameterDescriptor<SurfaceCombParameters, 19, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "SinkOrientation.Map"; }
		};

		struct Channel : Parameters::ParameterDescriptor<SurfaceCombParameters, 20, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "SinkOrientation.Channel"; }
		};
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<SurfaceCombParameters, 8, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};

		struct BlendDistance : Parameters::ParameterDescriptor<SurfaceCombParameters, 12, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "StrandGroup.BlendDistance"; }

			static float DefaultValue() { return 0; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};
	};

	// Minimum value for bending the strands with -1 being opposite of surface normal
	struct MinimumBendValue : Parameters::ParameterDescriptor<SurfaceCombParameters, 9, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "MinimumBendValue"; }

		static float DefaultValue() { return 0; }
	};

	// Maximum value for bending the strands with 1 being opposite of surface normal
	struct MaximumBendValue : Parameters::ParameterDescriptor<SurfaceCombParameters, 10, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
	{
		static char const* Name() { return "MaximumBendValue"; }

		static float DefaultValue() { return 0.5f; }
	};

	struct Sinks
	{
		struct Type : Parameters::ParameterDescriptor<SurfaceCombParameters, 101, SinkType[]>
		{
			static char const* Name() { return "Sinks.Type"; }
		};

		struct Position : Parameters::ParameterDescriptor<SurfaceCombParameters, 102, Ephere::Ornatrix::Vector3[]>
		{
			static char const* Name() { return "Sinks.Position"; }
		};

		struct Direction : Parameters::ParameterDescriptor<SurfaceCombParameters, 103, Ephere::Ornatrix::Vector3[], Parameters::Direction::In, Parameters::Purpose::Direction>
		{
			static char const* Name() { return "Sinks.Direction"; }
		};

		struct Coordinate : Parameters::ParameterDescriptor<SurfaceCombParameters, 104, Ephere::Geometry::SurfacePosition[]>
		{
			static char const* Name() { return "Sinks.Coordinate"; }
		};

		struct Length : Parameters::ParameterDescriptor<SurfaceCombParameters, 105, float[]>
		{
			static char const* Name() { return "Sinks.Length"; }
		};

		struct Group : Parameters::ParameterDescriptor<SurfaceCombParameters, 106, Ephere::Parameters::String[]>
		{
			static char const* Name() { return "Sinks.Group"; }
		};

		struct MirrorType : Parameters::ParameterDescriptor<SurfaceCombParameters, 107, SinkMirrorType[]>
		{
			static char const* Name() { return "Sinks.MirrorType"; }
		};

		struct SlopeRamp : Parameters::ParameterDescriptor<SurfaceCombParameters, 108, Ephere::Ornatrix::Ramp[]>
		{
			static char const* Name() { return "Sinks.SlopeRamp"; }
		};

		struct Rotation : Parameters::ParameterDescriptor<SurfaceCombParameters, 109, float[], Parameters::Direction::In, Parameters::Purpose::Angle>
		{
			static char const* Name() { return "Sinks.Rotation"; }

			static Parameters::AngleUnits const DefaultUnits = Parameters::AngleUnits::Radians;
		};

		struct Flow : Parameters::ParameterDescriptor<SurfaceCombParameters, 110, float[], Parameters::Direction::In, Parameters::Purpose::Fraction>
		{
			static char const* Name() { return "Sinks.Flow"; }
		};
	};

	struct StrandData
	{
		// Determines how the strand data will be assigned based on sinks
		struct Mode : Parameters::ParameterDescriptor<SurfaceCombParameters, 30, StrandDataModeType>
		{
			static char const* Name() { return "StrandData.Mode"; }

			static StrandDataModeType DefaultValue() { return StrandDataModeType::SinkSize; }
		};

		// When specified, values will be written to this channel
		struct Channel : Parameters::ParameterDescriptor<SurfaceCombParameters, 25, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "StrandData.Channel"; }
		};

		// When specified, a new channel with this name will be created and values written to it
		struct NewChannelName : Parameters::ParameterDescriptor<SurfaceCombParameters, 28, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandData.NewChannelName"; }

			static char const* DefaultValue() { return "SinkValues"; }
		};

		struct Value
		{
			// Value assigned to the biggest sinks
			struct Maximum : Parameters::ParameterDescriptor<SurfaceCombParameters, 26, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "StrandData.Value.Maximum"; }

				static float DefaultValue() { return 1.0f; }
			};

			// Value assigned to the smallest sinks
			struct Minimum : Parameters::ParameterDescriptor<SurfaceCombParameters, 29, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "StrandData.Value.Minimum"; }

				static float DefaultValue() { return 0.0f; }
			};

			// Determines how smooth the transition from minimum to maximum value should be
			struct Smooth : Parameters::ParameterDescriptor<SurfaceCombParameters, 31, float, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
			{
				static char const* Name() { return "StrandData.Value.Smooth"; }

				static float DefaultValue() { return 0.1f; }
			};
		};

		struct AbsoluteSinkSize
		{
			// When on, the ReferenceLength parameter is used to determine the maximum output value of sinks. Otherwise, shortest and longest sinks will be used to determine minimum and maximum values.
			struct Enabled : Parameters::ParameterDescriptor<SurfaceCombParameters, 32, bool>
			{
				static char const* Name() { return "StrandData.AbsoluteSinkSize.Enabled"; }

				static bool DefaultValue() { return true; }
			};

			// If "Sink Size" mode is used and "Absolute Sink Size" is on then this length determines the length of the sink corresponding to the maximum output value
			struct ReferenceLength : Parameters::ParameterDescriptor<SurfaceCombParameters, 33, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
			{
				static char const* Name() { return "StrandData.AbsoluteSinkSize.ReferenceLength"; }

				static float DefaultValue() { return 1.0f; }

				static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
			};
		};
	};

	// When on, output strand groups will be assigned based on each strand's closest sink group
	struct SetStrandGroups : Parameters::ParameterDescriptor<SurfaceCombParameters, 34, bool>
	{
		static char const* Name() { return "SetStrandGroups"; }

		static bool DefaultValue() { return false; }
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::SurfaceCombParameters::SinkType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::SurfaceCombParameters::SinkType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case SurfaceCombParameters::SinkType::Repel: return "Repel";
			case SurfaceCombParameters::SinkType::Attract: return "Attract";
			case SurfaceCombParameters::SinkType::Direct: return "Direct";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::SurfaceCombParameters::SinkOrientationType>
{
	enum { Count = 2 };

	static char const* GetValueName( Ephere::Ornatrix::SurfaceCombParameters::SinkOrientationType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case SurfaceCombParameters::SinkOrientationType::Absolute: return "Absolute";
			case SurfaceCombParameters::SinkOrientationType::Relative: return "Relative";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::SurfaceCombParameters::AlgorithmType>
{
	enum { Count = 2 };

	static char const* GetValueName( Ephere::Ornatrix::SurfaceCombParameters::AlgorithmType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case SurfaceCombParameters::AlgorithmType::MeshVertex: return "MeshVertex";
			case SurfaceCombParameters::AlgorithmType::Triangulation: return "Triangulation";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::SurfaceCombParameters::SinkMirrorType>
{
	enum { Count = 4 };

	static char const* GetValueName( Ephere::Ornatrix::SurfaceCombParameters::SinkMirrorType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case SurfaceCombParameters::SinkMirrorType::None: return "None";
			case SurfaceCombParameters::SinkMirrorType::X: return "X";
			case SurfaceCombParameters::SinkMirrorType::Y: return "Y";
			case SurfaceCombParameters::SinkMirrorType::Z: return "Z";
			default: return "";
		}
	}
};
template <> struct GetEnumInfo<Ephere::Ornatrix::SurfaceCombParameters::StrandDataModeType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::SurfaceCombParameters::StrandDataModeType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case SurfaceCombParameters::StrandDataModeType::SinkSize: return "SinkSize";
			case SurfaceCombParameters::StrandDataModeType::SinkDistance: return "SinkDistance";
			case SurfaceCombParameters::StrandDataModeType::SinkGroup: return "SinkGroup";
			default: return "";
		}
	}
};
}
