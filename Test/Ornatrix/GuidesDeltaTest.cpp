#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/MatrixUtilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/NativeTools/ChunkStreams.h"
#include "Ephere/NativeTools/IStream.h"
#include "Ephere/NativeTools/Random.h"
#include "Ephere/Ornatrix/Operators/GuidesEditor.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"
#include "Ephere/Ornatrix/Private/FloatArrayValueReader.h"
#include "Ephere/Ornatrix/Private/FloatArrayValueWriter.h"
#include "Ephere/Ornatrix/Private/GuidesDelta.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"
#include "Geometry/Native/PolygonMeshUtilities.h"
#include "NativeTools/Test/TestLogger.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

void ChangeHairAndComposeDelta( IHair const& originalHair, IHair& hair, GuidesDelta& delta, IPolygonMeshSA const& distributionMesh, ChangeTracking changeTracking, int start, int stride, bool random = true )
{
	hair.CopyFrom( originalHair, true, true, true, true, true );
	if( random )
	{
		MoveTipsRandomly( hair, 5.0f, start, stride, 2 );
	}
	else
	{
		MoveTips( hair, -Vector3::XAxis(), start, -1, stride );
	}

	GuidesDelta const delta2( originalHair, hair, ChangedHairComponents::All, changeTracking );
	delta.Compose( delta2, &distributionMesh );
}

void ComposeTest( IHair& hair, IHair& originalHair, IPolygonMeshSA const& distributionMesh, ChangeTracking changeTracking, int stride1 = 2, int stride2 = 1, int start1 = 0, int start2 = 0, bool random = true )
{
	if( random )
	{
		MoveTipsRandomly( hair, 5.0f, start1, stride1 );
	}
	else
	{
		MoveTips( hair, Vector3::XAxis(), start1, -1, stride1 );
	}

	GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, changeTracking );

	// Make sure first changes were correctly captured
	SharedHair hairWithFirstEdit;
	hairWithFirstEdit.CopyFrom( originalHair, true, true, true, true, true );
	delta.Apply( hairWithFirstEdit, &distributionMesh, GuidesDelta::ApplyParameters() );

	REQUIRE_THAT(
		hair.GetVertices( IHair::CoordinateSpace::Object ),
		IsVectorNearEqual( hairWithFirstEdit.GetVertices( IHair::CoordinateSpace::Object ) )
	);

	//ChangeHairAndComposeDelta( originalHair, hair, delta, distributionMesh, isMultiResolution, start1, stride1, random );

	SharedHair hairWithSecondEdit;
	ChangeHairAndComposeDelta( hair, hairWithSecondEdit, delta, distributionMesh, changeTracking, start2, stride2, random );

	// Delta should now compose and have both changes
	delta.Apply( originalHair, &distributionMesh, GuidesDelta::ApplyParameters() );

	auto const verticesAfterEdit = hairWithSecondEdit.GetVertices( IHair::CoordinateSpace::Object );
	auto const verticesAfterEditGuidesApply = originalHair.GetVertices( IHair::CoordinateSpace::Object );

	REQUIRE_THAT(
		verticesAfterEdit,
		IsVectorNearEqual( verticesAfterEditGuidesApply )
	);
}

void ShapeChangeSerializeTest( IHair& hair, IHair const& originalHair, IPolygonMeshSA const& distributionMesh, ChangeTracking changeTracking )
{
	stringstream outputStream;
	vector<Vector3> verticesBeforeSaving;

	{
		MoveTipsRandomly( hair, 5.0f );
		GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, changeTracking );

		SharedHair appliedHair;
		appliedHair.CopyFrom( originalHair, true, true, true, true, true );
		delta.Apply( appliedHair, &distributionMesh, GuidesDelta::ApplyParameters() );

		verticesBeforeSaving = appliedHair.GetVertices( IHair::CoordinateSpace::Object );

		StlOutputStream stream( outputStream );
		auto const sharedStream = ChunkOutputStreamStl::Create( stream );
		delta.Save( *sharedStream );
	}

	{
		GuidesDelta delta;
		StlInputStream stream( outputStream );
		auto const sharedStream = ChunkInputStreamStl::Create( stream );
		delta.Load( *sharedStream );

		SharedHair appliedHair;
		appliedHair.CopyFrom( originalHair, true, true, true, true, true );
		delta.Apply( appliedHair, &distributionMesh, GuidesDelta::ApplyParameters() );

		auto const verticesAfterLoading = appliedHair.GetVertices( IHair::CoordinateSpace::Object );

		REQUIRE_THAT(
			verticesBeforeSaving,
			IsVectorNearEqual( verticesAfterLoading )
		);
	}
}

void TestPlantGuide( IHair& originalHair, IHair& hair, IPolygonMeshSA const& distributionMesh, IPolygonMeshSA const& distributionMesh2, bool addStrandSelectionChannel = true )
{
	// Clear all hair, we'll plant a new guide
	hair.Clear();
	originalHair.Clear();

	GenerateRandomHairs( hair, 1, 1, &distributionMesh );

	if( addStrandSelectionChannel )
	{
		originalHair.SetStrandChannelCount( IHair::StrandDataType::PerStrand, 1 );
		hair.SetStrandChannelCount( IHair::StrandDataType::PerStrand, 1 );
	}

	GuidesDelta const delta(
		originalHair, hair, { true, true, false, false, false },
		ChangeTracking::Strand, &distributionMesh );
	delta.Apply( originalHair, &distributionMesh2, GuidesDelta::ApplyParameters() );

	REQUIRE_THAT( hair.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( originalHair.GetVertices( IHair::CoordinateSpace::Object ) ) );
}

void TestSerialize( IHair& hairBeforeEditing, IHair const& hairAfterEditing, IPolygonMeshSA const& distributionMesh, ChangeTracking changeTracking, bool binary )
{
	GuidesDelta const delta( hairBeforeEditing, hairAfterEditing, ChangedHairComponents::All, changeTracking, &distributionMesh );

	stringstream stringStream;

	GuidesDelta loadedDelta;
	if( binary )
	{
		// Save
		{
			StlOutputStream stlStream( stringStream );
			auto const stream = ChunkOutputStreamStl::Create( stlStream );
			delta.Save( *stream );
		}

		// Load
		{
			StlInputStream stlStream( stringStream );
			auto const stream = ChunkInputStreamStl::Create( stlStream );
			loadedDelta.Load( *stream );
		}
	}
	else
	{
		vector<Value> values;
		vector<int> flagIndices;

		// Save
		{
			ArrayValueWriter writer( values, flagIndices );
			delta.Save( writer );
		}

		// Load
		{
			ArrayValueReader reader( values, flagIndices );
			loadedDelta.Load( reader );
		}
	}

	loadedDelta.Apply( hairBeforeEditing, &distributionMesh, GuidesDelta::ApplyParameters() );
	REQUIRE_THAT( hairAfterEditing.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( hairBeforeEditing.GetVertices( IHair::CoordinateSpace::Object ) ) );
}

template<typename T>
T GetAverageVector( Span<T const> values1, Span<T const> values2, int start = 0, int stride = 1 )
{
	ASSERT( values1.size() == values2.size() );
	T result( 0 );
	for( auto iterator1 = values1.begin() + start, iterator2 = values2.begin() + start; iterator1 < values1.end(); iterator1 += stride, iterator2 += stride )
	{
		result += *iterator2 - *iterator1;
	}

	result /= static_cast<float>( values1.size() ) / static_cast<float>( stride );
	return result;
}

template<typename T>
vector<T> LinearInterpolate( Span<T const> values1, Span<T const> values2, Real amount )
{
	ASSERT( values1.size() == values2.size() );
	vector<T> result;
	result.reserve( values1.size() );
	for( auto iterator1 = values1.begin(), iterator2 = values2.begin(); iterator1 < values1.end(); ++iterator1, ++iterator2 )
	{
		result.push_back( Interpolate( *iterator2, *iterator1, amount ) );
	}

	return result;
}

template<typename T>
vector<T> LinearInterpolateRange( T const& start, T const& end, int count )
{
	vector<T> result;
	result.reserve( count );
	for( auto index = 0; index < count; ++index )
	{
		result.push_back( Interpolate( start, end, GetIndexPosition( index, count ) ) );
	}

	return result;
}

vector<SurfacePosition> GenerateRandomSurfacePositions( int const count, int const startFaceIndex = 0, int const endFaceIndex = 0, int const seed = 1 )
{
	vector<SurfacePosition> result( count );
	UniformRandomGenerator randomGenerator( seed );
	auto faceDistribution = randomGenerator.CreateUniformIntDistribution( endFaceIndex - startFaceIndex );
	auto uvDistribution = randomGenerator.CreateUniformFloatDistribution( 0.0f, 1.0f );
	generate( begin( result ), end( result ), [&faceDistribution, &uvDistribution, startFaceIndex]()
	{
		return SurfacePosition{ static_cast<unsigned>( faceDistribution.GetNextValue() + startFaceIndex ), Vector2f( uvDistribution.GetNextValue(), uvDistribution.GetNextValue() ) };
	} );

	return result;
}

GuidesDelta PlantStrands( int const count )
{
	SharedHair originalHair, hair;
	originalHair.SetUsesStrandTopology( true );
	hair.SetUsesStrandTopology( true );
	originalHair.SetUseSurfaceDependency2( true );
	hair.SetUseSurfaceDependency2( true );

	hair.SetStrandCount( count );
	hair.SetSurfaceDependencies2( GenerateRandomSurfacePositions( count ) );

	array points = { Vector3::Zero(), Vector3::ZAxis() };
	vector<Vector3> vertices;
	vertices.reserve( count * points.size() );
	vector<StrandTopology> topologies;
	topologies.reserve( count );
	for( auto index = 0; index < count; ++index )
	{
		Append( vertices, points );
		topologies.push_back( { static_cast<unsigned>( index * count ), static_cast<unsigned>( count ) } );
	}

	hair.SetVertexCount( Size( vertices ) );
	REQUIRE( HairUtilities( hair ).SetVertices( vertices ) );
	static_cast<IHair&>( hair ).SetStrandTopologies( topologies );

	GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

	return delta;
}

GuidesDelta CreateTwoStrands()
{
	SharedHair originalHair, hair;
	originalHair.SetUsesStrandTopology( true );
	hair.SetUsesStrandTopology( true );
	originalHair.SetUseSurfaceDependency2( true );
	hair.SetUseSurfaceDependency2( true );

	hair.SetStrandCount( 2 );
	hair.SetSurfaceDependencies2( array{ SurfacePosition { 0, { 0.2f, 0.5f } }, SurfacePosition { 0, { 0.3f, 0.5f } } } );
	static_cast<IHair&>( hair ).SetStrandTopologies( array{ StrandTopology { 0u, 0u }, StrandTopology { 0u, 0u } } );

	GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );
	delta.SetCreatedStrandPointCount( 2 );

	return delta;
}

void TestShapeChangeOfPlantedGuide( IHair& hair, IHair& originalHair, IPolygonMeshSA const& distributionMesh, ChangeTracking changeTracking )
{
	// Create a strand manually
	HairUtilities( hair ).AppendStrand( SurfacePosition{ 0, { 0.5f, 0.5f } }, array{ Vector3::Zero(), Vector3::ZAxis() } );
	GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, changeTracking );

	// Move the tips around
	SharedHair hair2;
	hair2.CopyFrom( hair );

	hair.ValidateStrandToObjectTransforms( &distributionMesh );
	MoveTips( hair, Vector3::XAxis() );
	{
		GuidesDelta const incrementalDelta( hair2, hair, ChangedHairComponents::All, changeTracking );
		delta.Compose( incrementalDelta, &distributionMesh );
	}

	// Apply the delta with moved points to original hair
	delta.Apply( originalHair, &distributionMesh, {} );

	// Vertices should match the ones after movement
	originalHair.ValidateStrandToObjectTransforms( &distributionMesh );
	REQUIRE_THAT( GetVerticesSortedByStrandIds( hair, IHair::CoordinateSpace::Object ), IsVectorNearEqual( GetVerticesSortedByStrandIds( originalHair, IHair::CoordinateSpace::Object ) ) );
}

template <class TContainer>
[[nodiscard]] constexpr auto middle( TContainer const& container ) -> decltype( container.begin() )
{
	// get middle of sequence
	return container.begin() + size( container ) / 2;
}

