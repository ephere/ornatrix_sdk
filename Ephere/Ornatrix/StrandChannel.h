// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Ornatrix/StrandChannelType.h"
#include "Ephere/Ornatrix/Types.h"

namespace Ephere { namespace Ornatrix
{

/** Utility structure containing both type and index used to identify a per-strand, per-vertex, or mesh color channel of a strands object */
struct StrandChannel
{
	StrandChannelType type;

	int index;


	StrandChannel():
		type( StrandChannelType_Unassigned ),
		index( 0 )
	{
	}

	StrandChannel( StrandChannelType type, int index ):
		type( type ),
		index( index )
	{
	}

	EPHERE_NODISCARD bool IsEmpty() const
	{
		return type == StrandChannelType_Unassigned;
	}

	EPHERE_NODISCARD bool IsAssigned() const
	{
		return !IsEmpty();
	}

	/** Prepares mesh's vertex colors to be used in this channel if type is set to vertex colors */
	void PrepareVertexColors( ITriangleMesh& mesh ) const
	{
		if( type == StrandChannelType_PerDistributionMeshVertex )
		{
			mesh.PrepareVertexColorSet( index );
		}
	}

	/** Prepares mesh's vertex colors to be used in this channel if type is set to vertex colors */
	void PrepareVertexColors( IPolygonMeshSA const& meshAccess ) const;

	EPHERE_NODISCARD bool IsApplicableTo( IHair const& guides ) const;

	static StrandChannel GetStrandChannelFromIdentifier( unsigned channelIdentifier );

	EPHERE_NODISCARD unsigned GetStrandChannelIdentifier() const;

	bool operator ==( StrandChannel const& other ) const
	{
		return type == other.type && index == other.index;
	}

	bool operator !=( StrandChannel const& other ) const
	{
		return !( *this == other );
	}

};

} }
