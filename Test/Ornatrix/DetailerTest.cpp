#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/Detailer.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

TEST_CASE( "Detailer" )
{
	SharedHair sharedHair;
	IHair& hair = sharedHair;
	SharedHair originalHair;

	auto const distributionMesh = GeneratePlane<Real>( 1 );
	GenerateZigzagHairs( hair, 1, 21, 14, 1, 2, 0, &distributionMesh );
	originalHair.CopyFrom( sharedHair, true, true, true, true, true );

	IHair& sourceHair = originalHair;
	Detailer detailer;
	detailer.parameters.isRendering = true;

	SECTION( "ExplodingStrandPoints" )
	{
		detailer.parameters.pointCount.render = 40;
		detailer.parameters.smoothing.pointCount = 20;
		detailer.parameters.smoothing.amount = 0;
		detailer.parameters.inputHair = &sourceHair;
		detailer.parameters.outputHair = &hair;
		detailer.parameters.distributionMesh = &distributionMesh;

		detailer.Apply();

		hair.ValidateStrandToObjectTransforms( &distributionMesh, false );
		auto const verticesWithoutSmoothing = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

		detailer.parameters.smoothing.amount = 1;
		detailer.Apply();
		hair.ValidateStrandToObjectTransforms( &distributionMesh, false );
		auto const verticesWithSmoothing = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

		REQUIRE_THAT(
			verticesWithoutSmoothing,
			IsVectorNearEqual( verticesWithSmoothing, 0.1_r )
		);
	}

	SECTION( "SmoothZeroLengthStrand" )
	{
		// Generate per-vertex zero length strands with 10 points each
		GenerateRandomHairs( hair, 1, 0.0f, &distributionMesh, 0.0f, 10 );
		originalHair.CopyFrom( hair );

		// Make sure smoothing is enabled
		detailer.parameters.pointCount.render = 5;
		detailer.parameters.smoothing.pointCount = 4;
		detailer.parameters.smoothing.amount = 0.5f;
		detailer.parameters.inputHair = &sourceHair;
		detailer.parameters.outputHair = &hair;
		detailer.parameters.distributionMesh = &distributionMesh;

		detailer.Apply();

		// Nothing should be changed
		vector const expectedVertices( detailer.parameters.pointCount.render(), sourceHair.GetVertices( IHair::CoordinateSpace::Object )[0] );
		REQUIRE_THAT(
			expectedVertices,
			IsVectorNearEqual( sharedHair.GetVertices( IHair::CoordinateSpace::Object ) )
		);
	}
}

TEST_CASE( "DetailerPerVertexUvs" )
{
	SharedHair hair;
	HairUtilities const hairUtilities( hair );
	hairUtilities.AppendStrand( { 0u, { 0.5_r, 0.5_r } }, array{ Vector3::Zero(), Vector3::ZAxis() } );

	hair.SetTextureCoordinateChannelCount( 1 );

	array const textureCoordinates = { TextureCoordinate { 0_r, 0_r, 0_r }, TextureCoordinate { 1_r, 0_r, 0_r } };
	hair.SetTextureCoordinates( 0, 0, Size( textureCoordinates ), textureCoordinates.data(), IHair::StrandDataType::PerVertex );

	Detailer detailer;
	detailer.parameters.isRendering = true;
	detailer.parameters.pointCount.render = 3;
	detailer.parameters.inputHair = &hair;

	SharedHair resultHair;
	detailer.parameters.outputHair = &resultHair;

	detailer.Apply();

	REQUIRE( resultHair.GetVertexCount() == 3 );
	REQUIRE( resultHair.GetTextureCoordinateChannelCount() == 1 );

	array<TextureCoordinate, 3> resultTextureCoordinates;
	resultHair.GetTextureCoordinates( 0, 0, Size( resultTextureCoordinates ), resultTextureCoordinates.data(), IHair::StrandDataType::PerVertex );
	REQUIRE_THAT( resultTextureCoordinates[0], IsNearEqual( TextureCoordinate{ 0_r, 0_r, 0_r } ) );
	REQUIRE_THAT( resultTextureCoordinates[1], IsNearEqual( TextureCoordinate{ 0.5_r, 0_r, 0_r } ) );
	REQUIRE_THAT( resultTextureCoordinates[2], IsNearEqual( TextureCoordinate{ 1_r, 0_r, 0_r } ) );
}

}