TEST_CASE( "GuidesDelta" )
{
	SharedHair sharedHair;
	HairUtilities const hairUtilities( sharedHair );
	IHair& hair = sharedHair;
	SharedHair originalHair;

	auto distributionMesh = GeneratePlane<Real>( 1 );
	GenerateRandomHairs( hair, 4, 1, &distributionMesh );
	originalHair.CopyFrom( sharedHair, true, true, true, true, true );

	GuidesDelta::ApplyParameters applyParameters;

	SECTION( "PlantedStrandSelectionWeightIs1" )
	{
		// Selection weight of newly created strands should be 1 so they can be brushed
		TestPlantGuide( originalHair, hair, distributionMesh, distributionMesh );

		// The planted guide will be applied to the original hair by guides delta so test that
		REQUIRE_THAT( dynamic_cast<const IHair&>( originalHair ).GetStrandChannelData( 0, 0 ), IsNearEqual( 1_r ) );
	}

	SECTION( "DeletingOneStrandAlsoRemovesItFromSelection" )
	{
		GuidesDelta delta( hair );

		// Track selection
		auto hair2 = sharedHair;
		{
			unordered_set const selection = { hair.GetStrandId( 0 ), hair.GetStrandId( 2 ) };
			hair2.SetSelectedStrandIdSet( selection );
			GuidesDelta const subDelta( hair, hair2, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh );
			delta.Compose( subDelta, &distributionMesh );
		}

		// Track deletion with strand selection
		auto hair3 = hair2;
		{
			vector deletedStrandIndices = { 2 };
			REQUIRE( hair3.DeleteStrandsByIndices( deletedStrandIndices ) );
			GuidesDelta const subDelta( hair2, hair3, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh );
			delta.Compose( subDelta, &distributionMesh );
		}

		delta.Apply( hair, &distributionMesh, applyParameters );
		REQUIRE( hair.GetSelectedStrandCount() == 1 );
		hair.CopyFrom( originalHair );

		// Track deletion without strand selection
		hair3.SetSelectedStrandIdSet( vector<StrandId>() );
		auto hair4 = hair3;
		{
			vector deletedStrandIndices = { 0 };
			REQUIRE( hair4.DeleteStrandsByIndices( deletedStrandIndices ) );
			GuidesDelta const subDelta( hair3, hair4, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh );
			delta.Compose( subDelta, &distributionMesh );
		}

		// After deleting the last remaining selected strand we should not have any selection left
		delta.Apply( hair, &distributionMesh, applyParameters );
		REQUIRE( hair.GetSelectedStrandCount() == 0 );
	}

	SECTION( "SurfaceChangeShapeMoveRoot" )
	{
		distributionMesh.InvalidateVertexNormals();

		// Surface must not be planar, a cube will do. Make sure the vertices are randomized a bit to ensure transforms up vectors are different.
		GenerateCube<Real>( distributionMesh, 1 );
		PolygonMeshUtilities( distributionMesh ).RandomizeVertexPositions();

		GenerateRandomHairs( sharedHair, 1, 1, &distributionMesh );
		originalHair.CopyFrom( sharedHair );

		// Change shape of the first strand
		MoveTipsRandomly( sharedHair, 5.0f );
		sharedHair.SetCoordinateSpace( IHair::CoordinateSpace::Object );
		GuidesDelta delta( originalHair, sharedHair, ChangedHairComponents::All, ChangeTracking::Surface );

		// Move the root
		auto sharedHair2 = sharedHair;
		sharedHair2.SetCoordinateSpace( IHair::CoordinateSpace::Object );
		GuidesEditor::MoveStrandRoot( HairUtilities( originalHair ).GetStrandToObjectTransform( 0 ), sharedHair2, 0, { 3, { 0.5f, 0.5f } }, &distributionMesh );

		GuidesDelta const delta2( sharedHair, sharedHair2, { false, true, false, false, true }, ChangeTracking::Surface );
		delta.Compose( delta2, &distributionMesh );

		// Apply final delta to original hair
		sharedHair = originalHair;
		delta.Apply( sharedHair, &distributionMesh, applyParameters );

		// The hair vertices should be identical with the ones modified directly
		REQUIRE_THAT(
			sharedHair2.GetVertices( IHair::CoordinateSpace::Object ),
			IsVectorNearEqual( sharedHair.GetVertices( IHair::CoordinateSpace::Object ) )
		);

		// Move root one more time
		auto sharedHair3 = sharedHair2;
		GuidesEditor::MoveStrandRoot( HairUtilities( originalHair ).GetStrandToObjectTransform( 0 ), sharedHair3, 0, { 2, {0.3f, 0.7f } }, &distributionMesh );

		GuidesDelta const delta3( sharedHair2, sharedHair3, { false, true, false, false, true }, ChangeTracking::Surface );
		delta.Compose( delta3, &distributionMesh );

		// Apply final delta to original hair
		sharedHair = originalHair;
		delta.Apply( sharedHair, &distributionMesh, applyParameters );

		// The hair vertices should be identical with the ones modified directly
		REQUIRE_THAT(
			sharedHair3.GetVertices( IHair::CoordinateSpace::Object ),
			IsVectorNearEqual( sharedHair.GetVertices( IHair::CoordinateSpace::Object ) )
		);
	}

	SECTION( "Apply" )
	{
		MoveTipsRandomly( hair, 5.0f );

		GuidesDelta const delta( originalHair, hair );

		delta.Apply( originalHair, &distributionMesh, applyParameters );

		auto const verticesAfterEditing = sharedHair.GetVertices( IHair::CoordinateSpace::Object );
		auto const verticesAfterApplying = originalHair.GetVertices( IHair::CoordinateSpace::Object );
		REQUIRE_THAT(
			verticesAfterEditing,
			IsVectorNearEqual( verticesAfterApplying )
		);

		// TODO: Also need to test application of creation, deletion, channel modification, etc.
	}

	// NOTE: We don't use ConvertToMultiResolution anywhere so this failing test is disabled until further notice
#if false
	SECTION( "ConvertToMultiResolution_OutputUnaffected" )
	{
		MoveTipsRandomly( hair, 5.0f, 0, 2 );
		GuidesDelta delta( originalHair, hair );

		SharedHair hairWithoutMultiResolutionGd, hairWithMultiResolutionGd;
		hairWithoutMultiResolutionGd.CopyFrom( originalHair, true, true, true, true, true );
		hairWithMultiResolutionGd.CopyFrom( originalHair, true, true, true, true, true );

		delta.Apply( hairWithoutMultiResolutionGd, &distributionMesh, applyParameters );
		delta.ChangeToMultiResolution( hairWithMultiResolutionGd );
		delta.Apply( hairWithMultiResolutionGd, &distributionMesh, applyParameters );

		const auto verticesWithoutMultiResolutionGd = hairWithoutMultiResolutionGd.GetVertices( IHair::CoordinateSpace::Object );
		const auto verticesWithMultiResolutionGd = hairWithMultiResolutionGd.GetVertices( IHair::CoordinateSpace::Object );

		// There should be no noticeable difference when converting to and from multi-resolution
		REQUIRE_THAT(
			verticesWithoutMultiResolutionGd,
			IsVectorNearEqual( verticesWithMultiResolutionGd )
		);
	}
#endif

	SECTION( "ConvertToSingleResolution_OutputUnaffected" )
	{
		MoveTipsRandomly( hair, 5.0f, 0, 2 );
		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh );

		SharedHair hairWithMultiResolutionGd, hairWithSingleResolutionGd;
		//hairWithMultiResolutionGd.CopyFrom( originalHair, true, true, true, true, true );
		// Make sure to have a different hair count to pass into the multi-resolution delta, to test the MR functionality
		GenerateRandomHairs( hairWithMultiResolutionGd, 6, 1, &distributionMesh );
		hairWithSingleResolutionGd.CopyFrom( hairWithMultiResolutionGd, true, true, true, true, true );

		delta.Apply( hairWithMultiResolutionGd, &distributionMesh, applyParameters );
		delta.ChangeToSingleResolution( hairWithMultiResolutionGd, &distributionMesh );

		delta.Apply( hairWithSingleResolutionGd, &distributionMesh, applyParameters );

		auto const verticesWithMultiResolutionGd = hairWithMultiResolutionGd.GetVertices( IHair::CoordinateSpace::Object );
		auto const verticesWithSingleResolutionGd = hairWithSingleResolutionGd.GetVertices( IHair::CoordinateSpace::Object );

		// There should be no noticeable difference when converting to and from multi-resolution
		REQUIRE_THAT( verticesWithMultiResolutionGd, IsVectorNearEqual( verticesWithSingleResolutionGd ) );
	}

	SECTION( "Compose" )
	{
		ComposeTest( hair, originalHair, distributionMesh, ChangeTracking::Strand );
	}

	SECTION( "ComposeMultiResolutionAllHairs" )
	{
		ComposeTest( hair, originalHair, distributionMesh, ChangeTracking::Surface, 1, 1 );
	}

	SECTION( "ComposeMultiResolutionSomeHairs" )
	{
		ComposeTest( hair, originalHair, distributionMesh, ChangeTracking::Surface, 1, 2, 0, 0, false );
	}

	SECTION( "ComposeMultiResolutionHalfHairs" )
	{
		hair.SetVertexCount( 0 );
		hair.SetStrandCount( 0 );
		GenerateRandomHairs( hair, 3, 1, &distributionMesh );
		originalHair.CopyFrom( sharedHair, true, true, true, true, true );

		ComposeTest( hair, originalHair, distributionMesh, ChangeTracking::Surface, 2, 1, 0, 2, false );
	}

	SECTION( "ComposeSingleToMultiResolutionDeletions" )
	{
		// Create a multi-resolution delta
		GuidesDelta mrDelta( hair );

		// Create a single-resolution delta with some hairs deleted
		GuidesDelta srDelta( hair );
		srDelta.DeleteStrandsSpecific( hair, HairUtilities( hair ).GetStrandIds( 0, 2 ) );

		// Compose the two deltas together
		mrDelta.Compose( srDelta, &distributionMesh );

		// Apply the multi-resolution delta to initial hairs. The two strands should be deleted.
		REQUIRE( originalHair.GetStrandCount() == 4 );
		mrDelta.Apply( originalHair, &distributionMesh, applyParameters );

		REQUIRE( originalHair.GetStrandCount() == 2 );
	}

	SECTION( "MultipleMRDeletions" )
	{
		GuidesDelta mrDelta( hair );

		// Delete first two and then third guides in two calls
		mrDelta.DeleteStrandsSurface( hair, HairUtilities( hair ).GetStrandIds( 0, 2 ), &distributionMesh );
		mrDelta.DeleteStrandsSurface( hair, HairUtilities( hair ).GetStrandIds( 2, 1 ), &distributionMesh );

		// Apply the delta, the three deleted guides should be gone
		REQUIRE( originalHair.GetStrandCount() == 4 );
		mrDelta.Apply( originalHair, &distributionMesh, applyParameters );
		REQUIRE( originalHair.GetStrandCount() == 1 );
	}

	SECTION( "DeletingPreservesVisibility" )
	{
		GuidesDelta delta( hair );

		// Hide first guide
		HairUtilities( hair ).SetHiddenStrandIndices( { 0 } );
		delta.Compose( GuidesDelta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh ), &distributionMesh );

		// Delete last guide
		auto hair2 = sharedHair;
		auto const deletedStrand = hair2.GetStrandCount() - 2;
		hair2.DeleteStrandsByIndices( &deletedStrand, 1 );
		delta.Compose( GuidesDelta( hair, hair2, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh ), &distributionMesh );

		delta.Apply( originalHair, &distributionMesh, applyParameters );

		// The first strand should still be hidden
		REQUIRE( originalHair.GetStrandCount() == 3 );
		REQUIRE( originalHair.GetHiddenStrandCount() == 1 );
	}

	SECTION( "DeletingPreservesVisibilityStrandSpecific" )
	{
		GuidesDelta delta( hair );

		// Hide first guide
		//HairUtilities( hair ).SetHiddenStrandIndices( { 0 } );
		//delta.Compose( GuidesDelta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Strand ) );
		delta.SetHiddenStrands( { 0 } );

		// Delete last guide
		GuidesDelta delta2( hair );
		delta2.DeleteStrands( hair, { false, false, false, true } );

		delta.Compose( delta2 );
		delta.Apply( originalHair, &distributionMesh, applyParameters );

		// The first strand should still be hidden
		REQUIRE( originalHair.GetStrandCount() == 3 );
		REQUIRE( originalHair.GetHiddenStrandCount() == 1 );
	}

	SECTION( "ApplySurfaceDeletionsWithChangedStrandIds" )
	{
		GuidesDelta mrDelta( hair );
		mrDelta.DeleteStrandsSurface( hair, HairUtilities( hair ).GetStrandIds( 0, 2 ), &distributionMesh );

		// Apply once to cache stuff two times
		auto hair2 = originalHair;
		mrDelta.Apply( hair2, &distributionMesh, applyParameters );
		hair2 = originalHair;
		mrDelta.Apply( hair2, &distributionMesh, applyParameters );

		// Change the strand ids
		HairUtilities( originalHair ).SetStrandIds( array{ 101u, 102u, 103u, 104u } );

		// Apply the delta, the three deleted guides should be gone
		mrDelta.Apply( originalHair, &distributionMesh, applyParameters );
		REQUIRE( originalHair.GetStrandCount() == 2 );
	}

	SECTION( "DeletionsAreUpdatedWhenInputHairRootsChange" )
	{
		GenerateRandomHairs( hair, 10, 1, &distributionMesh );
		originalHair.CopyFrom( sharedHair, true, true, true, true, true );

		GuidesDelta delta( hair );
		delta.DeleteStrandsSurface( hair, HairUtilities( hair ).GetStrandIds( 0, 2 ), &distributionMesh );

		// Apply to cache things
		auto hair2 = originalHair;
		delta.Apply( hair2, &distributionMesh, applyParameters );
		hair2 = originalHair;
		delta.Apply( hair2, &distributionMesh, applyParameters );

		auto const remainingHairCount = hair2.GetStrandCount();

		// Change the mesh geometry and re-generate the hairs
		PolygonMeshUtilities( distributionMesh ).RandomizeVertexPositions();
		GenerateRandomHairs( hair, 10, 1, &distributionMesh );

		// Shuffle strand indices
		HairUtilities( hair ).ChangeStrandOrder();

		//hair2 = originalHair;
		delta.Apply( hair, &distributionMesh, applyParameters );
		auto const remainingHairCount2 = hair.GetStrandCount();

		REQUIRE( remainingHairCount2 == remainingHairCount );
	}

	SECTION( "SerializeShapeChangesSingleBinary" )
	{
		ShapeChangeSerializeTest( hair, originalHair, distributionMesh, ChangeTracking::Strand );
	}

	SECTION( "SerializeShapeChangesMultiBinary" )
	{
		ShapeChangeSerializeTest( hair, originalHair, distributionMesh, ChangeTracking::Surface );
	}

	SECTION( "ApplyAmountAffectsRotations" )
	{
		hair.SetUseStrandRotations( true );
		auto constexpr rotation = 1.0f;
		hair.SetStrandRotations( 0, 1, &rotation );

		GuidesDelta delta( originalHair, hair );
		applyParameters.deltaAmount = 0.3f;
		delta.Apply( originalHair, &distributionMesh, applyParameters );

		REQUIRE( NearEqual( originalHair.GetStrandRotation( 0 ), applyParameters.deltaAmount * rotation ) );
	}

	SECTION( "MultiResolutionDelete" )
	{
		auto constexpr deletedStrandIndex = 0;
		REQUIRE( hair.DeleteStrandsByIndices( &deletedStrandIndex, 1 ) );

		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );
		delta.Apply( originalHair, &distributionMesh, applyParameters );

		auto const verticesAfterEdit = sharedHair.GetVertices( IHair::CoordinateSpace::Object );
		auto const verticesAfterApply = originalHair.GetVertices( IHair::CoordinateSpace::Object );
		REQUIRE_THAT( verticesAfterEdit, IsVectorNearEqual( verticesAfterApply ) );
	}

	SECTION( "SerializeMultiResolutionDeletionsBinary" )
	{
		auto constexpr deletedStrandIndex = 0;
		REQUIRE( hair.DeleteStrandsByIndices( &deletedStrandIndex, 1 ) );
		TestSerialize( originalHair, hair, distributionMesh, ChangeTracking::Surface, true );
	}

	SECTION( "SerializeMultiResolutionDeletionsArray" )
	{
		auto constexpr deletedStrandIndex = 0;
		REQUIRE( hair.DeleteStrandsByIndices( &deletedStrandIndex, 1 ) );
		TestSerialize( originalHair, hair, distributionMesh, ChangeTracking::Surface, false );
	}

	SECTION( "MultiResolutionDeleteChangeInputStrands" )
	{
		GenerateVertexHairs( hair, 1.0f, &distributionMesh );
		originalHair.CopyFrom( sharedHair, true, true, true, true, true );

		REQUIRE( hair.DeleteStrandsByIndices( array<int, 2> { 0, 1 } ) );

		GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh );

		auto constexpr changedInputHairCount = 100;
		SharedHair sharedChangedInputHair;
		GenerateRandomHairs( sharedChangedInputHair, changedInputHairCount, 1, &distributionMesh );

		auto const boundingBoxOriginalHair = sharedChangedInputHair.GetBoundingBox( &distributionMesh );
		auto const boundingBoxOriginalHairVolume = boundingBoxOriginalHair.volume();

		delta.Apply( sharedChangedInputHair, &distributionMesh, applyParameters );

		// About half of the input hairs should be removed
		auto const boundingBoxChangedHair = sharedChangedInputHair.GetBoundingBox( &distributionMesh );
		auto const boundingBoxChangedHairVolume = boundingBoxChangedHair.volume();

		REQUIRE( fabs( 1.0f - changedInputHairCount / 2.0f / static_cast<float>( sharedChangedInputHair.GetStrandCount() ) ) < 0.08f );
		REQUIRE( NearEqual( boundingBoxOriginalHairVolume / 2, boundingBoxChangedHairVolume, 0.3_r ) );
	}

	SECTION( "ComposeMRDeleteWithEmpty" )
	{
		GenerateVertexHairs( hair, 1.0f, &distributionMesh );
		originalHair.CopyFrom( sharedHair, true, true, true, true, true );

		array<int, 2> const deletedStrandIndices = { 0, 1 };
		REQUIRE( hair.DeleteStrandsByIndices( deletedStrandIndices.data(), int( deletedStrandIndices.size() ) ) );

		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );
		GuidesDelta delta2;
		delta2.ChangeToMultiResolution( hair );
		delta2.Compose( delta, &distributionMesh );
		delta2.Apply( originalHair, &distributionMesh, applyParameters );

		REQUIRE_THAT( sharedHair.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( originalHair.GetVertices( IHair::CoordinateSpace::Object ) ) );
	}

	SECTION( "ComposeMRDeleteWithNonEmpty" )
	{
		GenerateVertexHairs( hair, 1.0f, &distributionMesh );
		originalHair.CopyFrom( sharedHair, true, true, true, true, true );

		unique_ptr<GuidesDelta> deltaDelete1, deltaDelete2;
		{
			array<int, 2> const deletedStrandIndices = { 0, 1 };
			REQUIRE( hair.DeleteStrandsByIndices( deletedStrandIndices.data(), int( deletedStrandIndices.size() ) ) );
			deltaDelete1 = make_unique<GuidesDelta>( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );
		}

		SharedHair hair2;
		{
			hair2.CopyFrom( hair, true, true, true, true, true );
			array<int, 2> const deletedStrandIndices = { 0, 1 };
			REQUIRE( hair2.DeleteStrandsByIndices( deletedStrandIndices.data(), int( deletedStrandIndices.size() ) ) );
			deltaDelete2 = make_unique<GuidesDelta>( hair, hair2, ChangedHairComponents::All, ChangeTracking::Surface );
		}

		deltaDelete1->Compose( *deltaDelete2, &distributionMesh );
		deltaDelete1->Apply( originalHair, &distributionMesh, applyParameters );

		REQUIRE_THAT( hair2.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( originalHair.GetVertices( IHair::CoordinateSpace::Object ) ) );
	}

	SECTION( "PlantGuideWithSpecifiedShape" )
	{
		TestPlantGuide( originalHair, hair, distributionMesh, distributionMesh );
	}

	SECTION( "SerializeMultiResolutionGuideChangesBinary" )
	{
		MoveTipsRandomly( hair, 5.0f, 0, 2 );
		TestSerialize( originalHair, hair, distributionMesh, ChangeTracking::Surface, true );
	}

	SECTION( "SerializeMultiResolutionGuideChangesArray" )
	{
		MoveTipsRandomly( hair, 5.0f, 0, 2 );
		TestSerialize( originalHair, hair, distributionMesh, ChangeTracking::Surface, true );
	}

	SECTION( "ChangeMoveChangeSingleResolution" )
	{
		GenerateRandomHairs( hair, 1, 1, &distributionMesh );
		originalHair.CopyFrom( hair, true, true, true, true, true );

		MoveTipsRandomly( hair, 5.0f );
		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Strand );

		//// This first evaluation is important
		//SharedHair beforeHair;
		//beforeHair.CopyFrom( originalHair, true, true, true, true, true );
		//delta.Apply( beforeHair, &distributionMesh );
		//beforeHair.InvalidateStrandToObjectTransformsCache();
		//beforeHair.ValidateStrandToObjectTransforms( &distributionMesh );
		//REQUIRE_THAT( hair.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( beforeHair.GetVertices( IHair::CoordinateSpace::Object ) ) );
		SharedHair beforeHair;
		beforeHair.CopyFrom( hair, true, true, true, true, true );

		// Move roots
		vector<float> randomFloats( hair.GetStrandCount() * 2 );
		UniformRandomGenerator generator( 1 );
		generator.FillWithUniformFloats( randomFloats, 0.0f, 1.0f );
		vector<MeshSurfacePosition> surfacePositions( hair.GetStrandCount() );
		hair.GetSurfaceDependencies( 0, Size( surfacePositions ), surfacePositions.data() );
		auto randomFloatIterator = begin( randomFloats );
		for( auto& surfacePosition : surfacePositions )
		{
			surfacePosition.faceIndex = !surfacePosition.faceIndex;
			surfacePosition.barycentricCoordinate.x() = *randomFloatIterator++;
			surfacePosition.barycentricCoordinate.y() = *randomFloatIterator++;
			surfacePosition.barycentricCoordinate.z() = 1.0f - surfacePosition.barycentricCoordinate.x() - surfacePosition.barycentricCoordinate.y();
		}

		hair.SetSurfaceDependencies( 0, Size( surfacePositions ), surfacePositions.data() );

		auto constexpr rotationChange = HalfPif;
		hair.SetUseStrandRotations( true );
		hair.SetStrandRotations( 0, 1, &rotationChange );

		sharedHair.InvalidateStrandToObjectTransformsCache();
		hair.ValidateStrandToObjectTransforms( &distributionMesh );

		GuidesDelta const delta2( beforeHair, hair, ChangedHairComponents::All, ChangeTracking::Strand );
		delta.Compose( delta2, &distributionMesh );

		// This first evaluation is important
		beforeHair.CopyFrom( originalHair, true, true, true, true, true );
		delta.Apply( beforeHair, &distributionMesh, applyParameters );
		beforeHair.InvalidateStrandToObjectTransformsCache();
		beforeHair.ValidateStrandToObjectTransforms( &distributionMesh );
		REQUIRE_THAT( sharedHair.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( beforeHair.GetVertices( IHair::CoordinateSpace::Object ) ) );

		// Move strands again
		beforeHair.CopyFrom( hair, true, true, true, true, true );
		MoveTipsRandomly( hair, 0.001f );
		GuidesDelta const delta3( beforeHair, hair, ChangedHairComponents::All, ChangeTracking::Strand );
		delta.Compose( delta3, &distributionMesh );

		delta.Apply( originalHair, &distributionMesh, applyParameters );

		sharedHair.InvalidateStrandToObjectTransformsCache();
		sharedHair.ValidateStrandToObjectTransforms( &distributionMesh );

		originalHair.InvalidateStrandToObjectTransformsCache();
		originalHair.ValidateStrandToObjectTransforms( &distributionMesh );

		auto const modifiedHairVertices = sharedHair.GetVertices( IHair::CoordinateSpace::Object );
		auto const appliedHairVertices = originalHair.GetVertices( IHair::CoordinateSpace::Object );

		// ReSharper disable once CppRedundantCastExpression
		REQUIRE_THAT( modifiedHairVertices, IsVectorNearEqual( appliedHairVertices, 0.0005_r ) );
	}

	SECTION( "PlantGuideChangedInputTopology" )
	{
		auto const distributionMesh2 = GeneratePlane<Real>( 1, 3, 2 );
		TestPlantGuide( originalHair, hair, distributionMesh, distributionMesh2 );
	}

	SECTION( "PlantedGuideHasTextureCoordinates" )
	{
		originalHair.Clear();
		hair.Clear();

		GenerateRandomHairs( hair, 1, 1, &distributionMesh );

		GuidesDelta const delta(
			originalHair, hair, { true, true, false, false, false },
			ChangeTracking::Strand, &distributionMesh );

		originalHair.SetTextureCoordinateChannelCount( 1 );
		delta.Apply( originalHair, &distributionMesh, {} );

		REQUIRE( originalHair.GetTextureCoordinateChannelCount() == 1 );
		REQUIRE( originalHair.GetStrandTextureCoordinate( 0, 0 ).lengthSquared() > Epsilon );
	}

	SECTION( "PerVertexChannelDataChange" )
	{
		hair.SetStrandChannelCount( IHair::StrandDataType::PerVertex, 1 );

		// Use some random channel values
		vector<Real> channelValues( hair.GetStrandPointCount( 1 ) );
		UniformRandomGenerator( 1 ).FillWithUniformFloats( begin( channelValues ), end( channelValues ), 0.0f, 1.0f );

		hair.SetStrandChannelData( IHair::StrandDataType::PerVertex, 0, hair.GetStrandFirstVertexIndex( 1 ), Size( channelValues ), channelValues.data() );

		GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh );

		delta.Apply( originalHair, &distributionMesh, applyParameters );

		auto const appliedChannelValues = HairUtilities( originalHair ).GetStrandVertexData( 0, 1 );
		REQUIRE_THAT( channelValues, IsVectorNearEqual( appliedChannelValues ) );

		// All other strands should have zero/default channel values
		REQUIRE_THAT( vector<Real>( channelValues.size(), 0 ), IsVectorNearEqual( HairUtilities( originalHair ).GetStrandVertexData( 0, 0 ) ) );
		REQUIRE_THAT( vector<Real>( channelValues.size(), 0 ), IsVectorNearEqual( HairUtilities( originalHair ).GetStrandVertexData( 0, 2 ) ) );
		REQUIRE_THAT( vector<Real>( channelValues.size(), 0 ), IsVectorNearEqual( HairUtilities( originalHair ).GetStrandVertexData( 0, 3 ) ) );
	}

	SECTION( "DeleteStrandChannel" )
	{
		hair.SetStrandChannelCount( IHair::PerStrand, 1 );
		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		originalHair.CopyFrom( hair );
		hair.SetStrandChannelAllData( IHair::PerStrand, 0, hair.GetStrandCount(), 0.5f );
		delta.Compose( GuidesDelta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface ), &distributionMesh );

		REQUIRE( delta.GetChangeCount( GuidesDelta::Changes::StrandChannelCreations ) == 1 );
		REQUIRE( delta.GetChangeCount( GuidesDelta::Changes::StrandChannelDeletions ) == 0 );
		REQUIRE( delta.GetChangeCount( GuidesDelta::Changes::SurfaceStrandDataChanges ) == 1 );

		delta.DeleteStrandChannels( { true }, true );

		REQUIRE( delta.GetChangeCount( GuidesDelta::Changes::StrandChannelCreations ) == 0 );
		REQUIRE( delta.GetChangeCount( GuidesDelta::Changes::SurfaceStrandDataChanges ) == 0 );
	}

	SECTION( "AmountEffectOnGuideShapes" )
	{
		MoveTipsRandomly( hair, 5.0f, 0, 2 );
		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		auto const originalVertices = originalHair.GetVertices( IHair::CoordinateSpace::Object );

		applyParameters.deltaAmount = 0.5f;
		delta.Apply( originalHair, &distributionMesh, applyParameters );

		auto const changedVertices = sharedHair.GetVertices( IHair::CoordinateSpace::Object );
		auto const actualVertices = originalHair.GetVertices( IHair::CoordinateSpace::Object );

		REQUIRE_THAT( LinearInterpolate<Vector3>( originalVertices, changedVertices, 0.5 ), IsVectorNearEqual( actualVertices ) );
	}

	SECTION( "AmountEffectOnCreations" )
	{
		originalHair.Clear();
		GenerateRandomHairs( hair, 4, 1, &distributionMesh, 0, 0 );
		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		applyParameters.deltaAmount = 0.5f;
		delta.Apply( originalHair, &distributionMesh, applyParameters );

		REQUIRE( originalHair.GetStrandCount() == static_cast<int>( static_cast<float>( hair.GetStrandCount() ) * 0.5f ) );
	}

	SECTION( "CreationsWithAssignTopology" )
	{
		originalHair.Clear();
		GenerateRandomHairs( hair, 4, 1, &distributionMesh, 0, 0, -1, false, true );
		GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		originalHair.SetUsesStrandTopology( true );
		delta.Apply( originalHair, &distributionMesh, applyParameters );

		REQUIRE( originalHair.GetStrandCount() == hair.GetStrandCount() );
	}

	SECTION( "AmountEffectOnDeletions" )
	{
		auto const initialHairCount = hair.GetStrandCount();
		hair.Clear();
		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh );

		applyParameters.deltaAmount = 0.5f;
		delta.Apply( originalHair, &distributionMesh, applyParameters );

		REQUIRE( static_cast<int>( static_cast<float>( initialHairCount ) * applyParameters.deltaAmount ) == originalHair.GetStrandCount() );
	}

	SECTION( "DiffWithAddedStrands" )
	{
		GenerateRandomHairs( hair, 7, 1, &distributionMesh );
		GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		delta.Apply( originalHair, &distributionMesh, applyParameters );

		REQUIRE( originalHair.GetStrandCount() == hair.GetStrandCount() );
	}

	SECTION( "DiffWithAddedAndRotatedStrands" )
	{
		GenerateRandomHairs( hair, 1, 1, &distributionMesh, 0, 2, -1, false, false, 1, true );
		auto const addedStrandIndex = hair.GetStrandCount() - 1;
		hair.SetStrandRotation( addedStrandIndex, 0.5f );

		originalHair.ValidateStrandToObjectTransforms( &distributionMesh );
		hair.ValidateStrandToObjectTransforms( &distributionMesh );

		MoveTipsRandomly( hair, 1, addedStrandIndex );

		GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh );

		delta.Apply( originalHair, &distributionMesh, applyParameters );

		originalHair.ValidateStrandToObjectTransforms( &distributionMesh );

		REQUIRE_THAT( sharedHair.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( originalHair.GetVertices( IHair::CoordinateSpace::Object ) ) );
	}

	SECTION( "DiffShapeChangeWithVaryingPointCounts" )
	{
		GenerateRandomHairs( hair, 1, 1, &distributionMesh, 0, 2 );
		SharedHair hair2;
		GenerateRandomHairs( hair2, 1, 1, &distributionMesh, 0, 3 );

		// Make sure strands are in the same place to avoid root change
		hair2.SetSurfaceDependency2( 0, hair.GetSurfaceDependency2( 0 ) );

		// Move the vertex of second hair a bit to simulate a shape change
		MoveTipsRandomly( hair2, 1.0f );

		GuidesDelta const delta( hair, hair2, ChangedHairComponents::All, ChangeTracking::Surface );

		delta.Apply( hair, &distributionMesh, applyParameters );

		// Point count wasn't changed
		REQUIRE( hair.GetStrandPointCount( 0 ) == 2 );

		// But tip position was moved
		REQUIRE( hair.GetStrandPointCount( 0 ) == 2 );
		REQUIRE_THAT( HairUtilities( hair ).GetVertex( 1, IHair::CoordinateSpace::Object ), IsNearEqual( HairUtilities( hair2 ).GetVertex( 2, IHair::CoordinateSpace::Object ) ) );
	}

	SECTION( "DiffWithNewStrandAndShapeChange" )
	{
		// Original hair has a single straight strand a little offset from the center
		GenerateHairs( originalHair, vector{ SurfacePosition{ 0, {0.5f, 0.4f } } }, 1, &distributionMesh );
		// Modified hair has another strand in a different location and with both strands being not straight (changed shape)
		GenerateHairs(
			hair, vector{ SurfacePosition{ 0, { 0.5f, 0.4f } },
			SurfacePosition{ 0, { 0.5f, 0.6f } } }, 1, &distributionMesh );
		MoveTipsRandomly( hair );

		GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh );
		delta.Apply( originalHair, &distributionMesh, applyParameters );

		// The vertices should be identical
		REQUIRE_THAT( sharedHair.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( originalHair.GetVertices( IHair::CoordinateSpace::Object ) ) );
	}

	SECTION( "CreateRootChangeShapeThenCreateAnotherRoot" )
	{
		originalHair.Clear();
		hair.Clear();

		static auto constexpr AreVerticesStraight = []( Vector3 const& value )
		{
			return NearEqual( value.x(), 0_r ) && NearEqual( value.y(), 0_r );
		};

		// Create a single straight strand
		GenerateRandomHairs( hair, 1, 0, &distributionMesh, 0, 0, 0 );
		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );
		delta.SetCreatedStrandPointCount( 2 );
		SharedHair hair2;
		{
			hair2.CopyFrom( originalHair );
			delta.Apply( hair2, &distributionMesh, applyParameters );

			// We're supposed to have a straight strand
			auto const vertices = hair2.GetVertices( IHair::CoordinateSpace::Strand );
			REQUIRE( all_of( begin( vertices ), end( vertices ), AreVerticesStraight ) );
		}

		// Move the created strand
		{
			hair.CopyFrom( hair2 );
			MoveTipsRandomly( hair2, 1.0f );
			delta.Compose( GuidesDelta{ hair, hair2, ChangedHairComponents::All, ChangeTracking::Surface }, &distributionMesh );
			hair2.CopyFrom( originalHair );
			delta.Apply( hair2, &distributionMesh, applyParameters );

			// The strand is moved
			auto const vertices = hair2.GetVertices( IHair::CoordinateSpace::Strand );
			REQUIRE( !all_of( begin( vertices ), end( vertices ), AreVerticesStraight ) );
		}

		// Create another single strand
		{
			hair.CopyFrom( hair2 );
			GenerateRandomHairs( hair2, 1, 0, &distributionMesh, 0, 0, 0, false, false, 2, true );
			delta.Compose( GuidesDelta{ hair, hair2, ChangedHairComponents::All, ChangeTracking::Surface }, &distributionMesh );
			hair2.CopyFrom( originalHair );
			delta.Apply( hair2, &distributionMesh, applyParameters );

			// First strand should remain as it was, second strand should be straight
			auto const vertices = hair2.GetVertices( IHair::CoordinateSpace::Strand );
			REQUIRE( !all_of( begin( vertices ), middle( vertices ), AreVerticesStraight ) );
			REQUIRE( all_of( middle( vertices ), end( vertices ), AreVerticesStraight ) );
		}
	}

	SECTION( "CreatedStrandsHaveTextureCoordinates" )
	{
		originalHair.Clear();
		hair.Clear();

		// Create a single straight strand
		GenerateRandomHairs( hair, 1, 0, &distributionMesh, 0, 0, 0 );
		GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );
		delta.SetCreatedStrandPointCount( 2 );
		auto hair2 = originalHair;
		hair2.CopyFrom( originalHair );
		delta.Apply( hair2, &distributionMesh, applyParameters );

		REQUIRE( hair2.GetTextureCoordinateChannelCount() == 1 );
		REQUIRE( hair2.GetStrandTextureCoordinate( 0, 0 ).lengthSquared() > Epsilon );
	}

	// Prevents #5782
	SECTION( "CreatedStrandsHaveStrandIds" )
	{
		originalHair.Clear();
		hair.Clear();
		originalHair.SetUseStrandIds( false );
		hair.SetUseStrandIds( false );

		// Create a single straight strand
		GenerateRandomHairs( hair, 1, 0, &distributionMesh, 0, 0, 0 );
		GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );
		delta.SetCreatedStrandPointCount( 2 );
		auto hair2 = originalHair;
		hair2.CopyFrom( originalHair );

		REQUIRE( !hair2.HasStrandIds() );
		delta.Apply( hair2, &distributionMesh, applyParameters );

		REQUIRE( hair2.GetStrandCount() == 1 );
		REQUIRE( hair2.HasStrandIds() );
	}

	SECTION( "CreatedStrandInheritsStrandGroup" )
	{
		hair.SetUsesStrandGroups( true );
		hair.SetStrandGroups( array<int, 4> { 3, 3, 3, 3 } );
		originalHair.CopyFrom( hair );

		// Append a single empty root
		GenerateRandomHairs( hair, 1, 0, &distributionMesh, 0, 0, 0, false, false, 1, true );
		GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );
		delta.Apply( originalHair, &distributionMesh, applyParameters );

		REQUIRE( originalHair.GetStrandGroup( 4 ) == 3 );
	}

	SECTION( "CreateFromDifferingPointCounts" )
	{
		// Generate three source strands with three different point counts each
		//GenerateRandomHairs( originalHair, 3, 1, &distributionMesh, 0, 2, 20 );
		GenerateHairs(
			originalHair, GenerateVertexSurfaceDependencies( &distributionMesh ), 1, &distributionMesh, 0, 2, -1, false, false,
			false, array<int, 4> { 10, 10, 11, 9 } );
		hair.CopyFrom( originalHair );

		// Create five strands: one closest to each corner and one in the middle
		array<SurfacePosition, 5> const createdPositions =
		{
			SurfacePosition { 0u, { 0.1f, 0.1f } },
			SurfacePosition { 0u, { 0.9f, 0.1f } },
			SurfacePosition { 0u, { 0.1f, 0.9f } },
			SurfacePosition { 0u, { 0.9f, 0.9f } },
			SurfacePosition { 0u, { 0.5f, 0.5f } }
		};

		GenerateHairs( hair, createdPositions, 1, &distributionMesh, 0, 0, 0, false, false, true );
		GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );
		delta.SetCreatedStrandPointCount( hair.GetStrandPointCount( 0 ) );
		auto hair2 = originalHair;

		REQUIRE( delta.SetInterpolationMethod( SpatialPointInterpolator::Method::Triangulation ) );
		delta.Apply( hair2, &distributionMesh, applyParameters );

		// If we didn't crash it's good enough but check point count just in case too
		REQUIRE( hair2.GetStrandPointCount( 4 ) == hair2.GetStrandPointCount( 2 ) );
	}

	SECTION( "MultiResolutionDeleteWithoutDistributionMesh" )
	{
		hair.DeleteStrandsByIndices( array<int, 2> { 0, 3 } );
		REQUIRE( hair.GetStrandCount() == 2 );
		GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		auto const originalHairCount = originalHair.GetStrandCount();
		REQUIRE_LOG_WARNING_BLOCK( L"Distribution mesh is required, deletions were not applied" )
		{
			delta.Apply( originalHair, nullptr, applyParameters );
		}

		REQUIRE( originalHairCount == originalHair.GetStrandCount() );
	}

	SECTION( "CreateStrandAfterCreationsWithShapeChanges" )
	{
		applyParameters.createdStrandPointCount = 2;
		applyParameters.createdStrandLength = 1.0f;

		auto delta = CreateTwoStrands();

		originalHair.Clear();
		hair.Clear();
		originalHair.SetUsesStrandTopology( true );
		hair.SetUsesStrandTopology( true );

		SharedHair hair2;
		delta.Apply( hair, &distributionMesh, applyParameters );

		// Offset vertices
		{
			hair2.CopyFrom( hair );

			array<Vector3, 4> vertices;
			hair.GetVertices( 0, Size( vertices ), vertices.data(), IHair::CoordinateSpace::Object );
			vertices[1] += Vector3::XAxis();
			vertices[3] += Vector3::YAxis();
			hair.SetVertices( 0, Size( vertices ), vertices.data(), IHair::CoordinateSpace::Object );

			GuidesDelta const incrementalDelta( hair2, hair, ChangedHairComponents::All, ChangeTracking::Surface );
			delta.Compose( incrementalDelta, &distributionMesh );
		}

		hair.CopyFrom( originalHair );
		delta.Apply( hair, &distributionMesh, applyParameters );

		array<StrandId, 2> createdStrandIds{};
		hair.GetStrandIds( 0, Size( createdStrandIds ), createdStrandIds.data() );

		// Plant a new root
		array plantedVertices = { Vector3::Zero(), Vector3::ZAxis() };
		{
			hair2.CopyFrom( hair );
			HairUtilities( hair ).AppendStrand( SurfacePosition{ 0, { 0.5f, 0.5f } }, plantedVertices, IHair::CoordinateSpace::Object, &distributionMesh );

			GuidesDelta const incrementalDelta( hair2, hair, ChangedHairComponents::All, ChangeTracking::Surface );
			delta.Compose( incrementalDelta, &distributionMesh );
		}

		hair.CopyFrom( originalHair );
		delta.Apply( hair, &distributionMesh, applyParameters );

		// Make sure that after delta application the planted root points didn't change object space positions
		array<StrandId, 3> createdStrandIds2{};
		hair.GetStrandIds( 0, Size( createdStrandIds2 ), createdStrandIds2.data() );

		unordered_set createdStrandIdsSet( begin( createdStrandIds2 ), end( createdStrandIds2 ) );
		for( auto createdStrandId : createdStrandIds )
		{
			createdStrandIdsSet.erase( createdStrandId );
		}

		array<Vector3, 2> actualVertices;
		hair.ValidateStrandToObjectTransforms( &distributionMesh );

		hair.GetStrandPoints( hair.GetStrandIndexOrId( *createdStrandIdsSet.begin() ), 0, Size( actualVertices ), actualVertices.data(), IHair::CoordinateSpace::Object );

		REQUIRE_THAT( plantedVertices, IsVectorNearEqual( actualVertices ) );
	}

	SECTION( "ChangingStrandCountWithShapeModification" )
	{
		GenerateRandomHairs( hair, 4, 1, &distributionMesh, 0, 2, -1, false, false, 1 );
		originalHair.CopyFrom( sharedHair, true, true, true, true, true );

		// Record spatial shape changes to all strands
		MoveTipsRandomly( hair, 1.0f );
		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		// Create differently placed strands and apply the previous shape changes to them
		static auto constexpr UpdatedStrandCount = 5;
		SharedHair hairWithDifferentRoots;
		GenerateRandomHairs( hairWithDifferentRoots, UpdatedStrandCount, 1, &distributionMesh, 0, 2, -1, false, false, 2 );

		auto hairWithDifferentRootsAfterDelta = hairWithDifferentRoots;
		delta.Apply( hairWithDifferentRootsAfterDelta, &distributionMesh, applyParameters );

		// Move a single tip of new strands and record the shape changes
		static auto constexpr ChangedStrandIndex = 1;
		auto hairWithDifferentRootsAfterDeltaAndSingleHairMove = hairWithDifferentRootsAfterDelta;
		MoveTips( hairWithDifferentRootsAfterDeltaAndSingleHairMove, Vector3( 0.5f, 0.5, 0.0f ), ChangedStrandIndex, -1, 100 );

		GuidesDelta const incrementalDelta( hairWithDifferentRootsAfterDelta, hairWithDifferentRootsAfterDeltaAndSingleHairMove, ChangedHairComponents::All, ChangeTracking::Surface );
		delta.Compose( incrementalDelta, &distributionMesh );

		// Apply it to newly placed strands
		auto hairWithDifferentRootsAfterSingleStrandEditDelta = hairWithDifferentRoots;
		delta.Apply( hairWithDifferentRootsAfterSingleStrandEditDelta, &distributionMesh, applyParameters );

		// All hairs should remain in place
		REQUIRE_THAT(
			hairWithDifferentRootsAfterDeltaAndSingleHairMove.GetVertices( IHair::CoordinateSpace::Object ),
			IsVectorNearEqual( hairWithDifferentRootsAfterSingleStrandEditDelta.GetVertices( IHair::CoordinateSpace::Object ) ) );
	}

	SECTION( "ChangeShapeMoveRootChangeShape" )
	{
		// Record spatial shape changes to all strands
		MoveTipsRandomly( hair, 1.0f );
		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		// Move one of the hair strands
		{
			auto const hairWithShapeChange = sharedHair;
			SurfacePosition const movedSurfacePosition = { 0u, { .0f, .0f } };
			sharedHair.SetSurfaceDependencies2( 0, 1, &movedSurfacePosition );
			sharedHair.InvalidateStrandToObjectTransformsCache();
			hair.ValidateStrandToObjectTransforms( &distributionMesh );

			GuidesDelta const incrementalDelta( hairWithShapeChange, hair, ChangedHairComponents::All, ChangeTracking::Surface );
			delta.Compose( incrementalDelta, &distributionMesh );

			REQUIRE( delta.GetChangeCount( GuidesDelta::Changes::SurfaceShapeChanges ) == 4 );
		}

		// Change the shape of the moved strand
		{
			auto const hairWithMovedRoot = sharedHair;
			MoveTips( sharedHair, Vector3::XAxis(), 0, -1, 100 );
			GuidesDelta const incrementalDelta( hairWithMovedRoot, hair, ChangedHairComponents::All, ChangeTracking::Surface );
			delta.Compose( incrementalDelta, &distributionMesh );

			// One extra shape change recorded
			REQUIRE( delta.GetChangeCount( GuidesDelta::Changes::SurfaceShapeChanges ) == 4 );
		}

		// The modified hair should match the one with the moved strand
		auto hairWithMovedRootAndChangedShape = originalHair;
		delta.Apply( hairWithMovedRootAndChangedShape, &distributionMesh, applyParameters );

		REQUIRE_THAT( sharedHair.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( hairWithMovedRootAndChangedShape.GetVertices( IHair::CoordinateSpace::Object ) ) );

		// Change the shape of the moved strand again
		{
			auto const hairWithMovedRoot = sharedHair;
			MoveTips( sharedHair, Vector3::YAxis(), 0, -1, 100 );
			GuidesDelta const incrementalDelta( hairWithMovedRoot, hair, ChangedHairComponents::All, ChangeTracking::Surface );
			delta.Compose( incrementalDelta, &distributionMesh );

			REQUIRE( delta.GetChangeCount( GuidesDelta::Changes::SurfaceShapeChanges ) == 4 );
		}

		hairWithMovedRootAndChangedShape = originalHair;
		delta.Apply( hairWithMovedRootAndChangedShape, &distributionMesh, applyParameters );

		REQUIRE_THAT( sharedHair.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( hairWithMovedRootAndChangedShape.GetVertices( IHair::CoordinateSpace::Object ) ) );

		// If we decide to preserve strand shape when roots are moved in spatial mode then lines below should be enabled
#if 0
		// Move the same hair strand again
		{
			const auto hairWithShapeChange = sharedHair;
			const SurfacePosition movedSurfacePosition = { 0u, { 0.26f, 0.65f } };
			sharedHair.SetSurfaceDependencies2( 0, 1, &movedSurfacePosition );
			sharedHair.InvalidateStrandToObjectTransformsCache();
			hair.ValidateStrandToObjectTransforms( &distributionMesh );

			const GuidesDelta incrementalDelta( hairWithShapeChange, hair, ChangedHairComponents::All, ChangeTracking::Surface );
			delta.Compose( incrementalDelta, &distributionMesh );
		}

		hairWithMovedRootAndChangedShape = originalHair;
		delta.Apply( hairWithMovedRootAndChangedShape, &distributionMesh );

		const auto expectedVertices = hair.GetVertices( IHair::CoordinateSpace::Object );
		REQUIRE_THAT( expectedVertices, IsVectorNearEqual( hairWithMovedRootAndChangedShape.GetVertices( IHair::CoordinateSpace::Object ) ) );
#endif
	}

	SECTION( "TwoRootsChangeShapeMoveRootSurface" )
	{
		// Record spatial shape to one of the strands
		MoveTipsRandomly( hair, 1.0f, 0, 100 );
		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		auto const pointsAfterShapeChange = sharedHair.GetVertices( IHair::CoordinateSpace::Object, 0, 2 );

		// Move the hair strand whose shape was changed
		{
			auto const hairWithShapeChange = sharedHair;
			SurfacePosition const movedSurfacePosition = { 0u, { .7f, .7f } };
			sharedHair.SetSurfaceDependencies2( 0, 1, &movedSurfacePosition );
			sharedHair.InvalidateStrandToObjectTransformsCache();
			hair.ValidateStrandToObjectTransforms( &distributionMesh );

			GuidesDelta const incrementalDelta( hairWithShapeChange, hair, ChangedHairComponents::All, ChangeTracking::Surface );
			delta.Compose( incrementalDelta, &distributionMesh );
		}

		hair.CopyFrom( originalHair );
		delta.Apply( hair, &distributionMesh, applyParameters );
		auto const pointsAfterRootMove = sharedHair.GetVertices( IHair::CoordinateSpace::Object, 0, 2 );

		REQUIRE_THAT( pointsAfterShapeChange[1] - pointsAfterShapeChange[0], IsNearEqual( pointsAfterRootMove[1] - pointsAfterRootMove[0] ) );
	}

	SECTION( "PlantGuideAfterCreatingAndChangingShape" )
	{
		auto delta = CreateTwoStrands();

		originalHair.Clear();
		hair.Clear();
		originalHair.SetUsesStrandTopology( true );
		hair.SetUsesStrandTopology( true );

		SharedHair hair2;
		delta.Apply( hair, &distributionMesh, applyParameters );
		REQUIRE( hair.GetStrandCount() == 2 );

		// Move second strand tip
		{
			hair2.CopyFrom( hair );
			MoveTipsRandomly( hair, 1.0f, 1 );

			GuidesDelta const incrementalDelta( hair2, hair, ChangedHairComponents::All, ChangeTracking::Surface );
			delta.Compose( incrementalDelta, &distributionMesh );
		}

		REQUIRE_THAT( GetVerticesSortedByStrandIds( hair, IHair::CoordinateSpace::Object ), !IsVectorNearEqual( GetVerticesSortedByStrandIds( hair2, IHair::CoordinateSpace::Object ) ) );

		originalHair.Clear();
		delta.Apply( originalHair, &distributionMesh, applyParameters );

		// Plant a new strand
		StrandId constexpr plantedStrandId = 10;
		{
			hair2.CopyFrom( hair );

			auto const strandIndex = hair.GetStrandCount();
			auto const previousVertexCount = hair.GetVertexCount();

			hair.SetStrandCount( strandIndex + 1 );

			auto const vertices = LinearInterpolateRange( Vector3f::Zero(), Vector3f( 0.34f, 0.67f, 1.0f ), hair.GetStrandPointCount( 0 ) );
			hair.SetVertexCount( previousVertexCount + Size( vertices ) );

			SurfacePosition const surfacePosition = { 0, { 0.5f, 0.5f } };
			hair.SetSurfaceDependencies2( strandIndex, 1, &surfacePosition );

			StrandTopology const topology = { static_cast<unsigned>( previousVertexCount ), static_cast<unsigned>( vertices.size() ) };
			hair.SetStrandTopologies( strandIndex, 1, &topology );

			hair.SetStrandPoints( strandIndex, 0, Size( vertices ), vertices.data(), IHair::CoordinateSpace::Strand );

			hair.ValidateStrandToObjectTransforms( &distributionMesh );

			hair.SetStrandId( strandIndex, plantedStrandId );

			GuidesDelta const incrementalDelta( hair2, hair, ChangedHairComponents::All, ChangeTracking::Surface );
			delta.Compose( incrementalDelta, &distributionMesh );
		}

		hair2.Clear();
		delta.Apply( hair2, &distributionMesh, applyParameters );

		// Need to manually assign planted strand id so it matches the one we used for creating the strand
		hair2.SetStrandId( 2, plantedStrandId );

		REQUIRE_THAT( GetVerticesSortedByStrandIds( hair, IHair::CoordinateSpace::Object ), IsVectorNearEqual( GetVerticesSortedByStrandIds( hair2, IHair::CoordinateSpace::Object ) ) );
	}

	SECTION( "DeleteSpecificPlantedGuide" )
	{
		GenerateRandomHairs( hair, 1, 1, &distributionMesh );

		SharedHair hair2;
		GuidesDelta delta( hair2, hair, ChangedHairComponents::All, ChangeTracking::Strand );
		delta.Apply( hair2, &distributionMesh, applyParameters );

		//const auto createdStrandId = hair2.GetStrandId( 0 );

		// Delete the created strand
		auto index = 0;
		REQUIRE( hair.DeleteStrandsByIndices( &index, 1 ) );

		GuidesDelta const incrementalDelta( hair2, hair, ChangedHairComponents::All, ChangeTracking::Strand );
		delta.Compose( incrementalDelta, &distributionMesh );

		hair2.Clear();
		delta.Apply( hair2, &distributionMesh, applyParameters );

		// No strand is created
		REQUIRE( hair2.GetStrandCount() == 0 );
	}

	SECTION( "DeleteSpecificCreatedGuide" )
	{
		hair.SetUseStrandIds( true );
		GenerateRandomHairs( hair, 1, 0, &distributionMesh, 0, 0 );

		SharedHair hair2;
		hair2.SetUseStrandIds( true );
		GuidesDelta delta( hair2, hair, ChangedHairComponents::All, ChangeTracking::Surface );
		delta.Apply( hair2, &distributionMesh, applyParameters );

		// Delete the created strand
		auto index = 0;
		REQUIRE( hair.DeleteStrandsByIndices( &index, 1 ) );

		GuidesDelta const incrementalDelta( hair2, hair, ChangedHairComponents::All, ChangeTracking::Strand );
		delta.Compose( incrementalDelta, &distributionMesh );

		// We deleted the only previously created strand. The delta should be empty.
		REQUIRE( delta.GetChangeCount( GuidesDelta::Changes::SurfaceCreations ) == 0 );
		REQUIRE( delta.GetChangeCount( GuidesDelta::Changes::SpecificDeletions ) == 0 );
		//hair2.Clear();
		//delta.Apply( hair2, &distributionMesh, applyParameters );

		//// No strand is created
		//REQUIRE( hair2.GetStrandCount() == 0 );
	}

	// Test that changing a shape of a planted guide together with a nearby strand using surface tracking will not modify the planted guide further
	SECTION( "SurfaceShapeChangeOfPlantedGuide" )
	{
		TestShapeChangeOfPlantedGuide( hair, originalHair, distributionMesh, ChangeTracking::Surface );
	}

	SECTION( "StrandShapeChangeOfPlantedGuide" )
	{
		TestShapeChangeOfPlantedGuide( hair, originalHair, distributionMesh, ChangeTracking::Strand );
	}

	SECTION( "ChangeShapeAfterPlanting" )
	{
		auto delta = PlantStrands( 2 );

		originalHair.Clear();
		hair.Clear();
		originalHair.SetUsesStrandTopology( true );
		hair.SetUsesStrandTopology( true );

		SharedHair hair2;
		delta.Apply( hair, &distributionMesh, applyParameters );

		// Move both strand tips
		{
			hair2.CopyFrom( hair );
			MoveTips( hair, Vector3::XAxis() );

			GuidesDelta const incrementalDelta( hair2, hair, ChangedHairComponents::All, ChangeTracking::Surface );
			delta.Compose( incrementalDelta, &distributionMesh );
		}

		originalHair.Clear();
		delta.Apply( originalHair, &distributionMesh, applyParameters );

		REQUIRE_THAT( GetVerticesSortedByStrandIds( hair, IHair::CoordinateSpace::Object ), IsVectorNearEqual( GetVerticesSortedByStrandIds( originalHair, IHair::CoordinateSpace::Object ) ) );
	}

	// Tests that we can change first strand-tracked strand group and then surface-tracked with expected results
	SECTION( "ChangeBothStrandAndSurfaceStrandGroups" )
	{
		hair.SetUsesStrandGroups( true );
		originalHair.SetUsesStrandGroups( true );

		SharedHair hair2;
		hair2.CopyFrom( hair );

		GuidesDelta delta( hair );

		// Change strand group with strand-tracking
		{
			hair2.SetStrandGroup( 0, 1 );
			GuidesDelta const incrementalDelta( hair, hair2, ChangedHairComponents::All, ChangeTracking::Strand, &distributionMesh );
			delta.Compose( incrementalDelta, &distributionMesh );
		}

		hair.CopyFrom( originalHair );
		delta.Apply( hair, &distributionMesh, applyParameters );
		REQUIRE( hair.GetStrandGroup( 0 ) == 1 );

		// Change strand group with surface-tracking
		{
			hair.CopyFrom( hair2 );
			hair2.SetStrandGroup( 0, 2 );
			GuidesDelta const incrementalDelta( hair, hair2, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh );
			delta.Compose( incrementalDelta, &distributionMesh );
		}

		// We expect strand group 2
		hair.CopyFrom( originalHair );
		delta.Apply( hair, &distributionMesh, applyParameters );
		REQUIRE( hair.GetStrandGroup( 0 ) == 2 );
	}

	SECTION( "SurfaceTrackingChangeShapeTwoRootsSameSurfacePosition" )
	{
		// Create two strands in the same position
		GenerateRandomHairs( hair, 2, 1, &distributionMesh );
		hair.SetSurfaceDependency2( 1, hair.GetSurfaceDependency2( 0 ) );
		originalHair.CopyFrom( hair );

		// Record spatial shape changes to all strands
		MoveTipsRandomly( hair, 1.0f );
		GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		// Because both strands are in the same position and we're using surface tracking only one change should've been recorded
		REQUIRE( delta.GetChangeCount( GuidesDelta::Changes::SurfaceShapeChanges ) == 1 );
	}

	SECTION( "SurfaceTrackingCreateAndChangeStrandDoesNotModifyExistingStrands" )
	{
		applyParameters.createdStrandLength = 1;
		applyParameters.createdStrandPointCount = 2;

		// Create an initial single strand
		GenerateRandomHairs( hair, 1, 1, &distributionMesh );

		// Enable strand topology since we'll be creating hair roots
		REQUIRE( HairUtilities( hair ).EnableStrandTopology() );

		originalHair.CopyFrom( hair );

		GuidesDelta delta;

		// Create another strand using root creation
		{
			hair.SetStrandCount( 2 );
			hair.SetSurfaceDependency2( 1, SurfacePosition{ 0, Vector2f::Zero() } );
			hair.SetStrandTopologies( vector<StrandTopology> {
				{
					0u, 0u
				} }, 1 );

			GuidesDelta const incrementalDelta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );
			delta.Compose( incrementalDelta, &distributionMesh );
		}

		hair.CopyFrom( originalHair );
		delta.Apply( hair, &distributionMesh, applyParameters );

		REQUIRE( hair.GetStrandCount() == 2 );
		REQUIRE( hair.GetVertexCount() == 4 );

		hair.ValidateStrandToObjectTransforms( &distributionMesh );

		// Modify the shape of the created strand
		{
			auto strand2PointsAfterShapeChange = sharedHair.GetVertices( IHair::CoordinateSpace::Object, 2, 2 );
			strand2PointsAfterShapeChange[1] += Vector3::XAxis();

			auto const hairBeforeChange = sharedHair;  // NOLINT(performance-unnecessary-copy-initialization)
			REQUIRE( hairUtilities.SetVertices( strand2PointsAfterShapeChange, IHair::CoordinateSpace::Object, 2 ) );

			GuidesDelta const incrementalDelta( hairBeforeChange, hair, ChangedHairComponents::All, ChangeTracking::Surface );
			delta.Compose( incrementalDelta, &distributionMesh );
		}

		auto const hairVerticesAfterEdit = sharedHair.GetVertices( IHair::CoordinateSpace::Object );
		hair.CopyFrom( originalHair );
		delta.Apply( hair, &distributionMesh, applyParameters );
		hair.ValidateStrandToObjectTransforms( &distributionMesh );
		auto const hairVerticesAfterApply = sharedHair.GetVertices( IHair::CoordinateSpace::Object );

		REQUIRE_THAT( hairVerticesAfterEdit, IsVectorNearEqual( hairVerticesAfterApply ) );
	}

	SECTION( "SurfaceTrackingChangeShapeMoveRootToDifferentlyOrientedFace" )
	{
		// Use a cube to ensure that rotations of surface transforms will change along faces
		GenerateCube<Real>( distributionMesh, 1.0f, true, true );

		// Randomly offset cube's vertices to avoid right angle transform rotations
		{
			UniformRandomGenerator randomGenerator( 1 );
			vector<Vector3> vertices( distributionMesh.GetVertexCount() );
			distributionMesh.GetVertices( 0, Size( vertices ), vertices.data() );
			for( auto& vertex : vertices )
			{
				vertex += randomGenerator.GetNextSignedFloatValue() * 0.2f;
			}

			distributionMesh.SetVertices( 0, Size( vertices ), vertices.data() );
		}

		// Generate a single strand in the middle of first face
		originalHair.Clear();
		GenerateHairs( originalHair, array{ SurfacePosition { 0, { 0.3f, 0.3f } } }, 1, &distributionMesh );
		hair.CopyFrom( originalHair );

		// Record spatial shape changes to the single strand
		MoveTips( hair, Vector3::YAxis() );
		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		// Move the hair strand to a differently oriented face
		auto hair2 = sharedHair;
		GuidesEditor::MoveStrandRoot( HairUtilities( hair ).GetStrandToObjectTransform( 0 ), hair2, 0, SurfacePosition{ 2, Vector2f( 0.5f, 0.5f ) }, &distributionMesh );

		REQUIRE( fabs( hair2.GetStrandRotation( 0 ) - hair.GetStrandRotation( 0 ) ) > Epsilon );

		// Compose in the changes but only track changes to topology (moved root) and rotations, as a host would do it
		delta.Compose(
			GuidesDelta( hair, hair2, { false, true, false, false, true }, ChangeTracking::Surface ),
			&distributionMesh );

		auto hair3 = originalHair;
		delta.Apply( hair3, &distributionMesh, applyParameters );

		hair3.InvalidateStrandToObjectTransformsCache();
		hair3.ValidateStrandToObjectTransforms( &distributionMesh );

		REQUIRE_THAT( hair3.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( hair2.GetVertices( IHair::CoordinateSpace::Object ) ) );
	}

	SECTION( "PerStrandChannelAddAndSetValues" )
	{
		static auto constexpr ChannelValue = 2.0f;
		hair.SetStrandChannelCount( IHair::StrandDataType::PerStrand, 1 );
		hair.SetStrandChannelAllData( IHair::StrandDataType::PerStrand, 0, 1, ChannelValue );

		GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );
		REQUIRE( delta.GetChangeCount( GuidesDelta::Changes::StrandChannelCreations ) == 1 );
		REQUIRE( delta.GetChangeCount( GuidesDelta::Changes::SurfaceStrandDataChanges ) == 1 );
	}

	SECTION( "PerVertexChannelAddAndSetValues" )
	{
		static auto constexpr ChannelValue = 2.0f;
		hair.SetStrandChannelCount( IHair::StrandDataType::PerVertex, 1 );
		hair.SetStrandChannelAllData( IHair::StrandDataType::PerVertex, 0, 1, ChannelValue );

		GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );
		REQUIRE( delta.GetChangeCount( GuidesDelta::Changes::VertexChannelCreations ) == 1 );
		REQUIRE( delta.GetChangeCount( GuidesDelta::Changes::SurfaceVertexDataChanges ) == 1 );
	}

	SECTION( "ChangePropagatedStrandShapeWithSurfaceTracking" )
	{
		GenerateHairs(
			hair, array
			{
				// Strand on mesh surface
				SurfacePosition( 0u, Vector2f::Zero() ),
				// Propagated strand
				SurfacePosition( 0u, Vector2f( 0.5f, -1.0f ) )
			}, 1, &distributionMesh );
		originalHair.CopyFrom( hair );

		// Modify propagated strand's shape
		MoveTipsRandomly( hair, 1, 1 );

		GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		delta.Apply( originalHair, &distributionMesh, applyParameters );

		hair.ValidateStrandToObjectTransforms( &distributionMesh );
		originalHair.ValidateStrandToObjectTransforms( &distributionMesh );

		REQUIRE_THAT( sharedHair.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( originalHair.GetVertices( IHair::CoordinateSpace::Object ) ) );
	}

	SECTION( "ChangePropagatedStrandShapeWithSurfaceTracking2" )
	{
		GenerateHairs(
			hair, array
			{
				// Strand on mesh surface
				SurfacePosition( 0u, Vector2f::Zero() ),
				// Propagated strand 1
				SurfacePosition( 0u, Vector2f( 0.3f, -1.0f ) ),
				// Propagated strand 2
				SurfacePosition( 0u, Vector2f( 0.6f, -1.0f ) )
			}, 1, &distributionMesh );
		originalHair.CopyFrom( hair );

		// Modify propagated strand's shape
		MoveTipsRandomly( hair, 1, 1 );

		GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		delta.Apply( originalHair, &distributionMesh, applyParameters );

		hair.ValidateStrandToObjectTransforms( &distributionMesh );
		originalHair.ValidateStrandToObjectTransforms( &distributionMesh );

		REQUIRE_THAT( sharedHair.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( originalHair.GetVertices( IHair::CoordinateSpace::Object ) ) );
	}

	SECTION( "ChangePropagatedStrandShapeTwiceWithSurfaceTracking" )
	{
		GenerateHairs(
			hair, array
			{
				// Strand on mesh surface
				SurfacePosition( 0u, Vector2f::Zero() ),
				// Propagated strand
				SurfacePosition( 0u, Vector2f( 0.5f, -1.0f ) )
			}, 1, &distributionMesh );
		originalHair.CopyFrom( hair );

		// Modify propagated strand's shape
		MoveTipsRandomly( hair, 1, 1 );

		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		auto const hair2 = sharedHair;  // NOLINT(performance-unnecessary-copy-initialization)
		MoveTipsRandomly( hair, 1, 1, 1, 2 );
		GuidesDelta const incrementalDelta( hair2, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		delta.Compose( incrementalDelta, &distributionMesh );

		delta.Apply( originalHair, &distributionMesh, applyParameters );

		hair.ValidateStrandToObjectTransforms( &distributionMesh );
		originalHair.ValidateStrandToObjectTransforms( &distributionMesh );

		REQUIRE_THAT( sharedHair.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( originalHair.GetVertices( IHair::CoordinateSpace::Object ) ) );
	}

	SECTION( "ChangePropagatedStrandShapeTwiceWithSurfaceTracking2" )
	{
		GenerateHairs(
			hair, array
			{
				// Strand on mesh surface
				SurfacePosition( 0u, Vector2f::Zero() ),
				// Propagated strand 1
				SurfacePosition( 0u, Vector2f( 0.3f, -1.0f ) ),
				// Propagated strand 2
				SurfacePosition( 0u, Vector2f( 0.6f, -1.0f ) )
			}, 1, &distributionMesh );
		originalHair.CopyFrom( hair );

		// Modify propagated strand's shape
		MoveTipsRandomly( hair );
		hair.ValidateStrandToObjectTransforms( &distributionMesh );

		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh );

		// Do a few iterations of changes
		for( auto iteration = 0; iteration < 5; ++iteration )
		{
			auto const hair2 = sharedHair;  // NOLINT(performance-unnecessary-copy-initialization)
			MoveTipsRandomly( hair, 0.5f, iteration % 2 == 0 ? 1 : 2, 1, iteration + 2 );

			GuidesDelta const incrementalDelta( hair2, hair, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh );
			delta.Compose( incrementalDelta, &distributionMesh );
		}

		delta.Apply( originalHair, &distributionMesh, applyParameters );
		hair.ValidateStrandToObjectTransforms( &distributionMesh );
		originalHair.ValidateStrandToObjectTransforms( &distributionMesh );

		REQUIRE_THAT( sharedHair.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( originalHair.GetVertices( IHair::CoordinateSpace::Object ) ) );
	}

	SECTION( "NewStrandWithSourceMapping" )
	{
		auto const originalStrandIndex = 0, newStrandIndex = hair.GetStrandCount();

		// Add another strand
		auto const newStrandId = HairUtilities( hair ).AppendStrand( { 0u, { 0.1f, 0.1f } }, array{ Vector3::Zero(), Vector3::ZAxis() } );

		unordered_map<StrandId, StrandId> const strandSourceMap = { { newStrandId, hair.GetStrandId( originalStrandIndex ) } };

		GuidesDelta const delta(
			originalHair, hair, { true, true, false, false, false },
			ChangeTracking::Surface, &distributionMesh, &strandSourceMap );

		auto hair2 = originalHair;
		delta.Apply( hair2, &distributionMesh, {} );

		auto const pointsWithoutSourceShapeChange = HairUtilities( hair2 ).GetStrandPoints( newStrandIndex );

		// Modify the source shape and apply the delta again
		hair2 = originalHair;
		MoveTipsRandomly( hair2, 1, originalStrandIndex, 999 );
		delta.Apply( hair2, &distributionMesh, {} );

		auto const pointsWithSourceShapeChange = HairUtilities( hair2 ).GetStrandPoints( newStrandIndex );

		REQUIRE_THAT( pointsWithoutSourceShapeChange, !IsVectorNearEqual( pointsWithSourceShapeChange ) );
	}

	SECTION( "RenameNonExistingChannel" )
	{
		hair.SetStrandChannelCount( IHair::StrandDataType::PerStrand, 2 );
		HairUtilities( hair ).SetStrandChannelName( IHair::StrandDataType::PerStrand, 0, "test" );
		HairUtilities( hair ).SetStrandChannelName( IHair::StrandDataType::PerStrand, 1, "test2" );

		GuidesDelta delta( hair );
		delta.SetChannelName( 1, L"renamed", true );
		REQUIRE( delta.GetChanges() == GuidesDelta::Changes::StrandChannelRenames );

		hair.SetStrandChannelCount( IHair::StrandDataType::PerStrand, 1 );
		delta.Apply( hair, &distributionMesh, {} );

		REQUIRE( HairUtilities( hair ).GetStrandChannelName() == L"test" );
	}

	// The GuidesDelta type before 29105 was a simple value, now it is a shared_ptr and serializes a bool flag if the value is empty
	SECTION( "DeserializeFormatPre29105" )
	{
		auto delta = make_shared<GuidesDelta>();
		REQUIRE( delta->GetLoadedVersion() == GuidesDelta::GuidesDeltaVersion );
		auto data = Parameters::GetType<SharedPtr<GuidesDelta>>().ToString( &delta );

		// At this location is stored the version value
		data[7] = 123;
		auto loaded = make_shared<GuidesDelta>();
		REQUIRE( Parameters::GetType<SharedPtr<GuidesDelta>>().FromString( data, &loaded ) );
		REQUIRE( loaded != nullptr );
		REQUIRE( loaded->GetLoadedVersion() == 123 );

		// Remove the bool flag at the start of the data to simulate pre-29105 serialization
		data.erase( data.begin() );
		loaded = make_shared<GuidesDelta>();
		REQUIRE( Parameters::GetType<SharedPtr<GuidesDelta>>().FromString( data, &loaded ) );
		REQUIRE( loaded != nullptr );
		REQUIRE( loaded->GetLoadedVersion() == 123 );
	}
}

