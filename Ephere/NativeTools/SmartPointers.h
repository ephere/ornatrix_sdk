// Must compile with VC 2012 / GCC 4.8

// ReSharper disable CppClangTidyModernizeUseEqualsDefault
// ReSharper disable CppClangTidyModernizeUseEqualsDelete
#pragma once


struct Ephere_OwnerContainer;

// Delete the pointer if not null, then delete the Ephere_OwnerContainer if needed
typedef void( *Ephere_DeleteOwnerFunctionType )( void const* pointer, struct Ephere_OwnerContainer* );

typedef struct Ephere_OwnerContainer
{
	Ephere_DeleteOwnerFunctionType Deleter;
} Ephere_OwnerContainer;


// These macros can be useful in C code, enable them if needed or just copy/paste the code
#define EPHERE_DECLARE_OWNED_POINTER( Type ) \
	typedef struct { \
		Type* Pointer; /* NOLINT(bugprone-macro-parentheses) */ \
		Ephere_OwnerContainer* Owner; \
	} Type##_OwnedPointer


#define EPHERE_DELETE_OWNED_POINTER( owned ) \
	do if( (owned).Owner != NULL ) {\
		(owned).Owner->Deleter( (owned).Pointer, (owned).Owner ); \
		(owned).Owner = NULL; } while ( 0 )


#ifdef __cplusplus

#include "Ephere/NativeTools/Asserts.h"

#include <memory>

namespace Ephere
{

// Used to create non-owning shared_ptr
struct NullDeleter
{
	void operator()( void const* ) const
	{
	}
};

// A wrapper of common stateless deleters, along with a set of the most common deleter functions
template <typename T>
struct FunctionDeleter
{
	typedef void( *FunctionType )( T* );

	// ReSharper disable CppNonExplicitConvertingConstructor

	FunctionDeleter( FunctionType deleter = nullptr )
		: deleter_( deleter )
	{
	}

	template <typename U>
	FunctionDeleter( FunctionDeleter<U> other )
		: deleter_( reinterpret_cast<FunctionType>( other.GetDeleter() ) )
	{
		static_assert( std::is_convertible<T*, U*>::value || std::is_convertible<U*, T*>::value, "T* must be compatible with U*" );
	}

	// ReSharper restore CppNonExplicitConvertingConstructor

	void operator()( T* pointer ) const
	{
		if( deleter_ != nullptr )
		{
			deleter_( pointer );
		}
	}

	EPHERE_NODISCARD bool operator==( std::nullptr_t ) const
	{
		return deleter_ == nullptr;
	}

	EPHERE_NODISCARD bool operator!=( std::nullptr_t ) const
	{
		return deleter_ != nullptr;
	}

	EPHERE_NODISCARD FunctionType GetDeleter() const
	{
		return deleter_;
	}

	static void Empty( T* )
	{
	}

	static void Free( T* pointer )
	{
		static_assert( sizeof( T ) != 0, "T must be complete" );
		std::free( pointer );
	}

	// ReSharper disable once CppParameterMayBeConstPtrOrRef
	static void Checked( T* pointer )
	{
		static_assert( sizeof( T ) != 0, "T must be complete" );
		delete pointer;
	}

	// ReSharper disable once CppParameterMayBeConstPtrOrRef
	static void CheckedArray( T* pointer )
	{
		static_assert( sizeof( T ) > 0, "T must be complete" );
		delete[] pointer;
	}

	template <typename Derived>
	static void Mixin( T* pointer )
	{
		static_assert( sizeof( Derived ) > 0, "Derived must be complete" );
		delete static_cast<Derived*>( pointer );
	}

private:

	FunctionType deleter_;
};


template <typename T>
struct StaticDefaultDeleteOwner : Ephere_OwnerContainer
{
	static StaticDefaultDeleteOwner* GetInstance()
	{
		static StaticDefaultDeleteOwner Instance;
		return &Instance;
	}

private:

	StaticDefaultDeleteOwner()  // NOLINT(cppcoreguidelines-pro-type-member-init)
	{
		Deleter = Delete;
	}

