#include "PrecompiledHeaders.h"

#include "Ephere/Ornatrix/Operators/GuidesEditor.h"
#include "Ephere/Ornatrix/Private/HairUtilities.h"

#include "Utilities.h"

namespace Ephere::Ornatrix
{
using namespace std;
using namespace Geometry;

TEST_CASE( "GuidesEditorPlantGuides" )
{
	GuidesEditorImplementation guidesEditor( false );
	guidesEditor.parameters.distributionMesh = make_shared<SharedPolygonMesh>( GeneratePlane( 1_r ) );

	vector<Vector3> previousVertices;
	for( auto plantIndex = 0; plantIndex < 3; ++plantIndex )
	{
		if( !guidesEditor.parameters.targetHair().IsEmpty() )
		{
			guidesEditor.GetGuidesInternal().CopyFrom( *guidesEditor.parameters.targetHair() );
		}

		// Call BeginTrackingChanges to make sure guides we just initialized get copied into "initial" guides internally
		guidesEditor.PlantGuide( { 0, { 0.5f, static_cast<float>( plantIndex ) * 0.1f } }, guidesEditor.parameters.distributionMesh().Get(), true, 1_r, 2 );
		guidesEditor.EndTrackingChanges();

		guidesEditor.parameters.targetHair = make_shared<SharedHair>();
		guidesEditor.Apply();
		
		const_cast<IHair3*>( guidesEditor.parameters.targetHair.Edit()->Get() )->ValidateStrandToObjectTransforms( guidesEditor.parameters.distributionMesh().Get() );
		auto const vertices = guidesEditor.parameters.targetHair()->GetStrandPoints( plantIndex, IHair::CoordinateSpace::Object );

		REQUIRE( guidesEditor.parameters.targetHair()->GetStrandCount() == plantIndex + 1 );
		if( !previousVertices.empty() )
		{
			REQUIRE_THAT( previousVertices, !IsVectorNearEqualf( vertices ) );
		}

		previousVertices = vertices;
	}
}
	
}