TEST_CASE( "GuidesDeltaShapeChangesChangedInputTopology" )
{
	SharedHair originalHair, hair, modifiedHair;

	auto const distributionMesh = GeneratePlane<Real>( 1, 5, 5 );

	GenerateRandomHairs( hair, 4, 1, &distributionMesh );
	originalHair.CopyFrom( hair, true, true, true, true, true );
	modifiedHair.CopyFrom( hair, true, true, true, true, true );

	auto const distributionMeshDenser = GeneratePlane<Real>( 1, 5, 5 );
	auto const distributionMeshSparser = GeneratePlane<Real>( 1, 3, 3 );

	Vector3 const moveVector( 5, 0, 0 );
	MoveTips( hair, moveVector );

	function const checkHairChange = [&moveVector]( IHair const& hair1, IHair const& hair2 )
	{
		auto const vertices1 = hair1.GetVertices( IHair::CoordinateSpace::Object );
		auto const vertices2 = hair2.GetVertices( IHair::CoordinateSpace::Object );
		auto const averageTipChange = GetAverageVector<Vector3>( vertices1, vertices2, 1, 2 );
		REQUIRE_THAT( moveVector, IsNearEqual( averageTipChange ) );
	};

	GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh );
	GuidesDelta delta2;
	delta2.CopyChanges( delta, GuidesDelta::Changes::All );

	// ReSharper disable once CppVariableCanBeMadeConstexpr
	GuidesDelta::ApplyParameters const defaultParameters;

	// Apply delta with a different surface with a denser mesh
	SECTION( "DenserMesh" )
	{
		modifiedHair.CopyFrom( originalHair );
		delta.Apply( modifiedHair, &distributionMeshDenser, defaultParameters );
		checkHairChange( originalHair, modifiedHair );
	}

	SharedHair unmodifiedHair;

	// Try same but with copied delta
	SECTION( "DenserMeshCopiedDelta" )
	{
		GenerateRandomHairs( modifiedHair, 4, 1, &distributionMeshDenser );
		unmodifiedHair.CopyFrom( modifiedHair );
		delta2.Apply( modifiedHair, &distributionMeshDenser, defaultParameters );
		checkHairChange( unmodifiedHair, modifiedHair );
	}

	// Apply delta with a different surface with a sparser mesh
	SECTION( "SparserMeshCopiedDelta" )
	{
		GenerateRandomHairs( modifiedHair, 4, 1, &distributionMeshSparser );
		unmodifiedHair.CopyFrom( modifiedHair );
		delta2.Apply( modifiedHair, &distributionMeshSparser, defaultParameters );
		checkHairChange( unmodifiedHair, modifiedHair );
	}

	SECTION( "DenserMeshMoreHairs" )
	{
		GenerateRandomHairs( modifiedHair, 10, 1, &distributionMeshDenser );
		unmodifiedHair.CopyFrom( modifiedHair );
		delta2.Apply( modifiedHair, &distributionMeshDenser, defaultParameters );
		checkHairChange( unmodifiedHair, modifiedHair );
	}

	SECTION( "SparserMeshMoreHairs" )
	{
		GenerateRandomHairs( modifiedHair, 10, 1, &distributionMeshSparser );
		unmodifiedHair.CopyFrom( modifiedHair );
		delta2.Apply( modifiedHair, &distributionMeshSparser, defaultParameters );
		checkHairChange( unmodifiedHair, modifiedHair );
	}
}

