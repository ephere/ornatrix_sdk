#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/MatrixUtilities.h"
#include "Ephere/Geometry/Native/SharedPolygonMesh.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Operators/HairFromMeshStripsGenerator.h"
#include "Ephere/Ornatrix/Private/CommonHair.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

TEST_CASE( "HairFromMeshStripsGenerator" )
{
	auto const planeMesh = GeneratePlane<Real>( 1, 2, 2, true );

	SharedHair hair;

	auto constexpr stripCount = 3;
	HairFromMeshStripsGenerator generator;
	generator.parameters.stripMeshes.mesh.Resize( stripCount );
	generator.parameters.stripMeshes.uniqueId.Resize( stripCount );
	for( auto stripIndex = 0; stripIndex < stripCount; ++stripIndex )
	{
		generator.parameters.stripMeshes.mesh.SetElement( stripIndex, { make_shared< SharedPolygonMesh>( planeMesh ), Xform3::Translation( Vector3::XAxis() * static_cast<Real>( stripIndex ) ) } );
		generator.parameters.stripMeshes.uniqueId.SetElement( stripIndex, stripIndex + 1 );
	}
	
	generator.parameters.outputHair = HairParameter{ make_shared<SharedHair>(), Xform3::Identity() };
	generator.parameters.hairsPerStrip = 5;

	SECTION( "RemoveFirstStripWithOverrides" )
	{
		// Needed by the Override*() methods
		REQUIRE( generator.ValidateBoundaryVertices() );

		// Override per-strip settings for last two strips
		for( auto stripIndex = 1; stripIndex < stripCount; ++stripIndex )
		{
			generator.SelectStrips( { stripIndex } );
			generator.OverridePerStripDistributionMethod( static_cast<HairFromMeshStripsParameters::DistributionMethodType>( stripIndex ) );
			generator.OverridePerStripHairCount( stripIndex + 2 );
		}

		REQUIRE( generator.Validate() );
		REQUIRE( generator.parameters.outputHair()->GetStrandCount() == 5 + 3 + 4 );

		// Remove first strip
		generator.RemoveStrip( 0 );
		
		REQUIRE( generator.Validate() );
		auto const resultHair = generator.parameters.outputHair();
		REQUIRE( resultHair->GetStrandCount() == 3 + 4 );
	}

	SECTION( "RemoveAllStrips" )
	{
		REQUIRE( generator.Validate() );

		for( auto stripIndex = stripCount - 1; stripIndex >= 0; --stripIndex )
		{
			generator.RemoveStrip( 0 );
		}

		REQUIRE( generator.Validate() );
		REQUIRE( generator.parameters.outputHair()->GetStrandCount() == 0 );
	}
}
}
