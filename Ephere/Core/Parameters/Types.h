// Must compile with VC 2012 / GCC 4.8

// ReSharper disable CppUseTypeTraitAlias
#pragma once

#include "Ephere/Core/Parameters/Array.h"
#include "Ephere/Core/Parameters/String.h"
#include "Ephere/NativeTools/IStream.h"
#include "Ephere/NativeTools/MacroTools.h"
#include "Ephere/NativeTools/SmartPointers.h"

#include <cstdint>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <typeinfo>
#include <type_traits>

namespace Ephere { namespace Parameters
{

// Type identification
// May have to be replaced with our own implementation if it doesn't work across different compiler versions or module boundaries

struct TypeId
{
	std::type_info const* typeId;

	TypeId()
		: typeId( nullptr )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	TypeId( std::type_info const* typeId )
		: typeId( typeId )
	{
	}

	friend bool operator==( TypeId const& left, TypeId const& right )
	{
		// By name or by hash code? Apple Clang produces different hash codes in different modules, not sure about standard Clang
#if defined( __clang__ )
		auto const leftName = left.typeId->name();
		auto const rightName = right.typeId->name();
		return std::char_traits<char>::compare( leftName, rightName, std::char_traits<char>::length( leftName ) ) == 0;
#else
		return left.typeId->hash_code() == right.typeId->hash_code();
#endif
	}

	friend bool operator!=( TypeId const& left, TypeId const& right )
	{
		return !( left == right );
	}

	friend bool operator<( TypeId const& left, TypeId const& right )
	{
		// By name or by hash code? Apple Clang produces different hash codes in different modules, not sure about standard Clang
#if defined( __clang__ )
		auto const leftName = left.typeId->name();
		auto const rightName = right.typeId->name();
		return std::char_traits<char>::compare( leftName, rightName, std::char_traits<char>::length( leftName ) ) < 0;
#else
		return left.typeId->hash_code() < right.typeId->hash_code();
#endif
	}
};

template <typename T>
TypeId GetTypeId()
{
	return TypeId( &typeid( T ) );
}

inline bool AreSameType( TypeId const& left, TypeId const& right )
{
	return left == right;
}

template <typename T>
bool IsSame( TypeId const& type )
{
	return AreSameType( GetTypeId<T>(), type );
}

inline char const* GetTypeName( TypeId const& typeId )
{
	return typeId.typeId->name();
}

typedef UniquePtr<void>( *FactoryFunction )( );


// IType / Type

struct ITypeRegistry;

// ReSharper disable once CppClassNeedsConstructorBecauseOfUninitializedMember
struct IType  // NOLINT(cppcoreguidelines-pro-type-member-init)
{
	EPHERE_NODISCARD TypeId GetTypeId() const
	{
		return typeId_;
	}

	template <typename T>
	EPHERE_NODISCARD bool Is() const
	{
		return Is( Parameters::GetTypeId<T>() );
	}

	EPHERE_NODISCARD bool Is( TypeId other ) const
	{
		return AreSameType( GetTypeId(), other );
	}

	EPHERE_NODISCARD bool Is( IType const& other ) const
	{
		return Is( other.GetTypeId() );
	}

	template <typename T>
	EPHERE_NODISCARD bool IsCompatibleWith() const
	{
		if( IsArray() )
		{
			return std::is_array<T>::value && GetElementType().IsCompatibleWith<typename std::remove_extent<T>::type>();
		}

		return Is<T>()
			|| IsEnum() && std::is_same<T, int>::value
			|| Is<int>() && std::is_enum<T>::value;
	}

	EPHERE_NODISCARD bool IsCompatibleWith( IType const& other ) const
	{
		if( IsArray() )
		{
			return other.IsArray() && GetElementType().IsCompatibleWith( other.GetElementType() );
		}

		return Is( other )
			|| IsEnum() && other.Is<int>()
			|| Is<int>() && other.IsEnum();
	}

	EPHERE_NODISCARD int GetSize() const
	{
		return size_;
	}

	EPHERE_NODISCARD bool IsEnum() const
	{
		return isEnum_;
	}

	EPHERE_NODISCARD bool IsCopyable() const
	{
		return isCopyable_;
	}

	EPHERE_NODISCARD bool IsShareable() const
	{
		return isShareable_;
	}

	EPHERE_NODISCARD bool IsArray() const
	{
		return isArray_;
	}

	EPHERE_NODISCARD virtual IType const& GetElementType() const
	{
		return *this;
	}


