#include "PrecompiledHeaders.h"

#include "Ephere/Geometry/Native/MeshFromPointCloud.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/NativeTools/Profile/CallGraphProfiler.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

TEST_CASE( "MeshFromPointCloud" )
{
	static stringstream profilerLog;
	auto& profiler = Profile::GetProfilerInstance();
	profiler.SetLogger( []( string const& message )
	{
		profilerLog << message;
	} );

	profiler.Reset();
	REQUIRE( profiler.GetEnabledTime() == 0 );

	SECTION( "NearestNeighbourMinSizeFaces" )
	{
		auto const pointCount = 1000;
		vector<Vector3> randomPoints( pointCount ), normals( pointCount, Vector3::ZAxis() );
		//GenerateRandom<3, 1>( begin( randomPoints ), end( randomPoints ) );

		//const auto creationMethod = MeshFromPointCloud::CreationMethod::NearestNeighbourMinSizeFaces;

		//profiler.SetEnabled( true );
		//for( auto i = 0; i < 10; ++i )
		//{
		//	const auto meshFromPointCloud = MeshFromPointCloud::TryCreate( MakeSpan( randomPoints ), normals, creationMethod );
		//}
		//
		//profiler.SetEnabled( false );

		////INFO( profiler.Dump().c_str() );
		//REQUIRE( Profile::GetProfilerInstance().SaveToFiles( "ProfileResult", R"(D:\Program Files (x86)\Graphviz2.38\bin\dot.exe)" ) );
		//REQUIRE( profiler.GetProfiledTime() < 200000 );
	}

	SECTION( "FindClosest3Vertices" )
	{
		vector<Vector3> vertices = { {-0.289461493f, 0.0f, 0.404039383f }, { -28.2978783f, 0, 0.0663372055f }, {28.3920174f, 0, 0.151690736f } };
		auto const meshFromPointCloud = MeshFromPointCloud<Real>::TryCreate( MakeSpan( vertices ), vector( 3, Vector3::YAxis() ), 
			MeshFromPointCloud<Real>::CreationMethod::NearestNeighbourMinSizeFaces );

		vector<pair<Vector3, MeshFromPointCloud<Real>::FaceInfo>> const expectedResults =
		{
			{ Vector3( 27.7300816f, 0, 26.5752716f ), MeshFromPointCloud<Real>::FaceInfo { {0, 1, 2 }, { 0.0f, 0.0f, 1.0f } } },
			{ Vector3( -0.289461493f, 0.0f, 0.404039383f ), MeshFromPointCloud<Real>::FaceInfo { {0, 1, 2 }, { 1.0f, 0.0f, 0.0f } } },
			{ Vector3( -28.2978783f, 0, 0.0663372055f ), MeshFromPointCloud<Real>::FaceInfo { {0, 1, 2 }, { 0.0f, 1.0f, 0.0f } } },
			// ReSharper disable once CppRedundantCastExpression
			{ vertices[0] * 0.5_r +
			// ReSharper disable once CppRedundantCastExpression
				vertices[1] * 0.5_r, MeshFromPointCloud<Real>::FaceInfo { {0, 1, 2 }, { 0.5f, 0.5f, 0.0f } } }
		};

		for( auto const& expectedResult: expectedResults )
		{
			auto const nearestFaceInfo = meshFromPointCloud->GetNearestFaceInfo( expectedResult.first );
			REQUIRE( nearestFaceInfo.pointIndices == expectedResult.second.pointIndices );
			REQUIRE_THAT( nearestFaceInfo.barycentricCoordinate, NearEqualMatcher( expectedResult.second.barycentricCoordinate, 0.05_r ) );
		}
	}
}
}
