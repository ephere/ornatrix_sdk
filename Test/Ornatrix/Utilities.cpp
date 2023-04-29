#include "PrecompiledHeaders.h"

#include "Utilities.h"

#include "Ephere/Geometry/Native/IPolygonMesh.h"
#include "Ephere/Geometry/Native/MatrixUtilities.h"
#include "Ephere/Geometry/Native/PolygonMeshUtilities.h"
#include "Ephere/NativeTools/Random.h"
#include "Ephere/NativeTools/ScopeExit.h"
#include "Ephere/NativeTools/StlExtensions.h"
#include "Ephere/Ornatrix/Ornatrix.h"

namespace Ephere::Ornatrix
{

using namespace std;
using namespace Geometry;

void GenerateTriangle( IPolygonMeshSA& mesh, Real size, bool generateTextureCoordinates )
{
	mesh.SetTextureChannelCount( 0 );

	Vector3 const vertices[] = { Vector3::Zero(), Vector3::YAxis() * size, Vector3::XAxis() * size };
	mesh.SetVertexCount( 3 );
	mesh.SetVertices( 0, 3, vertices );

	int const vertexIndices[] = { 0, 1, 2 };
	mesh.SetPolygonCount( 1, 3 );
	mesh.SetPolygonVertexIndices( 0, 0, 3, vertexIndices );

	if( generateTextureCoordinates )
	{

	}
}

void GeneratePlane( IPolygonMeshSA& mesh, Real size, int xVertexCount, int yVertexCount, bool generateTextureCoordinates )
{
	vector<Vector3> vertices;
	vertices.reserve( xVertexCount * yVertexCount );

	vector<int> quadVertexIndices;
	quadVertexIndices.reserve( ( xVertexCount - 1 ) * ( yVertexCount - 1 ) );

	vector<TextureCoordinate> textureCoordinates;
	if( generateTextureCoordinates )
	{
		textureCoordinates.reserve( vertices.size() );
	}

	for( auto ySegmentIndex = 0; ySegmentIndex < yVertexCount; ++ySegmentIndex )
	{
		for( auto xSegmentIndex = 0; xSegmentIndex < xVertexCount; ++xSegmentIndex )
		{
			auto const vertexCount = Size( vertices );

			vertices.emplace_back( Vector3( xSegmentIndex * size - size / 2.0f, ySegmentIndex * size - size / 2.0f, 0.0f ) );

			if( generateTextureCoordinates )
			{
				textureCoordinates.emplace_back( TextureCoordinate( GetIndexPositionT<int, Real>( xSegmentIndex, xVertexCount ), 
					GetIndexPositionT<int, Real>( ySegmentIndex, yVertexCount ), 0.0f ) );
			}

			if( xSegmentIndex < xVertexCount - 1 && ySegmentIndex < yVertexCount - 1 )
			{
				quadVertexIndices.push_back( vertexCount );
				quadVertexIndices.push_back( vertexCount + 1 );
				quadVertexIndices.push_back( vertexCount + yVertexCount + 1 );
				quadVertexIndices.push_back( vertexCount + yVertexCount );
			}
		}
	}

	mesh.SetVertexCount( static_cast<int>( vertices.size() ) );
	mesh.SetVertices( 0, static_cast<int>( vertices.size() ), vertices.data() );

	mesh.SetPolygonCount( static_cast<int>( quadVertexIndices.size() ) / 4, 4 );
	mesh.SetPolygonVertexIndices( 0, 0, static_cast<int>( quadVertexIndices.size() ), quadVertexIndices.data() );

	if( generateTextureCoordinates )
	{
		mesh.SetTextureChannelCount( 1 );
		mesh.SetTextureCoordinateCount( 0, static_cast<int>( textureCoordinates.size() ) );
		mesh.SetTextureCoordinates( 0, 0, static_cast<int>( textureCoordinates.size() ), textureCoordinates.data() );
		mesh.SetPolygonTextureVertexIndices( 0, 0, 0, static_cast<int>( quadVertexIndices.size() ), quadVertexIndices.data() );
	}
}

static std::vector<SurfacePosition> GenerateRandomSurfaceDependencies( int count, IPolygonMeshSA const* distributionMesh, UniformRandomGenerator& randomGenerator )
{
	if( distributionMesh == nullptr )
	{
		return std::vector<SurfacePosition>();
	}

	auto const intGenerator = randomGenerator.CreateUniformIntDistribution( distributionMesh->GetPolygonCount() );

	std::vector<SurfacePosition> result;
	result.reserve( count );
	for( auto strandIndex = 0; strandIndex < count; ++strandIndex )
	{
		auto const faceIndex = intGenerator.GetNextValue();
		switch( distributionMesh->GetPolygonVertexCount( faceIndex ) )
		{
		case 3:
			{
				auto const barycentric = GetRandomBarycentricCoordinate<UniformRandomGenerator, Real>( randomGenerator );
				result.emplace_back( static_cast<unsigned>( faceIndex ), Vector2f( GetUvFromBarycentric( barycentric ) ) );
			}

			break;
		case 4:
			result.emplace_back( static_cast<unsigned>( faceIndex ), Vector2f( GetRandomBilinearCoordinate<UniformRandomGenerator, Real>( randomGenerator ) ) );
			break;
		default:
			// TODO:
			ASSERT_FAIL();
		}
	}

	return result;
}

std::vector<SurfacePosition> GenerateVertexSurfaceDependencies( IPolygonMeshSA const* distributionMesh )
{
	if( distributionMesh == nullptr )
	{
		return std::vector<SurfacePosition>();
	}

	auto const vertexCount = distributionMesh->GetVertexCount();

	std::vector<SurfacePosition> result;
	result.reserve( vertexCount );
	vector<bool> wasVertexAdded( vertexCount, false );
	vector<int> polygonVertexIndices;
	for( auto polygonIndex = 0, polygonCount = distributionMesh->GetPolygonCount(); polygonIndex < polygonCount; ++polygonIndex )
	{
		auto const polygonVertexCount = distributionMesh->GetPolygonVertexCount( polygonIndex );
		polygonVertexIndices.resize( polygonVertexCount );
		distributionMesh->GetPolygonVertexIndices( polygonIndex, 0, Size( polygonVertexIndices ), polygonVertexIndices.data() );

		auto triangleVertexIndex = 0;
		for( auto const vertexIndex: polygonVertexIndices )
		{
			if( !wasVertexAdded[vertexIndex] )
			{
				switch( polygonVertexCount )
				{
				case 3:
				{
					auto const barycentric = Vector3::Axis( triangleVertexIndex, 1.0f );
					result.emplace_back( static_cast<unsigned>( polygonIndex ), Vector2f( GetUvFromBarycentric( barycentric ) ) );
				}

					break;
				case 4:
					result.emplace_back( static_cast<unsigned>( polygonIndex ), Vector2f( GetPolygonVertexBilinearCoordinate<Real>( triangleVertexIndex ) ) );
					break;
				default:
					// TODO:
					ASSERT_FAIL();
				}

				wasVertexAdded[vertexIndex] = true;
			}

			++triangleVertexIndex;
		}
	}

	return result;
}

void GenerateHairs(
	IHair& hair, Span<SurfacePosition const> surfaceDependencies, Real length, IPolygonMeshSA const* distributionMesh, Real tipRandomness,
	int pointsPerStrandCountMin, int pointsPerStrandCountMax, bool generateTextureCoordinates, bool generateWidths, bool const append, Span<int const> pointPerStrandCounts )
{
	auto const hairUtilities = TheOrnatrixLibrary.library->CreateHairUtilities( hair );

	if( !append )
	{
		hair.Clear();
		hair.SetUsesStrandTopology( false );
	}

	hair.SetUseSurfaceDependency2( distributionMesh != nullptr );
	hair.SetCoordinateSpace( IHair::CoordinateSpace::Strand );

	auto const hasVaryingPointCount = 
		pointsPerStrandCountMax != -1 && pointsPerStrandCountMin != pointsPerStrandCountMax || !pointPerStrandCounts.empty() ||
		append && ( hair.HasStrandTopology() || pointsPerStrandCountMax != -1 || hair.GetGlobalStrandPointCount() != pointsPerStrandCountMin );

	if( hasVaryingPointCount )
	{
		if( !hairUtilities->EnableStrandTopology() )
		{
			return;
		}
	}
	else
	{
		hair.SetGlobalStrandPointCount( pointsPerStrandCountMin );
	}

	hair.SetUseStrandToObjectTransforms( true );
	hair.SetUseStrandIds( true );

	auto const count = surfaceDependencies.size();
	auto const startStrandIndex = append ? hair.GetStrandCount() : 0;
	hair.SetStrandCount( startStrandIndex + count );

	auto const strandIds = Iota<StrandId>( count, count );
	hair.SetStrandIds( startStrandIndex, Size( strandIds ), strandIds.data() );

	std::vector<Vector3> vertices;
	vertices.reserve( count * pointsPerStrandCountMin );

	UniformRandomGenerator randomGenerator( 1 );
	auto const intDistribution = randomGenerator.CreateUniformIntDistribution( hasVaryingPointCount ? pointsPerStrandCountMax - pointsPerStrandCountMin : 1 );

	vector<StrandTopology> strandTopology;
	strandTopology.reserve( count );

	for( auto index = 0; index < count; ++index )
	{
		auto const random1 = randomGenerator.GetNextSignedFloatValue();
		auto const random2 = randomGenerator.GetNextSignedFloatValue();
		Vector3 const tipPosition( random1 * tipRandomness, random2 * tipRandomness, length );

		auto const pointsPerStrandCount = hasVaryingPointCount ?
			                                  ( !pointPerStrandCounts.empty() ? pointPerStrandCounts[index] : pointsPerStrandCountMin + intDistribution.GetNextValue() ) :
			                                  pointsPerStrandCountMin;

		if( hasVaryingPointCount )
		{
			strandTopology.push_back( { static_cast<unsigned>( vertices.size() ), static_cast<unsigned>( pointsPerStrandCount ) } );
		}

		for( auto pointIndex = 0; pointIndex < pointsPerStrandCount; ++pointIndex )
		{
			auto const positionOnStrand = GetIndexPositionT<int, Real>( pointIndex, pointsPerStrandCount );
			vertices.push_back( Vector3::Zero() * ( 1.0f - positionOnStrand ) + tipPosition * positionOnStrand );
		}
	}

	if( generateWidths )
	{
		hair.SetUseWidths( true );
	}

	auto const startVertexIndex = append ? hair.GetVertexCount() : 0;
	if( !vertices.empty() )
	{
		hair.SetVertexCount( startVertexIndex + Size( vertices ) );
		hair.SetVertices( startVertexIndex, Size( vertices ), vertices.data(), IHair::CoordinateSpace::Strand );
	}

	if( generateTextureCoordinates )
	{
		hair.SetTextureCoordinateChannelCount( 1 );
	}

	if( !strandTopology.empty() )
	{
		hair.SetStrandTopologies( startStrandIndex, Size( strandTopology ), strandTopology.data() );
	}

	hair.SetSurfaceDependencies2( startStrandIndex, surfaceDependencies.size(), surfaceDependencies.data() );
	CheckedDynamicCast<IHair_Extension1&>( hair ).UpdateStrandTransformationsFromDistributionMesh( distributionMesh, startStrandIndex, -1, true, true );

	if( generateTextureCoordinates && distributionMesh != nullptr )
	{
		ASSERT( distributionMesh->GetTextureChannelCount() > 0 );

		vector<TextureCoordinate> textureCoordinates( count );
		PolygonMeshUtilities( *distributionMesh ).GetTextureCoordinates( surfaceDependencies, 0, textureCoordinates.data() );

		hair.SetTextureCoordinateChannelCount( 1 );
		hair.SetTextureCoordinates( 0, startStrandIndex, Size( textureCoordinates ), textureCoordinates.data(), IHair::StrandDataType::PerStrand );
	}
}

//SharedHair GenerateHairs( Span<SurfacePosition const> surfaceDependencies, Real length, IPolygonMeshSA const* distributionMesh, Real tipRandomness,
//	int pointsPerStrandCountMin, int pointsPerStrandCountMax, bool generateTextureCoordinates, bool generateWidths, bool const append, Span<int const> pointPerStrandCounts )
//{
//	SharedHair result;
//	GenerateHairs( result, surfaceDependencies, length, distributionMesh, tipRandomness, pointsPerStrandCountMin, pointsPerStrandCountMax,
//				   generateTextureCoordinates, generateWidths, append, pointPerStrandCounts );
//	return result;
//}

void GenerateVertexHairs(
	IHair& hair, Real length, IPolygonMeshSA const* distributionMesh, Real tipRandomness,
	int pointsPerStrandCountMin, int pointsPerStrandCountMax, bool generateTextureCoordinates, bool generateWidths )
{
	GenerateHairs(
		hair, GenerateVertexSurfaceDependencies( distributionMesh ), length, distributionMesh, tipRandomness,
		pointsPerStrandCountMin, pointsPerStrandCountMax, generateTextureCoordinates, generateWidths );
}

void GenerateRandomHairs( IHair& hair, int count, Real length, IPolygonMeshSA const* distributionMesh, Real tipRandomness, int pointsPerStrandCountMin, int pointsPerStrandCountMax, bool generateTextureCoordinates, bool generateWidths,
                          int const randomSeed, bool const append )
{
	UniformRandomGenerator randomGenerator( randomSeed );
	GenerateHairs( hair, GenerateRandomSurfaceDependencies( count, distributionMesh, randomGenerator ), length, distributionMesh, tipRandomness, pointsPerStrandCountMin, pointsPerStrandCountMax, generateTextureCoordinates, generateWidths, append );
}

void GenerateZigzagHairs( IHair& hair, int strandCount, int strandPointCount, int period, Real length, Real xDisplacement, Real xRandomness, IPolygonMeshSA const* distributionMesh )
{
	hair.SetUsesStrandTopology( false );
	hair.SetUseSurfaceDependency2( distributionMesh != nullptr );
	hair.SetGlobalStrandPointCount( strandPointCount );
	hair.SetUseStrandToObjectTransforms( true );

	hair.SetStrandCount( strandCount );
	hair.SetVertexCount( strandCount * strandPointCount );

	std::vector<Vector3> vertices;
	vertices.reserve( strandPointCount );

	UniformRandomGenerator randomGenerator( 1 );

	vertices.emplace_back( 0.0f, 0.0f, 0.0f );
	for( auto index = 1; index < strandPointCount; ++index )
	{
		auto const positionOnStrand = GetIndexPosition( index, strandPointCount );
		auto transversePosition = static_cast<Real>( index % period ) / static_cast<Real>( period - 1 ) * 2;
		transversePosition = transversePosition <= 1 ? transversePosition : 2 - transversePosition;
		auto const random1 = randomGenerator.GetNextSignedFloatValue();
		vertices.emplace_back( random1 * xRandomness + transversePosition * xDisplacement, 0_r, length * positionOnStrand );
	}

	for( auto strandIndex = 0; strandIndex < strandCount; ++strandIndex )
	{
		hair.SetVertices( strandIndex * strandPointCount, Size( vertices ), vertices.data(), IHair::CoordinateSpace::Strand );
	}

	auto const surfaceDependencies = GenerateRandomSurfaceDependencies( strandCount, distributionMesh, randomGenerator );
	hair.SetSurfaceDependencies2( 0, Size( surfaceDependencies ), surfaceDependencies.data() );
	CheckedDynamicCast<IHair_Extension1&>( hair ).UpdateStrandTransformationsFromDistributionMesh( distributionMesh, 0, -1, true, true );
}

void MoveTipsRandomly( IHair& hair, Real amount, int const start, int const stride, int seed )
{
	// Move tips of each strand randomly
	UniformRandomGenerator randomGenerator( seed );
	for( auto strandIndex = start, endIndex = hair.GetStrandCount(); strandIndex < endIndex; strandIndex += stride )
	{
		auto points = hair.GetStrandPoints( strandIndex, IHair::CoordinateSpace::Object );
		points.back() += randomGenerator.GetNextSignedFloatValue() * amount;
		hair.SetStrandPoints( strandIndex, points, IHair::CoordinateSpace::Object );
	}

	hair.InvalidateGeometryCache();
}

void MoveTips( IHair& hair, Vector3 const& direction, int const start, int const end, int const stride )
{
	for( auto strandIndex = start, endIndex = end != -1 ? end : hair.GetStrandCount(); strandIndex < endIndex; strandIndex += stride )
	{
		auto points = hair.GetStrandPoints( strandIndex, IHair::CoordinateSpace::Object );
		points.back() += direction;
		hair.SetStrandPoints( strandIndex, points, IHair::CoordinateSpace::Object );
	}

	hair.InvalidateGeometryCache();
}

vector<vector<Vector3>> GetStrandPoints( IHair const& hair, IHair::CoordinateSpace space )
{
	vector<vector<Vector3>> result( hair.GetStrandCount() );
	auto resultIterator = result.begin();
	for( auto strandIndex = 0; strandIndex < hair.GetStrandCount(); ++strandIndex, ++resultIterator )
	{
		resultIterator->resize( hair.GetStrandPointCount( strandIndex ) );
		hair.GetStrandPoints( strandIndex, 0, Size( *resultIterator ), resultIterator->data(), space );
	}

	return result;
}

std::vector<Vector3> GetVerticesSortedByStrandIds( IHair const& hair, IHair::CoordinateSpace space )
{
	auto const points = GetStrandPoints( hair, space );
	auto const strandIds = hair.GetStrandIds();

	vector<pair<StrandId, vector<Vector3>>> pointsAndIds;
	pointsAndIds.reserve( points.size() );
	auto pointIterator = begin( points );
	auto strandIdIterator = begin( strandIds );
	for( ; pointIterator != end( points ); ++pointIterator, ++strandIdIterator )
	{
		pointsAndIds.emplace_back( *strandIdIterator, *pointIterator );
	}

	Sort( pointsAndIds, [] ( auto const& value1, auto const& value2 )
	{
		return value1.first < value2.first;
	} );

	vector<Vector3> result;
	for( auto const& pointsAndId: pointsAndIds )
	{
		Append( result, pointsAndId.second );
	}

	return result;
}

vector<vector<Real>> GetStrandWidths( IHair const& hair )
{
	vector<vector<Real>> result( hair.GetStrandCount() );
	auto resultIterator = result.begin();
	for( auto strandIndex = 0; strandIndex < hair.GetStrandCount(); ++strandIndex, ++resultIterator )
	{
		*resultIterator = hair.GetStrandWidths( strandIndex );
	}

	return result;
}

void AddStrandChannel( IHair& hair, IHair::StrandDataType dataType, std::vector<Real> const& data, std::wstring const& name )
{
	auto const channelIndex = hair.GetStrandChannelCount( dataType );
	hair.SetStrandChannelCount( dataType, channelIndex + 1 );

	auto const count = Size( data );
	ASSERT( count == ( dataType == IHair::PerStrand ? hair.GetStrandCount() : hair.GetVertexCount() ) );
	ASSERT( hair.SetStrandChannelData( dataType, channelIndex, 0, count, data.data() ) );
	if( !name.empty() )
	{
		StrandChannelName channelName{ name };
		hair.SetStrandChannelNames( dataType, channelIndex, 1, &channelName );
	}
}

std::vector<Real> AddStrandChannelWithRandomData( IHair& hair, IHair::StrandDataType dataType, Real const minValue, Real const maxValue )
{
	auto const count = dataType == IHair::PerStrand ? hair.GetStrandCount() : hair.GetVertexCount();
	// ReSharper disable once CppRedundantCastExpression
	auto const randomValues = GetConvertedCast<vector<float>, Real>( UniformRandomGenerator( 1 ).GetUniformVector( count, static_cast<float>( minValue ),
		// ReSharper disable once CppRedundantCastExpression
		static_cast<float>( maxValue ) ) );
	AddStrandChannel( hair, dataType, randomValues );
	return randomValues;
}

void TestMicroInputModificationsEffectOnResult( IHair& sharedHair, IHair& originalHair, IPolygonMeshSA const& distributionMesh, function<void( int )> const& iterationFunction,
	Real const randomizeAmount, int const iterationCount, Real const acceptableError )
{
	GenerateRandomHairs( sharedHair, 1, 1, &distributionMesh, 0, 10 );
	originalHair.CopyFrom( sharedHair );

	// Apply frizzer a few times
	for( auto iteration = 0; iteration < iterationCount; ++iteration )
	{
		iterationFunction( iteration );
	}

// TODO: Remove when HairUtilities linking issue is resolved
#ifdef DOUBLE_PRECISION
	HairUtilities const hairUtilities( sharedHair );
	auto const vertices1 = hairUtilities.GetVertices( IHair::CoordinateSpace::Strand );

	// Reset hair and make a very slight change in shape
	sharedHair.CopyFrom( originalHair );
	static Vector3 const RandomizeMinimum( -randomizeAmount / 2 ), RandomizeMaximum( randomizeAmount / 2 );
	REQUIRE( hairUtilities.SetVertices( GetRandomized<3, 1, Real>( hairUtilities.GetVertices(), 1, &RandomizeMinimum, &RandomizeMaximum ) ) );
	//auto const smallRotation = Xform3::XYZRotation( randomizeEpsilon, -randomizeEpsilon, randomizeEpsilon );
	//REQUIRE( hairUtilities.SetVertices( Transform( hairUtilities.GetVertices(), smallRotation.VectorMultiplyFunction() ) ) );

	// Apply frizzer a few times again
	for( auto iteration = 0; iteration < iterationCount; ++iteration )
	{
		iterationFunction( iteration );
	}

	auto const vertices2 = hairUtilities.GetVertices( IHair::CoordinateSpace::Strand );// Transform( hairUtilities.GetVertices( IHair::CoordinateSpace::Strand ), Inverse( smallRotation ).VectorMultiplyFunction() );

	auto const error = ComputeAverageDistance<3, 1, Real>( vertices1, vertices2 );
	REQUIRE( error < acceptableError );
#else
	UNUSED_VALUE( randomizeAmount );
	UNUSED_VALUE( acceptableError );
#endif
}

vector<Vector3> GetUniquePoints( vector<Vector3> const& points, float const epsilon, vector<unordered_set<int>>* uniqueSets )
{
	// TODO: This can be significantly sped up through position hashing
	vector<Vector3> result;
	auto index = 0;
	for( auto const& point : points )
	{
		auto scope1 = IncrementOnScopeExit( index );

		auto existingPointResultIndex = -1;
		auto resultIndex = 0;
		for( auto const& uniquePoint : result )
		{
			auto scope2 = IncrementOnScopeExit( resultIndex );

			if( ( uniquePoint - point ).lengthSquared() <= epsilon )
			{
				existingPointResultIndex = resultIndex;
				break;
			}
		}

		if( existingPointResultIndex != -1 )
		{
			if( uniqueSets != nullptr )
			{
				uniqueSets->at( existingPointResultIndex ).insert( index );
			}

			continue;
		}

		result.push_back( point );

		if( uniqueSets != nullptr )
		{
			uniqueSets->push_back( { index } );
		}
	}

	return result;
}

}
