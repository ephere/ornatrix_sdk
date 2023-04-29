// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Plugins/CommonTypes.h"

namespace Ephere { namespace Plugins { namespace Ornatrix
{

/** Surface dependency:
	This class is optional for hair but mandatory to guides. It is required to update root position based
	on the deforming surface, to get correct UVW coordinates, and surface normal to generate strand transformations
*/
struct SurfaceDependency
{
	enum : unsigned
	{
		InvalidFaceIndex = static_cast<unsigned>( -1 ),

		//! Default group used for all strands
		DefaultGroup = 0,
	};

	SurfaceDependency()
		: group( DefaultGroup ),
		face( InvalidFaceIndex ),
		barycentricCoordinate( HostVector3( 0, 0, 0 ) )
	{
	}

	SurfaceDependency( unsigned short group, unsigned face, HostVector3 barycentricCoordinate )
		: group( group ),
		face( face ),
		barycentricCoordinate( barycentricCoordinate )
	{
	}

	//! Strand group index
	unsigned short group;

	//! Index into mesh face list
	unsigned face;

	//! Barycentric coordinates of this root on face
	HostVector3 barycentricCoordinate;
};

} } } // Ephere::Plugins::Ornatrix
