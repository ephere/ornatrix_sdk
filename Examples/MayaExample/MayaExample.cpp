// Example usage of the Ornatrix core interfaces

// Include Ornatrix interfaces and constructs
#include "Ephere/Plugins/Ornatrix/HairInterfacesMaya.h"

#include "ScaleHairOperatorNode.h"

#pragma warning( push, 2 )
#include <maya/MArgList.h>
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <maya/MPxCommand.h>
#include <maya/MSelectionList.h>
#pragma warning( pop )

#pragma comment( lib, "Foundation.lib" )
#pragma comment( lib, "OpenMaya.lib" )

using namespace Ephere::Plugins::Ornatrix;
using namespace std;

class OxPrintStrandCount : public MPxCommand
{
public:
	static MString GetName()
	{
		return "OxPrintStrandCount";
	}

	static void* Create()
	{
		return new OxPrintStrandCount();
	}

	MStatus	doIt( MArgList const& args ) override
	{
		MSelectionList list;

		if( args.length() > 0 )
		{
			MString argStr;
			args.get( 0, argStr );
			list.add( argStr );
		}
		else
		{
			MGlobal::getActiveSelectionList( list );
		}

		if( list.length() > 0 )
		{
			MObject object;
			list.getDependNode( 0, object );

			auto const sourceHair = GetHairInterface( object );

			shared_ptr<IHair__deprecated> sourceHair_deprecated;

			if( sourceHair != nullptr )
			{
				if( sourceHair != nullptr )
				{
					auto const count = sourceHair->GetStrandCount();
					MGlobal::displayInfo( MString( "Ornatrix hair interface found, strand count: " ) + count );
					setResult( count );
				}
			}
			else
			{
				MGlobal::displayInfo( MString( "No Ornatrix hair interface found" ) );
				return MS::kInvalidParameter;
			}
		}
		else
		{
			MGlobal::displayError( "No arguments specified" );
			return MS::kInvalidParameter;
		}

		return MS::kSuccess;
	}
};


Version GetOrnatrixPluginVersion()
{
	static Version version = []
	{
		auto obj = MFnPlugin::findPlugin( "Ornatrix" );
		if( obj.isNull() )
		{
			Version v = {};
			return v;
		}

		MFnPlugin plugin( obj );
		auto versionString = plugin.version().asChar();
		stringstream stream( versionString );
		string major, minor, fix, revision;
		if( getline( stream, major, '.' )
			&& getline( stream, minor, '.' )
			&& getline( stream, fix, '.' )
			&& getline( stream, revision, '.' ) )
		{
			Version v = { stoi( major ), stoi( minor ), stoi( fix ), stoi( revision ), versionString };
			return v;
		}

		Version v = { 0, 0, 0, 0, versionString };
		return v;
	}( );

	return version;
}

PLUGIN_EXPORT MStatus initializePlugin( MObject obj )
{
	MFnPlugin plugin( obj, "Ephere", "1.0" );

	// The example node refers to the HairData and GuidesData classes that are registered by Ornatrix, so Ornatrix must have been already loaded
	auto const ornatrix = MFnPlugin::findPlugin( "Ornatrix" );
	if( ornatrix.isNull() )
	{
		MGlobal::displayError( "Ornatrix is not loaded, it must be loaded before loading this plugin" );
		return MS::kFailure;
	}

	return
		plugin.registerCommand( "OxPrintStrandCount", OxPrintStrandCount::Create )
		&& plugin.registerNode(
			ScaleHairOperatorNode::GetName(),
			ScaleHairOperatorNode::GetMTypeId(),
			&ScaleHairOperatorNode::Create,
			&ScaleHairOperatorNode::Initialize,
		MPxNode::kDependNode )
		? MS::kSuccess : MS::kFailure;
}

PLUGIN_EXPORT MStatus uninitializePlugin( MObject obj )
{
	MFnPlugin plugin( obj );

	return
		plugin.deregisterNode( ScaleHairOperatorNode::GetMTypeId() )
		&& plugin.deregisterCommand( "OxPrintStrandCount" )
		? MS::kSuccess : MS::kFailure;
}
