#include "PrecompiledHeaders.h"

#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/GuidesFromMeshGenerator.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

TEST_CASE( "GuidesFromMeshGenerator" )
{
	auto const distributionMesh = GeneratePlane<Real>( 1, 2, 2, true );

	SharedHair sharedHair, sharedCacheHair;
	IHair& hair = sharedHair;

	GuidesFromMeshGenerator generator;
	generator.rootParameters.distributionMesh = &distributionMesh;
	generator.parameters.outputGuides = &hair;
	generator.parameters.pointCount = 2;

	SECTION( "StrandIdsEnabledWhen0OutputGuides" )
	{
		generator.rootParameters.rootCount = 0;
		generator.GenRoots();
		generator.UpdateState();
		REQUIRE( hair.HasStrandIds() );
	}
}
}
