#include "PrecompiledHeaders.h"

#include "Ephere/NativeTools/Test/Catch.h"

#include "Ephere/NativeTools/Any.h" // for `any_cast`
#include <cstddef> // for `offsetof`
#include <sstream> // for `ostringstream`
#include <string>
#include <typeindex>
#include <unordered_map>

#include "Ephere/Core/Params2/Declaration/Templates.h"


namespace Ephere::Test::Declaration
{
using namespace std;
using namespace Params::Declaration;

// Helper functions
// --------------------------------------------------

using Str = string;

void ToStrImpl
(
	ostringstream& res,
	BasicTypeInfo const& basicTypeInfo,
	void const* const ptr,
	unordered_map<type_index, CompositeTypeInfo const*> const& compositeMap
)
{
	if( auto const compositeIter = compositeMap.find( basicTypeInfo.stdTypeInfo );
		compositeIter != compositeMap.end() )
	{
		res << "{ ";
		auto firstField = true;
		for( auto const& field : compositeIter->second->fieldInfos )
		{
			if( !firstField )
			{
				res << ", ";
			}

			res << field.name << ": ";

			firstField = false;
			auto const fieldPtr = static_cast<char const*>( ptr ) + field.offset;
			ToStrImpl( res, field.basicTypeInfo, fieldPtr, compositeMap );
		}

		res << " }";
	}
	else
	{
		if( basicTypeInfo.stdTypeInfo == typeid( int ) )
		{
			res << *static_cast<int const*>( ptr );
		}
		else if( basicTypeInfo.stdTypeInfo == typeid( float ) )
		{
			res << *static_cast<float const*>( ptr );
		}
		else
		{
			FAIL( "Unexpected basic type" );
		}
	}
}

template<typename T>
string ToStr( T const& val, vector<CompositeTypeInfo> const& composites )
{
	unordered_map<type_index, CompositeTypeInfo const*> compositeMap;
	for( auto const& composite : composites )
	{
		compositeMap[composite.basicTypeInfo.stdTypeInfo] = &composite;
	}

	ostringstream res;
	ToStrImpl( res, compositeMap.at( typeid( T ) )->basicTypeInfo, &val, compositeMap );
	return res.str();
}


// Test basic params declaration and registration
// --------------------------------------------------

EPHERE_PARAMS_TEMPLATE
struct BasicParamsTemplate
{
	Type<int> myInt = Value{ "myInt" }( 11 );
	Type<float> myFloat = Value{ "myFloat" }( 2.2f );
};

using BasicParams = Instantiate<BasicParamsTemplate>;

TEST_CASE( "Params.Declaration.Basic" )
{
	SECTION( "UserStructDefaults" )
	{
		BasicParams params;
		REQUIRE( params.myInt == 11 );
		REQUIRE( params.myFloat == 2.2f );
	}

	SECTION( "RegistrationTemplates" )
	{
		FieldRegistration::Begin();
		RegisterTemplate<BasicParamsTemplate>();
		auto const composite = FieldRegistration::End();
		{
			auto const& basic = composite.basicTypeInfo;
			REQUIRE( basic.stdTypeInfo == typeid( BasicParams ) );
			REQUIRE( basic.size == sizeof( BasicParams ) );
		}

		REQUIRE( composite.fieldInfos.size() == 2 );
		{
			auto const& field = composite.fieldInfos[0];
			REQUIRE( field.basicTypeInfo.stdTypeInfo == typeid( int ) );
			REQUIRE( field.basicTypeInfo.size == sizeof( int ) );
			REQUIRE( field.name == Str( "myInt" ) );
			REQUIRE( field.offset == offsetof( BasicParams, myInt ) );
			REQUIRE( any_cast<int>( field.defaultValue ) == 11 );
		}
		{
			auto const& field = composite.fieldInfos[1];
			REQUIRE( field.basicTypeInfo.stdTypeInfo == typeid( float ) );
			REQUIRE( field.basicTypeInfo.size == sizeof( float ) );
			REQUIRE( field.name == Str( "myFloat" ) );
			REQUIRE( field.offset == offsetof( BasicParams, myFloat ) );
			REQUIRE( any_cast<float>( field.defaultValue ) == 2.2f );
		}

		BasicParams params;
		REQUIRE( ToStr( params, { composite } ) == "{ myInt: 11, myFloat: 2.2 }" );

		params.myInt *= 2;
		params.myFloat += 3.5f;
		REQUIRE( ToStr( params, { composite } ) == "{ myInt: 22, myFloat: 5.7 }" );
	}
}


// Test composite params declaration and registration
// --------------------------------------------------

EPHERE_PARAMS_TEMPLATE
struct ValuePairTemplate
{
	Type<float> val1 = Value{ "val1" }( 1.0f );
	Type<float> val2 = Value{ "val2" }( 2.0f );

