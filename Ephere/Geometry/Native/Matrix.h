#pragma once

#include <cmath>
#include <ostream>

namespace Ephere { namespace Geometry
{

//! Floating precision constant
const float Epsilon = 0.00001f;

// A very simple Matrix/Vector class. N - rows, M - columns. Row-major storage
template <unsigned N, unsigned M, typename T>
class Matrix
{
public:

	Matrix()
	{
	}

	Matrix( T xv, T yv )
	{
		static_assert( N == 2 && M == 1, "Wrong matrix size" );
		x() = xv;
		y() = yv;
	}

	Matrix( T xv, T yv, T zv )
	{
		static_assert( N == 3 && M == 1, "Wrong matrix size for this constructor" );
		x() = xv;
		y() = yv;
		z() = zv;
	}

	T x() const
	{
		static_assert( M == 1, "Not a vector" );
		return data_[0];
	}

	T y() const
	{
		static_assert( M == 1, "Not a vector" );
		static_assert( N >= 2, "No y" );
		return data_[1];
	}

	T z() const
	{
		static_assert( M == 1, "Not a vector" );
		static_assert( N >= 3, "No z" );
		return data_[2];
	}

	T& x()
	{
		static_assert( M == 1, "Not a vector" );
		return data_[0];
	}

	T& y()
	{
		static_assert( M == 1, "Not a vector" );
		static_assert( N >= 2, "No y" );
		return data_[1];
	}

	T& z()
	{
		static_assert( M == 1, "Not a vector" );
		static_assert( N >= 3, "No z" );
		return data_[2];
	}

	T operator[]( unsigned i ) const
	{
		static_assert( M == 1, "Not a vector" );
		return data_[i];
	}

	T& operator[](unsigned i)
	{
		static_assert( M == 1, "Not a vector" );
		return data_[i];
	}

	T operator()( unsigned rowIndex, unsigned columnIndex ) const
	{
		// assert( rowIndex < N );
		// assert( columnIndex < M );
		return data_[columnIndex * N + rowIndex];
	}

	T& operator()( unsigned rowIndex, unsigned columnIndex )
	{
		// assert( rowIndex < N );
		// assert( columnIndex < M );
		return data_[columnIndex * N + rowIndex];
	}

	Matrix& operator +=( const Matrix& m )
	{
		for( auto i = 0u; i < N * M; i++ )
		{
			data_[i] += m.data_[i];
		}

		return *this;
	}

	Matrix& operator -=( const Matrix& m )
	{
		for( auto i = 0u; i < N * M; i++ )
		{
			data_[i] -= m.data_[i];
		}

		return *this;
	}

	Matrix& operator*=( T value )
	{
		for( unsigned i = 0; i < N*M; i++ )
		{
			data_[i] *= value;
		}

		return *this;
	}

	Matrix& operator/=( T value )
	{
		for( unsigned i = 0; i < N*M; i++ )
		{
			data_[i] /= value;
		}

		return *this;
	}

	const Matrix<N, 1, T>& column( unsigned i ) const
	{
		// assert( i < M );
		return reinterpret_cast<const Matrix<N, 1, T>&>( data_[i*N] );
	}

	Matrix<M - 1, 1, T>	scale() const
	{
		Matrix<M - 1, 1, T> result;
		for( auto i = 0u; i < M - 1; ++i )
		{
			result[i] = column( i ).length();
		}

		return result;
	}

	T length() const
	{
		static_assert( M == 1, "Not a vector" );
		auto result = 0.0;
		for ( unsigned i = 0; i < N; ++i )
		{
			result += data_[i] * data_[i];
		}

		return T( std::sqrt( result ) );
	}

	bool HasNaNElements() const
	{
		for( auto i = 0u; i < N * M; i++ )
		{
			if( data_[i] != data_[i] ) // nan
			{
				return true;
			}
		}

		return false;
	}

	static Matrix Zero()
	{
		Matrix result = Matrix();
		for( int i = 0; i < N * M; ++i )
		{
			result.data_[i] = 0;
		}

		return result;
	}

	static Matrix Identity()
	{
		Matrix result = Matrix();
		for( int i = 0; i < N; ++i )
		{
			result.data_[i * N + i] = T( 1 );
		}

		return result;
	}

	friend bool operator==( const Matrix& m, const Matrix& n )
	{
		for( unsigned i = 0; i < N * M; ++i )
		{
			if( m.data_[i] != n.data_[i] )
			{
				return false;
			}
		}

		return true;
	}

private:

	T data_[N * M];
};


typedef Matrix<3, 1, float> TextureCoordinate;

typedef Matrix<2, 1, float> Vector2f;
typedef Matrix<3, 1, float> Vector3f;
typedef Matrix<3, 1, double> Vector3d;

typedef Matrix<3, 4, float> Xform3f;
typedef Matrix<3, 4, double> Xform3d;

typedef Vector3f TextureCoordinatef;
typedef Vector3d TextureCoordinated;


template <unsigned N, unsigned M, typename T>
Matrix<N, M, T> operator +( const Matrix<N, M, T> & m, const Matrix<N, M, T>& n )
{
	return Matrix<N, M, T>( m ) += n;
}

template <unsigned N, unsigned M, typename T>
Matrix<N, M, T> operator -( const Matrix<N, M, T> & m, const Matrix<N, M, T>& n )
{
	return Matrix<N, M, T>( m ) -= n;
}

template <typename T>
Matrix<3, 1, T> operator*( const Matrix<3, 4, T> & m, const Matrix<3, 1, T> & n )
{
	Matrix<3, 1, T> result;
	result[0] = m( 0, 0 ) * n[0] + m( 0, 1 ) * n[1] + m( 0, 2 ) * n[2] + m( 0, 3 );
	result[1] = m( 1, 0 ) * n[0] + m( 1, 1 ) * n[1] + m( 1, 2 ) * n[2] + m( 1, 3 );
	result[2] = m( 2, 0 ) * n[0] + m( 2, 1 ) * n[1] + m( 2, 2 ) * n[2] + m( 2, 3 );
	return result;
}

template <typename T>
Matrix<3, 1, T> operator*( const Matrix<3, 1, T> & n, const Matrix<3, 4, T> & m )
{
	return m * n;
}

template <unsigned N, unsigned M, typename T>
Matrix<N, M, T> operator*( const Matrix<N, M, T>& m, T value )
{
	return Matrix<N, M, T>( m ) *= value;
}

template <unsigned N, unsigned M, typename T>
Matrix<N, M, T> operator/( const Matrix<N, M, T>& m, T value )
{
	return Matrix<N, M, T>( m ) /= value;
}

template <unsigned N, unsigned M, typename T>
bool operator!=( const Matrix<N, M, T> & m, const Matrix<N, M, T> & n )
{
	return !( m == n );
}

template <typename C, unsigned N, unsigned M, typename T>
std::basic_ostream<C>& operator << ( std::basic_ostream<C>& stream, Matrix<N, M, T> const& matrix )
{
	auto first = true;
	stream << "[ ";
	for( unsigned j = 0; j < M; ++j )
	{
		if( first )
		{
			first = false;
		}
		else
		{
			stream << "| ";
		}

		for( unsigned i = 0; i < N; ++i )
		{
			stream << matrix( i, j ) << ' ';
		}
	}

	stream << ']';
	return stream;
}

} }