	static void Delete( void const* pointer, Ephere_OwnerContainer* )
	{
		delete static_cast<T const*>( pointer );
	}
};

template <typename T>
struct SharedOwner : Ephere_OwnerContainer
{
	static SharedOwner<T>* Create( std::shared_ptr<T> shared )
	{
		return new SharedOwner<T>( std::move( shared ) );
	}

private:

	// ReSharper disable once CppNonExplicitConvertingConstructor
	SharedOwner( std::shared_ptr<T> shared )
		: shared_( std::move( shared ) )
	{
		Deleter = Delete;
	}

	static void Delete( void const*, Ephere_OwnerContainer* owner )
	{
		delete static_cast<SharedOwner*>( owner );
	}

	std::shared_ptr<T> shared_;
};

template <typename T, class TDeleter>
struct DeleterOwner : Ephere_OwnerContainer
{
	static DeleterOwner<T, TDeleter>* Create( TDeleter deleter )
	{
		return new DeleterOwner<T, TDeleter>( std::move( deleter ) );
	}

protected:

	// ReSharper disable once CppNonExplicitConvertingConstructor
	DeleterOwner( TDeleter deleter )
		: deleter_( std::move( deleter ) )
	{
		Deleter = Delete;
	}

	static void Delete( void const* pointer, Ephere_OwnerContainer* owner )
	{
		auto* deleter = static_cast<DeleterOwner*>( owner );
		deleter->deleter_( const_cast<T*>( static_cast<T const*>( pointer ) ) );
		delete deleter;
	}

	TDeleter deleter_;
};

template <typename T>
class FunctionDeleterOwner : public DeleterOwner<T, typename FunctionDeleter<T>::FunctionType>
{
	// ReSharper disable once CppNonExplicitConvertingConstructor
	FunctionDeleterOwner( typename FunctionDeleter<T>::FunctionType deleter )
		: DeleterOwner<T, typename FunctionDeleter<T>::FunctionType>( deleter )
	{
	}
};


/* This class should be move-only (non-copyable), because the owner_ must be destroyed just once.
But the standard requires the deleter passed to shared_ptr to be CopyConstructible, even though that's not needed. Some implementations move the deleter on construction, but most
don't (especially older ones).
So we leave the class to be copyable, and we rely on shared_ptr calling operator() on the deleter exactly once, which with our Deleter implementation delets both the pointer and the owner.
*/
struct DeleterOwnerContainer
{
	explicit DeleterOwnerContainer( Ephere_OwnerContainer* owner )
		: owner_( owner )
	{
	}

	//DeleterOwnerContainer( DeleterOwnerContainer&& other ) EPHERE_NOEXCEPT
	//	: owner_( other.owner_ )
	//{
	//	other.owner_ = nullptr;
	//}

	void operator()( void const* pointer )
	{
		if( owner_ != nullptr )
		{
			owner_->Deleter( pointer, owner_ );
			owner_ = nullptr;
		}
	}

private:

	//DeleterOwnerContainer( DeleterOwnerContainer const& );
	//DeleterOwnerContainer& operator=( DeleterOwnerContainer const& );
	//DeleterOwnerContainer& operator=( DeleterOwnerContainer&& other ) EPHERE_NOEXCEPT;

	Ephere_OwnerContainer* owner_;
};

/*! Binary-compatible owning pointer similar to std::unique_ptr (moveable, not copyable). Uses Ephere_OwnerContainer as deleter.
Can pass object ownership across module boundaries, where STL smart pointers are not allowed. Use ToShared() to convert it to shared_ptr for internal use. */
template <typename T>
class UniquePtr
{
public:

	UniquePtr()
		: pointer_( nullptr )
		, owner_( nullptr )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	UniquePtr( std::nullptr_t )
		: pointer_( nullptr )
		, owner_( nullptr )
	{
	}

	explicit UniquePtr( T* pointer )
		: pointer_( pointer )
		, owner_( StaticDefaultDeleteOwner<T>::GetInstance() )
	{
		// UniquePtr may not hold a pointer to a mix-in base class in multiple inheritance, because the destructor will crash
		DEBUG_ONLY( ASSERT( static_cast<void const*>( this->get() ) == static_cast<void const*>( pointer ) ) );
	}

	UniquePtr( T* pointer, std::nullptr_t )
		: pointer_( pointer )
		, owner_( nullptr )
	{
	}