vector<Vector3> GetTranslations( Span<Xform3 const> values )
{
	return Transform( values, Xform3::GetTranslation );
}

TEST_CASE( "GuidesDeltaCreatedRootsChangedInputTopology" )
{
	SharedHair originalHair, hair, modifiedHair;

	originalHair.SetUseStrandToObjectTransforms( true );
	modifiedHair.SetUseStrandToObjectTransforms( true );

	auto const distributionMesh = GeneratePlane<Real>( 1, 5, 5 );
	auto const distributionMeshDenser = GeneratePlane<Real>( 1, 8, 8 );
	auto const distributionMeshSparser = GeneratePlane<Real>( 1, 3, 3 );

	// Generate roots only, no vertices or strands. This will make GuidesDelta generate new strand roots.
	GenerateRandomHairs( hair, 4, 1, &distributionMesh, 0, 0 );
	hair.ValidateStrandToObjectTransforms( &distributionMesh );
	auto const createdRootPositions = GetTranslations( HairUtilities( hair ).GetStrandToObjectTransforms() );

	function<void( IHair const&, IHair const& )> const checkHairChange = [&createdRootPositions]( auto const& /*hair1*/, auto const& hair2 )
	{
		auto const changedRootPositions = GetTranslations( HairUtilities( hair2 ).GetStrandToObjectTransforms() );
		REQUIRE_THAT( createdRootPositions, IsVectorNearEqual( changedRootPositions ) );
	};

	GuidesDelta const delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh );
	GuidesDelta delta2;
	delta2.CopyChanges( delta, GuidesDelta::Changes::All );

	// ReSharper disable once CppVariableCanBeMadeConstexpr
	GuidesDelta::ApplyParameters const defaultParameters;

	// Apply delta with a different surface with a denser mesh
	SECTION( "DenserMesh" )
	{
		modifiedHair.CopyFrom( originalHair );
		delta.Apply( modifiedHair, &distributionMeshDenser, defaultParameters );
		modifiedHair.ValidateStrandToObjectTransforms( &distributionMeshDenser );
		checkHairChange( originalHair, modifiedHair );
	}

	SharedHair unmodifiedHair;

	// Try same but with copied delta
	SECTION( "DenserMeshCopiedDelta" )
	{
		unmodifiedHair.CopyFrom( modifiedHair );
		delta2.Apply( modifiedHair, &distributionMeshDenser, defaultParameters );
		modifiedHair.ValidateStrandToObjectTransforms( &distributionMeshDenser );
		checkHairChange( unmodifiedHair, modifiedHair );
	}

	// Apply delta with a different surface with a sparser mesh
	SECTION( "SparserMeshCopiedDelta" )
	{
		unmodifiedHair.CopyFrom( modifiedHair );
		delta2.Apply( modifiedHair, &distributionMeshSparser, defaultParameters );
		modifiedHair.ValidateStrandToObjectTransforms( &distributionMeshSparser );
		checkHairChange( unmodifiedHair, modifiedHair );
	}

	SECTION( "DenserMeshMoreHairs" )
	{
		unmodifiedHair.CopyFrom( modifiedHair );
		delta2.Apply( modifiedHair, &distributionMeshDenser, defaultParameters );
		modifiedHair.ValidateStrandToObjectTransforms( &distributionMeshDenser );
		checkHairChange( unmodifiedHair, modifiedHair );
	}

	SECTION( "SparserMeshMoreHairs" )
	{
		unmodifiedHair.CopyFrom( modifiedHair );
		delta2.Apply( modifiedHair, &distributionMeshSparser, defaultParameters );
		modifiedHair.ValidateStrandToObjectTransforms( &distributionMeshSparser );
		checkHairChange( unmodifiedHair, modifiedHair );
	}
}

