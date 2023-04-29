#include "Ephere/Geometry/Native/IPolygonMesh.h"
#include "Ephere/Ornatrix/IHair.h"
#include "Ephere/Ornatrix/Ornatrix.h"
#include "Ephere/Ornatrix/Ramp.h"
#include "Ephere/Ornatrix/Operators/RootGeneratorParameters.g.h"

#include <iostream>

#define TEST( condition ) \
	if( condition ) {} else { std::cout << "Test failed in " << __FILE__ << '(' << __LINE__ << "), message: " << #condition << '\n'; std::exit( 1 ); } void(0)

using namespace Ephere;
using namespace Ephere::Ornatrix;

int main()
{
	TEST( Ramp( 1 ).Evaluate( 0.5f ) == 1 );


	auto logger = []( Log::Level level, char const* message )
	{
		std::cout << message << '\n';
	};

	std::string loadError;
	auto const ornatrixLibrary = LoadOrnatrixLibrary( logger, nullptr, "", &loadError );
	if( ornatrixLibrary.IsEmpty() )
	{
		std::cout << "Failed to load Ornatrix shared library: " << loadError << '\n';
		return 1;
	}

	auto const groom = ornatrixLibrary.grooms->DeserializeGroomFromFile( "SampleGroom.oxg.yaml" );
	TEST( groom );

	{
		auto const hairAndMesh = ornatrixLibrary.grooms->EvaluateGroom( *groom );
		TEST( hairAndMesh.first );
		TEST( hairAndMesh.second );

		TEST( hairAndMesh.first->GetStrandCount() == 30 );
		TEST( hairAndMesh.second->GetVertexCount() == 25 );
	}

	{
		auto const guidesFromMeshNode = groom->FindNode( "guidesFromMesh" );
		TEST( guidesFromMeshNode );

		auto const rootGenParams = guidesFromMeshNode->GetOperator().GetParameterSet( RootGeneratorParameters::GetName() );
		TEST( rootGenParams );
		rootGenParams->Set<RootGeneratorParameters::RootCount>( 50 );

		auto hair = ornatrixLibrary.grooms->EvaluateGroom( *groom ).first;
		TEST( hair );
		TEST( hair->GetStrandCount() == 50 );
	}

	std::cout << "All tests passed\n";
	return 0;
}
