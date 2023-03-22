// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/NativeTools/SmartPointers.h"

#include <memory>

namespace Ephere { namespace Ornatrix
{
struct ITextureMap;

struct TextureMapParameter
{
	TextureMapParameter()
		: textureChannel_( 0 )
		, invertValues_( false )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	TextureMapParameter( std::shared_ptr<ITextureMap> map, int textureChannel = 0, bool invertValues = false )
		: map_( SharedPtr<ITextureMap const>( std::move( map ) ) )
		, textureChannel_( textureChannel )
		, invertValues_( invertValues )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	TextureMapParameter( ITextureMap* map, int textureChannel = 0, bool invertValues = false )
		: map_( SharedPtr<ITextureMap const>::NotOwned( map ) )
		, textureChannel_( textureChannel )
		, invertValues_( invertValues )
	{
	}

	EPHERE_NODISCARD bool IsEmpty() const
	{
		return map_ == nullptr;
	}

	bool operator==( TextureMapParameter const& other ) const
	{
		return map_ == other.map_ && textureChannel_ == other.textureChannel_ && invertValues_ == other.invertValues_;
	}

	bool operator!=( TextureMapParameter const& other ) const
	{
		return !( *this == other );
	}

	EPHERE_NODISCARD ITextureMap const* GetMap() const EPHERE_NOEXCEPT
	{
		return map_.get();
	}

	EPHERE_NODISCARD std::shared_ptr<ITextureMap const> GetSharedMap() const
	{
		return map_.ToShared();
	}

	EPHERE_NODISCARD int GetTextureChannel() const EPHERE_NOEXCEPT
	{
		return textureChannel_;
	}

	EPHERE_NODISCARD bool GetInvertValues() const EPHERE_NOEXCEPT
	{
		return invertValues_;
	}


private:
	SharedPtr<ITextureMap const> map_;
	int textureChannel_;
	bool invertValues_;
};

} }