TEST_CASE( "GuidesDeltaDiffWithSurfacePositionAndShapeChange" )
{
	// Note that continuous order is set to false on purpose, so that the transforms will be different between the rows of quads in the mesh
	auto distributionMesh = GeneratePlane<Real>( 1, 3, 3, true, Xform3::Identity(), false );

	// Randomize vertex positions so that rotation of the strand changes as it is moved
	PolygonMeshUtilities( distributionMesh ).RandomizeVertexPositions();

	SharedHair hair;
	GenerateHairs( hair, array<SurfacePosition, 1> { SurfacePosition{ 0u, Vector2f::Zero() } }, 1, & distributionMesh, 0, 2, -1, false, true );
	auto hair2 = hair;

	// Move position and change shape
	hair2.SetSurfaceDependency2( 0, { 2u, Vector2f::Zero() } );
	MoveTips( hair2, Vector3::XAxis() );

	// Make sure we have valid transforms
	hair.InvalidateStrandToObjectTransformsCache();
	hair2.InvalidateStrandToObjectTransformsCache();
	hair.ValidateStrandToObjectTransforms( &distributionMesh );
	hair2.ValidateStrandToObjectTransforms( &distributionMesh );

	REQUIRE_THAT( HairUtilities( hair2 ).GetStrandToObjectTransform( 0 ).rotation(), !IsNearEqual( HairUtilities( hair ).GetStrandToObjectTransform( 0 ).rotation() ) );

	GuidesDelta const delta( hair, hair2, ChangedHairComponents::All, ChangeTracking::Surface );
	delta.Apply( hair, &distributionMesh, {} );

	REQUIRE_THAT( HairUtilities( hair2 ).GetStrandToObjectTransform( 0 ), IsNearEqual( HairUtilities( hair ).GetStrandToObjectTransform( 0 ) ) );
	REQUIRE_THAT( hair2.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( hair.GetVertices( IHair::CoordinateSpace::Object ) ) );
}

