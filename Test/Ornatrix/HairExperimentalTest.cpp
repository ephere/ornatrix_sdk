#include "PrecompiledHeaders.h"

#include "Ephere/Ornatrix/Private/HairUtilities.h"
#include "Ephere/Ornatrix/Private/Experimental/Hair.h"
#include "Ephere/Ornatrix/Private/Experimental/HairImpl.h"

#include "Ephere/Geometry/Native/Test/Utilities.h"

using namespace std;
using namespace Ephere::Geometry;
using namespace Ephere::Ornatrix;

TEST_CASE( "HairExperimental" )
{
	using namespace Experimental;

	SECTION( "Empty" )
	{
		HairBuilder builder;
		REQUIRE( builder.IsEmpty() );

		auto hair = builder.CreateHair();
		REQUIRE( hair.IsEmpty() );
	}

	SECTION( "Builder" )
	{
		Hair hair;

		{
			HairBuilder builder;
			builder.AddStrand(
				{ { 0, 0, 0 }, { 0, 0, 1 }, { 0, 0, 2 } }
			);

			hair = builder.CreateHair();
			REQUIRE( builder.IsEmpty() );
		}

		REQUIRE( hair.GetStrandCount() == 1 );
		REQUIRE_THAT(
			hair[0].GetPositions(),
			Catch::Equals<Vector3f>( { { 0, 0, 0 },{ 0, 0, 1 },{ 0, 0, 2 } } )
		);

		HairBuilder builder{ hair, { ChannelContent::Position } };
		builder[0].SetPositions( vector<Vector3>{
			{
				1, 0, 0
			}, { 1, 0, 1 } } );
		builder[0].SetPositions( vector<Vector3>{
			{
				2, 0, 2
			} }, 2 );
		auto hair2 = builder.CreateHair();
		REQUIRE_THAT(
			hair2[0].GetPositions(),
			Catch::Equals<Vector3f>( { { 1, 0, 0 },{ 1, 0, 1 },{ 2, 0, 2 } } )
		);
		REQUIRE_THAT(
			hair[0].GetPositions(),
			Catch::Equals<Vector3f>( { { 0, 0, 0 },{ 0, 0, 1 },{ 0, 0, 2 } } )
		);
	}

	SECTION( "Channel" )
	{
		REQUIRE( Channel{}.IsEmpty() );
	}

	SECTION( "HairImpl" )
	{
		HairImpl hair
		{
			{
				{ { 0, 0, 0 }, { 0, 0, 1 }, { 0, 0, 2 }, { 0, 0, 3 }, { 0, 0, 4 }, { 0, 0, 5 } },
				{ { 0, 10, 0 },{ 0, 10, 1 },{ 0, 10, 2 },{ 0, 10, 3 },{ 0, 10, 4 } },
			}
		};
		hair.SetStrandPoint( 0, 0, { 1, 1, 1 }, IHair::Object );
		REQUIRE( HairUtilities( hair ).GetStrandPoint( 0, 0, IHair::Object ) == Vector3f{ 1, 1, 1 } );
	}
}
