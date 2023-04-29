#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/Frizzer.h"
#include "Ephere/Ornatrix/Operators/Propagator.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"
#include "Ephere/Ornatrix/Private/TextureMapImage.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

TEST_CASE( "Frizzer" )
{
	SharedHair sharedHair;
	IHair& hair = sharedHair;
	SharedHair originalHair;

	auto const distributionMesh = GeneratePlane<Real>( 1 );
	GenerateRandomHairs( hair, 1, 1, &distributionMesh );
	originalHair.CopyFrom( sharedHair, true, true, true, true, true );

	auto verticesBeforeFrizzing = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

	auto testCurve = Ramp::IncreasingLine();

	Frizzer frizzer;
	frizzer.parameters.targetHair = &hair;
	frizzer.parameters.distributionMesh = &distributionMesh;
	frizzer.parameters.amount.ramp = testCurve;

	SECTION( "ZeroValueAtStartOnPropagatedStrands" )
	{
		Propagator propagator;
		//propagator.parameters.countPerStrand = 1;
		propagator.parameters.verticesPerRootCount = 2;
		propagator.parameters.targetHair = &hair;
		propagator.parameters.distributionMesh = &distributionMesh;
		propagator.parameters.sideCount = 1;
		propagator.parameters.rootCount = 1;

		propagator.Apply();

		hair.ValidateStrandToObjectTransforms( &distributionMesh, false );
		verticesBeforeFrizzing = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

		frizzer.parameters.amount.value = 0.00001f;

		testCurve = Ramp( 1.0f );
		frizzer.parameters.amount.ramp = testCurve;

		REQUIRE( frizzer.Compute() );

		hair.ValidateStrandToObjectTransforms( &distributionMesh, false );
		auto const verticesAfterFrizzing = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

		REQUIRE_THAT(
			verticesAfterFrizzing,
			IsVectorNearEqual( verticesBeforeFrizzing, 0.0001_r )
		);
	}

	SECTION( "MapWithZeroValues" )
	{
		TextureMapParameter mapParameter{ make_shared<TextureMapImage>( Image::ImageRgba16{ Image::PixelRgba16{ 0, 0, 0 } } ) };

		frizzer.parameters.amount.map = mapParameter;
/*
		map.SetPixel( Vector2i( 0, 0 ), { 1, 1, 1 } );
		frizzer.Compute( &distributionMesh );
		hair.ValidateStrandToObjectTransforms( &distributionMesh, false );
		const auto verticesAfterFullFrizzing = hair.GetVertices( IHair::CoordinateSpace::Object );*/

		GenerateRandomHairs( hair, 4, 1, &distributionMesh, 0, 5 );
		verticesBeforeFrizzing = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

		frizzer.Compute();
		hair.ValidateStrandToObjectTransforms( &distributionMesh, false );
		auto const verticesAfterBlackMapFrizzing = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

		// With map values all black we should get no frizzing
		REQUIRE_THAT(
			verticesAfterBlackMapFrizzing,
			IsVectorNearEqual( verticesBeforeFrizzing, 0.0001_r )
		);
	}

	SECTION( "ShortStrandExplosion" )
	{
		GenerateRandomHairs( hair, 2, 1, &distributionMesh );

		// Shorten strand 0:
		std::vector<Vector3> vertices( hair.GetStrandPointCount( 0 ) );
		hair.GetVertices( 0, Size( vertices ), vertices.data(), IHair::CoordinateSpace::Strand );
		auto segment = vertices[1] - vertices[0];
		segment.length( 0.01f );
		vertices[1] = vertices[0] + segment;
		hair.SetVertices( 0, Size( vertices ), vertices.data(), IHair::CoordinateSpace::Strand );

		//originalHair.CopyFrom( sharedHair, true, true, true, true, true );
		testCurve = Ramp( 1.0f );
		frizzer.parameters.amount.ramp = testCurve;

		hair.ValidateStrandToObjectTransforms( &distributionMesh, false );
		verticesBeforeFrizzing = sharedHair.GetVertices( IHair::CoordinateSpace::Strand );

		frizzer.parameters.amount.value = 10;
		frizzer.Compute();

		hair.ValidateStrandToObjectTransforms( &distributionMesh, false );
		auto const verticesAfterFrizzing = sharedHair.GetVertices( IHair::CoordinateSpace::Strand );

		auto const length1 = HairUtilities( hair ).GetStrandLength( 0 );
		auto const length2 = HairUtilities( hair ).GetStrandLength( 1 );

		REQUIRE( length1 * 100 < length2 );
	}

#ifdef DOUBLE_PRECISION
	SECTION( "FloatingPointPrecision" )
	{
		frizzer.parameters.amount.value = 5.0f;
		TestMicroInputModificationsEffectOnResult( sharedHair, originalHair, distributionMesh, [&frizzer]( int const iteration )
		{
			frizzer.parameters.randomSeed = iteration + 1;
			frizzer.Compute();
		} );
	}
#endif
	}
	}
