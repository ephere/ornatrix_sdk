// Must compile with VC 2012 / GCC 4.8

// ReSharper disable CppTooWideScopeInitStatement
#pragma once

#include "Ephere/NativeTools/Span.h"

#include <cstring> // memmove
#include <iterator>
#include <vector>

namespace Ephere { namespace Parameters
{

// Clang doesn't like memmove-ing polymorphic classes, the error is "destination for this 'memmove' call is a pointer to dynamic class 'X'; vtable pointer will be overwritten"
template <typename T>
struct IsTriviallyRelocatable : std::integral_constant<bool, !std::is_polymorphic<T>::value>
{
};

/*! Binary-compatible std::vector-like container. T must be move-assignable. size_type is signed.
If T is not "trivially relocatable", it must have a Ephere::Parameters::IsTriviallyRelocatable<T> specialization inheriting std::false_type
*/
template <typename T>
class Array
{
public:

	typedef T value_type;

	typedef int64_t size_type;
	typedef int64_t difference_type;

	typedef T* pointer;
	typedef T const* const_pointer;
	typedef T& reference;
	typedef T const& const_reference;

	typedef T* iterator;
	typedef T const* const_iterator;

	typedef std::reverse_iterator<T const*> const_reverse_iterator;
	typedef std::reverse_iterator<T*> reverse_iterator;

	typedef IsTriviallyRelocatable<T> IsTriviallyRelocatableType;


	struct Allocator
	{
		typedef void*( *AllocateFunc )( size_t );
		typedef void( *DeallocateFunc )( void* );

		AllocateFunc allocator;
		DeallocateFunc deallocator;

		static void* Malloc( size_t size )
		{
			return std::malloc( size );
		}

		static void Free( void* pointer )
		{
			std::free( pointer );
		}
	};

	static Allocator& GetDefaultAllocator()
	{
		static Allocator mallocFree = { Allocator::Malloc, Allocator::Free };
		return mallocFree;
	}


	Array() EPHERE_NOEXCEPT
		: data_( nullptr )
		, length_( 0 )
		, capacity_( 0 )
		, allocator_( &GetDefaultAllocator() )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	Array( Span<T const> span )  // NOLINT(cppcoreguidelines-pro-type-member-init)
		: allocator_( &GetDefaultAllocator() )
	{
		InitializeFromRange( span.begin(), span.end() );
	}

	template <class TInputIterator>
	Array( TInputIterator first, TInputIterator last )
		: allocator_( &GetDefaultAllocator() )
	{
		InitializeFromRange( first, last );
	}

	Array( Array const& other )  // NOLINT(cppcoreguidelines-pro-type-member-init)
		: allocator_( &GetDefaultAllocator() )
	{
		InitializeFromRange( other.begin(), other.end() );
	}

