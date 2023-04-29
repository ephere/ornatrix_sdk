#pragma once

#include "Ephere/NativeTools/ChunkStreams.h"
#include "Ephere/Ornatrix/Groom/IOperator.h"

namespace Ephere::Ornatrix::Groom::Operators
{
struct SomeData
{
	float x;
	std::vector<int> v;

	bool operator==( SomeData const& other ) const
	{
		return x == other.x && v == other.v;
	}

	bool operator!=( SomeData const& other ) const
	{
		return !( *this == other );
	}
};
}

namespace Ephere::Parameters
{
template <>
struct Type<Ornatrix::Groom::Operators::SomeData> final : TypeBase<Ornatrix::Groom::Operators::SomeData>
{
	enum : int { SerializeId = 999999 };

	int GetSerializeId() const override
	{
		return SerializeId;
	}

	bool WriteImpl( IOutputStream& buffer, Ornatrix::Groom::Operators::SomeData const& value ) const override
	{
		auto chunkStream = ChunkOutputStreamStl::Create( buffer );
		return
			Ephere::Write( *chunkStream, 1, value.x )
			&& Ephere::Write( *chunkStream, 2, value.v );
	}

	bool ReadImpl( IInputStream& buffer, Ornatrix::Groom::Operators::SomeData& value ) const override
	{
		auto chunkStream = ChunkInputStreamStl::Create( buffer );
		auto result = true;
		while( auto chunk = chunkStream->OpenChunk() )
		{
			switch( chunk->Id() )
			{
				case 1: result = result && Ephere::Read( *chunk, value.x ); break;
				case 2: result = result && Ephere::Read( *chunk, value.v ); break;
				default: return false;
			}
		}

		return result;
	}
};
}

#include "TestParametersImpl.g.h"

namespace Ephere::Ornatrix::Groom::Operators
{
struct TestOperator final : IOperator
{
	TestParameters::Container parameters;

	Parameters::IContainer const& GetParameterSet( int index ) const override
	{
		return index == 0 ? parameters : Parameters::Container::GetEmpty();
	}

	bool Apply( EvaluationContext& ) override
	{
		return !parameters.examples.targetHairExample().IsEmpty();
	}
};

extern OperatorDescriptor const* Test;

}
