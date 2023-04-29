// Must compile with VC 2012 / GCC 4.8

// ReSharper disable CppClangTidyModernizeUseEqualsDefault
#pragma once

#include "Types.h"
#include "Ephere/Geometry/Native/Matrix.h"

#include <algorithm>
#include <vector>

namespace Ephere { namespace Ornatrix
{

struct ITextureMap
{
	virtual ~ITextureMap()  // NOLINT(clang-diagnostic-deprecated-copy-with-user-provided-dtor)
	{
	}

	virtual float Evaluate( TextureCoordinate const& coordinate, Vector3 const* objectCoordinate = nullptr ) const = 0;

	virtual void Evaluate3( TextureCoordinate const* coordinates, int count, Vector3* result, void* textureTiles = nullptr ) const = 0;

	virtual Vector3 Evaluate3( TextureCoordinate const& coordinate, Vector3 const* objectCoordinate = nullptr ) const = 0;

	virtual void Evaluate( std::shared_ptr<void>& textureTiles ) const = 0;

	virtual float Evaluate( void const* textureTiles, TextureCoordinate const& coordinate ) const = 0;

	virtual Vector3 Evaluate3( void const* textureTiles, TextureCoordinate const& coordinate ) const = 0;

	/** Lets the host communicate if evaluation of its texture map from multiple threads cannot be done so that algorithms using the map can take care to avoid it */
	EPHERE_NODISCARD virtual bool IsTextureMappingThreadSafe() const = 0;

	EPHERE_NODISCARD virtual bool HasTextureObjectCoordinateSupport() const = 0;

	// Utility:

	void Evaluate3( Span<TextureCoordinate const> coordinates, Span<Vector3> result, void* textureTiles = nullptr ) const
	{
		Evaluate3( coordinates.data(), coordinates.size(), result.data(), textureTiles );
	}

	void Evaluate( TextureCoordinate const* coordinates, int count, Real* result, void* textureTiles = nullptr ) const
	{
		std::vector<Vector3> result3( count );
		Evaluate3( coordinates, count, result3.data(), textureTiles );
		std::transform( result3.begin(), result3.end(), result, []( Vector3 const& value )
		{
			return ( value.x() + value.y() + value.z() ) / 3;
		} );
	}
};

} }
