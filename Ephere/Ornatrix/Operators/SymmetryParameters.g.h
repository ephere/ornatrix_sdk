// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"
#include "Ephere/Ornatrix/ParameterComponents.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
} }

namespace Ephere { namespace Ornatrix {

struct SymmetryParameters
{
	enum { Version = 1 };

	enum { HighestId = 10, FirstUnusedId = 11 };

	static char const* GetName() { return "SymmetryParameters"; }

	enum class AxisType
	{
		X,
		Y,
		Z,
	};

	struct InputHair : Parameters::ParameterDescriptor<SymmetryParameters, 7, Ephere::Ornatrix::HairParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "InputHair"; }
	};

	struct OutputHair : Parameters::ParameterDescriptor<SymmetryParameters, 8, Ephere::Ornatrix::HairParameter, Parameters::Direction::Out, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "OutputHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<SymmetryParameters, 9, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<SymmetryParameters, 1, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};
	};

	struct IgnoreDistance
	{
		// When enabled, strands which are closer than specified distance to the symmetry plane will be ignored and not copied over to the other side.
		struct Enable : Parameters::ParameterDescriptor<SymmetryParameters, 2, bool>
		{
			static char const* Name() { return "IgnoreDistance.Enable"; }

			static bool DefaultValue() { return false; }
		};

		// Specifies the distance away from the symmetry plane that guides need to be in order to be copied to its other side
		struct Distance : Parameters::ParameterDescriptor<SymmetryParameters, 3, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "IgnoreDistance.Distance"; }

			static float DefaultValue() { return 1.0f; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};
	};

	// When enabled, the copied strands will be attached to the distribution surface and stick with it when it deforms.
	struct GroundMirroredStrands : Parameters::ParameterDescriptor<SymmetryParameters, 4, bool>
	{
		static char const* Name() { return "GroundMirroredStrands"; }

		static bool DefaultValue() { return true; }
	};

	struct MirrorPlane
	{
		// A predefined axis to align the mirror plane
		struct Axis : Parameters::ParameterDescriptor<SymmetryParameters, 10, AxisType>
		{
			static char const* Name() { return "MirrorPlane.Axis"; }

			static AxisType DefaultValue() { return AxisType::X; }
		};

		// Position of symmetry plane's origin
		struct Origin : Parameters::ParameterDescriptor<SymmetryParameters, 5, Ephere::Ornatrix::Vector3, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "MirrorPlane.Origin"; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};

		// Normal direction of symmetry plane's origin
		struct Normal : Parameters::ParameterDescriptor<SymmetryParameters, 6, Ephere::Ornatrix::Vector3, Parameters::Direction::In, Parameters::Purpose::UnitDirection, false, true>
		{
			static char const* Name() { return "MirrorPlane.Normal"; }
		};
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::SymmetryParameters::AxisType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::SymmetryParameters::AxisType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case SymmetryParameters::AxisType::X: return "X";
			case SymmetryParameters::AxisType::Y: return "Y";
			case SymmetryParameters::AxisType::Z: return "Z";
			default: return "";
		}
	}
};
}
