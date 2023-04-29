// Must compile with VC 2012 / GCC 4.8

// ReSharper disable CppVariableCanBeMadeConstexpr
// ReSharper disable CppClangTidyModernizeReturnBracedInitList
#pragma once

#include "Ephere/Geometry/Native/IPolygonMesh.h"
#include "Ephere/Ornatrix/IHair.h"
#include "Ephere/Plugins/Ornatrix/IHairContainer.h"

#include <maya/MDataHandle.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPlug.h>
#include <maya/MPxGeometryData.h>

// shared_ptr
#include <memory>

namespace Ephere { namespace Plugins { namespace Ornatrix
{

/** Gets an IInterfaceProvider pointer from a dependency node, used to access various interfaces of its output data
	@return An IInterfaceProvider pointer if the node implements one, nullptr otherwise
*/
inline IInterfaceProvider* GetInterfaceProvider( MObject const& node )
{
	MFnDependencyNode const depNode( node );

	MStatus status;

	auto const interfacePlug = depNode.findPlug( "interfaceProvider", false, &status );
	if( !status )
	{
		return nullptr;
	}

	auto handle = interfacePlug.asMDataHandle(
#if MAYA_API_VERSION < 20180000
		MDGContext::fsNormal,
#endif
		&status );
	if( !status )
	{
		return nullptr;
	}

	auto const result = static_cast<IInterfaceProvider*>( handle.asAddr() );
	interfacePlug.destructHandle( handle );
	return result;
}

/** Gets Ornatrix hair interface from a HairShape node
	@return shared_ptr to hair interface if object was Ornatrix HairShape and IHair was provided
*/
inline std::shared_ptr<IHair> GetHairInterface( MObject const& hairShapeNode )
{
	auto const interfaceProvider = GetInterfaceProvider( hairShapeNode );
	return interfaceProvider != nullptr ? interfaceProvider->GetInterface<IHair>( "outputHair" ) : std::shared_ptr<IHair>();
}

inline std::shared_ptr<IPolygonMeshSA> GetDistributionMeshInterface( MObject const& hairShapeNode )
{
	auto const interfaceProvider = GetInterfaceProvider( hairShapeNode );
	return interfaceProvider != nullptr ? interfaceProvider->GetInterface<IPolygonMeshSA>( "distributionMesh" ) : std::shared_ptr<IPolygonMeshSA>();
}


unsigned const EphereIdentifierBase = 0x00124400;

// Matches the virtual table layout of Ephere::Plugins::Autodesk::Maya::HairData (which is not exported), to give clients access to the IHair stored in it
class HairData : public MPxGeometryData, public IHairContainer, public IInterfaceProvider
{
public:
	// Stable version 2.3.6
	static int const IHairSupportRevision = 19497;

	static MTypeId GetTypeId()
	{
		return MTypeId( EphereIdentifierBase, 1 );
	}
};

// This class is used just to distinguish data containing "hair" (ready for rendering) from "guides" (with some extra info needed for editing).
// Some operators require or produce "guides", while the operators which don't care about guides accept both
class GuidesData : public HairData
{
public:
	static MTypeId GetTypeId()
	{
		return MTypeId( EphereIdentifierBase, 3 );
	}
};

} } } // Ephere::Plugins::Ornatrix