	static ITypeRegistry& GetRegistry();


	// A globally unique number that is used to identify the type on deserialization
	EPHERE_NODISCARD virtual int GetSerializeId() const = 0;

	virtual void Construct( void* uninitializedMemory ) const = 0;

	struct Deleter
	{
		IType const* type;

		explicit Deleter( IType const* type )
			: type( type )
		{
		}

		void operator()( void* pointer ) const
		{
			type->Destroy( pointer );
			delete static_cast<char*>( pointer );
		}
	};

	EPHERE_NODISCARD std::shared_ptr<void> Construct() const
	{
		std::shared_ptr<void> result( new char[GetSize()], Deleter( this ) );
		Construct( result.get() );
		return result;
	}

	virtual void Destroy( void* value ) const = 0;

	// Calls the assignment operator of source with target. This may either do full or shallow copy (sharing), depending on the implementation
	virtual void AssignValue( void const* source, void* target ) const = 0;

	// Makes a guaranteed copy of source into target, if possible. This is different from the assignment operator which might share the value (if held by shared_ptr)
	virtual void CopyValue( void const* source, void* target ) const = 0;

	// Moves source into target. This matches the C++ type's move semantics, if supported (simple types are just copied)
	virtual void MoveValue( void* source, void* target ) const = 0;

	// If the type has reference/share semantics (heavy data held by shared_ptr), the target value will be shared with the source. For types with value semantics this just makes a copy
	virtual void ShareValue( void const* source, void* target ) const = 0;


	virtual bool AreEqual( void const* left, void const* right ) const = 0;


	// Return true on success
	virtual bool Write( IOutputStream& buffer, void const* value ) const = 0;

	// Return true on success
	virtual bool Read( IInputStream& buffer, void* value ) const = 0;

	// Return empty string on failure, no type is supposed to be representable by an empty string, even in some "empty" state
	std::string ToString( void const* value ) const
	{
		std::ostringstream result;
		StlOutputStream wrapper( result );
		return Write( wrapper, value ) ? result.str() : std::string();
	}

	bool FromString( std::string const& buffer, void* value ) const
	{
		std::istringstream stream( buffer, std::ios::binary );
		StlInputStream wrapper( stream );
		return Read( wrapper, value );
	}

	EPHERE_NODISCARD std::shared_ptr<void> FromString( std::string const& buffer, FactoryFunction alternativeFactory = nullptr ) const
	{
		std::istringstream stream( buffer, std::ios::binary );
		StlInputStream wrapper( stream );
		auto result = alternativeFactory == nullptr ? Construct() : alternativeFactory().ToShared();
		if( result != nullptr && !Read( wrapper, result.get() ) )
		{
			result.reset();
		}

		return result;
	}

	virtual std::string ToReadableString( void const* value ) const = 0;

protected:

	virtual ~IType()  // NOLINT(modernize-use-equals-default)
	{
	}


	TypeId typeId_;

	// ReSharper disable CppUninitializedNonStaticDataMember
	int size_;

	bool isEnum_;

	bool isCopyable_;

	bool isShareable_;

	bool isArray_;
	// ReSharper restore CppUninitializedNonStaticDataMember
};

struct ArrayView
{
	void const* data;
	int count;
	int stride;

	EPHERE_NODISCARD bool IsEmpty() const
	{
		return count == 0;
	}

	template <typename T>
	EPHERE_NODISCARD Span<T const> AsSpan() const
	{
		ASSERT( sizeof( T ) == stride );
		return Span<T const>( static_cast<T const*>( data ), count );
	}

	template <typename T>
	EPHERE_NODISCARD T const& Front() const
	{
		ASSERT( sizeof( T ) == stride );
		return *static_cast<T const*>( data );
	}
};

/* The array operations are not implemented in terms of the Copy/Move/Share operations of the element's ITtype, but through the copy/move semantics of the element's C++ type.
This means that SetRange will share values with reference/share semantics.
*/
struct IArrayType : IType
{
	EPHERE_NODISCARD IType const& GetElementType() const override = 0;

	virtual int GetArrayLength( void const* value ) const = 0;

	virtual ArrayView GetValues( void const* value ) const = 0;

	// Copies a range of values via the element's copy assignment operator into this array
	virtual void SetRange( void* value, std::pair<void const*, int> elementValues, int destinationIndex ) const = 0;

	// Moves a range of values via the element's move assignment operator into this array
	virtual void MoveRange( void* value, std::pair<void*, int> elementValues, int destinationIndex ) const = 0;

