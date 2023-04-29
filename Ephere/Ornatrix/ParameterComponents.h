// Must compile with VC 2012 / GCC 4.8

// ReSharper disable CppClangTidyModernizeUseEqualsDefault
// ReSharper disable CppClangTidyModernizeReturnBracedInitList
#pragma once

#include "Ephere/Core/Parameters/Types.h"
#include "Ephere/Geometry/Native/ICurves.h"
#include "Ephere/Geometry/Native/IPolygonMesh.h"
#include "Ephere/Ornatrix/IHair.h"
#include "Ephere/Ornatrix/Ramp.h"
#include "Ephere/Ornatrix/StrandChannel.h"
#include "Ephere/Ornatrix/TextureMapParameter.h"
#include "Ephere/Ornatrix/Types.h"

#include <sstream>

namespace Ephere { namespace Ornatrix
{

template <class TGeometryInterface>
struct GeometryParameter
{
	typedef TGeometryInterface InterfaceType;

	GeometryParameter()
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	GeometryParameter( std::nullptr_t )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <typename T>
	GeometryParameter( std::shared_ptr<T> const& value, Xform3 const& xform = Xform3::Identity() )
		: value_( SharedPtr<TGeometryInterface const>( value ) ),
		objectToWorldTransform_( xform ),
		isReadOnly_( std::is_const<T>::value )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <typename T>
	GeometryParameter( SharedPtr<T> value, Xform3 const& xform = Xform3::Identity() )
		: value_( std::move( value ) ),
		objectToWorldTransform_( xform ),
		isReadOnly_( std::is_const<T>::value )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <typename T>
	GeometryParameter( T* value, Xform3 const& xform = Xform3::Identity() )
		: value_( SharedPtr<TGeometryInterface const>::NotOwned( value ) ),
		objectToWorldTransform_( xform ),
		isReadOnly_( std::is_const<T>::value )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	GeometryParameter( GeometryParameter const& other, Xform3 const& xform )
		: value_( other.value_ ),
		objectToWorldTransform_( xform ),
		isReadOnly_( other.isReadOnly_ )
	{
	}

	EPHERE_NODISCARD TGeometryInterface const* Get() const
	{
		return value_.get();
	}

	EPHERE_NODISCARD TGeometryInterface* Modify() const
	{
		return !isReadOnly_ ? const_cast<TGeometryInterface*>( value_.get() ) : nullptr;
	}

	EPHERE_NODISCARD std::shared_ptr<TGeometryInterface> GetModifiable() const
	{
		return !isReadOnly_ ? std::const_pointer_cast<TGeometryInterface>( value_.ToShared() ) : nullptr;
	}

	EPHERE_NODISCARD bool IsEmpty() const
	{
		return Get() == nullptr;
	}

	EPHERE_NODISCARD bool IsModifiable() const
	{
		return !IsEmpty() && !isReadOnly_;
	}

	EPHERE_NODISCARD TGeometryInterface const* operator->() const
	{
		return Get();
	}

	EPHERE_NODISCARD TGeometryInterface const& operator*() const
	{
		return *Get();
	}

	EPHERE_NODISCARD Xform3 const& GetObjectToWorldTransform() const
	{
		return objectToWorldTransform_;
	}

	bool operator==( GeometryParameter const& other ) const
	{
		return Get() == other.Get() && objectToWorldTransform_ == other.objectToWorldTransform_;
	}

	bool operator!=( GeometryParameter const& other ) const
	{
		return !( *this == other );
	}

protected:

	typedef GeometryParameter<TGeometryInterface> BaseType;

	SharedPtr<TGeometryInterface const> value_;

	Xform3 objectToWorldTransform_ = Xform3::Identity();

	bool isReadOnly_ = true;
};

template <class T, class U>
bool AreInTheSameSpace( GeometryParameter<T> const& left, GeometryParameter<U> const& right )
{
	return NearEqual( left.GetObjectToWorldTransform(), right.GetObjectToWorldTransform() );
}

template <class TParameterContainer>
bool CheckHairAndDistributionMeshParameters( TParameterContainer const& parameters )
{
	return parameters.targetHair().IsModifiable()
		&& ( parameters.distributionMesh().IsEmpty() || CHECK( AreInTheSameSpace( parameters.targetHair(), parameters.distributionMesh() ) ) );
}

struct PolygonMeshParameter : GeometryParameter<IPolygonMeshSA>
{
	PolygonMeshParameter()
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	PolygonMeshParameter( std::nullptr_t )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <typename T>
	PolygonMeshParameter( std::shared_ptr<T> const& value, Xform3 const& xform = Xform3::Identity() )
		: BaseType( std::move( value ), xform )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <typename T>
	PolygonMeshParameter( SharedPtr<T> value, Xform3 const& xform = Xform3::Identity() )
		: BaseType( std::move( value ), xform )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <typename T>
	PolygonMeshParameter( T* value, Xform3 const& xform = Xform3::Identity() )
		: BaseType( value, xform )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	PolygonMeshParameter( PolygonMeshParameter const& other, Xform3 const& xform )
		: BaseType( other, xform )
	{
	}

/* TODO:
	PolygonMeshParameter Clone() const
	{
		if( IsEmpty() )
		{
			return PolygonMeshParameter();
		}

		PolygonMeshParameter result{ std::make_shared<Geometry::SharedPolygonMesh>() };
		result->CopyFrom( *Get() );
		return result;
	}
*/
};

struct CurvesParameter : GeometryParameter<ICurves>
{
	CurvesParameter()
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	CurvesParameter( std::nullptr_t )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <typename T>
	CurvesParameter( std::shared_ptr<T> const& value, Xform3 const& xform = Xform3::Identity() )
		: BaseType( std::move( value ), xform )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <typename T>
	CurvesParameter( SharedPtr<T> value, Xform3 const& xform = Xform3::Identity() )
		: BaseType( std::move( value ), xform )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <typename T>
	CurvesParameter( T* value, Xform3 const& xform = Xform3::Identity() )
		: BaseType( value, xform )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	CurvesParameter( CurvesParameter const& other, Xform3 const& xform )
		: BaseType( other, xform )
	{
	}
};

struct HairParameter : GeometryParameter<IHair>
{
	HairParameter()
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	HairParameter( std::nullptr_t )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <typename T>
	HairParameter( std::shared_ptr<T> const& value, Xform3 const& xform = Xform3::Identity() )
		: BaseType( std::move( value ), xform )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <typename T>
	HairParameter( SharedPtr<T> value, Xform3 const& xform = Xform3::Identity() )
		: BaseType( std::move( value ), xform )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <typename T>
	HairParameter( T* value, Xform3 const& xform = Xform3::Identity() )
		: BaseType( value, xform )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	HairParameter( HairParameter const& other, Xform3 const& xform )
		: BaseType( other, xform )
	{
	}

