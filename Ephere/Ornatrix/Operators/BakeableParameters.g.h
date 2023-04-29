// Public parameter definition - generated automatically

// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Core/Parameters/Parameters.h"
#include "Ephere/Ornatrix/Private/GuidesDelta.h"


namespace Ephere { namespace Ornatrix {

struct BakeableParameters
{
	enum { Version = 1 };

	enum { HighestId = 2, FirstUnusedId = 3 };

	static char const* GetName() { return "BakeableParameters"; }

	enum class ChangeCacheBehaviorType
	{
		Off,
		Stored,
		Precomputed,
	};

	// Controls the behavior of caching changes created by this operator
	struct ChangeCacheBehavior : Parameters::ParameterDescriptor<BakeableParameters, 1, ChangeCacheBehaviorType>
	{
		static char const* Name() { return "ChangeCacheBehavior"; }

		static ChangeCacheBehaviorType DefaultValue() { return ChangeCacheBehaviorType::Off; }
	};

	struct Delta : Parameters::ParameterDescriptor<BakeableParameters, 2, SharedPtr<GuidesDelta>>
	{
		static char const* Name() { return "Delta"; }
	};

	struct Descriptor;
	struct Container;
};

} }

namespace Ephere
{
template <> struct GetEnumInfo<Ephere::Ornatrix::BakeableParameters::ChangeCacheBehaviorType>
{
	enum { Count = 3 };

	static char const* GetValueName( Ephere::Ornatrix::BakeableParameters::ChangeCacheBehaviorType value )
	{
		using namespace Ephere::Ornatrix;
		switch( value )
		{
			case BakeableParameters::ChangeCacheBehaviorType::Off: return "Off";
			case BakeableParameters::ChangeCacheBehaviorType::Stored: return "Stored";
			case BakeableParameters::ChangeCacheBehaviorType::Precomputed: return "Precomputed";
			default: return "";
		}
	}
};
}