TEST_CASE( "ComposeDeltaContainingShapeChangesWithHairWithShapeAndRootChanges" )
{
	auto const distributionMesh = GeneratePlane( 1_r );

	SharedHair hair;
	GenerateHairs( hair, array{ SurfacePosition{ 0u, Vector2f::Zero() }, SurfacePosition{ 0u, Vector2f::XAxis() } }, 1, &distributionMesh );

	// Just do a shape change for first delta
	auto originalHair = hair;
	auto hair2 = hair;
	MoveTips( hair2, Vector3::XAxis(), 0, 1 );

	GuidesDelta delta1( hair, hair2, ChangedHairComponents::All, ChangeTracking::Surface );

	// Move both root and tip to another location for second delta
	hair = hair2;
	MoveTips( hair2, Vector3::YAxis(), 0, 1 );
	hair2.SetSurfaceDependency2( 0, { 0u, Vector2f::XAxis() * 0.9_r } );
	hair2.InvalidateStrandToObjectTransformsCache();
	hair2.ValidateStrandToObjectTransforms( &distributionMesh );

	GuidesDelta const delta2( hair, hair2, ChangedHairComponents::All, ChangeTracking::Surface );

	delta1.Compose( delta2, &distributionMesh );

	delta1.Apply( originalHair, &distributionMesh, {} );

	// Delta1 should change original hair into final hair
	REQUIRE_THAT( hair2.GetVertices( IHair::CoordinateSpace::Object ), IsVectorNearEqual( originalHair.GetVertices( IHair::CoordinateSpace::Object ) ) );
}

