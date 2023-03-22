// Must compile with VC 2012 / GCC 4.8

// ReSharper disable CppClangTidyModernizeUseEqualsDefault
// ReSharper disable CppClangTidyModernizeUseEqualsDelete
// ReSharper disable CppClangTidyModernizeLoopConvert
#pragma once

#include "Ephere/Core/Parameters/String.h"
#include "Ephere/NativeTools/LoadDynamicLibrary.h"
#include "Ephere/NativeTools/Log.h"
#include "Ephere/NativeTools/SmartPointers.h"
#include "Ephere/NativeTools/StringToolsBase.h"
#include "Ephere/Ornatrix/PythonInterfaces.h"
#include "Ephere/Ornatrix/Groom/IGraph.h"
#include "Ephere/Ornatrix/Groom/IOperator.h"

#ifdef ORNATRIX_EXPORTS
#	define ORNATRIX_API EPHERE_API_EXPORT
#else
#	define ORNATRIX_API EPHERE_API_IMPORT
#endif

namespace Ephere
{
namespace Ornatrix
{

// Matches UsdTimeCode::Default()
double const TimeUndefined = std::numeric_limits<double>::quiet_NaN();

inline bool TimeIsDefined( double time )
{
	return !std::isnan( time );
}

namespace Groom
{
struct IGraph;
}

struct ILibraryBase
{
	virtual ~ILibraryBase()
	{
	}
};

struct GroomSerializeSettings
{
	explicit GroomSerializeSettings(
		double time = TimeUndefined,
		bool writeDefaultValues = true,
		bool ignoreExtraFiles = false,
		bool createZipArchiveIfNeeded = true )
		: time( time )
		, writeDefaultValues( writeDefaultValues )
		, ignoreExtraFiles( ignoreExtraFiles )
		, createZipArchiveIfNeeded( createZipArchiveIfNeeded )
	{
	}

	double time;

	bool writeDefaultValues;
	bool ignoreExtraFiles;

	bool createZipArchiveIfNeeded;
};

struct SerializedGroom
{
	Parameters::String contentBuffer;

	Parameters::Array<Parameters::String> extraFileNames;
	Parameters::Array<Parameters::String> extraContentBuffers;


	void Clear()
	{
		contentBuffer.clear();
		extraFileNames.clear();
		extraContentBuffers.clear();
	}

	EPHERE_NODISCARD int FindExtraFileIndex( std::string_view name ) const
	{
		auto const existing = std::find( extraFileNames.begin(), extraFileNames.end(), name );
		return existing != extraFileNames.end() ? static_cast<int>( existing - extraFileNames.begin() ) : -1;
	}

	void AddOrUpdateExtraFile( Parameters::String name, Parameters::String content )
	{
		if( name.empty() || content.empty() )
		{
			return;
		}

		auto const existing = std::find( extraFileNames.begin(), extraFileNames.end(), name );
		if( existing != extraFileNames.end() )
		{
			auto const index = static_cast<int>( existing - extraFileNames.begin() );
			extraFileNames[index] = std::move( name );
			extraContentBuffers[index] = std::move( content );
		}
		else
		{
			extraFileNames.push_back( std::move( name ) );
			extraContentBuffers.push_back( std::move( content ) );
		}
	}
};

struct GroomInfo  // NOLINT(cppcoreguidelines-pro-type-member-init)
{
	int version;
	Parameters::Array<double> timeSamples;

	EPHERE_NODISCARD bool IsValid() const
	{
		return version > 0;
	}
};

struct IGroomSerializer
{
	virtual ~IGroomSerializer()
	{
	}

	EPHERE_NODISCARD virtual StringView GroomFileFormatExtension() const = 0;

	EPHERE_NODISCARD virtual StringView GroomZippedFormatExtension() const = 0;

	EPHERE_NODISCARD virtual bool ContentsHasCorrectFormat( StringView contents ) const = 0;

	EPHERE_NODISCARD bool HasGroomExtension( std::string_view filePath, std::string_view extension ) const;

	EPHERE_NODISCARD bool HasGroomExtension( std::string_view filePath ) const
	{
		return HasGroomExtension( filePath, GroomFileFormatExtension() ) || HasGroomExtension( filePath, GroomZippedFormatExtension() );
	}

	virtual bool GetGroomInfoFromContentBuffer( StringView contents, GroomInfo& ) const = 0;

	// Archive-related flags in GroomSerializeSettings are ignored. The serializer doesn't care about archiving
	virtual bool SerializeGroom(
		Groom::IGraph const&,
		GroomSerializeSettings const&,
		SerializedGroom& ) const = 0;

