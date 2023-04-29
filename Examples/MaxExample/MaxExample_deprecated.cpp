/**! @file MaxExample_deprecated.cpp
Example usage of old (deprecated) Ornatrix core interfaces (major version 5 or earlier).

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

	//! The radius of hair strand this vertex
	float radius;
};

//! A pretty generic structure to represent a single hair strand
struct HairStrand
{
	//! Vertices making up a poly-line representing the hair strand
	vector<HairVertex> verticesInObjectSpace;
};

/** Extract hair information from Ornatrix hair
@param sourceHair Source hair object
@param sourceRenderingProperties Source rendering properties object
@param resultStrands Result strands
*/
void ExtractHairInformation_deprecated( const IHair__deprecated& sourceHair, const IHairRenderingPropertiesContainer& sourceRenderingProperties, vector<HairStrand>& resultStrands )
{
	const auto strandCount = sourceHair.GetStrandCount__deprecated();
	resultStrands.resize( strandCount );

	// Index of the mapping channel for which to return UV coordinates. It is 0 by default but can be anything if multiple mapping channels are used and a specific
	// one is needed.
	static const unsigned MappingChannelIndex = 0;

	// Iterate over all the strands in the hair
	for( auto strandIndex = 0u; strandIndex < strandCount; strandIndex++ )
	{
		auto& resultStrand = resultStrands[strandIndex];

		const auto pointCount = sourceHair.GetStrandPointCount__deprecated( strandIndex );
		resultStrand.verticesInObjectSpace.resize( pointCount );

		// Iterate over the strand's points
		for( auto pointIndex = 0u; pointIndex < pointCount; pointIndex++ )
		{
			auto& resultVertex = resultStrand.verticesInObjectSpace[strandIndex];

			// Get the object-space position of the point
			const auto point = sourceHair.GetStrandPointInObjectCoordinates( strandIndex, pointIndex );
			resultVertex.positionInObjectCoordinate[0] = float( point.x );
			resultVertex.positionInObjectCoordinate[1] = float( point.y );
			resultVertex.positionInObjectCoordinate[2] = float( point.z );

			// Get the texture coordinate at point
			const auto textureCoordinate = sourceHair.GetTextureCoordinate( strandIndex, pointIndex, MappingChannelIndex );
			resultVertex.textureCoordinate[0] = float( textureCoordinate.x );
			resultVertex.textureCoordinate[1] = float( textureCoordinate.y );

			// Try to find the rendering properties for the strand group to which this strand belongs
			const IHairRenderingProperties* renderingProperties = nullptr;
			if( sourceHair.KeepsSurfaceDependency() )
			{
				renderingProperties = sourceRenderingProperties.GetHairRenderingProperties( sourceHair.GetSurfaceDependency__deprecated( strandIndex ).group );
			}

			// If there are no rendering properties specifically for the strand group we requested just use the default ones
			if( renderingProperties == nullptr || !renderingProperties->IsUsed() )
			{
				// If rendering properties were turned off
				resultVertex.radius = 1.0f;
			}
			else
			{
				resultVertex.radius = renderingProperties->GetGlobalStrandRadius();

				// If a curve is used multiply radius by it
				if( renderingProperties->GetRadiusCurve() != nullptr )
				{
					const auto positionAlongStrand = float( pointIndex ) / float( pointCount - 1 );
					resultVertex.radius *= renderingProperties->GetRadiusCurve()->Evaluate( positionAlongStrand );
				}

				// If a thickness map is used multiply radius by it
				if( renderingProperties->GetRadiusMap() != nullptr )
				{
					// Use whichever evaluation function current host supports to evaluate the texture at current texture coordinate.
					// For example, in 3dsmax, this would involve calling Texmap::EvalMono
#if FALSE
					resultVertex.radius *= Evaluate( renderingProperties->GetRadiusMap(), textureCoordinate );
#endif
				}

				// If a strand channel is provided multiply radius by it
				if( renderingProperties->GetRadiusStrandDataChannel() != 0 )
				{
					// TODO: Provide sample code
				}
			}
		}
	}
}

/** Extract hair information from a 3dsmax geometry object
@param object 3dsmax object
@param sourceRenderingProperties Source rendering properties object
@param resultStrands Result strands
*/
void ExtractHairInformation_deprecated( Object& object, vector<HairStrand>& resultStrands )
{
	IHair__deprecated* sourceHair;
	IHairRenderingPropertiesContainer* sourceRenderingProperties;
	GetHairInterfaces( object, sourceHair, sourceRenderingProperties );
	ExtractHairInformation_deprecated( *sourceHair, *sourceRenderingProperties, resultStrands );
}



#define ScriptPrint (the_listener->edit_stream->printf)

def_visible_primitive( OxPrintStrandCount_deprecated, "OxPrintStrandCount_deprecated" );

Value* OxPrintStrandCount_deprecated_cf( Value **arg_list, int count )
{
	check_arg_count( OxPrintStrandCount, 1, count );
	if( auto node = arg_list[0]->to_node() )
	{
		IHair__deprecated* hair;
		IHairRenderingPropertiesContainer* renderingProperties;
		if( GetHairInterfaces( *node, hair, renderingProperties ) )
		{
			ScriptPrint( _M( "Ornatrix hair interface found, strand count: %d\n" ), hair->GetStrandCount__deprecated() );
		}
		else
		{
			ScriptPrint( _M( "Object is not Ornatrix hair\n" ) );
		}
	}

	return &ok;
}
