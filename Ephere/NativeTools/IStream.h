// Must compile with VC 2012 / GCC 4.8

#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include <iostream>

namespace Ephere
{
struct IStream
{
	virtual ~IStream()
	{
	}

	virtual bool IsOk() const = 0;

	virtual bool Eof() const = 0;

	virtual int GetPosition() const = 0;

	virtual void Seek( int position ) = 0;
};

struct IInputStream : IStream
{
	virtual void Read( void* buffer, int length ) = 0;

	template <typename T>
	bool Read( T& data )
	{
		Read( reinterpret_cast<char*>( &data ), sizeof( data ) );
		return IsOk();
	}
};

struct IOutputStream : IStream
{
	virtual void Write( void const* buffer, int length ) = 0;

	template <typename T>
	bool Write( T const& data )
	{
		Write( reinterpret_cast<char const*>( &data ), sizeof( data ) );
		return IsOk();
	}

#if 0 // TODO?
	template <typename T>
	bool Write( Span<const T> data )
	{
		Write( reinterpret_cast<const char*>( data.data() ), data.size_bytes() );
		return IsOk();
	}
#endif
};

template <class TBase, class TStream>
class StlStream : public TBase
{
public:

	StlStream( TStream& stream )
		: stream_( &stream )
	{
	}

	bool IsOk() const override
	{
		return !stream_->fail();
	}

	bool Eof() const override
	{
		return stream_->eof();
	}

protected:

	TStream* stream_;
};

class StlInputStream : public StlStream<IInputStream, std::istream>
{
	typedef StlStream<IInputStream, std::istream> BaseType;

public:

	StlInputStream( std::istream& stream )
		: BaseType( stream )
	{
	}

	int GetPosition() const override
	{
		return static_cast<int>( stream_->tellg() );
	}

	void Seek( int position ) override
	{
		stream_->seekg( position );
	}

	void Read( void* buffer, int length ) override
	{
		stream_->read( static_cast<char*>( buffer ), length );
	}
};

class StlOutputStream : public StlStream<IOutputStream, std::ostream>
{
	typedef StlStream<IOutputStream, std::ostream> BaseType;

public:

	StlOutputStream( std::ostream& stream )
		: BaseType( stream )
	{
	}

	int GetPosition() const override
	{
		return static_cast<int>( stream_->tellp() );
	}

	void Seek( int position ) override
	{
		stream_->seekp( position );
	}

	void Write( void const* buffer, int length ) override
	{
		stream_->write( static_cast<char const*>( buffer ), length );
	}
};

}
