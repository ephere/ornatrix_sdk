#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/Curler.h"
#include "Ephere/Ornatrix/Operators/Propagator.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

TEST_CASE( "CurlerBasic" )
{
	SharedHair sharedHair;
	IHair& hair = sharedHair;

	{
		constexpr auto StrandCount = 2;
		constexpr auto PointCount = 5;
		hair.SetUsesStrandTopology( false );
		hair.SetGlobalStrandPointCount( PointCount );
		hair.SetUseStrandToObjectTransforms( true );

		hair.SetStrandCount( StrandCount );
		hair.SetVertexCount( StrandCount * PointCount );
		hair.SetStrandPoints( 0, 0, PointCount, std::array<Vector3, 6>{ { { 0, 0, 0 }, { 0, 0, 1 }, { 0, 0, 2 }, { 0, 0, 3 }, { 0, 0, 4 } } }.data(), IHair::CoordinateSpace::Strand );
		hair.SetStrandPoints( 1, 0, PointCount, std::array<Vector3, 5>{ { { 0, 10, 0 }, { 0, 10, 1 }, { 0, 10, 2 }, { 0, 10, 3 }, { 0, 10, 4 } } }.data(), IHair::CoordinateSpace::Strand );
	}

	Curler curler;
	curler.parameters.targetHair = &hair;
	curler.parameters.phase.value = 2.0f;
	// To match the old default
	curler.parameters.magnitude.ramp = Ramp( 1 );

	curler.Compute();

	REQUIRE( hair.GetStrandCount() == 2 );

	REQUIRE_THAT(
		hair.GetStrandPoints( 0, IHair::Strand ),
		IsVectorNearEqual( vector<Vector3>{ { 0, 0, 0 }, { 0, -2, 1 }, { 0, 0, 2 }, { 0, -2, 3 }, { 0, 0, 4 } } )
	);
	REQUIRE_THAT(
		hair.GetStrandPoints( 1, IHair::Strand ),
		IsVectorNearEqual( vector<Vector3>{ { 0, 10, 0 }, { 0, 8, 1 }, { 0, 10, 2 }, { 0, 8, 3 }, { 0, 10, 4 } } )
	);
}

TEST_CASE( "Curler" )
{
	SharedHair sharedHair;
	IHair& hair = sharedHair;
	SharedHair originalHair;

	auto const distributionMesh = GeneratePlane<Real>( 1 );
	GenerateRandomHairs( hair, 1, 1, &distributionMesh );
	originalHair.CopyFrom( sharedHair, true, true, true, true, true );

	auto verticesBeforeFrizzing = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

	auto const testCurve = Ramp::IncreasingLine();

	Curler curler;
	curler.parameters.targetHair = &hair;
	curler.parameters.distributionMesh = &distributionMesh;
	curler.parameters.magnitude.ramp = testCurve;

#ifdef DOUBLE_PRECISION
	SECTION( "FloatingPointPrecision" )
	{
		auto constexpr iterationCount = 10;
		vector<Real> phaseOffsets( iterationCount );
		GenerateRandom( begin( phaseOffsets ), end( phaseOffsets ), 1, 0, TwoPir );
		
		curler.parameters.magnitude.value = 5.0f;
		
		TestMicroInputModificationsEffectOnResult( sharedHair, originalHair, distributionMesh, [&curler, &phaseOffsets]( auto const iteration )
		{
			curler.parameters.phaseOffset = static_cast<float>( phaseOffsets[iteration] );
			//curler.parameters.randomSeed = iteration + 1;
			curler.Compute();
		}, 0.01f, iterationCount, 0.01f );
	}
#endif
}

// #6171
TEST_CASE( "TwoCurlersOnDifferentGroupsAreIndependent" )
{
	SharedHair sharedHair;
	IHair& hair = sharedHair;
	SharedHair originalHair;

	auto const distributionMesh = GeneratePlane<Real>( 1 );
	GenerateHairs( hair, array{ SurfacePosition { 0, { 0.5f, 0.0f } }, SurfacePosition { 0, { 0.5f, 1.0f } } }, 1, &distributionMesh );

	hair.SetUsesStrandGroups( true );
	hair.SetStrandGroups( array{ 1, 2 } );

	originalHair.CopyFrom( sharedHair );

	Curler curler1, curler2;
	curler1.parameters.phase.value = 0.2_r;
	curler2.parameters.phase.value = 0.2_r;
	curler1.parameters.magnitude.value = 1_r;
	curler2.parameters.magnitude.value = 1_r;
	curler1.parameters.targetHair = &hair;
	curler2.parameters.targetHair = &hair;
	curler1.parameters.distributionMesh = &distributionMesh;
	curler2.parameters.distributionMesh = &distributionMesh;

	curler1.parameters.strandGroup.pattern = "1";
	curler2.parameters.strandGroup.pattern = "2";

	curler1.Compute();
	curler2.Compute();

	auto const pointsAfterEvaluation1 = hair.GetStrandPoints( 1, IHair::CoordinateSpace::Object );

	// Modify parameters of first curler and re-evaluate
	hair.CopyFrom( originalHair );

	curler1.parameters.magnitude.value = 10_r;
	curler1.Compute();
	curler2.Compute();

	auto const pointsAfterEvaluation2 = hair.GetStrandPoints( 1, IHair::CoordinateSpace::Object );

	// The shape of second strand should not be affected by changes done to first strand
	REQUIRE_THAT( pointsAfterEvaluation1, IsVectorNearEqual( pointsAfterEvaluation2 ) );
}

}
