#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/Filter.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

TEST_CASE( "Filter" )
{
	SharedHair inputHair, outputHair;

	auto const distributionMesh = GeneratePlane<Real>( 1 );
	GenerateRandomHairs( inputHair, 4, 1, &distributionMesh );

	// This mimics how operators are applied in practice, where output hair is passed in as a copy of input hair
	outputHair = inputHair;

	Filter filter;
	filter.parameters.inputHair = &inputHair;
	filter.parameters.outputHair = &outputHair;

	SECTION( "ZeroOutput" )
	{
		filter.parameters.fraction.value = 0_r;
		filter.Compute();

		REQUIRE( outputHair.GetStrandCount() == 0 );
		REQUIRE( outputHair.GetVertexCount() == 0 );
	}
}

}
