// Must compile with VC 2012 / GCC 4.8

// ReSharper disable CppClangTidyModernizeUseEqualsDefault
// ReSharper disable CppClangTidyModernizeReturnBracedInitList
// ReSharper disable CppRedundantExplicitTemplateArguments
#pragma once

#include "Ephere/Core/Parameters/String.h"
#include "Ephere/NativeTools/StlExtensions.h"

namespace Ephere { namespace Moov
{
class Solver;
class CaptureHandle;
} }

namespace Ephere { namespace Ornatrix
{

class MoovHairSimulator;

namespace Groom
{
struct IGraph;
}


struct PythonParameter
{
	enum class Type
	{
		Int, Float, Bool, String, DataGenerator, Enum, ChannelSelector, RampCurve, Time, Group, Separator, Invalid
	};

	Type type = Type::Invalid;
	Parameters::String name;
};

struct MoovDataGeneratorContainer;

struct RampCurveContainer;


struct IPythonContext
{
	virtual ~IPythonContext()
	{
	}

	virtual bool InitFromPythonScript( char const* script, wchar_t const* fileName = L"", wchar_t const* additionalDirectory = L"" ) = 0;

	virtual void Invalidate() = 0;

	EPHERE_NODISCARD virtual bool IsValid() const = 0;
};

template <class TParameter = PythonParameter>
struct ParameterIterator;

struct IPythonContextWithParameters : IPythonContext
{
	virtual void CreateParametersFromPythonTuple( Parameters::String* paramTypesList = nullptr ) = 0;

	EPHERE_NODISCARD virtual int GetParametersCount() const = 0;

	virtual PythonParameter& GetParameter( int index ) = 0;

	EPHERE_NODISCARD virtual int FindParameterIndex( char const* attrName ) const = 0;

	virtual bool SaveParameterValues( char const* filePath ) const = 0;

	virtual bool LoadParameterValues( char const* filePath ) = 0;

	Iterable<ParameterIterator<>> GetParameters();
};

template <class TParameter>
struct ParameterIterator
{
	typedef std::forward_iterator_tag iterator_category;
	typedef TParameter value_type;
	typedef value_type* pointer;
	typedef value_type& reference;
	typedef std::ptrdiff_t difference_type;

	explicit ParameterIterator( IPythonContextWithParameters* container = nullptr, int index = 0 )
		: container( container ),
		index( index )
	{
	}

	ParameterIterator& operator++()
	{
		++index;
		return *this;
	}

	bool operator==( ParameterIterator const& other ) const
	{
		ASSERT( container == other.container );
		return index == other.index;
	}

	bool operator!=( ParameterIterator const& other ) const
	{
		return !( *this == other );
	}

	TParameter& operator*() const
	{
		return static_cast<TParameter&>( container->GetParameter( index ) );
	}

	IPythonContextWithParameters* container;
	int index;
};

inline Iterable<ParameterIterator<>> IPythonContextWithParameters::GetParameters()
{
	return Iterable<ParameterIterator<>>( ParameterIterator<>( this, 0 ), ParameterIterator<>( this, GetParametersCount() ) );
}

struct IBrushPythonContext : IPythonContext
{
	EPHERE_NODISCARD virtual Parameters::String GetBrushName() const = 0;
};

struct IGraphPythonContext : IPythonContextWithParameters
{
	virtual bool InvokeSetValueFunction( Groom::IGraph& graph, char const* parameterName, int value ) = 0;

	virtual bool InvokeSetValueFunction( Groom::IGraph& graph, char const* parameterName ) = 0;
};

struct IMoovPythonContext : IPythonContextWithParameters
{
	virtual MoovHairSimulator& GetSimulator() = 0;

	virtual Moov::Solver& GetSolver() = 0;

	EPHERE_NODISCARD virtual Moov::CaptureHandle GetInitialState() const = 0;

	virtual void SetInitialState( Moov::CaptureHandle ) = 0;

	virtual bool Evaluate( double timeSec, bool resetSolver, bool resetHairModel ) = 0;
};

struct IPythonScripter
{
	virtual ~IPythonScripter()
	{
	}

	virtual bool InitializePython() = 0;

	virtual Parameters::WString GetDefaultScriptPath( wchar_t const* projectName = L"", wchar_t const* subDirectoryName = L"", char const* environmentVariableName = "" ) = 0;

	EPHERE_NODISCARD virtual UniquePtr<IBrushPythonContext> CreateBrushContext() const = 0;

	EPHERE_NODISCARD virtual UniquePtr<IGraphPythonContext> CreateGraphContext() const = 0;

	EPHERE_NODISCARD virtual UniquePtr<IMoovPythonContext> CreateMoovContext() const = 0;
};

IPythonScripter* GetPythonScripter();

} }