	Array( Array&& other ) EPHERE_NOEXCEPT
		: data_( other.data_ )
		, length_( other.length_ )
		, capacity_( other.capacity_ )
		, allocator_( other.allocator_ )
	{
		other.data_ = nullptr;
		other.length_ = other.capacity_ = 0;
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <class TContainer, typename TEnableIf = typename std::enable_if<std::is_convertible<typename TContainer::value_type, T>::value, int>::type>
	Array( TContainer const& container )  // NOLINT(cppcoreguidelines-pro-type-member-init)
		: allocator_( &GetDefaultAllocator() )
	{
		InitializeFromRange( container.begin(), container.end() );
	}

#ifdef EPHERE_HAVE_CPP17
	Array( std::initializer_list<T> init )  // NOLINT(cppcoreguidelines-pro-type-member-init)
		: allocator_( &GetDefaultAllocator() )
	{
		InitializeFromRange( init.begin(), init.end() );
	}
#endif

	~Array()
	{
		Destroy();
		Deallocate();
	}

	static Array Repeat( size_type count )
	{
		Array result;
		result.resize( count );
		return result;
	}

	static Array Repeat( size_type count, T const& element )
	{
		Array result;
		result.resize( count, element );
		return result;
	}

	Array& operator=( Array const& other )  // NOLINT(bugprone-unhandled-self-assignment)
	{
		if( this != &other )
		{
			assign( other.begin(), other.end() );
		}

		return *this;
	}

	Array& operator=( Array&& other ) EPHERE_NOEXCEPT
	{
		Array temp( std::move( other ) );
		swap( temp );
		return *this;
	}


	EPHERE_NODISCARD Allocator& GetAllocator() const EPHERE_NOEXCEPT
	{
		return *allocator_;
	}

	void SetAllocator( Allocator& allocator ) EPHERE_NOEXCEPT
	{
		ASSERT( data_ == nullptr );
		allocator_ = &allocator;
	}

	EPHERE_NODISCARD bool empty() const EPHERE_NOEXCEPT
	{
		return length_ == 0;
	}

	EPHERE_NODISCARD size_type size() const EPHERE_NOEXCEPT
	{
		return length_;
	}

	EPHERE_NODISCARD size_type capacity() const EPHERE_NOEXCEPT
	{
		return capacity_;
	}

	//size_t getMemorySize( bool own = true, bool allocated = true ) const
	//{
	//	auto result = capacity() * sizeof( T );
	//	for ( auto i = 0; i < length_; ++i )
	//	{
	//		result += GetAllocatedMemorySize( ( *this )[i] );
	//	}

	//	return result;
	//}


	void clear()
	{
		Destroy();
		length_ = 0;
	}

	bool reserve( size_type count )
	{
		if( count <= 0 )
		{
			return true;
		}

		auto const currentCapacity = capacity();
		size_type newCapacity;
		if( data_ == nullptr )
		{
			newCapacity = count;
		}
		else if( count > currentCapacity )
		{
			newCapacity = std::max( currentCapacity + currentCapacity / 2, count );
		}
		else
		{
			return true;
		}

		auto newData = Allocate( newCapacity );
		if( newData == nullptr )
		{
			return false;
		}

		Relocate( newData, IsTriviallyRelocatableType() );
		Deallocate();
		data_ = newData;
		return true;
	}

	bool resize( size_type count )
	{
		if( !reserve( count ) )
		{
			return false;
		}

		for( auto iter = data_ + length_, last = data_ + count; iter < last; ++iter )
		{
			::new ( static_cast<void*>( iter ) ) T();
		}

		length_ = count;
		return true;
	}

	bool resize( size_type count, T const& value )
	{
		if( !reserve( count ) )
		{
			return false;
		}

		if( count > length_ )
		{
			std::uninitialized_fill( data_ + length_, data_ + count, value );
		}

		length_ = count;
		return true;
	}

	void swap( Array& other ) EPHERE_NOEXCEPT
	{
		std::swap( data_, other.data_ );
		std::swap( length_, other.length_ );
		std::swap( capacity_, other.capacity_ );
		std::swap( allocator_, other.allocator_ );
	}

	template <class TInputIterator>
	void assign( TInputIterator first, TInputIterator last )
	{
		auto distance = std::distance( first, last );
		if( distance <= 0 )
		{
			clear();
			return;
		}

		if( reserve( distance ) )
		{
			auto shorter = std::min( length_, static_cast<size_type>( distance ) );
			std::copy( first, first + shorter, data_ );
			std::uninitialized_copy( first + shorter, last, end() );
			Destroy( distance, length_ );
			length_ = distance;
		}
	}

	template <class TContainer>
	void assign( TContainer const& container, typename TContainer::value_type* = nullptr )
	{
		assign( container.begin(), container.end() );
	}


	iterator push_back( T const& el )
	{
		return insert( end(), el );
	}

	iterator push_back( T&& el )
	{
		return insert( end(), std::move( el ) );
	}

	iterator insert( iterator pos, T const& el )
	{
		return insert( pos, std::move( T( el ) ) );
	}

	iterator insert( iterator pos, T&& el )
	{
		auto index = pos - data_;
		ASSERT( 0 <= index );
		ASSERT( static_cast<size_type>( index ) <= length_ );
		if( !reserve( length_ + 1 ) )
		{
			return end();
		}

		InsertMoveOne( index, std::move( el ), IsTriviallyRelocatableType() );
		++length_;
		return data_ + index;
	}

	template <class It>
	iterator insert( iterator pos, It first, It last )
	{
		auto distance = std::distance( first, last );
		if( distance <= 0 )
		{
			return end();
		}

		auto index = pos - data_;
		ASSERT( 0 <= index );
		ASSERT( static_cast<size_type>( index ) <= length_ );
		if( !reserve( length_ + distance ) )
		{
			return end();
		}

		InsertCopyMany( static_cast<size_type>( index ), static_cast<int>( distance ), first, last, IsTriviallyRelocatableType() );
		length_ += distance;
		return data_ + index;
	}

	void erase( const_iterator pos, size_type count = 1 )
	{
		if( count <= 0 )
		{
			return;
		}

		auto index = pos - data_;
		ASSERT( 0 <= index );
		ASSERT( static_cast<size_type>( index ) < length_ );
		ASSERT( index + count <= length_ );

		MoveLeft( index, count, IsTriviallyRelocatableType() );
		length_ -= count;
	}

	void erase( const_iterator first, const_iterator last )
	{
		ASSERT( data_ <= first );
		ASSERT( first <= last );
		ASSERT( last <= end() );
		erase( first, last - first );
	}


	const_reference operator[] ( size_type i ) const EPHERE_NOEXCEPT
	{
		return data_[i];
	}

	reference operator[] ( size_type i ) EPHERE_NOEXCEPT
	{
		return data_[i];
	}

	EPHERE_NODISCARD const_reference at( size_type i ) const
	{
		ASSERT_RANGE( i < length_ );
		return data_[i];
	}

	reference at( size_type i )
	{
		ASSERT_RANGE( i < length_ );
		return data_[i];
	}

	EPHERE_NODISCARD T const& front() const
	{
		ASSERT( 0 < length_ );
		return *data_;
	}

	reference front()
	{
		ASSERT( 0 < length_ );
		return *data_;
	}

	EPHERE_NODISCARD T const& back() const
	{
		ASSERT( 0 < length_ );
		return data_[length_ - 1];
	}

	reference back()
	{
		ASSERT( 0 < length_ );
		return data_[length_ - 1];
	}


	EPHERE_NODISCARD T const* data() const EPHERE_NOEXCEPT
	{
		return data_;
	}

	T* data() EPHERE_NOEXCEPT
	{
		return data_;
	}

	EPHERE_NODISCARD const_iterator begin() const EPHERE_NOEXCEPT
	{
		return data_;
	}

	EPHERE_NODISCARD const_iterator end() const EPHERE_NOEXCEPT
	{
		return data_ + length_;
	}

	iterator begin() EPHERE_NOEXCEPT
	{
		return data_;
	}

	iterator end() EPHERE_NOEXCEPT
	{
		return data_ + length_;
	}

	EPHERE_NODISCARD const_reverse_iterator rbegin() const EPHERE_NOEXCEPT
	{
		return const_reverse_iterator( end() );
	}

	EPHERE_NODISCARD const_reverse_iterator rend() const EPHERE_NOEXCEPT
	{
		return const_reverse_iterator( begin() );
	}

	reverse_iterator rbegin() EPHERE_NOEXCEPT
	{
		return reverse_iterator( end() );
	}

	reverse_iterator rend() EPHERE_NOEXCEPT
	{
		return reverse_iterator( begin() );
	}

	operator std::vector<T>() const
	{
		return std::vector<T>( begin(), end() );
	}

private:

	T* Allocate( size_type count )
	{
		capacity_ = count;
		return static_cast<T*>( allocator_->allocator( count * sizeof( T ) ) );
	}

	void Deallocate()
	{
		if( data_ != nullptr )
		{
			allocator_->deallocator( data_ );
		}
	}

	void Destroy( size_type first, size_type last )
	{
		for( auto i = first; i < last; ++i )
		{
			( data_ + i )->~T();
		}
	}

	void Relocate( T* newData, std::true_type )
	{
		// T is "trivially relocatable"
		if( length_ > 0 )
		{
			// On Linux memmove crashes if length is 0
			std::memmove( static_cast<void*>( newData ), data_, length_ * sizeof( T ) );  // NOLINT(bugprone-undefined-memory-manipulation)
		}
	}

	void Relocate( T* newData, std::false_type )
	{
#ifdef EPHERE_HAVE_CPP17
		static_assert( std::is_move_assignable<T>::value );
		std::uninitialized_move( data_, data_ + length_, newData );
#else
		auto last = data_ + length_;
		auto dest = newData;
		for( auto current = data_; current != last; ++dest, ++current )
			::new ( static_cast<void*>( std::addressof( *dest ) ) ) T( std::move( *current ) );
#endif
		Destroy( 0, length_ );
	}

	void InsertMoveOne( size_type index, T&& el, std::true_type )
	{
		// Linux issues a warning when calling memmove on pointers to objects with non-trivial copy ctor, need to cast the pointer to void*
		std::memmove( static_cast<void*>( data_ + index + 1 ), static_cast<void*>( data_ + index ), ( length_ - index ) * sizeof( T ) );
		::new ( static_cast<void*>( data_ + index ) ) T( std::move( el ) );
	}

	void InsertMoveOne( size_type index, T&& el, std::false_type )
	{
		::new ( static_cast<void*>( data_ + length_ ) ) T( std::move( el ) );
		std::rotate( data_ + index, data_ + length_, data_ + length_ + 1 );
	}

	template <typename It>
	void InsertCopyMany( size_type index, int distance, It first, It last, std::true_type )
	{
		// Linux issues a warning when calling memmove on pointers to objects with non-trivial copy ctor, need to cast the pointer to void*
		std::memmove( static_cast<void*>( data_ + index + distance ), static_cast<void*>( data_ + index ), ( length_ - index ) * sizeof( T ) );
		std::uninitialized_copy( first, last, data_ + index );
	}

	template <typename It>
	void InsertCopyMany( size_type index, int distance, It first, It last, std::false_type )
	{
		std::uninitialized_copy( first, last, data_ + length_ );
		std::rotate( data_ + index, data_ + length_, data_ + length_ + distance );
	}

	void MoveLeft( size_type index, size_type count, std::true_type )
	{
		Destroy( index, index + count );
		// Linux issues a warning when calling memmove on pointers to objects with non-trivial copy ctor, need to cast the pointer to void*
		std::memmove( static_cast<void*>( data_ + index ), static_cast<void*>( data_ + index + count ), ( length_ - index - count ) * sizeof( T ) );
	}

	void MoveLeft( size_type index, size_type count, std::false_type )
	{
		std::move( data_ + index + count, data_ + length_, data_ + index );
		Destroy( length_ - count, length_ );
	}

	void Destroy()
	{
		Destroy( 0u, length_ );
	}

	template <class TInputIterator>
	void InitializeFromRange( TInputIterator first, TInputIterator last )
	{
		auto const distance = static_cast<int>( std::distance( first, last ) );
		if( distance <= 0 )
		{
			length_ = 0;
			data_ = nullptr;
		}
		else
		{
			data_ = Allocate( distance );
			if( data_ == nullptr )
			{
				length_ = 0;
			}
			else
			{
				length_ = distance;
				std::uninitialized_copy( first, last, data_ );
			}
		}
	}


	T* data_;

	size_type length_;

	size_type capacity_;

	Allocator* allocator_;
};

template <typename T, class C>
bool operator==( Array<T> const& a, C const& b )
{
	return static_cast<int>( a.size() ) == static_cast<int>( b.size() )
		&& ( a.empty() || std::equal( a.begin(), a.end(), b.begin() ) );
}

template <typename T, class C>
bool operator!=( Array<T> const& a, C const& b )
{
	return ! ( a == b );
}

template <typename T>
T const* begin( Array<T> const& array )
{
	return array.begin();
}

template <typename T>
T const* end( Array<T> const& array )
{
	return array.end();
}

template <typename T>
T* begin( Array<T>& array )
{
	return array.begin();
}

template <typename T>
T* end( Array<T>& array )
{
	return array.end();
}

} }
