#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/MatrixUtilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/Propagator.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

TEST_CASE( "Propagator" )
{
	Propagator propagator;

	SharedHair sharedHair;
	IHair& hair = sharedHair;
	SharedHair originalHair;

	auto const distributionMesh = GeneratePlane<Real>( 1 );
	GenerateRandomHairs( hair, 1, 1, &distributionMesh );
	originalHair.CopyFrom( sharedHair, true, true, true, true, true );

	SECTION( "ZeroLength" )
	{
		propagator.parameters.targetHair = &hair;
		propagator.parameters.distributionMesh = &distributionMesh;
		propagator.parameters.rootCount = 1;
		propagator.parameters.sideCount = 1;
		//propagator.parameters.countPerStrand = 2;
		propagator.parameters.verticesPerRootCount = 2;
		propagator.parameters.length.amount.value = 0.0f;
		propagator.parameters.length.randomness = 0.0f;

		propagator.Apply();
		hair.ValidateStrandToObjectTransforms( &distributionMesh );

		auto const verticesAfterPropagation = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

		REQUIRE( verticesAfterPropagation.size() == 4 );
		REQUIRE( GetPointLineSegmentDistanceSquared( verticesAfterPropagation[2], verticesAfterPropagation[0], verticesAfterPropagation[1] ) < Epsilon );
		REQUIRE( GetPointLineSegmentDistanceSquared( verticesAfterPropagation[3], verticesAfterPropagation[0], verticesAfterPropagation[1] ) < Epsilon );

		auto const createdStrandToObjectTransform = HairUtilities( hair ).GetStrandToObjectTransform( 1 );
		REQUIRE_THAT( createdStrandToObjectTransform, !IsNearEqual( Xform3::Zero() ) );
	}
}
}