	EPHERE_NODISCARD virtual UniquePtr<Groom::IGraph> DeserializeGroom(
		SerializedGroom const&,
		double time = TimeUndefined,
		Span<std::pair<Parameters::TypeId, Parameters::FactoryFunction> const> = Span<std::pair<Parameters::TypeId, Parameters::FactoryFunction> const>(),
		StringView extraFilesBaseDirectory = "" ) const = 0;
};

// Deserialization from buffers supports .usda format only. Loading a binary .usd(c) from a buffer is not supported by the USD library
struct IUsdSerializer : IGroomSerializer
{
	static char const* ExtensionUsd()
	{
		return ".usda";
	}

	static char const* ExtensionUsdZip()
	{
		return ".usdz";
	}


	static bool HasUsdExtension( std::string_view filePath )
	{
		return EndsWith<CaseInsensitiveCharTraits<char>>( filePath, ExtensionUsd() )
			|| EndsWith<CaseInsensitiveCharTraits<char>>( filePath, ExtensionUsdZip() );
	}

	static bool IsUsdAsciiContents( std::string_view text )
	{
		return StartsWith( text, "#usda " );
	}

	static bool IsBinaryUsdContents( std::string_view text )
	{
		return StartsWith( text, "PXR-USDC" );
	}

	static bool IsUsdContents( std::string_view text )
	{
		return IsBinaryUsdContents( text ) || IsUsdAsciiContents( text );
	}


	EPHERE_NODISCARD virtual bool IsUsdFile( StringView filePath ) const = 0;


	struct ExportHairOptions
	{
		struct EvaluateResult
		{
			IHair* hair = nullptr;
			Xform3 transform = Xform3::Identity();
			bool canceled = false;
		};

		typedef EvaluateResult( *NodeEvaluatorFunctionType )( void* node, double time, double completedPercent );

		explicit ExportHairOptions(
			NodeEvaluatorFunctionType evaluator,
			bool exportTextureCoordinates = true,
			bool exportWidths = true,
			bool exportNormals = false,
			bool exportStrandData = false,
			int upAxis = 1,
			double timeStart = 0,
			double timeEnd = 0,
			double timeStep = 1,
			double framesPerSecond = 24 )
			: evaluator( evaluator ),
			exportTextureCoordinates( exportTextureCoordinates ),
			exportWidths( exportWidths ),
			exportNormals( exportNormals ),
			exportStrandData( exportStrandData ),
			upAxis( upAxis ),
			timeStart( timeStart ),
			timeEnd( timeEnd ),
			timeStep( timeStep ),
			framesPerSecond( framesPerSecond )
		{
		}

		NodeEvaluatorFunctionType evaluator;

		bool exportTextureCoordinates;
		bool exportWidths;
		bool exportNormals;
		bool exportStrandData;
		int upAxis;

		double timeStart;
		double timeEnd;
		double timeStep;
		double framesPerSecond;
	};

	//! If objectName is not specified, the default name "ornatrixHair" is used.
	virtual bool ExportHair( void* node, ExportHairOptions const&, StringView filePath, StringView objectName = "" ) const = 0;
};

struct IGrooms
{
	// Enum preferred to static const int, see https://stackoverflow.com/questions/5391973/undefined-reference-to-static-const-int
	enum
	{
		GroomFormatVersion = 3
	};

	virtual ~IGrooms()
	{
	}

	static char const* BaseFileExtension()
	{
		return ".oxg";
	}

	static char const* DefaultFileExtension()
	{
		return ".oxg.yaml";
	}

	static char const* DefaultZipFileExtension()
	{
		return ".oxg.zip";
	}

	EPHERE_NODISCARD bool HasGroomExtension( std::string_view filePath, bool zippedOnly = false ) const
	{
		// Check the USD extension without loading the USD plugin
		if( !zippedOnly && EndsWith<CaseInsensitiveCharTraits<char>>( filePath, IUsdSerializer::ExtensionUsd() )
			|| EndsWith<CaseInsensitiveCharTraits<char>>( filePath, IUsdSerializer::ExtensionUsdZip() ) )
		{
			return true;
		}

		auto hasExt = [&filePath]( StringView ext )
		{
			return EndsWith<CaseInsensitiveCharTraits<char>>( filePath, ext )
				&& EndsWith<CaseInsensitiveCharTraits<char>>( filePath.substr( 0, filePath.length() - ext.length() ), BaseFileExtension() );
		};

		auto serializers = GetGroomSerializers();
		for( auto index = 0; index < static_cast<int>( serializers.size() ); ++index )
		{
			if( !zippedOnly && hasExt( serializers[index]->GroomFileFormatExtension() )
				|| hasExt( serializers[index]->GroomZippedFormatExtension() ) )
			{
				return true;
			}
		}

		return false;
	}