	UniquePtr( T* pointer, Ephere_OwnerContainer* owner )
		: pointer_( pointer )
		, owner_( owner )
	{
	}

	UniquePtr( T* pointer, typename FunctionDeleter<T>::FunctionType deleter )
		: pointer_( pointer )
		, owner_( FunctionDeleterOwner<T>::Create( deleter ) )
	{
	}

	UniquePtr( UniquePtr&& other ) EPHERE_NOEXCEPT
		: pointer_( other.pointer_ )
		, owner_( other.owner_ )
	{
		other.pointer_ = nullptr;
		other.owner_ = nullptr;
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <typename U>
	UniquePtr( UniquePtr<U>&& other )
		: pointer_( other.pointer_ )
		, owner_( other.owner_ )
	{
		// UniquePtr may not hold a pointer to a mix-in base class in multiple inheritance, because the destructor will crash
		DEBUG_ONLY( ASSERT( static_cast<const void*>( this->get() ) == static_cast<const void*>( other.get() ) || owner_ == nullptr ) );
		other.pointer_ = nullptr;
		other.owner_ = nullptr;
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <typename U, typename TDeleter>
	UniquePtr( std::unique_ptr<U, TDeleter>&& other )
		: pointer_( other.get() )
		, owner_( DeleterOwner<U, TDeleter>::Create( std::move( other.get_deleter() ) ) )
	{
		// UniquePtr may not hold a pointer to a mix-in base class in multiple inheritance, because the destructor will crash
		DEBUG_ONLY( ASSERT( static_cast<const void*>( this->get() ) == static_cast<const void*>( other.get() ) ) );
		other.release();
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	UniquePtr( std::shared_ptr<T> shared )
		: pointer_( shared.get() )
		, owner_( shared != nullptr ? SharedOwner<T>::Create( std::move( shared ) ) : nullptr )
	{
	}

	~UniquePtr()
	{
		if( owner_ != nullptr )
		{
			owner_->Deleter( pointer_, owner_ );
		}
	}

	template <typename U>
	static UniquePtr NotOwned( U* value )
	{
		return UniquePtr( value, nullptr );
	}

	template <typename U>
	UniquePtr& operator=( UniquePtr<U>&& other )
	{
		if( static_cast<void*>( this ) == static_cast<void*>( &other ) )
		{
			return *this;
		}

		if( this->get() != other.get() )
		{
			UniquePtr( std::move( other ) ).swap( *this );
		}
		else if( owner_ == nullptr )
		{
			owner_ = other.owner_;
			other.pointer_ = nullptr;
			other.owner_ = nullptr;
		}
		else if( other.owner_ != nullptr )
		{
			// It's an error to hold the same pointer with two non-empty deleters. We "deactivate" one of them to avoid the double destruction
			other.pointer_ = nullptr;
			other.owner_ = nullptr;
			ASSERT_FAIL();
		}

		return *this;
	}

	template <typename U, typename TDeleter>
	UniquePtr& operator=( std::unique_ptr<U, TDeleter>&& other )
	{
		UniquePtr( std::move( other ) ).swap( *this );
		return *this;
	}

	// In C++17 this is not needed, as you can always use != nullptr, even in if statements. But this class is supposed to be used in C++11 too.
	explicit operator bool() const EPHERE_NOEXCEPT
	{
		return pointer_ != nullptr;
	}

	EPHERE_NODISCARD T* get() const EPHERE_NOEXCEPT
	{
		return pointer_;
	}

	EPHERE_NODISCARD T* operator->() const EPHERE_NOEXCEPT
	{
		return get();
	}

	typename std::add_lvalue_reference<T>::type operator*() const
	{
		DEBUG_ONLY( ASSERT( pointer_ != nullptr ) );
		return *get();
	}

	void reset()
	{
		UniquePtr().swap( *this );
	}

	template <typename U>
	void reset( U* pointer )
	{
		if( pointer != this->get() )
		{
			UniquePtr( pointer ).swap( *this );
		}
	}

	T* release()
	{
		owner_ = nullptr;
		auto result = pointer_;
		pointer_ = nullptr;
		return result;
	}

	Ephere_OwnerContainer* ReleaseOwnership()
	{
		auto const result = owner_;
		owner_ = nullptr;
		return result;
	}

	void swap( UniquePtr& other ) EPHERE_NOEXCEPT
	{
		std::swap( pointer_, other.pointer_ );
		std::swap( owner_, other.owner_ );
	}

	EPHERE_NODISCARD Ephere_OwnerContainer* get_deleter() const EPHERE_NOEXCEPT
	{
		return owner_;
	}

	std::shared_ptr<T> ToShared()
	{
		if( pointer_ == nullptr )
		{
			return std::shared_ptr<T>();
		}

		if( auto const owner = ReleaseOwnership() )
		{
			return std::shared_ptr<T>( pointer_, DeleterOwnerContainer( owner ) );
		}

		return std::shared_ptr<T>( pointer_, NullDeleter() );
	}

private:

	template <typename U>
	friend class UniquePtr;

	T* pointer_;
	Ephere_OwnerContainer* owner_;
};

template< class T1, class T2>
bool operator==( UniquePtr<T1> const& x, UniquePtr<T2> const& y ) EPHERE_NOEXCEPT
{
	return x.get() == y.get();
}

template< class T1, class T2>
bool operator!=( UniquePtr<T1> const& x, UniquePtr<T2> const& y ) EPHERE_NOEXCEPT
{
	return x.get() != y.get();
}

template< class T1, class T2>
bool operator<( UniquePtr<T1> const& x, UniquePtr<T2> const& y ) EPHERE_NOEXCEPT
{
	return x.get() < y.get();
}

template< class T1, class T2>
bool operator<=( UniquePtr<T1> const& x, UniquePtr<T2> const& y ) EPHERE_NOEXCEPT
{
	return x.get() <= y.get();
}

template< class T1, class T2>
bool operator>( UniquePtr<T1> const& x, UniquePtr<T2> const& y ) EPHERE_NOEXCEPT
{
	return x.get() > y.get();
}

template< class T1, class T2>
bool operator>=( UniquePtr<T1> const& x, UniquePtr<T2> const& y ) EPHERE_NOEXCEPT
{
	return x.get() >= y.get();
}

template <typename T>
bool operator==( UniquePtr<T> const& left, std::nullptr_t ) EPHERE_NOEXCEPT
{
	return left.get() == nullptr;
}

template <typename T>
bool operator==( std::nullptr_t, UniquePtr<T> const& right ) EPHERE_NOEXCEPT
{
	return right.get() == nullptr;
}

template <typename T>
bool operator!=( UniquePtr<T> const& left, std::nullptr_t ) EPHERE_NOEXCEPT
{
	return left.get() != nullptr;
}

template <typename T>
bool operator!=( std::nullptr_t, UniquePtr<T> const& right ) EPHERE_NOEXCEPT
{
	return right.get() != nullptr;
}

template <typename T>
UniquePtr<T> MakeUniquePtrNotOwning( T* pointer )
{
	return UniquePtr<T>::NotOwned( pointer );
}

#ifdef EPHERE_HAVE_CPP11

template <class T, class... TArgs, typename std::enable_if<!std::is_array<T>::value, int>::type = 0>
EPHERE_NODISCARD UniquePtr<T> MakeUniquePtr( TArgs&&... args )
{
	return UniquePtr<T>( new T( std::forward<TArgs>( args )... ) );
}

#endif

template <typename TDerived, typename TBase>
std::unique_ptr<TDerived> static_unique_cast( std::unique_ptr<TBase>&& base )
{
	return std::unique_ptr<TDerived>( static_cast<TDerived*>( base.release() ) );
}

template <typename TDerived, typename TBase>
UniquePtr<TDerived> static_unique_cast( UniquePtr<TBase>&& base )
{
	auto owner = base.ReleaseOwnership();
	auto other = base.release();
	// Can't let UniquePtr keep a pointer to a mix-in base class in multiple inheritance, as the destructor will break
	ASSERT( static_cast<void const*>( static_cast<TDerived*>( other ) ) == static_cast<void const*>( other ) );
	return UniquePtr<TDerived>( static_cast<TDerived*>( other ), owner );
}


struct OwnerContainerCopyable : Ephere_OwnerContainer
{
	//void ( *ThisDeleter )( OwnerContainerCopyable* );
	OwnerContainerCopyable* ( *Cloner )( OwnerContainerCopyable*, void const* pointer );
};


template <typename T>
struct SharedOwnerCopyable : OwnerContainerCopyable
{
	explicit SharedOwnerCopyable( std::shared_ptr<T> shared )
		: shared_( std::move( shared ) )
	{
		Deleter = Delete;
		Cloner = Clone;
	}

private:

	static void Delete( void const*, Ephere_OwnerContainer* owner )
	{
		delete static_cast<SharedOwnerCopyable*>( owner );
	}

	static OwnerContainerCopyable* Clone( OwnerContainerCopyable* owner, void const* )
	{
		return new SharedOwnerCopyable( *static_cast<SharedOwnerCopyable*>( owner ) );
	}

	std::shared_ptr<T> shared_;
};

/*! Used to pass object ownership across module boundaries, where STL smart pointers are not allowed. Similar to std::shared_ptr: copying shares the ownership, but is not thread-safe.
Do not use directly, use ToShared() to convert it to shared_ptr for internal use. */
template <typename T>
class SharedPtr
{
public:

	SharedPtr()
		: pointer_( nullptr ),
		owner_( nullptr )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	SharedPtr( std::nullptr_t ) EPHERE_NOEXCEPT
		: pointer_( nullptr ),
		owner_( nullptr )
	{
	}

	//! Takes ownership over a raw pointer
	explicit SharedPtr( T* pointer )
		: pointer_( pointer )
		, owner_( pointer != nullptr ? new SharedOwnerCopyable<T>( std::shared_ptr<T>( pointer ) ) : nullptr )
	{
	}

	//! Takes ownership over a raw pointer
	template <class TOther>
	explicit SharedPtr( TOther* pointer )
		: pointer_( pointer )
		, owner_( pointer != nullptr ? new SharedOwnerCopyable<TOther>( std::shared_ptr<TOther>( pointer ) ) : nullptr )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	template <typename TOther>
	SharedPtr( SharedPtr<TOther> const& other )
		: pointer_( other.get() )
		, owner_( other.owner_ != nullptr ? other.owner_->Cloner( other.owner_, other.pointer_ ) : nullptr )
	{
	}

	//! Wraps a raw pointer without owning it
	template <class TOther>
	static SharedPtr NotOwned( TOther* pointer )
	{
		return SharedPtr( pointer, nullptr );
	}

	//! Creates a default-constructed object
	static SharedPtr DefaultConstruct()
	{
		return SharedPtr( new T() );
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	SharedPtr( std::shared_ptr<T> const& shared )
		: pointer_( shared.get() )
		, owner_( shared != nullptr ? new SharedOwnerCopyable<T>( shared ) : nullptr )
	{
	}

	// ReSharper disable once CppNonExplicitConvertingConstructor
	SharedPtr( std::unique_ptr<T>&& pointer )
		: pointer_( pointer.release() )
		, owner_( pointer_ != nullptr ? new SharedOwnerCopyable<T>( pointer_ ) : nullptr )
	{
	}

	SharedPtr( SharedPtr const& other )
		: pointer_( other.get() ),
		owner_( other.owner_ != nullptr ? other.owner_->Cloner( other.owner_, other.pointer_ ) : nullptr )
	{
	}

	SharedPtr( SharedPtr&& other ) EPHERE_NOEXCEPT
		: pointer_( other.get() ),
		owner_( other.ReleaseOwnership() )
	{
	}

	SharedPtr& operator=( SharedPtr const& other )  // NOLINT(bugprone-unhandled-self-assignment)
	{
		if( pointer_ != other.get() )
		{
			Destroy();
			pointer_ = other.get();
			owner_ = other.owner_ != nullptr ? other.owner_->Cloner( other.owner_, other.pointer_ ) : nullptr;
		}

		return *this;
	}

	template <typename TOther>
	SharedPtr& operator=( SharedPtr<TOther> const& other )  // NOLINT(bugprone-unhandled-self-assignment)
	{
		SharedPtr( other ).swap( *this );
		return *this;
	}

	SharedPtr& operator=( SharedPtr&& other ) EPHERE_NOEXCEPT
	{
		if( pointer_ != other.get() )
		{
			Destroy();
			pointer_ = other.get();
			owner_ = other.ReleaseOwnership();
		}

		return *this;
	}

	~SharedPtr()
	{
		Destroy();
	}

	T* get() const
	{
		return pointer_;
	}

	T* operator ->() const
	{
		return get();
	}

	void reset()
	{
		SharedPtr().swap( *this );
	}

	typename std::add_lvalue_reference<T>::type operator*() const
	{
		// TODO: Assert
		return *pointer_;
	}

	bool operator==( std::nullptr_t ) const
	{
		return pointer_ == nullptr;
	}

	bool operator!=( std::nullptr_t ) const
	{
		return pointer_ != nullptr;
	}

	explicit operator bool() const
	{
		return pointer_ != nullptr;
	}

	//operator std::shared_ptr<T>() const
	//{
	//	return ToShared();
	//}

	void swap( SharedPtr& other ) EPHERE_NOEXCEPT
	{
		std::swap( pointer_, other.pointer_ );
		std::swap( owner_, other.owner_ );
	}

	OwnerContainerCopyable* ReleaseOwnership()
	{
		auto const result = owner_;
		owner_ = nullptr;
		return result;
	}

	// Converts this pointer to a shared_ptr
	std::shared_ptr<T> ToShared() const
	{
		if( pointer_ == nullptr )
		{
			return std::shared_ptr<T>();
		}

		if( auto newOwner = owner_ != nullptr ? owner_->Cloner( owner_, pointer_ ) : nullptr )
		{
			return std::shared_ptr<T>( pointer_, [newOwner]( T* pointer )
			{
				newOwner->Deleter( pointer, newOwner );
			} );
		}

		return std::shared_ptr<T>( pointer_, NullDeleter() );
	}

private:

	template <typename TOther>
	friend class SharedPtr;

	SharedPtr( T* pointer, OwnerContainerCopyable* owner )
		: pointer_( pointer ),
		owner_( owner )
	{
	}

	void Destroy() const
	{
		if( owner_ != nullptr )
		{
			owner_->Deleter( pointer_, owner_ );

			// No need to set owner_ to nullptr, owner_ is never used after Destroy()
		}
	}

	T* pointer_;
	OwnerContainerCopyable* owner_;
};

template <typename T1, typename T2>
EPHERE_NODISCARD bool operator==( SharedPtr<T1> const& left, SharedPtr<T2> const& right ) EPHERE_NOEXCEPT
{
	return left.get() == right.get();
}

template <typename T1, typename T2>
EPHERE_NODISCARD bool operator==( SharedPtr<T1> const& left, T2 const* right ) EPHERE_NOEXCEPT
{
	return left.get() == right;
}

template <typename T1, typename T2>
EPHERE_NODISCARD bool operator==( T1 const* left, SharedPtr<T2> const& right ) EPHERE_NOEXCEPT
{
	return left == right.get();
}

template <typename T1, typename T2>
EPHERE_NODISCARD bool operator!=( SharedPtr<T1> const& left, SharedPtr<T2> const& right ) EPHERE_NOEXCEPT
{
	return !( left == right );
}

template <typename T1, typename T2>
EPHERE_NODISCARD bool operator!=( SharedPtr<T1> const& left, T2 const* right ) EPHERE_NOEXCEPT
{
	return !( left == right );
}

template <typename T1, typename T2>
EPHERE_NODISCARD bool operator!=( T1 const* left, SharedPtr<T2> const& right ) EPHERE_NOEXCEPT
{
	return !( left == right );
}

template <typename T1, typename T2>
EPHERE_NODISCARD bool operator<( SharedPtr<T1> const& left, T2 const* right ) EPHERE_NOEXCEPT
{
	return left.get() < right;
}

template <typename T1, typename T2>
EPHERE_NODISCARD bool operator<( T1 const* left, SharedPtr<T2> const& right ) EPHERE_NOEXCEPT
{
	return left < right.get();
}


template <typename T>
std::shared_ptr<T> MakeSharedPtrNotOwned( T* pointer )
{
	return std::shared_ptr<T>( pointer, NullDeleter() );
}

template <typename T>
std::shared_ptr<T> MakeSharedPtrNotOwned( std::unique_ptr<T> const& pointer )
{
	return std::shared_ptr<T>( pointer.get(), NullDeleter() );
}

}

#endif
