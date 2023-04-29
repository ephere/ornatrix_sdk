#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/NativeTools/ChunkStreams.h"
#include "Ephere/NativeTools/Random.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"
#include "Geometry/Native/NearestNeighbourFinder.h"
#include "Geometry/Native/PolygonMeshUtilities.h"

using namespace std;
using namespace Ephere;
using namespace Geometry;
using namespace Ornatrix;

template<typename T>
bool HasNearEqualVector( vector<vector<T>> const& values, vector<T> const& findValue, Real epsilon )
{
	return AnyOf( values, [matcher = IsVectorNearEqual( findValue, epsilon )]( auto const& value ) { return matcher.match( value ); } );
}

template<unsigned N, typename TPredicate>
bool AnyNeighbor( Span<Matrix<N, 1, Real> const> positions, TPredicate&& predicate )
{
	NearestNeighbourFinderCommon const nearestNeighbourFinder( positions );
	size_t positionIndex = 0;
	array<size_t, 2> closestTwoIndices {};
	return any_of( begin( positions ), end( positions ), [&nearestNeighbourFinder, &predicate, &positionIndex, &closestTwoIndices]( auto const& position )
	{
		nearestNeighbourFinder.FindClosest( position, closestTwoIndices );

		// Use second closest index since closest one will always be the position itself
		return predicate( positionIndex++, closestTwoIndices[1] );
	} );
}

void TestHairDirectionsWithDeformedMesh( SharedPolygonMesh& distributionMesh, SharedHair& sharedHair, bool expectSmoothTransition )
{
	IHair& hair = sharedHair;
	
	// We need at least two faces
	distributionMesh = GeneratePlane<Real>( 1, 3 );
	GenerateRandomHairs( hair, 50, 1, &distributionMesh );

	// Move all tips forward and down
	hair.ValidateStrandToObjectTransforms( &distributionMesh );
	MoveTips( hair, Vector3( 1, 0, -1 ) );

	// Move two of the mesh vertices to deform it
	auto meshVertices = PolygonMeshUtilities( distributionMesh ).GetVertices();
	// ReSharper disable once CppUseAutoForNumeric
	auto constexpr vertexChangeMultiplier( 0.2_r );
	meshVertices[0] += Vector3::XAxis() * vertexChangeMultiplier;
	meshVertices[1] += Vector3::XAxis() * vertexChangeMultiplier;
	PolygonMeshUtilities( distributionMesh ).SetVertices( meshVertices );

	sharedHair.InvalidateStrandToObjectTransformsCache();
	hair.ValidateStrandToObjectTransforms( &distributionMesh );
	auto const rootPositions = HairUtilities( hair ).GetRootPositions( 0, -1 );
	auto rootToTipDirections = HairUtilities( hair ).GetRootToTipDirections();
	TransformSelf( rootToTipDirections, Vector3::Normalize );

	// Require that all strands flow smoothly without any sudden changes in direction if expectSmoothTransition is true
	REQUIRE( AnyNeighbor( Span<const Vector3>( rootPositions ), [&rootToTipDirections]( auto index1, auto index2 )
	{
		return ( rootToTipDirections[index1] - rootToTipDirections[index2] ).length() > 0.1_r;
	} ) == !expectSmoothTransition );
}