	virtual void Resize( void* value, int newSize, void const* newValue ) const = 0;

	virtual void Erase( void* value, int begin, int count ) const = 0;
};

template <typename T, bool IsEnum = false, bool IsArray = false>
struct TypeImpl;

template <typename T>
struct Type final : TypeImpl<T, std::is_enum<T>::value, std::is_array<T>::value>
{
};

template <typename T>
typename std::conditional<std::is_array<T>::value, IArrayType, IType>::type const& GetType()
{
	return Type<T>::GetInstance();
}


// IType implementation helpers

template <typename T, bool IsEnum, bool IsArray>
struct TypeImpl
{
	static_assert( sizeof( T ) == 0, "Type<T> was not specialized for this type, it is not supported" );
};

template <typename T>
struct BitwiseSerialization
{
	static bool Write( IOutputStream& buffer, T const& value )
	{
		return buffer.Write( value );
	}

	static bool Read( IInputStream& buffer, T& value )
	{
		return buffer.Read( value );
	}
};

template <typename T>
struct NoSerialization
{
	static bool Write( IOutputStream&, T const& )
	{
		return false;
	}

	static bool Read( IInputStream&, T& )
	{
		return false;
	}
};

template <typename T>
struct Serialization : std::conditional<std::is_arithmetic<T>::value, BitwiseSerialization<T>, NoSerialization<T>>::type
{
};

template <typename T>
struct DefaultStringConversion
{
	static std::string ToReadableString( T const& value )
	{
		std::ostringstream result;
		result << value;
		return result.str();
	}
};

template <typename T>
struct EnumStringConversion
{
	static std::string ToReadableString( T const& value )
	{
		std::ostringstream result;
		result << static_cast<int>( value );
		return result.str();
	}
};

template <typename T>
struct NoStringConversion
{
	static std::string ToReadableString( T const& )
	{
		return "?";
	}
};

template <typename T>
struct StringConversion : std::conditional<std::is_arithmetic<T>::value && !std::is_enum<T>::value, DefaultStringConversion<T>, NoStringConversion<T>>::type
{
};

template <typename T, typename TValue = T, class TSerialize = Serialization<TValue>, class TStringConvert = StringConversion<TValue>, class TBase = IType>
struct TypeBase : TBase
{
	// We require parameter types to be copyable and moveable, to allow copying and moving whole parameter containers.
	// Sharing on copy as in shared_ptr is allowed, it's up to the user to be aware of that behavior.
	// A type may have custom copy operations and a disabled operator=, it then needs to implement Type<>::CopyValue accordingly.
#ifdef EPHERE_HAVE_CPP11
	static_assert( std::is_copy_constructible<TValue>::value, "Type must be copyable" );
	static_assert( std::is_copy_assignable<TValue>::value, "Type must be copyable" );
	static_assert( std::is_move_constructible<TValue>::value, "Type must be moveable" );
	static_assert( std::is_move_assignable<TValue>::value, "Type must be moveable" );
#endif

	static bool const IsArray = std::is_array<T>::value;
	static bool const IsEnum = std::is_enum<T>::value;
	typedef TValue ElementType;
	typedef TValue ValueType;
	typedef TStringConvert StringConversionType;

	enum : int
	{
		SerializeId = 0
	};


	TypeBase()
	{
		this->typeId_ = Parameters::GetTypeId<T>();
		this->size_ = sizeof( ValueType );
		this->isEnum_ = IsEnum;
		this->isCopyable_ = std::is_copy_constructible<ValueType>::value;
		this->isShareable_ = false;
		this->isArray_ = IsArray;
	}

	static Type<T> const& GetInstance()
	{
		static Type<T> instance;
		return instance;
	}


	EPHERE_NODISCARD int GetSerializeId() const override
	{
		return SerializeId;
	}

	void Construct( void* uninitializedMemory ) const override
	{
		// 'new ValueType()' as opposed to just 'new ValueType' initializes primitive types with 0
		new( uninitializedMemory ) ValueType();
	}

	void Destroy( void* value ) const override
	{
		UNUSED_VALUE( value );
		Cast( value ).~ValueType();
	}


	void AssignValue( void const* source, void* target ) const override
	{
		// No checks, parameters must be valid
		Cast( target ) = Cast( source );
	}

	void CopyValue( void const* source, void* target ) const override
	{
		// No checks, parameters must be valid
		Cast( target ) = Cast( source );
	}

	void MoveValue( void* source, void* target ) const override
	{
		// No checks, parameters must be valid
		Cast( target ) = std::move( Cast( source ) );
	}

