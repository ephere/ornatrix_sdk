#include "main.h"
#include "Ephere/Plugins/Ornatrix/HairInterfacesC4D.h"

// Development plugin id -- Not for production
// Replace with your own, More info here:
// https://developers.maxon.net/?page_id=3224
#define ID_OX_SDK_EXAMPLE	1000010

//
// Run from console with an Ornatrix Hair Object selected:
//
// # Print strand count
// c4d.CallCommand(1000010)
//
// # Create spline object
// c4d.CallCommand(1000010,1)
//
// # Create polygon object
// c4d.CallCommand(1000010,2)
//

//------------------------------------------------

// C4D R20 compatibility
#if (API_VERSION >= 20000)
#	define UNDOTYPE_NEW (UNDOTYPE::NEWOBJ)
#	define SPLINETYPE_LINEAR (SPLINETYPE::LINEAR)
#	define ROTATIONORDER_XYZGLOBAL (ROTATIONORDER::XYZGLOBAL)
#	define COPYFLAGS_0 (COPYFLAGS::NONE)
#endif

// C4D R21 compatibility
#if API_VERSION >= 21000
#	define Execute_Signature Execute( BaseDocument* doc, GeDialog* parentManager )
#	define ExecuteSubID_Signature ExecuteSubID( BaseDocument* doc, Int32 subId, GeDialog* )
#else
#	define Execute_Signature Execute( BaseDocument* doc )
#	define ExecuteSubID_Signature ExecuteSubID( BaseDocument* doc, Int32 subId )
#endif

using namespace Ephere::Ornatrix;
using namespace Ephere::Plugins::Ornatrix;

class OrnatrixSdkExampleCommand : public CommandData
{
	INSTANCEOF( OrnatrixSdkExampleCommand, CommandData )
	
public:
	
	static CommandData *Alloc()
	{
		return NewObjClear( OrnatrixSdkExampleCommand );
	}
	
	Bool Execute_Signature override;
	Bool ExecuteSubID_Signature override;
	
private:
	
	SplineObject* GenerateSingleSplineFromHair( IHair* sourceHair );
	
	PolygonObject* ExtractPolygonFromHairObject( BaseObject* hairObject );
	
};

Bool OrnatrixSdkExampleCommand::Execute_Signature
{
#if API_VERSION >= 21000
	return ExecuteSubID( doc, 0, parentManager );
#else
	return ExecuteSubID( doc, 0 );
#endif
}

Bool OrnatrixSdkExampleCommand::ExecuteSubID_Signature
{
	// Get selected object
	auto baseObject = doc->GetActiveObject();
	
	// Retrieve IHair
	auto hairInterface = GetHairInterface( baseObject );
	if( hairInterface.first == nullptr )
	{
		GePrint( String( hairInterface.second.c_str() ) );
		return false;
	}
	
	std::shared_ptr<IHair>& sourceHair = hairInterface.first;
	
	// no sub-commmand or sub-command 0
	// Print strand count
	auto strandCount = Int32( sourceHair->GetStrandCount() );
	GePrint( "Strand Count: " + String::IntToString( strandCount ) );
	
	BaseObject* newObject = nullptr;
	
	// sub-command 1
	// Create spline object with hair strand
	if( subId == 1 )
	{
		auto spline = GenerateSingleSplineFromHair( sourceHair.get() );
		if( spline != nullptr )
		{
			spline->SetName( baseObject->GetName() + String(" as Spline") );
			GePrint( String( "Created Spline: " ) + spline->GetName() );
			newObject = static_cast<BaseObject*>( spline );
		}
	}
	// sub-command 2
	// Extract polygon from hair object
	else if( subId == 2 )
	{
		auto polygon = ExtractPolygonFromHairObject( baseObject );
		if( polygon != nullptr )
		{
			polygon->SetName( baseObject->GetName() + String(" as Polygon") );
			GePrint( String( "Created Polygon: " ) + polygon->GetName() );
			newObject = static_cast<BaseObject*>( polygon );
		}
		else
		{
			GePrint( String( "Ornatix Hair object is not generating polygon. Add a Mesh From Strands modifier." ) );
		}
	}
	else if( subId > 2 )
	{
		GePrint( String( "Unknown sub-command" ) );
	}
	
	if( newObject != nullptr )
	{
		auto transform = HPBToMatrix( MatrixToHPB( baseObject->GetMg(), ROTATIONORDER_XYZGLOBAL ), ROTATIONORDER_XYZGLOBAL );
		transform.off = baseObject->GetMg().off;
		newObject->SetMg( transform );
		
		doc->InsertObject( newObject, nullptr, nullptr );
		
		doc->StartUndo();
		doc->AddUndo( UNDOTYPE_NEW, newObject );
		doc->EndUndo();
	}
	
	// Retrieve Distribution Mesh
	auto meshInterface = GetDistributionMeshInterface( baseObject );
	if( meshInterface.first != nullptr )
	{
		std::shared_ptr<IPolygonMeshSA>& mesh = meshInterface.first;
		const auto faceCount = mesh->GetPolygonCount();
		const auto triangleCount = mesh->GetTriangleCount();
		GePrint( "Distribution mesh has " + String::IntToString( faceCount ) + " polys, " + String::IntToString( triangleCount ) + " triangles" );
	}
	else
	{
		GePrint( String( hairInterface.second.c_str() ) );
	}
	
	return true;
}

