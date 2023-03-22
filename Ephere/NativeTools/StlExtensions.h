// Must compile with VC 2012 / GCC 4.8 (partial C++11), except the parts wrapped in #ifdef EPHERE_HAVE_CPP17

// For proper usage of std::begin/std::end() see https://en.cppreference.com/w/cpp/iterator/begin, section Overloads

// ReSharper disable CppTooWideScope
// ReSharper disable CppClangTidyModernizeUseEqualsDelete
#pragma once

#include "Ephere/NativeTools/Asserts.h"
#include "Ephere/NativeTools/Span.h"

#include <algorithm>
#include <istream>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Ephere
{

/*! Asserts that the pointer returned by dynamic_cast is not null.
To be used when side-casting (from A to B which are unrelated but both bases of the actual object type) which is known to be correct */
template <class TTargetPointer, class TFrom>
TTargetPointer CheckedDynamicCast( TFrom* pointer )
{
	static_assert( std::is_pointer<TTargetPointer>::value, "Target type must be a pointer type" );
	if( pointer == nullptr )
	{
		return nullptr;
	}

	auto result = dynamic_cast<TTargetPointer>( pointer );
	ASSERT( result != nullptr );
	return result;
}

/*! Asserts that the pointer returned by dynamic_cast is not null.
To be used when side-casting (from A to B which are unrelated but both bases of the actual object type) which is known to be correct */
template <class TTargetReference, class TBase>
TTargetReference CheckedDynamicCast( TBase& reference )
{
	static_assert( std::is_reference<TTargetReference>::value, "Target type must be a reference type" );
	return *CheckedDynamicCast<typename std::remove_reference<TTargetReference>::type*>( &reference );
}

//! Asserts that the static_cast from Base to Derived type is indeed possible. Has runtime cost, as it replaces the static_cast with dynamic_cast and checks the result
template <class TDerived, class TBase>
TDerived CheckedStaticCast( TBase* pointer )
{
	static_assert( std::is_pointer<TDerived>::value, "TDerived must be a pointer type" );
	static_assert( sizeof( static_cast<TDerived>( pointer ) ) > 0, "static_cast from TBase to TDerived must be valid" );
	return CheckedDynamicCast<TDerived>( pointer );
}

//! Asserts that the static_cast from Base to Derived type is indeed possible. Has runtime cost, as it replaces the static_cast with dynamic_cast and checks the result
template <class TDerived, class TBase>
TDerived CheckedStaticCast( TBase& reference )
{
	static_assert( std::is_reference<TDerived>::value, "TDerived must be a reference type" );
	static_assert( sizeof( static_cast<TDerived>( reference ) ) != 0, "static_cast from TBase to TDerived must be valid" );
	return CheckedDynamicCast<TDerived>( reference );
}

//! Utility function performing a static cast to be used in transforms
template <typename TSource, typename TDestination>
TDestination StaticCast( TSource const& value )
{
	return static_cast<TDestination>( value );
}

template <typename TSource, typename TDestination>
TDestination CheckedNarrowCast( TSource const& value )
{
	auto const result = StaticCast<TSource, TDestination>( value );
	// Check that narrowing did not change the value
	DEBUG_ONLY( ASSERT( value == static_cast<TSource>( result ) ) );
	return result;
}

template <typename TSource>
int ToInt( TSource const& value )
{
	return CheckedNarrowCast<TSource, int>( value );
}

class Noncopyable
{
protected:
	Noncopyable()
	{
	}
	~Noncopyable()
	{
	}
private:
	// emphasize the following members are private
	Noncopyable( Noncopyable const& );
	Noncopyable const& operator=( Noncopyable const& );
};


template <class TContainer, typename TElement, bool AddConst = false>
struct IndexedIterator
{
	typedef typename std::conditional<AddConst, TElement const, TElement>::type value_type;
	typedef value_type* pointer;
	typedef value_type& reference;
	typedef std::ptrdiff_t difference_type;
	typedef std::forward_iterator_tag iterator_category;

	typedef typename std::conditional<AddConst, TContainer const, TContainer>::type OwnerType;

	OwnerType* owner;
	int index;

	IndexedIterator( OwnerType& owner, int index )
		: owner( &owner ),
		index( index )
	{
	}

	IndexedIterator& operator++()
	{
		++index;

		return *this;
	}

	bool operator==( IndexedIterator const& other ) const
	{
		return index == other.index && owner == other.owner;
	}

	bool operator!=( IndexedIterator const& other ) const
	{
		return !( *this == other );
	}

	// Concrete types should implement this:
	//reference operator*() const;
};


template <typename T>
struct StaticSize
{
	static int const Value = T::Size;
};

template <typename T, size_t N>
struct StaticSize<T[N]>
{
	static int const Value = static_cast<int>( N );
};

template <typename T, size_t N>
struct StaticSize<std::array<T, N>>
{
	static int const Value = static_cast<int>( N );
};

// As we prefer signed int for sizes instead of STL's unsigned size_t, this is a helper to get the size() of a container as an int
template <typename T>
int Size( T const& container )
{
	return static_cast<int>( container.size() );
}

template <typename T, size_t N>
int Size( T const ( & )[N] )
{
	return static_cast<int>( N );
}

template <class TContainer>
EPHERE_NODISCARD int IndexOfElement( TContainer const& container, typename TContainer::value_type const& value, int startIndex = 0 )
{
	using std::begin;
	using std::end;
	auto start = begin( container );
	std::advance( start, startIndex );
	auto it = std::find( start, end( container ), value );
	return it != end( container ) ? static_cast<int>( std::distance( start, it ) ) : -1;
}

template <class TContainer, class TPredicate>
EPHERE_NODISCARD int IndexOf( TContainer const& container, TPredicate predicate, int startIndex = 0 )
{
	using std::begin;
	using std::end;
	auto start = begin( container );
	std::advance( start, startIndex );

	for( auto iterator = start; iterator != end( container ); ++iterator )
	{
		if( predicate( *iterator ) )
		{
			return static_cast<int>( std::distance( start, iterator ) );
		}
	}

	return -1;
}

template <class TIterator, class T>
EPHERE_NODISCARD bool Contains( TIterator first, TIterator last, T const& value )
{
	return std::find( first, last, value ) != last;
}

template <class TContainer, class T>
EPHERE_NODISCARD bool Contains( TContainer const& container, T const& value )
{
	using std::begin;
	using std::end;
	return Contains( begin( container ), end( container ), value );
}

template <class TContainer, class T>
EPHERE_NODISCARD bool BinarySearch( TContainer const& container, T const& value )
{
	using std::begin;
	using std::end;
	return std::binary_search( begin( container ), end( container ), value );
}

template <class TContainer, class TPredicate>
EPHERE_NODISCARD auto FindIf( TContainer& container, TPredicate predicate ) -> decltype( container.begin() )
{
	return std::find_if( container.begin(), container.end(), predicate );
}

template <class TContainer, class TPredicate>
EPHERE_NODISCARD auto FindFirstOrNull( TContainer&& container, TPredicate predicate ) -> decltype( &*container.begin() )
{
	auto location = std::find_if( container.begin(), container.end(), predicate );
	return location != container.end() ? &*location : nullptr;
}

template <class TContainer, class TPredicate>
EPHERE_NODISCARD auto FindLastOrNull( TContainer&& container, TPredicate predicate ) -> decltype( &*container.begin() )
{
	auto location = std::find_if( container.rbegin(), container.rend(), predicate );
	return location != container.end() ? &*location : nullptr;
}

template <class TContainer, class TPredicate, typename TDefault>
EPHERE_NODISCARD auto FindFirstOrDefault( TContainer&& container, TDefault&& defaultValue, TPredicate predicate ) -> typename std::decay<TContainer>::type::value_type
{
	auto location = std::find_if( std::forward<TContainer>( container ).begin(), std::forward<TContainer>( container ).end(), predicate );
	return location != std::forward<TContainer>( container ).end() ? *location : std::forward<TDefault>( defaultValue );
}

template <class TContainer, class TPredicate>
EPHERE_NODISCARD bool AllOf( TContainer& container, TPredicate predicate )
{
	using std::begin;
	using std::end;
	return std::all_of( begin( container ), end( container ), predicate );
}

template <class TContainer, class TPredicate>
EPHERE_NODISCARD bool AllOf( TContainer const& container, TPredicate predicate )
{
	using std::begin;
	using std::end;
	return std::all_of( begin( container ), end( container ), predicate );
}

template <class TContainer1, class TContainer2, class TPredicate>
EPHERE_NODISCARD bool AllOf( TContainer1 const& container1, TContainer2 const& container2, TPredicate predicate )
{
	auto const smallestSize = std::min( container1.size(), container2.size() );
	auto container2Iterator = container2.begin();
	return std::all_of( container1.begin(), container1.begin() + smallestSize, [&container2Iterator, &predicate]( typename TContainer1::value_type const& value1 ) mutable
	{
		return predicate( value1, *container2Iterator++ );
	} );
}

template <class TContainer, class TPredicate>
EPHERE_NODISCARD bool NoneOf( TContainer const& container, TPredicate predicate )
{
	using std::begin;
	using std::end;
	return std::none_of( begin( container ), end( container ), predicate );
}

template <class TContainer, class TPredicate>
EPHERE_NODISCARD bool AnyOf( TContainer& container, TPredicate predicate )
{
	using std::begin;
	using std::end;
	return std::any_of( begin( container ), end( container ), predicate );
}

template <class TContainer, class TPredicate>
EPHERE_NODISCARD bool AnyOf( TContainer const& container, TPredicate predicate )
{
	using std::begin;
	using std::end;
	return std::any_of( begin( container ), end( container ), predicate );
}

template <typename T>
void ExpandIfNeeded( std::vector<T>& vector, int newSize )
{
	if( newSize > Size( vector ) )
	{
		vector.resize( newSize );
	}
}

template <typename T>
void ExpandIfNeeded( std::vector<T>& vector, int newSize, T const& newValue )
{
	if( newSize > Size( vector ) )
	{
		vector.resize( newSize, newValue );
	}
}

template <class TContainer, class TPredicate>
void Erase( TContainer& container, TPredicate predicate )
{
	auto end = std::remove_if( container.begin(), container.end(), predicate );
	container.erase( end, container.end() );
}

template <class TContainer>
void Fill( TContainer& container, typename TContainer::value_type value )
{
	using std::begin;
	using std::end;
	std::fill( begin( container ), end( container ), value );
}

template <class TSourceContainer, class TDestinationContainer>
void Copy( TSourceContainer const& source, TDestinationContainer&& destination )
{
	using std::begin;
	using std::end;
	ASSERT( destination.size() >= static_cast<size_t>( std::distance( begin( source ), end( source ) ) ) );
	std::copy( begin( source ), end( source ), begin( destination ) );
}

template <class TSourceContainer, class TDestinationContainer = TSourceContainer>
TDestinationContainer GetCopy( TSourceContainer const& source, int const startIndex = 0, int count = -1 )
{
	using std::begin;
	using std::end;
	if( count == -1 )
	{
		count = Size( source ) - startIndex;
	}

	ASSERT( startIndex + count <= Size( source ) );

	TDestinationContainer result;
	std::copy_n( begin( source ) + startIndex, count, begin( result ) );

	return result;
}

template <int StartIndex = 0, int DestinationCount = -1, class T, std::size_t SourceCount>
std::array<T, DestinationCount> GetArrayCopy( std::array<T, SourceCount> const& source )
{
	return GetCopy<std::array<T, SourceCount>, std::array<T, DestinationCount>>( source, StartIndex, DestinationCount );
}

template <typename TContainer, class TSource>
void Append( TContainer& target, TSource const& source )
{
	target.insert( target.end(), source.begin(), source.end() );
}

template <typename TIterator1, typename TIterator2, typename TDestIterator, typename TFunctor>
void Transform( TIterator1 const begin1, TIterator1 const end1, TIterator2 const begin2, TIterator2 const end2, TDestIterator resultBegin, TFunctor&& functor )
{
	auto iterator1 = begin1;
	auto iterator2 = begin2;
	auto result = resultBegin;

	for( ; iterator1 != end1 && iterator2 != end2; ++iterator1, ++iterator2, ++result )
	{
		*result = functor( *iterator1, *iterator2 );
	}
}

template <typename TIterator1, typename TIterator2, typename TDestIterator, typename TFunctor>
void Transform( TIterator1 const begin1, TIterator1 const end1, TIterator2 const begin2, TDestIterator resultBegin, TFunctor&& functor )
{
	auto iterator1 = begin1;
	auto iterator2 = begin2;
	auto result = resultBegin;

	for( ; iterator1 != end1; ++iterator1, ++iterator2, ++result )
	{
		*result = functor( *iterator1, *iterator2 );
	}
}

template <typename TContainer1, typename TContainer2, typename TFunctor>
EPHERE_NODISCARD auto Transform2( TContainer1 const& container1, TContainer2 const& container2, TFunctor&& functor ) ->
std::vector<decltype( functor( std::declval<typename TContainer1::value_type>(), std::declval<typename TContainer2::value_type>() ) )>
{
	using TResultElement = decltype( functor( std::declval<typename TContainer1::value_type>(), std::declval<typename TContainer2::value_type>() ) );
	std::vector<TResultElement> result;
	result.reserve( container1.size() );

	using std::begin;
	using std::end;
	Transform( begin( container1 ), end( container1 ), begin( container2 ), std::back_inserter( result ), functor );

	return result;
}

template <typename TContainer, typename TFunctor>
void TransformSelf( TContainer& container, TFunctor&& functor )
{
	using std::begin;
	using std::end;
	std::transform( begin( container ), end( container ), begin( container ), functor );
}

template <typename TIterator1, typename TDestIterator, typename TFunctor>
void TransformResultByReference( TIterator1 const begin1, TIterator1 const end1, TDestIterator const resultBegin, TFunctor&& functor )
{
	auto result = resultBegin;
	for( auto iterator1 = begin1; iterator1 != end1; ++iterator1, ++result )
	{
		functor( *iterator1, *result );
	}
}

template <typename TIterator1, typename TIterator2, typename TDestIterator, typename TFunctor>
void TransformResultByReference( TIterator1 const begin1, TIterator1 const end1, TIterator2 const begin2, TDestIterator const resultBegin, TFunctor&& functor )
{
	auto result = resultBegin;
	auto iterator2 = begin2;
	for( auto iterator1 = begin1; iterator1 != end1; ++iterator1, ++iterator2, ++result )
	{
		functor( *iterator1, *iterator2, *result );
	}
}

template <typename TIterator, typename TDestIterator, typename TFunctor, typename TPredicate>
void TransformWhere( TIterator const begin, TIterator const end, TDestIterator resultBegin, TFunctor&& functor, TPredicate&& predicate )
{
	auto result = resultBegin;

	for( auto iterator = begin; iterator != end; ++iterator, ++result )
	{
		if( predicate( *iterator ) )
		{
			*result = functor( *iterator );
		}
	}
}

template <typename TContainer, typename TFunctor, typename TPredicate>
EPHERE_NODISCARD auto TransformWhere( TContainer const& container, TFunctor&& functor, TPredicate&& predicate ) -> std::vector<decltype( functor( std::declval<typename TContainer::value_type>() ) )>
{
	using TResultElement = decltype( functor( std::declval<typename TContainer::value_type>() ) );
	std::vector<TResultElement> result;
	result.reserve( container.size() );

	using std::begin;
	using std::end;
	TransformWhere( begin( container ), end( container ), back_inserter( result ), functor, predicate );

	return result;
}

template <typename TContainer>
void Sort( TContainer& container )
{
	using std::begin;
	using std::end;
	std::sort( begin( container ), end( container ) );
}

template <typename TContainer, typename TPredicate>
void Sort( TContainer& container, TPredicate&& predicate )
{
	using std::begin;
	using std::end;
	std::sort( begin( container ), end( container ), predicate );
}

template <typename TContainer, typename TPredicate>
EPHERE_NODISCARD TContainer GetSorted( TContainer const& container, TPredicate&& predicate )
{
	using std::begin;
	using std::end;
	auto result = container;
	std::sort( begin( result ), end( result ), predicate );
	return result;
}

template <typename TContainer>
EPHERE_NODISCARD TContainer GetSorted( TContainer const& container )
{
	auto result = container;
	Sort( result );
	return result;
}

// The GetMemoryUsage functions return just the memory allocated dynamically by the object, not the memory it occupies (*this)

template <typename T>
int64_t GetMemoryUsage( T const& value )
{
	return value.GetMemoryUsage();
}

template <typename T>
int64_t GetMemoryUsage( std::vector<T> const& vector )
{
	return vector.capacity() * sizeof( T );
}

template <typename T>
int64_t GetMemoryUsage( std::unique_ptr<T> const& value )
{
	return static_cast<int64_t>( value ? GetMemoryUsage( *value ) : 0 );
}

template <typename T>
int64_t GetMemoryUsage( std::shared_ptr<T> const& value )
{
	return static_cast<int64_t>( value ? GetMemoryUsage( *value ) / value.use_count() : 0 );
}

inline int64_t GetMemoryUsage( std::vector<bool> const& vector )
{
	return vector.capacity() / 8;
}

template <typename TKey, typename TValue>
int64_t GetMemoryUsage( std::unordered_map<TKey, TValue> const& hashMap )
{
	// This is a rough estimate of the minimum usage
	// VC's hash map contains a list of key/value pairs and a vector with list iterators, the size of both should not be less than the number of elements
	return hashMap.size() * ( sizeof( std::pair<TKey, TValue> ) + sizeof( void* ) /* list iterator */ + 2 * sizeof( void* ) /* list previous/next pointers */ );
}

template <typename TKey>
int64_t GetMemoryUsage( std::unordered_set<TKey> const& hashSet )
{
	// This is a rough estimate of the minimum usage
	// VC's hash set contains a list of keys and a vector with list iterators, the size of both should not be less than the number of elements
	return hashSet.size() * ( sizeof( TKey ) + sizeof( void* ) );
}


template <typename TSource, typename TDest>
void CopyArray( TSource const& source, TDest& destination )
{
	static_assert( StaticSize<TSource>::Value > 0 && StaticSize<TDest>::Value > 0, "Cannot determine array sizes" );
	static_assert( StaticSize<TSource>::Value == StaticSize<TDest>::Value, "Mismatched array sizes" );
	using std::begin;
	using std::end;
	using TSourceElem = typename std::decay< decltype( *begin( source ) ) >::type;
	using TDestElem = typename std::decay< decltype( *begin( destination ) ) >::type;
	std::transform( begin( source ), end( source ), begin( destination ), []( TSourceElem const& value )
	{
		return TDestElem( value );
	} );
}

template<typename T>
void ResizeInterlacedArray( std::vector<T>& array, T const& defaultValue, int const previousElementCount, int const newElementCount, int const newBlockCount )
{
	// If the array is empty simply allocate necessary amount of elements
	if( previousElementCount == 0 || array.empty() )
	{
		array.resize( newBlockCount * newElementCount, defaultValue );
		return;
	}

	// If array is not empty resize the array and copy previous elements into correct ranges
	auto const newSize = newElementCount * newBlockCount;
	std::vector<T> result( newSize );
	auto const copiedDataCount = std::min( newElementCount, previousElementCount );

	for( auto blockIndex = 0; blockIndex < newBlockCount; ++blockIndex )
	{
		auto const firstSourceIndex = blockIndex * previousElementCount;
		auto const firstDestinationIndex = blockIndex * newElementCount;

		// Copy previous to new, make sure the previousElementCount argument is correct!
		ASSERT_NO_THROW( copiedDataCount <= Size( array ) );
		std::copy( array.begin() + firstSourceIndex, array.begin() + firstSourceIndex + copiedDataCount, result.begin() + firstDestinationIndex );

		// Fill the rest with defaults
		std::fill( result.begin() + firstDestinationIndex + copiedDataCount, result.begin() + firstDestinationIndex + newElementCount, defaultValue );
	}

	array = result;
}


template <typename T>
bool StreamWrite( std::ostream& output, T const& data )
{
	output.write( reinterpret_cast<char const*>( &data ), sizeof( data ) );
	return static_cast<bool>( output );
}

template <typename T>
bool StreamWrite( std::ostream& output, std::vector<T> const& data )
{
	output.write( reinterpret_cast<char const*>( &data[0] ), sizeof( data[0] ) * data.size() );
	return static_cast<bool>( output );
}

template <typename T>
bool StreamRead( std::istream& input, T& data )
{
	input.read( reinterpret_cast<char*>( &data ), sizeof( data ) );
	return static_cast<bool>( input );
}


template<typename TContainer, typename TResultIterator, typename TPredicate>
void CopyIf( TContainer const& container, TResultIterator resultBegin, TPredicate predicate )
{
	using std::begin;
	using std::end;
	std::copy_if( begin( container ), end( container ), resultBegin, predicate );
}

template<typename TContainer, typename TPredicate>
std::vector<typename std::decay<typename TContainer::value_type>::type> Subset( TContainer const& container, TPredicate predicate )
{
	std::vector<typename std::decay<typename TContainer::value_type>::type> result;
	CopyIf( container, std::back_inserter( result ), predicate );
	return result;
}

template<typename TKeyContainer, typename TValueContainer>
std::unordered_map<typename std::decay<typename TKeyContainer::value_type>::type, typename std::decay<typename TValueContainer::value_type>::type> MakeUnorderedMap( TKeyContainer const& keys, TValueContainer const& values )
{
	std::unordered_map<typename std::decay<typename TKeyContainer::value_type>::type, typename std::decay<typename TValueContainer::value_type>::type> result;
	using std::begin;
	using std::end;
	auto keyIterator = begin( keys );
	auto valueIterator = begin( values );
	for( ; keyIterator != end( keys ) && valueIterator != end( values ); ++keyIterator, ++valueIterator )
	{
		result.insert( std::make_pair( *keyIterator, *valueIterator ) );
	}

	return result;
}

/** Delete items with specified indices from a vector
 * @param values Vector of values affected by this method
 * @param indices Array of indices to delete from values vector
 */
template<typename T>
void DeleteIndexSet( std::vector<T>& values, Span<int const> indices )
{
	if( indices.empty() )
	{
		return;
	}

	ASSERT( !values.empty() );

	for( auto index = indices.size() - 1; index >= 0; --index )
	{
		values.erase( values.begin() + indices[index] );
	}
}

template<typename T>
void DeleteValueSet( std::vector<T>& values, Span<T const> valuesToDelete )
{
	if( valuesToDelete.empty() || values.empty() )
	{
		return;
	}

	// TODO: Maybe use a set to speed things up for larger vector sizes
	for( auto valueToDelete : valuesToDelete )
	{
		auto findIterator = std::find( values.begin(), values.end(), valueToDelete );
		if( findIterator != values.end() )
		{
			values.erase( findIterator );
		}
	}
}

template<typename TContainer>
typename TContainer::value_type Accumulate( TContainer const& container )
{
	using std::begin;
	using std::end;
	return std::accumulate( begin( container ), end( container ), typename TContainer::value_type( 0 ) );
}

template<typename TContainer, typename TValue>
TValue Accumulate( TContainer const& container, TValue const value )
{
	using std::begin;
	using std::end;
	return std::accumulate( begin( container ), end( container ), value );
}

template<typename TContainer, typename TValue, typename TReduceFunction>
TValue Accumulate( TContainer const& container, TValue const value, TReduceFunction&& reduceOp )
{
	using std::begin;
	using std::end;
	return std::accumulate( begin( container ), end( container ), value, std::forward<TReduceFunction>( reduceOp ) );
}

template<typename TContainer1, typename TContainer2, typename TPredicateFunc>
bool AnyOf( TContainer1 const& container1, TContainer2 const& container2, TPredicateFunc&& predicateFunction )
{
	using std::begin;
	using std::end;
	auto iterator1 = begin( container1 );
	auto iterator2 = begin( container2 );
	for( ; iterator1 != end( container1 ) && iterator2 != end( container2 ); ++iterator1, ++iterator2 )
	{
		if( predicateFunction( *iterator1, *iterator2 ) )
		{
			return true;
		}
	}

	return false;
}

template<typename TContainer1>
int TrueCount( TContainer1 const& vector )
{
	return static_cast<int>( std::count_if(
		vector.begin(),
		vector.end(),
		[]( bool const value )
	{
		return value;
	} ) );
}

template<typename TContainer, typename TPredicateFunction>
int CountIf( TContainer const& container, TPredicateFunction&& predicateFunction )
{
	using std::begin;
	using std::end;
	return static_cast<int>( std::count_if( begin( container ), end( container ), predicateFunction ) );
}

template<typename TContainer, typename TFunction>
void ForEach( TContainer&& container, TFunction&& function )
{
	using std::begin;
	using std::end;
	std::for_each( begin( std::forward<TContainer>( container ) ), end( std::forward<TContainer>( container ) ), std::forward<TFunction>( function ) );
}

template<typename TContainer1, typename TContainer2, typename TFunction>
void ForEach( TContainer1& container1, TContainer2& container2, TFunction&& function )
{
	using std::begin;
	using std::end;
	auto iterator1 = begin( container1 );
	auto iterator2 = begin( container2 );
	for( ; iterator1 != end( container1 ) && iterator2 != end( container2 ); ++iterator1, ++iterator2 )
	{
		function( *iterator1, *iterator2 );
	}
}

template<typename TContainer1, typename TContainer2, typename TContainer3, typename TFunction>
void ForEach( TContainer1& container1, TContainer2& container2, TContainer3& container3, TFunction&& function )
{
	using std::begin;
	using std::end;
	auto iterator1 = begin( container1 );
	auto iterator2 = begin( container2 );
	auto iterator3 = begin( container3 );
	for( ; iterator1 != end( container1 ) && iterator2 != end( container2 ) && iterator3 != end( container3 ); ++iterator1, ++iterator2, ++iterator3 )
	{
		function( *iterator1, *iterator2, *iterator3 );
	}
}

template<typename TIterator, typename TContainer>
void IncrementIfNotEmpty( TIterator& value, TContainer const& container )
{
	if( container.empty() )
	{
		return;
	}

	++value;
}

template<typename TContainer>
auto BeginIfNotEmpty( TContainer& container, size_t offset = 0 ) -> decltype( container.begin() )
{
	if( container.empty() )
	{
		return typename TContainer::iterator();
	}

	return container.begin() + offset;
}

template<typename TContainer>
void ReverseSelf( TContainer& container )
{
	using std::begin;
	using std::end;
	std::reverse( begin( container ), end( container ) );
}

template<typename TContainer>
TContainer Reverse( TContainer const& container )
{
	auto result = container;
	ReverseSelf( result );
	return result;
}

template<typename TElement>
std::vector<TElement> Iota( size_t count, TElement value = 0 )
{
	std::vector<TElement> result( count );
	using std::begin;
	using std::end;
	std::iota( begin( result ), end( result ), value );
	return result;
}

template<typename TContainer1, typename TContainer2>
auto SetDifference( TContainer1 const& container1, TContainer2 const& container2 ) -> std::vector<typename TContainer1::value_type>
{
	std::vector<typename TContainer1::value_type> result;
	result.reserve( container1.size() );
	using std::begin;
	using std::end;
	std::set_difference( begin( container1 ), end( container1 ), begin( container2 ), end( container2 ), std::back_inserter( result ) );
	return result;
}

//! Returns a vector containing integer values not found in a provided container, up to a specified amount
template<typename TContainer>
std::vector<int> GetInvertedSet( TContainer const& values, int const totalCount, bool const areValuesOrdered = false )
{
	auto const fullySelectedArray = Iota( totalCount, 0 );
	std::vector<int> result;
	result.reserve( totalCount );

	using std::begin;
	using std::end;
	if( areValuesOrdered )
	{
		std::set_difference( fullySelectedArray.begin(), fullySelectedArray.end(), begin( values ), end( values ), std::back_inserter( result ) );
	}
	// Values not ordered, need to order them first
	else
	{
		std::vector<int> orderedValues( begin( values ), end( values ) );
		Sort( orderedValues );
		std::set_difference( fullySelectedArray.begin(), fullySelectedArray.end(), orderedValues.begin(), orderedValues.end(), std::back_inserter( result ) );
	}

	return result;
}

template<typename TContainerSource, typename TContainerResult>
TContainerResult ConvertTo( TContainerSource const& container )
{
	using std::begin;
	using std::end;
	return { begin( container ), end( container ) };
}

template<typename TContainerSource>
auto ConvertToSet( TContainerSource const& container ) -> std::set<typename TContainerSource::value_type>
{
	return ConvertTo<TContainerSource, std::set<typename TContainerSource::value_type>>( container );
}

template<typename TContainerSource>
auto ConvertToUnorderedSet( TContainerSource const& container ) -> std::unordered_set<typename TContainerSource::value_type>
{
	return ConvertTo<TContainerSource, std::unordered_set<typename TContainerSource::value_type>>( container );
}

template<typename TContainerSource>
auto ConvertToVector( TContainerSource const& container ) -> std::vector<typename TContainerSource::value_type>
{
	return ConvertTo<TContainerSource, std::vector<typename TContainerSource::value_type>>( container );
}

/// <summary>
/// Analyzes element changes and reordering between two collections.
/// </summary>
/// <remarks>
/// The collections are permitted to have repeating elements, in which case a "first come first serve" match-up is used.
/// </remarks>
/// <typeparam name="T">Type of collections</typeparam>
/// <param name="before">
/// First collection with values before changes
/// <example>
/// Example 1: a,b,c,d,e,f,g
/// Example 2: a,b,b,c,c,d,e
/// </example>
/// </param>
/// <param name="after">
/// Second collection with values after changes
/// <example>
/// Example 1: a,x,y,b,c,g,f
/// Example 2: x,c,c,c,b,e,d
/// </example>
/// </param>
/// <param name="changesToBefore">
/// An array of same size as before parameter containing remapping of unchanged indices to the after array or -1 for deleted values
/// <example>
/// Example 1: 0, 3, 4, -1, -1, 6, 5
/// Example 2: -1, 4, -1, 1, 2, 6, 5
/// </example>
/// </param>
/// <param name="changesToAfter">
/// An array of same size as after parameter containing remapping of unchanged indices to the before array or -1 for added values
/// <example>
/// Example 1: 0, -1, -1, 1, 2, 6, 5
/// Example 2: -1, 3, 4, -1, 1, 6, 5
/// </example>
/// </param>
template<typename T>
void Difference( Span<T const> before, Span<T const> after, std::vector<int>& changesToBefore, std::vector<int>& changesToAfter )
{
	//PROFILE_FUNCTION;

	// Fill before and after mapping indices with respective added and removed flags by default
	changesToBefore.resize( before.size(), -1 );
	changesToAfter.resize( after.size(), -1 );

	std::map<T, int> lastBeforeValueLocation;
	auto beforeIndex = 0, afterIndex = 0;

	for( auto afterElement = after.begin(); afterElement != after.end(); ++afterElement )
	{
		auto previousBeforeIndex = -1;
		auto previousBeforeIndexIterator = lastBeforeValueLocation.find( *afterElement );
		if( previousBeforeIndexIterator != lastBeforeValueLocation.end() )
		{
			previousBeforeIndex = previousBeforeIndexIterator->second;
		}

		beforeIndex = IndexOfElement( before, *afterElement, previousBeforeIndex != -1 ? previousBeforeIndex + 1 : 0 );

		// There exists an element in before collection matching the current one in after collection, create a mapping between the two
		if( beforeIndex != -1 )
		{
			changesToBefore[beforeIndex] = afterIndex;
			changesToAfter[afterIndex] = beforeIndex;

			// Record the last location where we found an existing value so that it will not be searched a second time
			if( previousBeforeIndex == -1 )
			{
				lastBeforeValueLocation.insert( std::make_pair( *afterElement, beforeIndex ) );
			}
			else
			{
				lastBeforeValueLocation[*afterElement] = beforeIndex;
			}
		}

		++afterIndex;
	}
}

template<class TContainer1, class TContainer2>
TContainer1 SetSymmetricDifference( TContainer1 const& container1, TContainer2 const& container2 )
{
	using std::begin;
	using std::end;
	std::vector<typename TContainer1::value_type> resultVector( std::max( container1.size() + container2.size() ) );
	auto iterator = std::set_symmetric_difference( begin( container1 ), end( container1 ), begin( container2 ), end( container2 ), resultVector.begin() );
	resultVector.resize( iterator - resultVector.begin() );

	return { resultVector.begin(), resultVector.end() };
}

template<class TContainer1, class TContainer2>
void EraseValuesNotInSet( TContainer1& container1, TContainer2 const& container2 )
{
	using std::begin;
	using std::end;
	std::vector<typename TContainer1::value_type> container1Vector( begin( container1 ), end( container1 ) );
	Erase( container1Vector, [&container2]( typename TContainer1::value_type const& element )
	{
		return container2.find( element ) != std::end( container2 );
	} );
	container1.clear();
	container1.insert( container1Vector.begin(), container1Vector.end() );
}

template<typename T>
std::vector<T> GetShuffledIndices( T count, T startIndex = 0 )
{
	auto result = Iota( count, startIndex );
	std::random_device rng;
	std::mt19937 urng( rng() );
	std::shuffle( result.begin(), result.end(), urng );

	return result;
}

template<typename T>
void GetArrayAsSet( std::vector<bool> const& selection, std::unordered_set<T>& result )
{
	result.clear();
	for( auto index = 0; index < Size( selection ); ++index )
	{
		if( !selection[index] )
		{
			continue;
		}

		result.insert( static_cast<T>( index ) );
	}
}

template<typename T>
std::unordered_set<T> GetArrayAsSet( std::vector<bool> const& selection )
{
	std::unordered_set<T> result;
	GetArrayAsSet( selection, result );

	return result;
}

template<typename T>
std::vector<bool> GetArrayAsElementSelection( std::unordered_set<T> const& selection, int elementCount = -1 )
{
	if( elementCount == -1 )
	{
		if( selection.empty() )
		{
			return {};
		}

		elementCount = *std::max_element( std::begin( selection ), std::end( selection ) ) + 1;
	}

	std::vector<bool> result( elementCount, false );
	for( auto const selectedIndex : selection )
	{
		result[selectedIndex] = true;
	}

	return result;
}


template <class It>
class Iterable
{
public:

	typedef typename std::decay<decltype( *std::declval<It>() )>::type value_type;
	typedef It iterator;
	typedef It const_iterator;
	typedef value_type const* const_pointer;
	typedef value_type* pointer;

	Iterable()
	{
	}

	Iterable( It begin, It end )
		: begin_( begin ),
		end_( end )
	{
	}

	EPHERE_NODISCARD It begin() const
	{
		return begin_;
	}

	EPHERE_NODISCARD It end() const
	{
		return end_;
	}

	EPHERE_NODISCARD int size() const
	{
		return static_cast<int>( std::distance( begin(), end() ) );
	}

	std::vector<value_type> ToVector() const
	{
		return std::vector<value_type>( begin(), end() );
	}

private:

	It begin_, end_;
};

template <class It>
Iterable<It> MakeIterable( It begin, It end )
{
	return Iterable<It>( begin, end );
}

template <class It>
Iterable<It> MakeIterable( It begin, int count )
{
	return Iterable<It>( begin, begin + count );
}


template <class TIterator>
class StrideIterator
{
public:
	typedef typename std::iterator_traits<TIterator>::value_type value_type;
	typedef typename std::iterator_traits<TIterator>::reference reference;
	typedef typename std::iterator_traits<TIterator>::difference_type difference_type;
	typedef typename std::iterator_traits<TIterator>::pointer pointer;
	//typedef std::random_access_iterator_tag iterator_category;
	typedef std::forward_iterator_tag iterator_category;

	// constructors
	StrideIterator()
		: iterator_(),
		end_(),
		stride_( 0 )
	{
	}

	StrideIterator( TIterator current, TIterator end, difference_type stride )
		: iterator_( current ),
		end_( end ),
		stride_( stride )
	{
		DEBUG_ONLY( ASSERT( stride > 0 ) );
	}

	StrideIterator( StrideIterator const& other )
		: iterator_( other.iterator_ ),
		end_( other.end_ ),
		stride_( other.stride_ )
	{
	}

	// operators
	StrideIterator& operator++()
	{
		for( auto i = stride_; iterator_ != end_ && i > 0; --i )
		{
			++iterator_;
		}

		return *this;
	}

	StrideIterator operator++( int )
	{
		StrideIterator tmp = *this;
		++* this;
		return tmp;
	}

	StrideIterator& operator+=( difference_type const count )
	{
		while( count > 0 && iterator_ != end_ )
		{
			++* this;
			--count;
		}

		return *this;
	}

	//StrideIterator& operator--()
	//{
	//	iterator_ -= stride_;
	//	return *this;
	//}

	//StrideIterator operator--( int )
	//{
	//	StrideIterator tmp = *this;
	//	iterator_ -= stride_;
	//	return tmp;
	//}

	//StrideIterator& operator-=( difference_type const count )
	//{
	//	iterator_ -= count * stride_;
	//	return *this;
	//}

	//reference operator[]( difference_type const index )
	//{
	//	return iterator_[index * stride_];
	//}

	reference operator*()
	{
		return *iterator_;
	}

	friend StrideIterator operator+( StrideIterator const& iterator, difference_type count )
	{
		auto result = iterator;
		result += count;
		return result;
	}

	friend StrideIterator operator+( difference_type count, StrideIterator const& iterator )
	{
		return iterator + count;
	}

	friend bool operator==( StrideIterator const& left, StrideIterator const& right )
	{
		DEBUG_ONLY( ASSERT( left.stride_ == right.stride_ ) );
		return left.iterator_ == right.iterator_;
	}

	friend bool operator!=( StrideIterator const& left, StrideIterator const& right )
	{
		DEBUG_ONLY( ASSERT( left.stride_ == right.stride_ ) );
		return left.iterator_ != right.iterator_;
	}

	friend bool operator<( StrideIterator const& left, StrideIterator const& right )
	{
		DEBUG_ONLY( ASSERT( left.stride_ == right.stride_ ) );
		return left.iterator_ < right.iterator_;
	}

	//friend StrideIterator operator-( StrideIterator const& iterator, difference_type count )
	//{
	//	return StrideIterator( iterator.iterator_ - count * iterator.stride_, iterator.stride_ );
	//}

	//friend difference_type operator-( StrideIterator const& left, StrideIterator const& right )
	//{
	//	DEBUG_ONLY( ASSERT( left.stride_ == right.stride_ ) );
	//	return ( left.iterator_ - right.iterator_ ) / left.stride_;
	//}

private:

	TIterator iterator_;
	TIterator end_;
	difference_type stride_;
};

inline int GetCountFilteredByStride( int x, int stride )
{
	DEBUG_ONLY( ASSERT( stride > 0 ) );
	return ( x + stride - 1 ) / stride;
}

inline int RoundUpToNearestMultiple( int x, int divisor )
{
	DEBUG_ONLY( ASSERT( divisor > 0 ) );
	return ( x + divisor - 1 ) / divisor * divisor;
}

template <class It>
Iterable<StrideIterator<It>> MakeStridedIterable( It begin, It end, int stride )
{
	return Iterable<StrideIterator<It>>( StrideIterator<It>{ begin, end, stride }, StrideIterator<It>{ end, end, stride } );
}

template <class TContainer>
Iterable<StrideIterator<typename TContainer::const_iterator>> MakeStridedIterable( TContainer const& container, int stride )
{
	typedef typename TContainer::const_iterator It;
	return Iterable<StrideIterator<typename TContainer::const_iterator>>( StrideIterator<It>{ container.begin(), container.end(), stride }, StrideIterator<It>{ container.end(), container.end(), stride } );
}

// Leave just the elements at stride multiples
template <typename T>
void FilterByStride( std::vector<T>& data, int stride )
{
	if( stride > 1 )
	{
		Copy( MakeStridedIterable( data, stride ), data );
		data.resize( GetCountFilteredByStride( Size( data ), stride ) );
	}
}

/** Convenience function to "or" two enum classes */
template<typename TEnum>
TEnum BinaryOr( TEnum value1, TEnum value2 )
{
	return TEnum( static_cast<uint64_t>( value1 ) | static_cast<uint64_t>( value2 ) );
}

/** Convenience function to "or" multiple enum classes */
template<typename TEnum>
TEnum TupleOr( Span<TEnum const> values )
{
	return Accumulate( values, TEnum( 0 ), BinaryOr<TEnum> );
}

/** Convenience function to "or" multiple enum classes if their accompanying flag is set to true */
template<typename TEnum>
TEnum OrIf( Span<std::pair<bool, TEnum> const> values )
{
	auto orredValues = Subset( values, []( std::pair<bool, TEnum> const& value )
	{
		return value.first;
	} );

	auto enumValues = Transform( orredValues, []( std::pair<bool, TEnum> const& value )
	{
		return value.second;
	} );

	return TupleOr<TEnum>( enumValues );
}


#ifdef EPHERE_HAVE_CPP17

template<typename TContainer1, typename TContainer2>
auto SetUnion( TContainer1 const& container1, TContainer2 const& container2 ) -> std::vector<typename TContainer1::value_type>
{
	std::vector<typename TContainer1::value_type> result;
	result.reserve( container1.size() + container2.size() );
	using std::begin;
	using std::end;
	set_union( begin( container1 ), end( container1 ), begin( container2 ), end( container2 ), std::back_inserter( result ) );

	return result;
}

template<typename T>
void UpdateSelection( std::unordered_set<T>& selection, std::unordered_set<T> const& newSelection, bool const isCtrlDown, bool const isAltDown/*, bool isShiftDown*/ )
{
	// Pressing Ctrl adds to selection
	if( isCtrlDown )
	{
		selection = ConvertToUnorderedSet( SetUnion( selection, newSelection ) );
		return;
	}

	// Pressing Alt subtracts from selection
	if( isAltDown )
	{
		EraseValuesNotInSet( selection, newSelection );
		return;
	}

	// Assign selection if nothing is pressed
	selection = newSelection;
}

/** For each element in container executes function if predicate for that element returns true */
template<typename TContainer, typename TFunction, typename TPredicate>
void ForEachWhere( TContainer& container, TFunction&& function, TPredicate&& predicate )
{
	ForEach( container, [&function, &predicate]( auto&& value )
	{
		if( !predicate( value ) )
		{
			return;
		}

	function( value );
	} );
}

/** For each element in container executes function if predicate for that element returns true */
template<typename TContainer, typename TFunction, typename TPredicate, typename TAlwaysExecuteFunction>
void ForEachWhere( TContainer& container, TFunction&& function, TPredicate&& predicate, TAlwaysExecuteFunction&& alwaysExecute )
{
	ForEach( container, [&function, &predicate, &alwaysExecute]( auto&& value )
	{
		if( !predicate( value ) )
		{
			alwaysExecute( value );
			return;
		}

	function( value );
	alwaysExecute( value );
	} );
}

/** For each element in container executes function if predicate for that element returns true */
template<typename TContainer, typename TFunction, typename TPredicate>
bool ForEachWhereUntilFail( TContainer&& container, TFunction&& function, TPredicate&& predicate )
{
	using std::begin;
	using std::end;
	for( auto first = begin( container ), last = end( container ); first != last; ++first )
	{
		if( predicate( *first ) && !function( *first ) )
		{
			return false;
		}
	}

	return true;
}

template<typename TKey, typename TValue, typename TInitKey, typename TInitValue = TValue>
std::map<TKey, TValue> CreateMap( std::initializer_list<std::pair<TInitKey, TInitValue>> pairs )
{
	std::map<TKey, TValue> result;

	for( auto const& pair : pairs )
	{
		result[TKey( pair.first )] = TValue( pair.second );
	}

	return result;
}

template <typename TContainer, typename TDestContainer, typename TFunctor>
void Transform( TContainer&& container, TDestContainer& result, TFunctor&& functor, bool appendResult = false )
{
	auto const previousResultSize = appendResult ? result.size() : 0;

	if constexpr( std::is_default_constructible_v<typename TDestContainer::value_type> )
	{
		result.resize( previousResultSize + container.size() );
		std::transform( container.begin(), container.end(), result.begin() + previousResultSize, functor );
	}
	else
	{
		if constexpr( std::is_same_v<TContainer, TDestContainer> )
		{
			ASSERT( &container != &result, "Cannot transform a container of non-default-constructible elements into itself" );
		}

		if( !appendResult )
		{
			result.clear();
		}

		result.reserve( previousResultSize + container.size() );
		std::transform( container.begin(), container.end(), std::back_inserter( result ), functor );
	}
}

template <typename TContainer, typename TFunctor>
EPHERE_NODISCARD auto Transform( TContainer&& container, TFunctor&& functor ) -> std::vector<decltype( functor( *std::declval<TContainer>().begin() ) )>
{
	using TResultElement = decltype( functor( *std::declval<TContainer>().begin() ) );
	std::vector<TResultElement> result;
	Transform( std::forward<TContainer>( container ), result, functor );
	return result;
}

template<typename TFromContainer, typename TToContainer>
void ConvertCast( TFromContainer const& fromContainer, TToContainer& toContainer )
{
	Transform( fromContainer, toContainer, StaticCast<typename TFromContainer::value_type, typename TToContainer::value_type> );
}

template <typename TContainer>
bool HasDuplicateEntries( TContainer const& container )
{
	return std::adjacent_find( begin( container ), end( container ) ) != end( container );
}

template<typename TFromContainer, typename TTo>
std::vector<TTo> GetConvertedCast( TFromContainer const& fromContainer )
{
	std::vector<TTo> toContainer;
	ConvertCast( fromContainer, toContainer );
	return toContainer;
}

template<typename TFromContainer>
std::vector<double> ToDouble( TFromContainer const& fromContainer )
{
	return GetConvertedCast<TFromContainer, double>( fromContainer );
}

template<typename TFromContainer>
std::vector<float> ToSingle( TFromContainer const& fromContainer )
{
	return GetConvertedCast<TFromContainer, float>( fromContainer );
}

template<typename TFunction>
auto Generate( int count, TFunction function )
{
	using TResultElement = typename std::invoke_result<TFunction>::type;
	std::vector<TResultElement> result( count );
	std::generate( result.begin(), result.end(), function );
	return result;
}


class HashTuple
{
	template<class T>
	struct Component
	{
		T const& value;
		explicit Component( T const& value ) : value( value )
		{
		}

		uintmax_t operator,( uintmax_t n ) const
		{
			n ^= std::hash<T>()( value );
			n ^= n << ( sizeof( uintmax_t ) * 4 - 1 );
			return n ^ std::hash<uintmax_t>()( n );
		}
	};

public:
	template<class Tuple>
	size_t operator()( Tuple const& tuple ) const
	{
		return std::hash<uintmax_t>()( std::apply( []( auto const& ... xs )
		{
			return ( Component( xs ), ..., 0 );
		}, tuple ) );
	}
};

#endif

// This function is much cheaper than deleting elements one by one through vector::erase() function
// Especially whenever the considering elements for deletion are noticeable
// Notice that the indices for deleting should be sorted ascending
template<typename T, class TIContainer>
void ShrinkValueSetByIndex( std::vector<T>& values, TIContainer const& sortedIndices )
{
	auto const valueSize = static_cast<int>( values.size() );

	auto itrIdx = sortedIndices.begin();
	auto currIdx = *itrIdx;
	auto nextDeleteIdx = ++itrIdx != sortedIndices.end() ? *itrIdx : valueSize;

	auto nextIdx = currIdx + 1;

	auto const FMoveValues = [&currIdx, &nextIdx, &nextDeleteIdx, &values]()
	{
		while( nextIdx != nextDeleteIdx )
		{
			values[currIdx] = values[nextIdx];
			++currIdx;
			++nextIdx;
		}
	};

	while( nextDeleteIdx != valueSize )
	{
		FMoveValues();

		++nextIdx;
		nextDeleteIdx = ++itrIdx != sortedIndices.end() ? *itrIdx : valueSize;
	}

	FMoveValues();

	values.resize( valueSize - static_cast<int>( sortedIndices.size() ) );
}

template<typename T, class TIContainer>
void ShrinkValueSetByIndex( std::vector<T>& values, TIContainer const& sortedIndices, int rangeSize, int rangeCount )
{
	auto const valueSize = static_cast<int>( values.size() );
	auto currIdx = 0;
	auto nextIdx = 0;

	auto offset = 0;

	for( int range = 0; range < rangeCount; ++range, offset += rangeSize )
	{
		auto itrIdx = sortedIndices.begin();
		auto const endOfRange = rangeSize + offset;
		auto nextDeleteIdx = *itrIdx + offset;

		auto const FMoveValues = [&currIdx, &nextIdx, &nextDeleteIdx, &values]()
		{
			while( nextIdx != nextDeleteIdx )
			{
				values[currIdx] = values[nextIdx];
				++currIdx;
				++nextIdx;
			}
		};

		while( nextDeleteIdx != endOfRange )
		{
			FMoveValues();

			++nextIdx;
			nextDeleteIdx = ++itrIdx != sortedIndices.end() ? *itrIdx + offset : endOfRange;
		}

		FMoveValues();
	}

	values.resize( valueSize - static_cast<int>( sortedIndices.size() ) * rangeCount );
}

// This function is much cheaper than deleting elements one by one through vector::erase() function
// Especially whenever the considering elements for deletion are noticeable
// Notice that the indices range for deleting should be sorted ascending
template<typename T>
void ShrinkValueSetByIndex( std::vector<T>& values, std::vector<std::pair<int, int>> const& sortedIndicesRange )
{
	auto const valueSize = static_cast<int>( values.size() );

	auto itrIdx = sortedIndicesRange.begin();
	auto currIdx = itrIdx->first;
	auto nextDeleteIdx = itrIdx + 1 != sortedIndicesRange.end() ? *( itrIdx + 1 ) : std::pair<int, int>{ valueSize, 0 };

	auto nextIdx = sortedIndicesRange[0].first + sortedIndicesRange[0].second;

	auto totalRemoves = sortedIndicesRange[0].second;

	auto const FMoveValues = [&currIdx, &nextIdx, &nextDeleteIdx, &values]()
	{
		while( nextIdx < nextDeleteIdx.first )
		{
			values[currIdx] = values[nextIdx];
			++currIdx;
			++nextIdx;
		}
	};

	while( nextDeleteIdx.first != valueSize )
	{
		FMoveValues();

		nextIdx += nextDeleteIdx.second;
		totalRemoves += nextDeleteIdx.second;
		++itrIdx;
		nextDeleteIdx = itrIdx + 1 != sortedIndicesRange.end() ? *( itrIdx + 1 ) : std::pair<int, int>{ valueSize, 0 };
	}

	FMoveValues();

	values.resize( valueSize - totalRemoves );
}

// This function could be used for deleting specific elements range of a collection in repetitive manner.
// e.g, for the a vector of 30 values, and range size of 10, if the deleting index is 5, then a 5th , 15th and 25th elements are removed from the collection
template<typename T>
void ShrinkValueSetByIndex( std::vector<T>& values, std::vector<std::pair<int, int>> const& sortedIndicesRange, int rangeSize, int rangeCount )
{
	auto const valueSize = static_cast<int>( values.size() );
	auto currIdx = 0;
	auto nextIdx = 0;

	auto offset = 0;
	auto totalRemoves = 0;

	for( auto range = 0; range < rangeCount; ++range, offset += rangeSize )
	{
		auto const endOfRange = rangeSize + offset;
		auto deletingItr = sortedIndicesRange.begin();
		auto deletingIdx = deletingItr->first + offset;

		auto const FMoveValues = [&currIdx, &nextIdx, &deletingIdx, &values]()
		{
			while( nextIdx < deletingIdx )
			{
				values[currIdx] = values[nextIdx];
				++currIdx;
				++nextIdx;
			}
		};

		while( deletingIdx != endOfRange )
		{
			FMoveValues();

			nextIdx += deletingItr->second;
			totalRemoves += deletingItr->second;
			++deletingItr;
			deletingIdx = deletingItr != sortedIndicesRange.end() ? deletingItr->first + offset : endOfRange;

		}

		FMoveValues();
	}

	values.resize( valueSize - totalRemoves );
}

template<typename T>
void ShrinkValueSetByValues( std::vector<T>& values, Span<T const> deletingValues )
{
	auto const valueSize = static_cast<int>( values.size() );

	std::vector<int> deletingIndices;
	deletingIndices.reserve( deletingValues.size() );
	int foundIndices = 0;

	for( auto i = 0; i < valueSize && foundIndices != static_cast<int>( deletingValues.size() ); ++i )
	{
		for( auto itrValue = deletingValues.begin(); itrValue != deletingValues.end(); ++itrValue )
		{
			if( values[i] == *itrValue )
			{
				deletingIndices.push_back( i );
				++foundIndices;
				break;
			}
		}
	}

	if( !deletingIndices.empty() )
	{
		ShrinkValueSetByIndex( values, deletingIndices );
	}
}



}
