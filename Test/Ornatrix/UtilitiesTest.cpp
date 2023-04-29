#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Ornatrix/Private/GuideControlPointsOptimizer.h"
#include "Ephere/Ornatrix/Private/Utilities.h"

#include "Ephere/Geometry/Native/PolygonMeshUtilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/NativeTools/Test/Catch.h"
#include "Ephere/Ornatrix/Groom/Evaluator.h"
#include "Ephere/Ornatrix/Groom/Graph.h"
#include "Ephere/Ornatrix/Groom/Operators.h"
#include "Ephere/Ornatrix/Operators/BakedHairParametersImpl.g.h"
#include "Ephere/Ornatrix/Operators/FilterParametersImpl.g.h"
#include "Ephere/Ornatrix/Private/AlembicExport.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"
#include "Geometry/Native/RayTracing/Ray.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

TEST_CASE( "HasOverlappingValues" )
{
	REQUIRE( HasOverlappingValues( EncodeMultiStrandGroup( array{ 1, 2 } ), 1 ) );
	REQUIRE( !HasOverlappingValues( EncodeMultiStrandGroup( array{ 1, 2 } ), 3 ) );
	REQUIRE( !HasOverlappingValues( 1, 3 ) );
	REQUIRE( HasOverlappingValues( 3, 3 ) );
	REQUIRE( HasOverlappingValues( EncodeMultiStrandGroup( array{ 1, 2 } ), EncodeMultiStrandGroup( array{ 2, 3 } ) ) );
	REQUIRE( !HasOverlappingValues( EncodeMultiStrandGroup( array{ 1, 2 } ), EncodeMultiStrandGroup( array{ 3, 4 } ) ) );
}

TEST_CASE( "CurveManipulator" )
{
	GuideGeometryOptimizer::PreserveLengthsContext context;

	SECTION( "PreserveTotalLength_DuplicatePoints" )
	{
		array originalPoints = { Vector3::Zero(), Vector3::ZAxis() / 2_r, Vector3::XAxis() },
			targetPoints = { Vector3::Zero(), Vector3::XAxis(), Vector3::XAxis() };

		GuideGeometryOptimizer::PreserveTotalLength( originalPoints, targetPoints, false );

		REQUIRE( NearEqual( targetPoints[2], Vector3::XAxis() ) );
	}
}

TEST_CASE( "HairUtilities" )
{
	SharedHair hair;
	HairUtilities const hairUtilities( hair );

	SECTION( "GetStrandChannels" )
	{
		hair.SetStrandChannelCount( IHair::StrandDataType::PerStrand, 4 );

		vector<StrandChannelName> const channelNames = { { L"Selection" }, { L"Color_R" }, { L"Color_G" }, { L"Color_B" } };
		hair.SetStrandChannelNames( IHair::StrandDataType::PerStrand, 0, Size( channelNames ), channelNames.data() );

		auto const channels = hairUtilities.GetStrandChannels( IHair::StrandDataType::PerStrand );
		REQUIRE( channels.size() == 2 );
		REQUIRE( channels[0].first == wstring( channelNames[0].name ) );
		REQUIRE( channels[0].second == HairUtilities::StrandChannelVectorSuffix::None );
		REQUIRE( wstring( channels[1].first ) == wstring( L"Color" ) );
		REQUIRE( channels[1].second == HairUtilities::StrandChannelVectorSuffix::RGB );
	}

	SECTION( "DeleteStrandChannels" )
	{
		REQUIRE( hairUtilities.AddStrandChannel( IHair::StrandDataType::PerStrand, L"Channel1", HairUtilities::StrandChannelVectorSuffix::None ) == 0 );
		REQUIRE( hairUtilities.AddStrandChannel( IHair::StrandDataType::PerStrand, L"Channel2", HairUtilities::StrandChannelVectorSuffix::RGB ) == 1 );
		REQUIRE( hairUtilities.AddStrandChannel( IHair::StrandDataType::PerStrand, L"Channel3", HairUtilities::StrandChannelVectorSuffix::UV ) == 4 );

		REQUIRE( hair.GetStrandChannelCount( IHair::StrandDataType::PerStrand ) == 6 );
		REQUIRE( hairUtilities.DeleteStrandChannel( IHair::StrandDataType::PerStrand, 1, true ) );
		REQUIRE( hair.GetStrandChannelCount( IHair::StrandDataType::PerStrand ) == 3 );
		REQUIRE( hairUtilities.DeleteStrandChannel( IHair::StrandDataType::PerStrand, 0, true ) );
		REQUIRE( hair.GetStrandChannelCount( IHair::StrandDataType::PerStrand ) == 2 );
		REQUIRE( hairUtilities.DeleteStrandChannel( IHair::StrandDataType::PerStrand, 0, true ) );
		REQUIRE( hair.GetStrandChannelCount( IHair::StrandDataType::PerStrand ) == 0 );
	}
}