	EPHERE_NODISCARD std::string AddGroomExtension( std::string const& filePath, std::string_view extension = DefaultFileExtension() ) const
	{
		return HasGroomExtension( filePath ) ? filePath : filePath + std::string( extension );
	}

	EPHERE_NODISCARD virtual Parameters::ITypeRegistry& GetTypeRegistry() const = 0;

	EPHERE_NODISCARD virtual Groom::IOperatorRegistry& GetOperatorRegistry() const = 0;

	EPHERE_NODISCARD virtual UniquePtr<Groom::IGraph> CreateGroom( Groom::CoordinateSystemProperties const& ) const = 0;

	bool GetGroomInfoFromFile( std::string_view filePath, GroomInfo& info ) const
	{
		SerializedGroom serialized;
		return LoadGroomFromFile( filePath, serialized ) && GetGroomInfoFromContentBuffer( serialized.contentBuffer, info );
	}

	bool GetGroomInfoFromContentBuffer( std::string_view contents, GroomInfo& info ) const
	{
		auto const serializer = GetGroomSerializerFromContentBuffer( contents );
		return serializer != nullptr && serializer->GetGroomInfoFromContentBuffer( contents, info );
	}

	EPHERE_NODISCARD bool IsGroomFile( std::string_view filePath ) const
	{
		GroomInfo info;
		return GetGroomInfoFromFile( filePath, info );
	}

	EPHERE_NODISCARD bool IsGroomFileOfSupportedVersion( std::string_view filePath ) const
	{
		GroomInfo info;
		return GetGroomInfoFromFile( filePath, info ) && info.version == GroomFormatVersion;
	}

	// The first one is the default
	EPHERE_NODISCARD virtual Parameters::Array<IGroomSerializer*> GetGroomSerializers() const = 0;

	EPHERE_NODISCARD IGroomSerializer* GetGroomSerializerByFileExtension( std::string_view extension ) const
	{
		return FindFirstOrDefault( GetGroomSerializers(), nullptr, [&extension]( IGroomSerializer const* s )
		{
			return static_cast<std::string_view>( s->GroomFileFormatExtension() ) == extension;
		} );
	}

	EPHERE_NODISCARD IGroomSerializer* GetGroomSerializerFromContentBuffer( std::string_view contents ) const
	{
		return FindFirstOrDefault( GetGroomSerializers(), nullptr, [&contents]( IGroomSerializer const* s )
		{
			return s->ContentsHasCorrectFormat( contents );
		} );
	}

	virtual bool LoadGroomFromFile( StringView filePath, SerializedGroom& ) const = 0;

	/*! If no IGroomSerializer* is passed, it is derived from the file extension, or the default one is used.
	If outputFilePath is empty, no files are written. */
	virtual bool SerializeGroom(
		Groom::IGraph const&,
		Parameters::String& outputFilePath,
		GroomSerializeSettings const&,
		SerializedGroom&,
		IGroomSerializer* serializer = nullptr ) const = 0;

	/*! A single file is produced. If the extension implies zip, it will be zipped.
	If the extension is just .oxg or .oxg.yaml/usda, it will be zipped only if needed (contains more than one file).
	Returns the path of the result file or empty on failure */
	Parameters::String SerializeGroomToFile(
		Groom::IGraph const& graph,
		std::string_view filePath,
		double time = TimeUndefined,
		IGroomSerializer* serializer = nullptr,
		bool writeDefaultValues = true,
		bool ignoreExtraFiles = false,
		bool createZipArchiveIfNeeded = true ) const
	{
		if( filePath.empty() )
		{
			return "";
		}

		GroomSerializeSettings const settings( time, writeDefaultValues, ignoreExtraFiles, createZipArchiveIfNeeded );
		SerializedGroom result;
		Parameters::String outputFilePath = filePath;
		return SerializeGroom( graph, outputFilePath, settings, result, serializer ) ? outputFilePath : "";
	}

