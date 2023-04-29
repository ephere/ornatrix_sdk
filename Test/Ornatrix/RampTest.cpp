#include "PrecompiledHeaders.h"

#include "Utilities.h"
#include "Ephere/Geometry/Native/Test/Utilities.h"
#include "Ephere/Ornatrix/Ramp.h"

using namespace Ephere;
using namespace Ornatrix;
using namespace Geometry;
using namespace std;

TEST_CASE( "Ramp" )
{
	SECTION( "StringConstuctor" )
	{
		Ramp const ramp( "Sp 0 1 Sp 1 1" );
		REQUIRE( ramp.GetKnots().size() == 2 );
		REQUIRE( ramp.GetKnots()[0].index == 0 );
		REQUIRE( ramp.GetKnots()[0].position == 0 );
		REQUIRE( ramp.GetKnots()[0].value == 1 );
		REQUIRE( ramp.GetKnots()[0].type == Interpolation::Spline );
		REQUIRE( ramp.GetKnots()[1].index == 1 );
		REQUIRE( ramp.GetKnots()[1].position == 1 );
		REQUIRE( ramp.GetKnots()[1].value == 1 );
		REQUIRE( ramp.GetKnots()[1].type == Interpolation::Spline );
	}

	SECTION( "Ramp" )
	{
		REQUIRE( Ramp( "" ) == Ramp() );
		REQUIRE( Ramp( "0" ) == Ramp() );
		REQUIRE( Ramp( "0 1" ) == Ramp{ { 0, 1 } } );
		REQUIRE( Ramp( "0 0 1 1" ) == Ramp{ { 0, 0 }, { 1, 1 } } );
		REQUIRE(
			Ramp( "St 0 1 Li 0.2 1 Sm 0.4 1 Sp 1 1" ).IsNearEqual(
				Ramp{ { 0, 1, Interpolation::Step }, { 0.2_r, 1, Interpolation::Linear }, { 0.4_r, 1, Interpolation::Smooth }, { 1, 1, Interpolation::Spline } } ) );
		REQUIRE( Ramp().ToString().empty() );
		REQUIRE(
			Ramp{ { 0, 1, Interpolation::Step }, { 0.2_r, 1, Interpolation::Linear }, { 0.4_r, 1, Interpolation::Smooth }, { 1, 1, Interpolation::Spline } }.ToString()
			== "St 0 1 Li 0.2 1 Sm 0.4 1 Sp 1 1" );

		auto data = Ramp( 1 ).ToFloatVector();
		REQUIRE( data == std::vector<Real>{ Ramp::CountToken + 3, 3, 0, 1, 3, 0.6_r, 1, 3, 1, 1 } );

		REQUIRE( Ramp( std::vector<Real>{ 0, 0, 1, 1 } ) == Ramp{ { -0.5, 0 }, { 0, 0 }, { 1, 1 }, { 1.5, 1 } } );
		REQUIRE( Ramp( std::vector<Real>{ Ramp::CountToken + 2, 3, 0, 0, 0, 1, 1 } ) == Ramp{ { 0, 0 }, { 1, 1, Interpolation::Step } } );

		data = Ramp::ToFloatVector( std::vector{ Ramp( 1 ), Ramp{ { 0, 0 }, { 1, 1 } } } );
		REQUIRE( data.size() == 17 );
		auto ramps = Ramp::FromFloatVector( data );
		REQUIRE( ramps == std::vector{ Ramp( 1 ), Ramp{ { 0, 0 }, { 1, 1 } } } );
	}
}