TEST_CASE( "DeletingPreservesVisibilityOfCreatedStrands" )
{
	SharedHair const originalHair;
	SharedHair hair;

	auto const distributionMesh = GeneratePlane<Real>( 1, 2, 2 );

	// Generate roots only, no vertices or strands. This will make GuidesDelta generate new strand roots.
	GenerateVertexHairs( hair, 1, &distributionMesh, 0, 0, 0 );

	GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh );

	// Hide first guide
	auto hair2 = hair;
	HairUtilities( hair2 ).SetHiddenStrandIndices( { 0 } );
	delta.Compose( GuidesDelta( hair, hair2, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh ), &distributionMesh );

	// Delete last guide
	hair = hair2;
	auto const deletedStrand = hair2.GetStrandCount() - 2;
	hair2.DeleteStrandsByIndices( &deletedStrand, 1 );
	delta.Compose( GuidesDelta( hair, hair2, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh ), &distributionMesh );

	REQUIRE( hair2.GetStrandCount() == 3 );
	REQUIRE( hair2.GetHiddenStrandCount() == 1 );

	SharedHair finalHair;
	delta.Apply( finalHair, &distributionMesh, {} );

	// The first strand should still be hidden
	REQUIRE( finalHair.GetStrandCount() == 3 );
	REQUIRE( finalHair.GetHiddenStrandCount() == 1 );
}

