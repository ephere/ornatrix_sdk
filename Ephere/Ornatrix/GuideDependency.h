// Must compile with VC 2012 / GCC 4.8

#pragma once

namespace Ephere { namespace Ornatrix
{

//! Using dependency on 3 guides (max)
int const MaxGuideInterpolationCount = 3;

/** Guide dependency struct:
	Mostly used with hair this class allows every hair strand to point to the guide that it depends on.
	Most useful implementation for this is channel inheritance, but it can be also used for updating strand
	shape, export to game engines, and hair hierarchy maintenance
*/
struct GuideDependency
{
	enum : unsigned
	{
		InvalidRootIndex = static_cast<unsigned>( -1 ),
	};

	//! Index of MaxGuideInterpolationCount closest guides
	// TODO: Change to StrandId
	std::array<unsigned, MaxGuideInterpolationCount> closestRootIndices;

	//! Distances to MaxGuideInterpolationCount closest guides
	std::array<float, MaxGuideInterpolationCount> closestRootDistances;
};

struct GuideDependency2
{
	unsigned strandId;
	float weight;
};

} } // Ephere::Ornatrix
