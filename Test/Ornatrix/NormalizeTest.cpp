#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/Normalize.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

TEST_CASE( "Normalize" )
{
	SharedHair sharedHair;
	IHair& hair = sharedHair;

	auto const distributionMesh = GeneratePlane<Real>( 1 );

	Normalize normalize;
	normalize.parameters.targetHair = &hair;
	normalize.parameters.distributionMesh = &distributionMesh;

	SECTION( "ZeroLengthStrand" )
	{
		static auto const PointCount = 10;
		GenerateRandomHairs( hair, 20, 0, &distributionMesh, 0, PointCount );

		auto const verticesBeforeApplication = hair.GetStrandPoints( 0, IHair::CoordinateSpace::Object );

		REQUIRE( normalize.Compute() );

		auto const verticesAfterApplication = hair.GetStrandPoints( 0, IHair::CoordinateSpace::Object );

		REQUIRE_THAT(
			vector( PointCount, verticesBeforeApplication.front() ),
			IsVectorNearEqual( verticesAfterApplication )
		);
	}

	SECTION( "PerStrandGroup" )
	{
		const static auto PointCount = 10;
		GenerateRandomHairs( hair, 2000, 1, &distributionMesh, 0, PointCount );

		// Set strand group of 2 for all strands with positive x tip coordinate
		hair.SetUsesStrandGroups( true );
		auto const tipPositions = HairUtilities( hair ).GetTipPositions();
		auto const strandGroups = Transform( tipPositions, []( auto const& position )
		{
			return position.x() < 0 ? 1 : 2;
		} );
		hair.SetStrandGroups( strandGroups );

		SharedHair originalHair;
		originalHair.CopyFrom( sharedHair, true, true, true, true, true );

		REQUIRE( normalize.Compute() );

		const auto verticesAfterApplication = hair.GetStrandPoints( 0, IHair::CoordinateSpace::Object );

		IHair& hair2 = originalHair;
		normalize.parameters.targetHair = &hair2;
		normalize.parameters.perStrandGroup = true;

		REQUIRE( normalize.Compute() );

		const auto verticesAfterApplication2 = hair2.GetStrandPoints( 0, IHair::CoordinateSpace::Object );

		REQUIRE_THAT(
			vector( PointCount, verticesAfterApplication.front() ),
			!IsVectorNearEqual( verticesAfterApplication2 )
		);
	}
}
}
