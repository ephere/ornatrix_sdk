#pragma once

#include "Matrix.h"

#include <algorithm>

namespace Ephere { namespace Geometry
{

//! Axis-aligned box
template <unsigned N, typename T>
class Box
{
	typedef	Matrix<N,1,T>	Vector;

public:

	//! No initialization
	Box()
	{
	}

	Box( const Vector& pmin, const Vector& pmax ) :
		pmin_( pmin ),
		pmax_( pmax )
	{
	}


	/**
	Construct a box centered at a point with specified dimensions
	@param diameter If true dimensions are used as diamater, otherwise radius
	*/
	Box( const Vector& pcenter, const Vector& dim, bool diameter ) :
		pmin_( pcenter - ( diameter ? ( dim / T( 2 ) ) : ( dim ) ) ),
		pmax_( pcenter + ( diameter ? ( dim / T( 2 ) ) : ( dim ) ) )
	{
	}

	static Box Empty()
	{
		Box<N, T> result;
		result.setEmpty();
		return result;
	}

	void operator+=( const Vector& p )
	{
		for( unsigned i = 0; i < N; ++i )
		{
			pmin()[i] = std::min( pmin()[i], p[i] );
			pmax()[i] = std::max( pmax()[i], p[i] );
		}
	}

	bool isEmpty() const
	{
		for( unsigned i = 0; i < N; i++ )
		{
			if( pmin()[i] >= pmax()[i] )
			{
				return true;
			}
		}
		return false;
	}

	void operator+=( const Box& b )
	{
		if( b.isEmpty() )
		{
			return;
		}

		*this += b.pmin();
		*this += b.pmax();
	}

	const Vector& pmin() const
	{
		return pmin_;
	}

	Vector& pmin()
	{
		return pmin_;
	}

	const Vector& pmax() const
	{
		return pmax_;
	}

	Vector& pmax()
	{
		return pmax_;
	}

	void enlarge( const Matrix<N*2,1,T>& sides )
	{
		for( unsigned i=0; i<N; i++)
		{
			pmin()[i] -= sides;
			pmax()[i] += sides;
		}
	}

private:

	Vector pmin_;
	Vector pmax_;
};

typedef Box<3, float> Box3f;

template <unsigned N, unsigned M, typename T>
Box<N,T> operator* ( const Matrix<N,M,T>&, const Box<N,T>& );

template <unsigned N, unsigned M, typename T>
Box<N,T> operator* ( const Box<N,T>&, const Matrix<N,M,T>& );

template <unsigned N, typename T>
Box<N,T> operator+ ( const Box<N,T>&, const Box<N,T>& );

template <unsigned N, typename T>
Box<N,T> operator* ( const Box<N,T>&, const Box<N,T>& );

} }