	//! A single in-memory buffer is produced. It will be zipped only if needed (contains more than one file). Returns the buffer
	Parameters::String SerializeGroomToBuffer(
		Groom::IGraph const& graph,
		double time = TimeUndefined,
		IGroomSerializer* serializer = nullptr,
		bool writeDefaultValues = true,
		bool ignoreExtraFiles = false ) const
	{
		GroomSerializeSettings const settings( time, writeDefaultValues, ignoreExtraFiles );
		Parameters::String outputFilePath;
		SerializedGroom result;
		if( !SerializeGroom( graph, outputFilePath, settings, result, serializer ) )
		{
			return "";
		}

		return std::move( result.contentBuffer );
	}

	//! If no IGroomSerializer* is passed, it is derived from the file extension or the data
	EPHERE_NODISCARD virtual UniquePtr<Groom::IGraph> DeserializeGroom(
		StringView inputFilePath,
		SerializedGroom const&,
		double time = TimeUndefined,
		IGroomSerializer* = nullptr,
		Span<std::pair<Parameters::TypeId, Parameters::FactoryFunction> const> = Span<std::pair<Parameters::TypeId, Parameters::FactoryFunction> const>(),
		StringView extraFilesBaseDirectory = "" ) const = 0;

	//! Expects a single file with the serialized groom. That's either an archive or a groom file without any additional files
	EPHERE_NODISCARD UniquePtr<Groom::IGraph> DeserializeGroomFromFile(
		std::string_view filePath,
		double time = TimeUndefined,
		IGroomSerializer* serializer = nullptr,
		Span<std::pair<Parameters::TypeId, Parameters::FactoryFunction> const> factories = Span<std::pair<Parameters::TypeId, Parameters::FactoryFunction> const>() ) const
	{
		SerializedGroom const groom;
		return DeserializeGroom( filePath, groom, time, serializer, factories );
	}

	// Expects a single buffer with the serialized groom. That's either an archive or a groom without any additional files
	EPHERE_NODISCARD UniquePtr<Groom::IGraph> DeserializeGroomFromContentBuffer(
		std::string_view contentBuffer,
		double time = TimeUndefined,
		IGroomSerializer* serializer = nullptr,
		Span<std::pair<Parameters::TypeId, Parameters::FactoryFunction> const> factories = Span<std::pair<Parameters::TypeId, Parameters::FactoryFunction> const>() ) const
	{
		SerializedGroom groom;
		groom.contentBuffer = Parameters::String::MakeView( contentBuffer );
		return DeserializeGroom( "", groom, time, serializer, factories );
	}

	virtual std::pair<UniquePtr<IHair>, UniquePtr<IPolygonMeshSA>> EvaluateGroom( Groom::IGraph&, Groom::EvaluationContext* = nullptr ) const = 0;
};

struct IHairUtilities
{
	virtual ~IHairUtilities()
	{
	}

	EPHERE_NODISCARD virtual IHair& GetHair() const = 0;

	virtual bool EnableStrandTopology() const = 0;  // NOLINT(modernize-use-nodiscard)
};

struct ILibrary1 : ILibraryBase
{
	EPHERE_NODISCARD virtual Log::LogCallbackFunctionType GetLogCallback() const = 0;

	virtual void SetLogCallback( Log::LogCallbackFunctionType ) = 0;

	EPHERE_NODISCARD virtual IPythonScripter* GetPythonScripter() const = 0;

	virtual void SetPythonScripter( IPythonScripter* ) const = 0;


	EPHERE_NODISCARD virtual IGrooms* GetGrooms() const = 0;

	// Doesn't try to load the USD plugin, just returns the pointer if it was already loaded
	EPHERE_NODISCARD virtual IUsdSerializer* GetUsdSerializer() const = 0;

	EPHERE_NODISCARD virtual IUsdSerializer* LoadUsdSerializer() = 0;


	EPHERE_NODISCARD virtual UniquePtr<IHair> CreateHair( bool initAsGuides ) const = 0;

	EPHERE_NODISCARD virtual UniquePtr<IHairUtilities> CreateHairUtilities( IHair& ) const = 0;

	EPHERE_NODISCARD virtual UniquePtr<IPolygonMeshSA> CreateMesh() const = 0;

	EPHERE_NODISCARD virtual Ramp CreateRampFromString( StringView ) const = 0;

	EPHERE_NODISCARD virtual Parameters::String RampToString( Ramp const& ) const = 0;

	EPHERE_NODISCARD virtual Parameters::String SaveTextureImageToPng( ITextureMap const* ) const = 0;

	EPHERE_NODISCARD virtual UniquePtr<ITextureMap> LoadTextureImageFromPng( Span<char const> pngData ) const = 0;
};

typedef ILibrary1 ILibrary;

}
}