	EPHERE_NODISCARD HairParameter Clone() const
	{
		if( IsEmpty() )
		{
			return HairParameter();
		}

		if( auto const* ext1 = CheckedDynamicCast<IHair_Extension1 const*>( Get() ) )
		{
			auto copy = ext1->CloneHair();
			HairParameter result{ copy.ToShared(), objectToWorldTransform_ };
			return result;
		}

		return HairParameter();
	}
};

} }

namespace Ephere { namespace Parameters
{

template <>
struct Type<Geometry::SurfacePosition> final
	: TypeBase<Geometry::SurfacePosition, Geometry::SurfacePosition, BitwiseSerialization<Geometry::SurfacePosition>, DefaultStringConversion<Geometry::SurfacePosition>>
{
};

template <>
struct Type<Ornatrix::Vector3> final : TypeBase<Ornatrix::Vector3, Ornatrix::Vector3, BitwiseSerialization<Ornatrix::Vector3>, DefaultStringConversion<Ornatrix::Vector3>>
{
};

template <>
struct Type<Ornatrix::Xform3> final : TypeBase<Ornatrix::Xform3, Ornatrix::Xform3, BitwiseSerialization<Ornatrix::Xform3>, DefaultStringConversion<Ornatrix::Xform3>>
{
};

template <>
struct StringConversion<Ornatrix::StrandChannel>
{
	static std::string ToReadableString( Ornatrix::StrandChannel const& channel )
	{
		std::ostringstream stream;
		stream << "[ " << channel.type << ' ' << channel.index << " ]";
		return stream.str();
	}
};

template <>
struct Type<Ornatrix::StrandChannel> final : TypeBase<Ornatrix::StrandChannel>
{
};

template <>
struct Type<Ornatrix::Ramp> final : TypeBase<Ornatrix::Ramp, Ornatrix::Ramp, NoSerialization<Ornatrix::Ramp>, DefaultStringConversion<Ornatrix::Ramp>>
{
};

template <>
struct StringConversion<Ornatrix::TextureMapParameter>
{
	static std::string ToReadableString( Ornatrix::TextureMapParameter const& map )
	{
		std::ostringstream stream;
		stream << "[ " << ( map != nullptr ? "<map>" : "<null>" ) << ' ' << map.GetTextureChannel() << ' ' << ( map.GetInvertValues() ? "true" : "false" ) << " ]";
		return stream.str();
	}
};

template <>
struct Type<Ornatrix::TextureMapParameter> final : TypeBase<Ornatrix::TextureMapParameter>
{
};

template <>
struct StringConversion<Ornatrix::PolygonMeshParameter>
{
	static std::string ToReadableString( Ornatrix::PolygonMeshParameter const& mesh )
	{
		if( mesh.IsEmpty() )
		{
			return "<null>";
		}

		std::ostringstream stream;
		stream << "[ " << mesh->GetVertexCount() << ' ' << mesh->GetPolygonCount() << " / " << mesh.GetObjectToWorldTransform() << " ]";
		return stream.str();
	}
};

template <>
struct Type<Ornatrix::PolygonMeshParameter> final : TypeBase<Ornatrix::PolygonMeshParameter>
{
};

template <>
struct Type<Ornatrix::CurvesParameter> final : TypeBase<Ornatrix::CurvesParameter>
{
};

template <>
struct StringConversion<Ornatrix::HairParameter>
{
	static std::string ToReadableString( Ornatrix::HairParameter const& hair )
	{
		if( hair.IsEmpty() )
		{
			return "<null>";
		}

		std::ostringstream stream;
		stream << "[ " << hair->GetStrandCount() << " / " << hair.GetObjectToWorldTransform() << " ]";
		return stream.str();
	}
};

template <>
struct Serialization<Ornatrix::HairParameter>
{
	static bool Write( IOutputStream&, Ornatrix::HairParameter const& );

	static bool Read( IInputStream&, Ornatrix::HairParameter& );
};

template <>
struct Type<Ornatrix::HairParameter> final : TypeBase<Ornatrix::HairParameter>
{
	void CopyValue( void const* source, void* target ) const override
	{
		*static_cast<Ornatrix::HairParameter*>( target ) = static_cast<Ornatrix::HairParameter const*>( source )->Clone();
	}

	void ShareValue( void const* source, void* target ) const override
	{
		*static_cast<Ornatrix::HairParameter*>( target ) = *static_cast<Ornatrix::HairParameter const*>( source );
	}
};

} }