TEST_CASE( "HairBase" )
{
	SharedHair sharedHair;
	IHair& hair = sharedHair;
	
	auto distributionMesh = GeneratePlane<Real>( 1 );

	SECTION( "SubSetWithVaryingPointCount" )
	{
		// Generate hairs with varying point counts per strand
		GenerateRandomHairs( hair, 10, 1, &distributionMesh, 1.0f, 2, 5 );

		auto const fullHairStrandPoints = GetStrandPoints( hair, IHair::CoordinateSpace::Object );
		auto const fullHairStrandsWidths = GetStrandWidths( hair );

		SharedHair sharedSubsetHair;
		IHair& subSetHair = sharedSubsetHair;
		HairUtilities( sharedHair ).GetSubset( subSetHair, 0.5f, &distributionMesh );

		auto const subSetHairStrandsPoints = GetStrandPoints( subSetHair, IHair::CoordinateSpace::Object );
		auto const subSetHairStrandsWidths = GetStrandWidths( hair );

		// Make sure that strand vertices in sub-set hair match one strand in full hair each
		for( auto const& subSetHairStrandPoints : subSetHairStrandsPoints )
		{
			REQUIRE( HasNearEqualVector( fullHairStrandPoints, subSetHairStrandPoints, Epsilonr ) );
		}

		for( auto const& subSetHairStrandWidths : subSetHairStrandsWidths )
		{
			REQUIRE( HasNearEqualVector( fullHairStrandsWidths, subSetHairStrandWidths, Epsilonr ) );
		}
	}

	SECTION( "DisplacementValue" )
	{
		static auto constexpr InitialLength = 1.0_r;
		GenerateRandomHairs( hair, 1, InitialLength, &distributionMesh );

		static auto const TestDisplacementOffsets = { 0.0f, 2.5f, -3.0f };

		for( auto const testDisplacementOffset : TestDisplacementOffsets )
		{
			CheckedStaticCast<HairBaseSA&>( hair ).SetDisplacementOffset( testDisplacementOffset );

			hair.ValidateStrandToObjectTransforms( &distributionMesh );
			auto const tipPosition = hair.GetVertices( IHair::CoordinateSpace::Object ).back();

			REQUIRE( tipPosition.z() == InitialLength + testDisplacementOffset );
		}
	}

	// Tests that surface dependency hash doesn't change between evaluations
	SECTION( "SurfaceDependencyHashNoChange" )
	{
		GenerateRandomHairs( hair, 100, 1, &distributionMesh );

		auto previousEvaluation = CheckedStaticCast<HairBaseSA const&>( hair ).GetSurfaceDependencyHash();
		for( auto i = 0; i < 5; ++i )
		{
			auto const currentHash = CheckedStaticCast<HairBaseSA const&>( hair ).GetSurfaceDependencyHash();
			REQUIRE( currentHash == previousEvaluation );
			previousEvaluation = currentHash;
		}
	}

	SECTION( "ConvertToNgonSurfaceDependency" )
	{
		GenerateRandomHairs( hair, 10, 1, &distributionMesh );

		hair.ValidateStrandToObjectTransforms( &distributionMesh );
		auto const verticesWithTriangleSurfaceDependency = hair.GetVertices( IHair::CoordinateSpace::Object );
		vector<Xform3> strandTransformsWithTriangleSurfaceDependency( hair.GetStrandCount() ), strandTransformsWithPolygonSurfaceDependency( hair.GetStrandCount() );
		hair.GetStrandToObjectTransforms( 0, Size( strandTransformsWithTriangleSurfaceDependency ), strandTransformsWithTriangleSurfaceDependency.data() );

		hair.SetCoordinateSpace( IHair::CoordinateSpace::Strand );
		REQUIRE( !HairUtilities( hair ).ConvertToNgonSurfaceDependency( distributionMesh ) );
		CheckedStaticCast<HairBaseSA&>( hair ).InvalidateStrandToObjectTransformsCache();
		hair.ValidateStrandToObjectTransforms( &distributionMesh );
		hair.GetStrandToObjectTransforms( 0, Size( strandTransformsWithPolygonSurfaceDependency ), strandTransformsWithPolygonSurfaceDependency.data() );

		auto const verticesWithPolygonSurfaceDependency = hair.GetVertices( IHair::CoordinateSpace::Object );

		REQUIRE_THAT( strandTransformsWithTriangleSurfaceDependency, IsVectorNearEqual( strandTransformsWithPolygonSurfaceDependency ) );
		REQUIRE_THAT( verticesWithTriangleSurfaceDependency, IsVectorNearEqual( verticesWithPolygonSurfaceDependency ) );
	}

	SECTION( "DeleteStrandsWithVertexData" )
	{
		GenerateRandomHairs( hair, 4, 1, &distributionMesh );
		auto const randomValues = AddStrandChannelWithRandomData( hair, IHair::StrandDataType::PerVertex );

		array<StrandId, 2> const deletedStrandIds = { hair.GetStrandId( 0 ), hair.GetStrandId( 2 ) };
		REQUIRE( hair.DeleteStrands( deletedStrandIds.data(), Size( deletedStrandIds ) ) );

		array<Real, 2> strandData {};
		REQUIRE( hair.GetStrandChannelData( IHair::StrandDataType::PerVertex, 0, hair.GetStrandFirstVertexIndex( 0 ), Size( strandData ), strandData.data() ) );

		array<Real, 2> const expectedValues1 = { randomValues[2], randomValues[3] };
		REQUIRE_THAT( expectedValues1, IsVectorNearEqual( strandData ) );

		array<Real, 2> const expectedValues2 = { randomValues[6], randomValues[7] };
		REQUIRE( hair.GetStrandChannelData( IHair::StrandDataType::PerVertex, 0, hair.GetStrandFirstVertexIndex( 1 ), Size( strandData ), strandData.data() ) );
		REQUIRE_THAT( expectedValues2, IsVectorNearEqual( strandData ) );
	}

	SECTION( "DeleteStrandsWithStrandTextureCoordinates" )
	{
		GenerateRandomHairs( hair, 4, 1, &distributionMesh );
		//auto const randomValues = AddStrandChannelWithRandomData( hair, IHair::StrandDataType::PerVertex );
		hair.SetTextureCoordinateChannelCount( 1 );

		static array const TextureCoordinates{
			TextureCoordinate { 0.0_r, 0.0_r, 0.0_r },
			TextureCoordinate { 1.0_r, 0.0_r, 0.0_r },
			TextureCoordinate { 0.0_r, 1.0_r, 0.0_r },
			TextureCoordinate { 1.0_r, 1.0_r, 0.0_r }, };
		hair.SetTextureCoordinates( 0, 0, Size( TextureCoordinates ), TextureCoordinates.data(), IHair::StrandDataType::PerStrand );

		array<StrandId, 2> const deletedStrandIds = { hair.GetStrandId( 0 ), hair.GetStrandId( 2 ) };
		REQUIRE( hair.DeleteStrands( deletedStrandIds.data(), Size( deletedStrandIds ) ) );
		
		REQUIRE( hair.GetTextureCoordinateChannelCount() == 1 );

		array<TextureCoordinate, 2> remainingTextureCoordinates;
		hair.GetTextureCoordinates( 0, 0, Size( remainingTextureCoordinates ), remainingTextureCoordinates.data(), IHair::StrandDataType::PerStrand );
		REQUIRE( remainingTextureCoordinates == array<TextureCoordinate, 2> { TextureCoordinates[1], TextureCoordinates[3] } );
	}

	SECTION( "CombineWithRotatedRoots" )
	{
		GenerateRandomHairs( hair, 4, 1, &distributionMesh );

		MoveTipsRandomly( hair, 2.0f );
		vector<Real> rootRotations( hair.GetStrandCount() );
		UniformRandomGenerator randomGenerator( 1 );
		randomGenerator.FillWithUniformFloats( rootRotations, -Pif, Pif );

		hair.SetUseStrandRotations( true );
		hair.SetStrandRotations( 0, Size( rootRotations ), rootRotations.data(), IHair::PerStrand );

		sharedHair.InvalidateStrandToObjectTransformsCache();
		hair.ValidateStrandToObjectTransforms( &distributionMesh );
		auto const originalVertices = hair.GetVertices( IHair::CoordinateSpace::Object );

		SharedHair mergedHair;
		mergedHair.SetUseSurfaceDependency2( true );
		HairUtilities( mergedHair ).CombineWith( hair );
		mergedHair.InvalidateStrandToObjectTransformsCache();
		mergedHair.ValidateStrandToObjectTransforms( &distributionMesh );
		auto const mergedVertices = mergedHair.GetVertices( IHair::CoordinateSpace::Object );
		REQUIRE_THAT( originalVertices, IsVectorNearEqual( mergedVertices ) );

		vector<SurfacePosition> originalSurfacePositions( hair.GetStrandCount() ), mergedSurfacePositions( hair.GetStrandCount() );
		hair.GetSurfaceDependencies2( 0, Size( originalSurfacePositions ), originalSurfacePositions.data() );
		mergedHair.GetSurfaceDependencies2( 0, Size( mergedSurfacePositions ), mergedSurfacePositions.data() );
		REQUIRE( originalSurfacePositions == mergedSurfacePositions );
	}

	SECTION( "BasicChunkSerialization" )
	{
		GenerateRandomHairs( hair, 4, 1, &distributionMesh, 1, 3, -1, true, true, 1 );

		stringstream stringStream;

		// Save
		{
			StlOutputStream stlStream( stringStream );
			auto const stream = ChunkOutputStreamStl::Create( stlStream );
			sharedHair.Save( *stream );
		}

		SharedHair loadedHair;

		// Load
		{
			StlInputStream stlStream( stringStream );
			auto const stream = ChunkInputStreamStl::Create( stlStream );
			loadedHair.Load( *stream );
		}

		REQUIRE_THAT( sharedHair.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( loadedHair.GetVertices( IHair::CoordinateSpace::Object ) ) );
		REQUIRE_THAT( HairUtilities( sharedHair ).GetWidths(), IsVectorNearEqual( HairUtilities( loadedHair ).GetWidths() ) );
	}

	SECTION( "FirstEdgeVertexTangents" )
	{
		hair.SetSurfaceTangentComputeMethod( SurfaceTangentComputeMethod::FirstEdge );
		TestHairDirectionsWithDeformedMesh( distributionMesh, sharedHair, true );
	}

	SECTION( "ObjectSpaceVertexTangents" )
	{
		hair.SetSurfaceTangentComputeMethod( SurfaceTangentComputeMethod::ObjectSpace );
		TestHairDirectionsWithDeformedMesh( distributionMesh, sharedHair, true );
	}

	SECTION( "GeodesicVertexTangents" )
	{
		hair.SetSurfaceTangentComputeMethod( SurfaceTangentComputeMethod::Geodesic );
		TestHairDirectionsWithDeformedMesh( distributionMesh, sharedHair, true );
	}

	SECTION( "TextureCoordinateVertexTangents" )
	{
		hair.SetSurfaceTangentComputeMethod( SurfaceTangentComputeMethod::TextureCoordinate );
		TestHairDirectionsWithDeformedMesh( distributionMesh, sharedHair, true );
	}

	SECTION( "SubsetCopiesGroups" )
	{
		// Generate hairs with varying point counts per strand
		GenerateVertexHairs( hair, 1, &distributionMesh );
		hair.SetUsesStrandGroups( true );
		// ReSharper disable once CppRedundantExplicitTemplateArguments
		hair.SetStrandGroups( vector<int>( hair.GetStrandCount(), 2 ) );

		SharedHair sharedSubsetHair;
		IHair& subsetHair = sharedSubsetHair;
		HairUtilities( hair ).GetSubset( subsetHair, Iota( 2, 0 ), &distributionMesh );

		REQUIRE( subsetHair.HasStrandGroups() );
		REQUIRE( subsetHair.GetStrandGroup( 0 ) == 2 );
	}

	// TODO: This may be needed to improve performance but some operators (e.g. Clump) don't work correctly yet
	//SECTION( "ReorderingStrandsDoesntChangeTopologyHash" )
	//{
	//	// Generate hairs with varying point counts per strand
	//	GenerateVertexHairs( hair, 1, &distributionMesh );
	//	auto const topologyHash1 = hair.GetTopologyHash();

	//	HairUtilities( hair ).ChangeStrandOrder();
	//	auto const topologyHash2 = hair.GetTopologyHash();

	//	REQUIRE( topologyHash1 == topologyHash2 );
	//}
}