	bool operator==( ValuePairTemplate const& other ) const
	{
		return val1 == other.val1 && val2 == other.val2;
	}
};

using ValuePair = Instantiate<ValuePairTemplate>;

EPHERE_PARAMS_TEMPLATE
struct CompositeParamsTemplate
{
	Type<ValuePair> valPair = Value{ "valPair" }( ValuePair{ 1.1f } );
};

using CompositeParams = Instantiate<CompositeParamsTemplate>;

TEST_CASE( "Params.Declaration.Composite" )
{
	SECTION( "UserStructDefaults" )
	{
		CompositeParams params;
		REQUIRE( params.valPair.val1 == 1.1f );
		REQUIRE( params.valPair.val2 == 2.0f );
	}

	SECTION( "RegistrationTemplates" )
	{
		FieldRegistration::Begin();
		RegisterTemplate<ValuePairTemplate>();
		auto const valuePairComposite = FieldRegistration::End();

		FieldRegistration::Begin();
		RegisterTemplate<CompositeParamsTemplate>();
		auto const paramsComposite = FieldRegistration::End();

		{
			auto const& composite = valuePairComposite;
			{
				auto const& basic = composite.basicTypeInfo;
				REQUIRE( basic.stdTypeInfo == typeid( ValuePair ) );
				REQUIRE( basic.size == sizeof( ValuePair ) );
			}

			REQUIRE( composite.fieldInfos.size() == 2 );
			{
				auto const& field = composite.fieldInfos[0];
				REQUIRE( field.basicTypeInfo.stdTypeInfo == typeid( float ) );
				REQUIRE( field.basicTypeInfo.size == sizeof( float ) );
				REQUIRE( field.name == Str( "val1" ) );
				REQUIRE( field.offset == offsetof( ValuePair, val1 ) );
				REQUIRE( any_cast<float>( field.defaultValue ) == 1.0f );
			}
			{
				auto const& field = composite.fieldInfos[1];
				REQUIRE( field.basicTypeInfo.stdTypeInfo == typeid( float ) );
				REQUIRE( field.basicTypeInfo.size == sizeof( float ) );
				REQUIRE( field.name == Str( "val2" ) );
				REQUIRE( field.offset == offsetof( ValuePair, val2 ) );
				REQUIRE( any_cast<float>( field.defaultValue ) == 2.0f );
			}
		}
		{
			auto const& composite = paramsComposite;
			{
				auto const& basic = composite.basicTypeInfo;
				REQUIRE( basic.stdTypeInfo == typeid( CompositeParams ) );
				REQUIRE( basic.size == sizeof( CompositeParams ) );
			}

			REQUIRE( composite.fieldInfos.size() == 1 );

			auto const& field = composite.fieldInfos[0];
			REQUIRE( field.basicTypeInfo.stdTypeInfo == typeid( ValuePair ) );
			REQUIRE( &field.basicTypeInfo == &valuePairComposite.basicTypeInfo );
			REQUIRE( field.name == Str( "valPair" ) );
			REQUIRE( field.offset == offsetof( CompositeParams, valPair ) );
			REQUIRE( any_cast<ValuePair>( field.defaultValue ) == ValuePair{ 1.1f } );
		}

		vector<CompositeTypeInfo> const composites = { valuePairComposite, paramsComposite };

		CompositeParams params;
		REQUIRE( ToStr( params, composites ) == "{ valPair: { val1: 1.1, val2: 2 } }" );

		params.valPair.val1 /= 2.0f;
		params.valPair.val2 -= 0.1f;
		REQUIRE( ToStr( params, composites ) == "{ valPair: { val1: 0.55, val2: 1.9 } }" );
	}
}

}