TEST_CASE( "PolygonMeshUtilities" )
{
	SharedPolygonMesh mesh;
	PolygonMeshUtilities const meshUtilities( mesh );

	SECTION( "HitTest Ray-Ngon" )
	{
		meshUtilities.GenerateCircleNgon( 5 );

		UniformRandomGenerator generator{ RandomGeneratorContext{ ProjectId::Geometry, static_cast<AlgorithmId>( 0 ), 0, 1 } };

		auto constexpr sampleCount = 5;
		for( auto sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex )
		{
			auto const radialPosition = generator.GetNextFloatValue<Real>() * Pir;
			auto const internalScaling = generator.GetNextFloatValue<Real>() * 0.9_r;

			Vector3 const samplePointInside( cos( radialPosition ) * internalScaling, sin( radialPosition ) * internalScaling, 0_r );

			SurfacePosition resultPosition;

			auto const origin = samplePointInside - Vector3::ZAxis();
			auto const dir = Vector3::ZAxis();
			ERTRayHit rayHit{ { origin.x(), origin.y(), origin.z() }, { dir.x(), dir.y(), dir.z() } };
			REQUIRE( meshUtilities.HitTest( rayHit, resultPosition ) );

			// Now make sure that the returned position results in the same cartesian coordinate
			REQUIRE_THAT( meshUtilities.GetSurfacePosition( resultPosition ), IsNearEqual( samplePointInside ) );
		}
	}
}

// This test can be used to convert an .abc file into one with fewer strands. It only works for a single frame though.
// Replace "ThreeCurves.abc" with the absolute path to your .abc here (like "C:\\hair issue\\casey_main.abc") and get the resulting "Filtered.abc" from bin/Debug
TEST_CASE( "ConvertAlembicWithFilter" )
{
	using namespace Groom;
	static char constexpr FilterNodeName[] = "filter";

	Graph groom
	{
		{ BakedHairNodeName, Operators::BakedHair },
		{ FilterNodeName, Operators::Filter },
	};

	groom.AutoConnect( { BakedHairNodeName, FilterNodeName } );

	auto& bakeParameters = *groom[BakedHairNodeName].GetOperator().GetParameterSet<BakedHairParameters>();
	// Put the absolute path to your .abc here, like "C:\\hair issue\\casey_main.abc"
	bakeParameters.sourceFilePaths = { "ThreeCurves.abc" };

	auto& filterParameters = *groom[FilterNodeName].GetOperator().GetParameterSet<FilterParameters>();

	// Remove all but each 10th strand
	filterParameters.filteredPattern = "^0:-1:10";
	filterParameters.predicate = FilterParameters::PredicateType::ByIndex;

	REQUIRE( Evaluator{ groom }.Evaluate( nullptr, 1 ) );
	auto const resultHair = GetEndPointHair( groom );
	REQUIRE( resultHair.IsModifiable() );

	auto& hair = *resultHair.Get();
	AlembicExporter exporter;

	AlembicExporter::AlembicNode node;
	node.name = L"Test";

	exporter.SetEvaluator( [&hair]( void* /*hostNode*/, double const /*time*/, float const /*completedFraction*/, bool& /*wasCancelled*/ )
	{
		AlembicExporter::AlembicNodeState result = {};
		result.hair = &hair;
		result.xform = Xform3::Identity();
		return result;
	} );

	exporter.filePath = L"Filtered.abc";
	exporter.Export( vector{ node }, vector{ 0.0 } );

	SharedHair loadedHair;
	REQUIRE( loadedHair.LoadFromFile( exporter.filePath.c_str(), HairBaseSA::HairComponentSelection{} ) );
	REQUIRE( loadedHair.GetStrandCount() == 1 );

	FileRemove( exporter.filePath );
}

}
