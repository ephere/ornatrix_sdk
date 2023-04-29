// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"

namespace Ephere { namespace Ornatrix {
struct HairParameter;
struct PolygonMeshParameter;
struct StrandChannel;
struct TextureMapParameter;
} }

namespace Ephere { namespace Ornatrix {

struct FilterParameters
{
	enum { Version = 1 };

	enum { HighestId = 11, FirstUnusedId = 12 };

	static char const* GetName() { return "FilterParameters"; }

	enum class PredicateType
	{
		Random,
		Even,
		ByGroup,
		ByIndex,
	};

	struct InputHair : Parameters::ParameterDescriptor<FilterParameters, 5, Ephere::Ornatrix::HairParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "InputHair"; }
	};

	struct OutputHair : Parameters::ParameterDescriptor<FilterParameters, 6, Ephere::Ornatrix::HairParameter, Parameters::Direction::Out, Parameters::Purpose::Undefined, true>
	{
		static char const* Name() { return "OutputHair"; }
	};

	struct DistributionMesh : Parameters::ParameterDescriptor<FilterParameters, 7, Ephere::Ornatrix::PolygonMeshParameter, Parameters::Direction::In, Parameters::Purpose::DistributionMesh>
	{
		static char const* Name() { return "DistributionMesh"; }
	};

	struct StrandGroup
	{
		struct Pattern : Parameters::ParameterDescriptor<FilterParameters, 1, Ephere::Parameters::String>
		{
			static char const* Name() { return "StrandGroup.Pattern"; }

			static char const* DefaultValue() { return ""; }
		};

		struct BlendDistance : Parameters::ParameterDescriptor<FilterParameters, 11, float, Parameters::Direction::In, Parameters::Purpose::Distance, false, true>
		{
			static char const* Name() { return "StrandGroup.BlendDistance"; }

			static float DefaultValue() { return 0; }

			static Parameters::DistanceUnits const DefaultUnits = Parameters::DistanceUnits::Centimeters;
		};
	};

	// Determines how the strands are filtered out
	struct Predicate : Parameters::ParameterDescriptor<FilterParameters, 2, PredicateType>
	{
		static char const* Name() { return "Predicate"; }

		static PredicateType DefaultValue() { return PredicateType::Random; }
	};

	// When using random predicate specifies what fraction of hair will be filtered out
	struct Fraction
	{
		struct Value : Parameters::ParameterDescriptor<FilterParameters, 3, float, Parameters::Direction::In, Parameters::Purpose::Fraction, false, true>
		{
			static char const* Name() { return "Fraction.Value"; }

			static float DefaultValue() { return 0.5f; }
		};

		struct Channel : Parameters::ParameterDescriptor<FilterParameters, 9, Ephere::Ornatrix::StrandChannel>
		{
			static char const* Name() { return "Fraction.Channel"; }
		};

		struct Map : Parameters::ParameterDescriptor<FilterParameters, 8, Ephere::Ornatrix::TextureMapParameter, Parameters::Direction::In, Parameters::Purpose::Undefined, false, true>
		{
			static char const* Name() { return "Fraction.Map"; }
		};
	};

	// Pattern which specifies which strand groups or indices, depending on predicate type, will be filtered out
	struct FilteredPattern : Parameters::ParameterDescriptor<FilterParameters, 4, Ephere::Parameters::String>
	{
		static char const* Name() { return "FilteredPattern"; }

		static char const* DefaultValue() { return ""; }
	};

	struct RandomSeed : Parameters::ParameterDescriptor<FilterParameters, 10, int, Parameters::Direction::In, Parameters::Purpose::RandomSeed, false, true>
	{
		static char const* Name() { return "RandomSeed"; }

		static int DefaultValue() { return 1; }
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::FilterParameters::PredicateType>
{
	enum { Count = 4 };

	static char const* GetValueName( Ephere::Ornatrix::FilterParameters::PredicateType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case FilterParameters::PredicateType::Random: return "Random";
			case FilterParameters::PredicateType::Even: return "Even";
			case FilterParameters::PredicateType::ByGroup: return "ByGroup";
			case FilterParameters::PredicateType::ByIndex: return "ByIndex";
			default: return "";
		}
	}
};
}
