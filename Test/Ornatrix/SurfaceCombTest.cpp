#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/SurfaceComber.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"
#include "Ephere/Ornatrix/Private/TextureMapImage.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

TEST_CASE( "SurfaceComber" )
{
	SharedHair sharedHair;
	HairUtilities const hairUtilities( sharedHair );

	IHair& hair = sharedHair;
	SharedHair originalHair;

	auto const distributionMesh = GeneratePlane<Real>( 1 );
	GenerateRandomHairs( hair, 1, 1, &distributionMesh );
	originalHair.CopyFrom( sharedHair, true, true, true, true, true );

	SurfaceComber op;
	op.parameters.targetHair = &hair;
	op.parameters.distributionMesh = &distributionMesh;

	auto const sinkIndex = op.AddSink();
	op.SetSinkCoordinate( sinkIndex, { 0, { 0.5f, 0.5f } } );
	op.SetSinkDirection( sinkIndex, Vector3::XAxis() );

	auto const verticesBefore = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

	SECTION( "OneSink" )
	{
		bool dummy;
		op.Compute( dummy );

		auto const verticesAfter = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

		REQUIRE_THAT( verticesBefore, !IsVectorNearEqual( verticesAfter ) );
	}

	SECTION( "SetExistingStrandChannelData" )
	{
		hair.SetStrandChannelCount( IHair::PerStrand, 1 );

		op.parameters.algorithm = SurfaceCombParameters::AlgorithmType::Triangulation;
		op.parameters.applyTo.direction = false;
		op.parameters.applyTo.shape = false;
		op.parameters.applyTo.strandData = true;

		op.parameters.strandData.channel = { StrandChannelType_PerStrand, 0 };
		op.parameters.strandData.value.maximum = 0.7f;

		bool dummy;
		op.Compute( dummy );

		// Vertices should not have been modified
		auto const verticesAfter = sharedHair.GetVertices( IHair::CoordinateSpace::Object );
		REQUIRE_THAT( verticesBefore, IsVectorNearEqual( verticesAfter ) );

		// But strand channel should have been modified
		auto const channelData = hairUtilities.GetStrandChannelData( IHair::PerStrand, 0 );
		REQUIRE( NearEqual( channelData[0], static_cast<Real>( op.parameters.strandData.value.maximum() ) ) );
	}

	SECTION( "SetExistingVertexChannelData" )
	{
		hair.SetStrandChannelCount( IHair::PerVertex, 1 );

		op.parameters.algorithm = SurfaceCombParameters::AlgorithmType::Triangulation;
		op.parameters.applyTo.direction = false;
		op.parameters.applyTo.shape = false;
		op.parameters.applyTo.strandData = true;

		op.parameters.strandData.channel = { StrandChannelType_PerVertex, 0 };
		op.parameters.strandData.value.maximum = 0.7f;

		bool dummy;
		op.Compute( dummy );

		// Vertices should not have been modified
		auto const verticesAfter = sharedHair.GetVertices( IHair::CoordinateSpace::Object );
		REQUIRE_THAT( verticesBefore, IsVectorNearEqual( verticesAfter ) );

		// But strand channel should have been modified
		auto const channelData = hairUtilities.GetStrandChannelData( IHair::PerVertex, 0 );
		REQUIRE( NearEqual( channelData.front(), static_cast<Real>( op.parameters.strandData.value.maximum() ) ) );
		REQUIRE( NearEqual( channelData.back(), static_cast<Real>( op.parameters.strandData.value.maximum() ) ) );
	}

	SECTION( "SlopeMap" )
	{
		bool dummy;
		op.Compute( dummy );
		auto const verticesBeforeMap = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

		TextureMapParameter mapParameter{ make_shared<TextureMapImage>( Image::ImageRgba16{ Image::PixelRgba16{ 100, 100, 100 } } ) };

		op.parameters.slope.map = mapParameter;

		hair.CopyFrom( originalHair );
		op.Compute( dummy );
		auto const verticesAfterMap = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

		REQUIRE_THAT( verticesBeforeMap, !IsVectorNearEqual( verticesAfterMap ) );
	}
}

// Tests for an issue where a sink placed at the exact same coordinate as a strand will incorrectly set strand's shape when using "Mesh Vertex" algorithm
TEST_CASE( "SurfaceCombMeshVertexModeSinkSamePositionAsStrand" )
{
	SharedHair hair;
	auto const distributionMesh = GeneratePlane<Real>( 1, 3, 3 );

	SurfaceComber op;
	op.parameters.targetHair = &hair;
	op.parameters.distributionMesh = &distributionMesh;
	op.parameters.algorithm = SurfaceCombParameters::AlgorithmType::MeshVertex;
	op.parameters.chaos.value = 0_r;

	SurfacePosition const sinkSurfacePosition = { 0, { 1.0f, 1.0f } };
	auto const sinkDirection = -Vector3::XAxis();

	auto const sinkIndex = op.AddSink();
	op.SetSinkCoordinate( sinkIndex, sinkSurfacePosition );
	op.SetSinkType( sinkIndex, SurfaceCombParameters::SinkType::Direct );
	op.SetSinkDirection( sinkIndex, sinkDirection );

	GenerateHairs( hair, array{ sinkSurfacePosition }, 1, &distributionMesh );

	bool dummy;
	op.Compute( dummy );
	auto const vertices = hair.GetVertices( IHair::CoordinateSpace::Object );

	auto const strandDirection = ( vertices[1] - vertices[0] ).normalized();
	REQUIRE_THAT( 1_r, IsNearEqual( Dot( op.GetSinkDirectionInObjectCoordinates( sinkIndex ), strandDirection ) ) );
}

TEST_CASE( "SurfaceCombMirroredSinkLengthAfterNonMirroredSink" )
{
	SurfaceComber op;

	auto const distributionMesh = GeneratePlane<Real>( 1, 3, 3 );
	op.parameters.distributionMesh = &distributionMesh;
	op.parameters.targetHair = make_shared<SharedHair>();

	SurfacePosition const sink1SurfacePosition = { 0, { 1.0f, 1.0f } },
		sink2SurfacePosition = { 0, { 0.5f, 0.5f } };

	// Add a non-mirrored sink
	auto const nonMirroredSinkIndex = op.AddSink();
	op.SetSinkCoordinate( nonMirroredSinkIndex, sink1SurfacePosition );
	op.SetSinkType( nonMirroredSinkIndex, SurfaceCombParameters::SinkType::Direct );
	op.SetSinkDirection( nonMirroredSinkIndex, Vector3::XAxis() );

	auto const mirroredSinkIndex = op.AddSink();
	op.SetSinkCoordinate( mirroredSinkIndex, sink2SurfacePosition );
	op.SetSinkType( mirroredSinkIndex, SurfaceCombParameters::SinkType::Direct );
	op.SetSinkDirection( mirroredSinkIndex, Vector3::XAxis() );
	op.SetSinkMirrorType( mirroredSinkIndex, SurfaceCombParameters::SinkMirrorType::X );

	// Compute to validate positions and mirrored sinks
	bool dummy;
	op.Compute( dummy );

	op.SetSinkLength( mirroredSinkIndex, 2_r );

	REQUIRE_THAT( 2_r, IsNearEqual( op.GetSinkLength( op.GetMirroredSinkIndex( mirroredSinkIndex ) ) ) );
}

}