SplineObject* OrnatrixSdkExampleCommand::GenerateSingleSplineFromHair( IHair* sourceHair )
{
	const auto strandCount = unsigned( sourceHair->GetStrandCount() );
	const auto vertexCount = unsigned( sourceHair->GetVertexCount() );
	if( strandCount == 0 || vertexCount == 0 )
	{
		return nullptr;
	}
	
	auto spline = SplineObject::Alloc( 0, SPLINETYPE_LINEAR );
	if( spline == nullptr )
	{
		return nullptr;
	}
	
	spline->ResizeObject( Int32( vertexCount ), Int32( strandCount ) );
	
	auto pointArray = spline->GetPointW();
	auto segmentArray = spline->GetSegmentW();
	if( pointArray == nullptr || segmentArray == nullptr )
	{
		return nullptr;
	}
	
	auto vertexIndex = 0;
	
	for( auto strandIndex = 0u ; strandIndex < strandCount ; ++strandIndex )
	{
		const auto strandPointCount = unsigned( sourceHair->GetStrandPointCount( strandIndex ) );
		
		segmentArray[strandIndex].closed = false;
		segmentArray[strandIndex].cnt = Int32( strandPointCount );
		
		auto strandPoints = sourceHair->GetStrandPoints( strandIndex, IHair::CoordinateSpace::Object );
		
		for( auto strandPointIndex = 0u ; strandPointIndex < strandPointCount && strandPointIndex < strandPoints.size() ; strandPointIndex++ )
		{
			auto point = strandPoints[ strandPointIndex ];
			pointArray[vertexIndex++] = ::Vector64( point.x(), point.y(), point.z() );
		}
	}
	
	return spline;
}

PolygonObject* OrnatrixSdkExampleCommand::ExtractPolygonFromHairObject( BaseObject* hairObject )
{
	auto cache = hairObject->GetCache( nullptr );
	if( cache == nullptr || !cache->IsInstanceOf( Opolygon ) )
	{
		return nullptr;
	}
	
	auto deformCache = cache->GetDeformCache();
	if( deformCache == nullptr || !deformCache->IsInstanceOf( Opolygon ) )
	{
		return nullptr;
	}
	
	auto polygon = PolygonObject::Alloc( 0, 0 );
	if ( polygon != nullptr )
	{
		deformCache->CopyTo( polygon, COPYFLAGS_0, nullptr );
	}
	
	return polygon;
}

Bool RegisterOrnatrixOrnatrixSdkExampleCommand()
{
	Int32 flags = PLUGINFLAG_HIDEPLUGINMENU;
	return RegisterCommandPlugin( ID_OX_SDK_EXAMPLE, String("Ornatrix Sdk Example"), flags, nullptr, String("Call this command with an Ornatrix Hair Object selected"), OrnatrixSdkExampleCommand::Alloc() );
}

//------------------------------------------------