TEST_CASE( "CreatedRootWithVertexTangentInterpolation" )
{
	auto const distributionMesh = GeneratePlane<Real>( 1, 2, 2 );

	SharedHair hair;
	GenerateHairs( hair, array{ SurfacePosition { 0, { 0.5f, 0.5f } } }, 1, &distributionMesh );
	MoveTipsRandomly( hair );

	hair.SetSurfaceTangentComputeMethod( SurfaceTangentComputeMethod::TextureCoordinate );
	hair.ValidateStrandToObjectTransforms( &distributionMesh );

	auto hairWithAddedRoot = hair;
	GenerateHairs( hairWithAddedRoot, array{ SurfacePosition { 0, { 0.50001f, 0.50001f } } }, 1, &distributionMesh, 0, 0, 0, false, false, true );
	GuidesDelta const delta( hair, hairWithAddedRoot, ChangedHairComponents::All, ChangeTracking::Surface, &distributionMesh );

	auto testedHair = hair;
	delta.Apply( testedHair, &distributionMesh, { 1_r, nullptr, {}, {}, 1_r, 2 } );
	REQUIRE( testedHair.ValidateStrandToObjectTransforms( &distributionMesh ) );

	REQUIRE_THAT( HairUtilities( testedHair ).GetStrandPoints( 0, IHair::CoordinateSpace::Object ),
				  IsVectorNearEqual( HairUtilities( testedHair ).GetStrandPoints( 1, IHair::CoordinateSpace::Object ), 0.0001_r ) );
}

TEST_CASE( "DontChangeWidthsWhenEmpty" )
{
	auto const distributionMesh = GeneratePlane<Real>( 1, 2, 2 );

	SharedHair hair;
	GenerateHairs( hair, array{ SurfacePosition {} }, 1, &distributionMesh, 0, 2, -1, false, true );

	vector const widths = { 0.067_r, 0.09_r };
	hair.SetWidths( 0, Size( widths ), widths.data() );

	GuidesDelta const delta;
	delta.Apply( hair, &distributionMesh, {} );

	REQUIRE( hair.GetWidthsVector() == widths );
}

TEST_CASE( "GuidesDelta With Width Changes" )
{
	SharedHair sharedHair;
	IHair& hair = sharedHair;
	SharedHair originalHair;

	auto const distributionMesh = GeneratePlane( 1_r );
	GenerateRandomHairs( hair, 4, 1, &distributionMesh, 0, 2, -1, false, true );
	originalHair.CopyFrom( sharedHair, true, true, true, true, true );
	GenerateRandomHairs( hair, 4, 1, &distributionMesh, 0, 2, -1, false, true, 2 );
	REQUIRE_THAT(
		hair.GetVertices( IHair::CoordinateSpace::Object ),
		!IsVectorNearEqual( originalHair.GetVertices( IHair::CoordinateSpace::Object ) )
	);

	GuidesDelta::ApplyParameters applyParameters;

	SECTION( "ApplyWidths" )
	{
		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Strand );

		// Make sure first changes were correctly captured
		SharedHair hairWithFirstEdit;
		hairWithFirstEdit.CopyFrom( originalHair, true, true, true, true, true );
		delta.Apply( hairWithFirstEdit, &distributionMesh, GuidesDelta::ApplyParameters() );

		REQUIRE_THAT(
			hair.GetVertices( IHair::CoordinateSpace::Object ),
			IsVectorNearEqual( hairWithFirstEdit.GetVertices( IHair::CoordinateSpace::Object ) )
		);
	}

	SECTION( "ApplyWidthsMR" )
	{
		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		// Make sure first changes were correctly captured
		SharedHair hairWithFirstEdit;
		hairWithFirstEdit.CopyFrom( originalHair, true, true, true, true, true );
		delta.Apply( hairWithFirstEdit, &distributionMesh, GuidesDelta::ApplyParameters() );

		REQUIRE_THAT(
			hair.GetVertices( IHair::CoordinateSpace::Object ),
			IsVectorNearEqual( hairWithFirstEdit.GetVertices( IHair::CoordinateSpace::Object ) )
		);
	}

	SECTION( "ComposeWithWidths" )
	{
		ComposeTest( hair, originalHair, distributionMesh, ChangeTracking::Strand );
	}

	SECTION( "ComposeWithWidthsMR" )
	{
		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		// Make sure first changes were correctly captured
		SharedHair hairWithFirstEdit;
		hairWithFirstEdit.CopyFrom( originalHair, true, true, true, true, true );
		delta.Apply( hairWithFirstEdit, &distributionMesh, GuidesDelta::ApplyParameters() );

		REQUIRE_THAT(
			hair.GetVertices( IHair::CoordinateSpace::Object ),
			IsVectorNearEqual( hairWithFirstEdit.GetVertices( IHair::CoordinateSpace::Object ) )
		);

		SharedHair hairWithSecondEdit;
		GenerateRandomHairs( hairWithSecondEdit, 4, 1, &distributionMesh, 0, 2, -1, false, true, 50 );

		GuidesDelta const delta2( hair, hairWithSecondEdit, ChangedHairComponents::All, ChangeTracking::Surface );
		delta.Compose( delta2, &distributionMesh );

		// Delta should now compose and have both changes
		delta.Apply( originalHair, &distributionMesh, GuidesDelta::ApplyParameters() );

		auto const verticesAfterEdit = hairWithSecondEdit.GetVertices( IHair::CoordinateSpace::Object );
		auto const verticesAfterEditGuidesApply = originalHair.GetVertices( IHair::CoordinateSpace::Object );

		REQUIRE_THAT(
			verticesAfterEdit,
			IsVectorNearEqual( verticesAfterEditGuidesApply )
		);
	}

	SECTION( "SerializeGuideChangesBinary" )
	{
		TestSerialize( originalHair, hair, distributionMesh, ChangeTracking::Strand, true );
	}

	SECTION( "SerializeGuideChangesArray" )
	{
		TestSerialize( originalHair, hair, distributionMesh, ChangeTracking::Strand, false );
	}

	SECTION( "SerializeMultiResolutionGuideChangesBinary" )
	{
		TestSerialize( originalHair, hair, distributionMesh, ChangeTracking::Surface, true );
	}

	SECTION( "SerializeMultiResolutionGuideChangesArray" )
	{
		TestSerialize( originalHair, hair, distributionMesh, ChangeTracking::Surface, false );
	}

	SECTION( "ApplyWidthChangesWithChangedInputTopology" )
	{
		GuidesDelta delta( originalHair, hair, ChangedHairComponents::All, ChangeTracking::Surface );

		// Make sure first changes were correctly captured
		SharedHair randomizedInputHair;
		GenerateRandomHairs( randomizedInputHair, 5, 1, &distributionMesh, 0, 2, -1, false, true, 3 );
		delta.Apply( randomizedInputHair, &distributionMesh, GuidesDelta::ApplyParameters() );

		for( auto i = 0; i < randomizedInputHair.GetStrandCount(); ++i )
		{
			auto widths = randomizedInputHair.GetStrandWidths( i );
			CHECK_FALSE( widths.empty() );
		}
	}
}

}
