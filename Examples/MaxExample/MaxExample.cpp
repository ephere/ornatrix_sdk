/**! @file MaxExample.cpp
The purpose of this file is to demonstrate, in a basic way, the usage of Ornatrix core host-independent interfaces. It is
not meant to be used for other purposes.

To compile/link this file fix the header and library search paths in LocalBuild.props or in the project properties
*/

// Ornatrix interfaces use host-specific types, so we need to specify the host
#define AUTODESK_3DSMAX

// Include Ornatrix interfaces and constructs
#include "Ephere/Plugins/Ornatrix/HairInterfacesMax.h"

#pragma warning( push, 2 )
#if MAX_RELEASE >= 13900 // R14 (2012) and above
#include "MAXScript/MAXScript.h"
#include "MAXScript/util/listener.h"
#include "MAXScript/macros/define_instantiation_functions.h"
#else
#include "MAXScrpt/maxscrpt.h"
#include "MAXScrpt/Listener.h"
#include "MAXScrpt/definsfn.h"
#endif
#pragma warning( pop )

#include <vector>

#pragma comment( lib, "maxscrpt.lib" )

using namespace Ephere::Plugins::Ornatrix;
using namespace std;

//! A pretty generic structure to represent a hair vertex
struct HairVertex
{
	//! The XYZ position of the vertex in object coordinates
	float positionInObjectCoordinate[3];

	//! The UV texture coordinate of this vertex
	float textureCoordinate[2];

	//! The width (diameter) of hair strand this vertex
	float width;
};

//! A pretty generic structure to represent a single hair strand
struct HairStrand
{
	//! Vertices making up a poly-line representing the hair strand
	vector<HairVertex> verticesInObjectSpace;
};

/** Extract hair information from Ornatrix hair
@param sourceHair Source hair object
@param resultStrands Result strands
*/
void ExtractHairInformation( const IHair& sourceHair, vector<HairStrand>& resultStrands )
{
	// Get the total number of strands in the source hair class
	const auto strandCount = sourceHair.GetStrandCount();

	// Allocate the storage where we will store the resulting hair information
	resultStrands.resize( strandCount );

	// Index of the mapping channel for which to return UV coordinates. It is 0 by default but can be anything if multiple mapping channels are used and a specific
	// one is needed.
	static const unsigned MappingChannelIndex = 0;

	// Get an array of integers specifying how many points there are in each strand of the source hair and indices of the first vertex in global vertices array for each strand.
	// These will be used to get hair information inside the loop below.
	std::vector<int> pointCounts( strandCount ), firstVertexIndices( strandCount );
	sourceHair.GetStrandPointCounts( 0, int( pointCounts.size() ), pointCounts.data() );
	sourceHair.GetStrandFirstVertexIndices( 0, int( firstVertexIndices.size() ), firstVertexIndices.data() );

	// Define the storage where we will store vertex positions, texture coordinates, and widths for each hair point
	std::vector<Vector3> points, textureCoordinates;
	std::vector<float> widths;

	// Iterate over all the strands in the hair
	auto pointCountIterator = pointCounts.begin(), firstVertexIndexIterator = firstVertexIndices.begin();
	for( auto strandIndex = 0; strandIndex < strandCount; ++strandIndex, ++pointCountIterator, ++firstVertexIndexIterator )
	{
		// We will be populating this strand
		auto& resultStrand = resultStrands[strandIndex];

		// The number of vertices in this strand
		const auto pointCount = *pointCountIterator;

		// Index of the first vertex defining this strand in the hair's global vertex array
		const auto firstVertexIndex = *firstVertexIndexIterator;

		resultStrand.verticesInObjectSpace.resize( pointCount );

		// Get strand vertex positions in object coordinates.
		if( int( points.size() ) != pointCount )
		{
			points.resize( pointCount );
		}

		sourceHair.GetStrandPoints( strandIndex, 0, int( points.size() ), points.data(), IHair::Object );

		// Get texture coordinates of each point in the strand. Note that we're only getting one mapping channel. There can be multiple channels in the hair, each
		// containing its own set of texture coordinates.
		if( int( textureCoordinates.size() ) != pointCount )
		{
			textureCoordinates.resize( pointCount );
		}

		sourceHair.GetTextureCoordinates( MappingChannelIndex, firstVertexIndex, int( textureCoordinates.size() ), textureCoordinates.data(), IHair::PerVertex );

		// Get the array of width values for the current strand. If width was never defined in the hair default values will be returned.
		if( int( widths.size() ) != pointCount )
		{
			widths.resize( pointCount );
		}

		sourceHair.GetWidths( firstVertexIndex, int( widths.size() ), widths.data() );

		// Iterate over the strand's points
		auto pointIterator = points.begin();
		auto textureCoordinateIterator = textureCoordinates.begin();
		auto resultVertexIterator = resultStrand.verticesInObjectSpace.begin();
		auto widthIterator = widths.begin();
		for( auto pointIndex = 0; pointIndex < pointCount; ++pointIndex, ++pointIterator, ++resultVertexIterator, ++textureCoordinateIterator, ++widthIterator )
		{
			auto& resultVertex = *resultVertexIterator;

			// Get the object-space position of the point
			resultVertex.positionInObjectCoordinate[0] = pointIterator->x();
			resultVertex.positionInObjectCoordinate[1] = pointIterator->y();
			resultVertex.positionInObjectCoordinate[2] = pointIterator->z();

			// Get the texture coordinate at point
			resultVertex.textureCoordinate[0] = textureCoordinateIterator->x();
			resultVertex.textureCoordinate[1] = textureCoordinateIterator->y();

			// Set the width of the vertex
			resultVertex.width = *widthIterator;
		}
	}
}

/** Extract hair information from a 3dsmax geometry object into a generic structure representing hair strands
@param object 3dsmax object
@param resultStrands Result strands
*/
void ExtractHairInformation( Object& object, vector<HairStrand>& resultStrands )
{
	if( auto sourceHair = GetHairInterface( object ) )
	{
		ExtractHairInformation( *sourceHair, resultStrands );
	}
}



#define ScriptPrint (the_listener->edit_stream->printf)

def_visible_primitive( OxPrintStrandCount, "OxPrintStrandCount" );

Value* OxPrintStrandCount_cf( Value **arg_list, int count )
{
	check_arg_count( OxPrintStrandCount, 1, count );
	if( const auto node = arg_list[0]->to_node() )
	{
		if( auto hair = GetHairInterface( *node ) )
		{
			ScriptPrint( _M( "Ornatrix hair interface found, strand count: %d\n" ), hair->GetStrandCount() );
		}
		else
		{
			ScriptPrint( _M( "Object is not Ornatrix hair\n" ) );
		}
	}

	return &ok;
}


HINSTANCE DllInstance;

BOOL APIENTRY DllMain( HINSTANCE dllInstance, DWORD reason, LPVOID /*lpvReserved*/ )
{
	switch( reason )
	{
		case DLL_PROCESS_ATTACH:
			DllInstance = dllInstance;
			break;
		default:
			break;
	}

	return TRUE;
}

#if 1
void LibInit()
{
}
#else
int LibNumberClasses()
{
	return 0;
}

ClassDesc* LibClassDesc( int index )
{
	return nullptr;
}
#endif

const TCHAR* LibDescription()
{
	return _T( "Ornatrix Max Example" );
}

ULONG LibVersion()
{
	// Return the version of the Max SDK
	return VERSION_3DSMAX;
}

ULONG CanAutoDefer()
{
	return FALSE;
}