	void ShareValue( void const* source, void* target ) const override
	{
		return CopyValue( source, target );
	}


	bool AreEqual( void const* left, void const* right ) const override
	{
		return Cast( left ) == Cast( right );
	}


	virtual bool WriteImpl( IOutputStream& buffer, ValueType const& value ) const
	{
		return TSerialize::Write( buffer, value );
	}

	virtual bool ReadImpl( IInputStream& buffer, ValueType& value ) const
	{
		return TSerialize::Read( buffer, value );
	}

	bool Write( IOutputStream& buffer, void const* value ) const override
	{
		return WriteImpl( buffer, Cast( value ) );
	}

	bool Read( IInputStream& buffer, void* value ) const override
	{
		return ReadImpl( buffer, Cast( value ) );
	}


	std::string ToReadableString( void const* value ) const override
	{
		return TStringConvert::ToReadableString( Cast( value ) );
	}


	static ValueType const& Cast( void const* value )
	{
		return *static_cast<ValueType const*>( value );
	}

	static ValueType& Cast( void* value )
	{
		return *static_cast<ValueType*>( value );
	}
};

template <typename TElement, typename TArray = Array<TElement>>
struct TypeBaseArray : TypeBase<TElement[], TArray, NoSerialization<TArray>, NoStringConversion<TArray>, IArrayType>
{
#ifdef EPHERE_HAVE_CPP11
	static_assert( std::is_destructible<Type<TElement>>::value, "Array element doesn't have a Type<> definition" );
#endif

	typedef TypeBase<TElement[], TArray> BaseType;
	typedef typename BaseType::ValueType ValueType;
	typedef typename TArray::value_type ElementType;


	EPHERE_NODISCARD IType const& GetElementType() const override
	{
		return GetType<TElement>();
	}

	int GetArrayLength( void const* value ) const override
	{
		return static_cast<int>( BaseType::Cast( value ).size() );
	}

	ArrayView GetValues( void const* value ) const override
	{
		auto const& array = BaseType::Cast( value );
		return ArrayView{ !array.empty() ? array.data() : nullptr, static_cast<int>( array.size() ), sizeof( ElementType ) };
	}

	void SetRange( void* value, std::pair<void const*, int> elementValues, int destinationIndex ) const override
	{
		auto& array = BaseType::Cast( value );
		auto begin = static_cast<ElementType const*>( elementValues.first );
		ASSERT( begin != nullptr && elementValues.second > 0 );
		ASSERT( destinationIndex + elementValues.second <= static_cast<int>( array.size() ) );
		std::copy( begin, begin + elementValues.second, array.begin() + destinationIndex );
	}

	void MoveRange( void* value, std::pair<void*, int> elementValues, int destinationIndex ) const override
	{
		auto& array = BaseType::Cast( value );
		auto begin = static_cast<ElementType*>( elementValues.first );
		ASSERT( begin != nullptr && elementValues.second > 0 );
		ASSERT( destinationIndex + elementValues.second <= static_cast<int>( array.size() ) );
		std::move( begin, begin + elementValues.second, array.begin() + destinationIndex );
	}

	void Resize( void* value, int newSize, void const* newValue ) const override
	{
		auto& array = BaseType::Cast( value );
		if( newValue != nullptr )
		{
			array.resize( newSize, *static_cast<ElementType const*>( newValue ) );
		}
		else
		{
			array.resize( newSize, ElementType() );
		}
	}

	void Erase( void* value, int begin, int count ) const override
	{
		auto& array = BaseType::Cast( value );
		ASSERT( begin >= 0 );
		ASSERT( begin + count <= static_cast<int>( array.size() ) );
		array.erase( array.begin() + begin, array.begin() + begin + count );
	}


	// ReSharper disable CppHidingFunction
	bool Write( IOutputStream& buffer, void const* value ) const override
	{
		auto const startPosition = buffer.GetPosition();
		auto count = 0;
		if( !buffer.Write( count ) )
		{
			return false;
		}

		auto const& array = BaseType::Cast( value );

		for( auto index = 0u; index < array.size(); ++index )
		{
			if( !buffer.Write( array[index] ) )
			{
				count = 0;
				break;
			}

			++count;
		}

		auto const endPosition = buffer.GetPosition();
		buffer.Seek( startPosition );
		if( !buffer.Write( count ) )
		{
			return false;
		}

		if( count > 0 )
		{
			buffer.Seek( endPosition );
		}

		return true;
	}