ORNATRIX_API Ephere::Ornatrix::ILibraryBase* InitializeLibrary( Ephere::Log::LogCallbackFunctionType = nullptr, Ephere::Ornatrix::IPythonScripter* = nullptr );

namespace Ephere
{
namespace Ornatrix
{

struct OrnatrixLibrary
{
	~OrnatrixLibrary()
	{
	}

	OrnatrixLibrary()
		: libraryBase( nullptr ),
		library( nullptr ),
		grooms( nullptr )
	{
	}

	// VC 2012 doesn't provide automatic move semantics, need to define it explicitly
	OrnatrixLibrary( OrnatrixLibrary&& other ) EPHERE_NOEXCEPT
		: handle( std::move( other.handle ) ),
		libraryBase( other.libraryBase ),
		library( other.library ),
		grooms( other.grooms )
	{
	}

	OrnatrixLibrary& operator=( OrnatrixLibrary&& other ) EPHERE_NOEXCEPT
	{
		handle = std::move( other.handle );
		libraryBase = other.libraryBase;
		library = other.library;
		grooms = other.grooms;
		return *this;
	}

	OrnatrixLibrary( OrnatrixLibrary const& other )
		: handle( other.handle ),
		libraryBase( other.libraryBase ),
		library( other.library ),
		grooms( other.grooms )
	{
	}

	OrnatrixLibrary& operator=( OrnatrixLibrary const& other )
	{
		if( this != &other )
		{
			handle = other.handle;
			libraryBase = other.libraryBase;
			library = other.library;
			grooms = other.grooms;
		}

		return *this;
	}

	std::shared_ptr<void> handle;

	ILibraryBase* libraryBase;
	ILibrary* library;
	IGrooms* grooms;

	EPHERE_NODISCARD bool IsEmpty() const
	{
		return libraryBase == nullptr || grooms == nullptr;
	}

	void Reset()
	{
		library = nullptr;
		grooms = nullptr;
		handle.reset();
	}

	EPHERE_NODISCARD bool CheckUsdLoaded() const
	{
		if( library != nullptr && library->LoadUsdSerializer() != nullptr )
		{
			return true;
		}

		LOG( Error, "Ornatrix shared library or USD plugins not loaded" );
		return false;
	}


	EPHERE_NODISCARD Groom::OperatorDescriptor const* FindOperator( StringView name ) const
	{
		return grooms != nullptr ? grooms->GetOperatorRegistry().Find( name ) : nullptr;
	}
};

inline OrnatrixLibrary LoadOrnatrixLibrary( Log::LogCallbackFunctionType logger, IPythonScripter* pythonScripter, std::string const& libraryDirectory = "", std::string* error = nullptr )
{
	OrnatrixLibrary result;
	std::string path( MAKE_DYNLIB_FILENAME( "Ephere.Ornatrix" ) );
	if( !libraryDirectory.empty() )
	{
		path = libraryDirectory + "/" + path;
	}

	result.handle = LoadDynamicLibrary( path.c_str() );
	if( result.handle != nullptr )
	{
		if( auto const initializeLibrary = reinterpret_cast<decltype( &InitializeLibrary )>( Ephere_GetDynamicLibraryFunction( result.handle.get(), "InitializeLibrary" ) ) )
		{
			result.libraryBase = initializeLibrary( logger, pythonScripter );
			result.library = dynamic_cast<ILibrary*>( result.libraryBase );
			if( result.library != nullptr )
			{
				result.grooms = result.library->GetGrooms();
			}
			else if( error != nullptr )
			{
				*error = "Version mismatch";
			}
		}
	}
	else if( error != nullptr )
	{
		*error = GetLoadDynamicLibraryError();
	}

	return result;
}

inline OrnatrixLibrary LoadOrnatrixLibrary( std::string const& libraryDirectory = "", std::string* error = nullptr )
{
	return LoadOrnatrixLibrary( nullptr, nullptr, libraryDirectory, error );
}


// Define this in your module if you use it, initialize it in your module initialization function (main, DllMain, etc.)
extern OrnatrixLibrary TheOrnatrixLibrary;


inline bool IGroomSerializer::HasGroomExtension( std::string_view filePath, std::string_view extension ) const
{
	if( !EndsWith<CaseInsensitiveCharTraits<char>>( filePath, extension ) )
	{
		return false;
	}

	filePath.remove_suffix( extension.length() );
	return EndsWith<CaseInsensitiveCharTraits<char>>( filePath, IGrooms::BaseFileExtension() );
}

}
}