	bool Read( IInputStream& buffer, void* value ) const override
	{
		typename ValueType::size_type size;
		if( !buffer.Read( size ) )
		{
			return false;
		}

		auto imported = ValueType::Repeat( size );
		for( auto index = 0u; index < size; ++index )
		{
			if( !buffer.Read( imported[index] ) )
			{
				return false;
			}
		}

		BaseType::Cast( value ) = std::move( imported );
		return true;
	}

	std::string ToReadableString( void const* value ) const override
	{
		auto const& array = BaseType::Cast( value );
		std::ostringstream result;
		result << array.size() << ' ' << '[';
		for( auto i = 0; i < array.size(); ++i )
		{
			result << ' ' << Type<ElementType>::StringConversionType::ToReadableString( array[i] );
		}

		result << ' ' << ']';
		return result.str();
	}
	// ReSharper restore CppHidingFunction
};


// Built-in types

template <>
struct Type<bool> final : TypeBase<bool>
{
};

template <>
struct Type<int> final : TypeBase<int>
{
};

template <>
struct Type<std::int64_t> final : TypeBase<int64_t>
{
};

template <>
struct Type<float> final : TypeBase<float>
{
};

template <typename T>
struct TypeImpl<T, true> : TypeBase<T, T, BitwiseSerialization<T>, EnumStringConversion<T>>
{
};

template <typename T>
struct TypeImpl<T[], false, true> : TypeBaseArray<T>
{
};

template <>
struct Type<String> final : TypeBase<String, String, NoSerialization<String>, DefaultStringConversion<String>>
{
};

template <typename T>
struct Type<Array<T>> final : TypeBaseArray<T>
{
};

template <typename T>
struct Type<SharedPtr<T>> final : TypeBase<SharedPtr<T>>
{
#ifdef EPHERE_HAVE_CPP11
	static_assert( std::is_destructible<Type<T>>::value, "T doesn't have a Type<> definition" );
#endif

	enum : int
	{
		SerializeId = Type<T>::SerializeId
	};

	// ReSharper disable CppHidingFunction
	EPHERE_NODISCARD int GetSerializeId() const override
	{
		return SerializeId;
	}

	void Construct( void* uninitializedMemory ) const override
	{
		auto* value = new( uninitializedMemory ) SharedPtr<T>;
		*value = SharedPtr<T>::DefaultConstruct();
	}

	void CopyValue( void const* source, void* target ) const override
	{
		Type<T>().CopyValue( this->Cast( source ).get(), this->Cast( target ).get() );
	}

	void ShareValue( void const* source, void* target ) const override
	{
		this->Cast( target ) = this->Cast( source );
	}
	// ReSharper restore CppHidingFunction

	bool WriteImpl( IOutputStream& buffer, SharedPtr<T> const& value ) const override
	{
		auto isEmpty = value == nullptr;
		return buffer.Write( !isEmpty ) && ( isEmpty || Type<T>::GetInstance().Write( buffer, value.get() ) );
	}

	bool ReadImpl( IInputStream& buffer, SharedPtr<T>& value ) const override
	{
		bool notEmpty;
		if( !buffer.Read( notEmpty ) )
		{
			return false;
		}

		if( notEmpty )
		{
			ASSERT( value != nullptr );
			return Type<T>::GetInstance().Read( buffer, value.get() );
		}

		return true;
	}
};


struct ITypeRegistry
{
	virtual ~ITypeRegistry()  // NOLINT(modernize-use-equals-default)
	{
	}

	template <class T>
	bool RegisterType()
	{
		static_assert( Type<T>::SerializeId > 0, "A Type<> needs a positive SerializeId to be registered for deserialization" );
		return AddType( Type<T>::SerializeId, &GetType<T>() );
	}

	virtual bool AddType( int serializeId, IType const* type ) = 0;

	EPHERE_NODISCARD virtual IType const* FindType( int serializeId ) const = 0;
};

struct TypeRegistry : ITypeRegistry
{
	bool AddType( int serializeId, IType const* type ) override
	{
		if( types.find( serializeId ) != types.end() )
		{
			return false;
		}

		types[serializeId] = type;
		return true;
	}

	EPHERE_NODISCARD IType const* FindType( int serializeId ) const override
	{
		auto const position = types.find( serializeId );
		return position != types.end() ? position->second : nullptr;
	}

	std::map<int, IType const*> types;
};

inline ITypeRegistry& IType::GetRegistry()
{
	static TypeRegistry registry;
	return registry;
}

} }
