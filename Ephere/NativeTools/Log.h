// Must compile with VC 2012 / GCC 4.8 (partial C++11)

#pragma once

// LineInfo
namespace Ephere { namespace Log
{

enum Level
{
	//...
	Undefined,

	//! Logging is disabled, no messages will be printed
	None,

	/*! Unexpected error - a logical error in the code (ASSERT/CHECK) or a fatal environment error (example: a critical file is missing).
	The app stops execution (if possible, e.g. not being a plugin DLL inside a host), or advises the user to save his work and exit the app. */
	Error,

	/*! An environment error after which recovery is possible and execution is safe to continue, most likely with some feature failing to work.
	Example: failed I/O or any OS API call. */
	Warning,

	/*! Information which might be of interest to the users, not just to the developers. It's not a problem, execution continues normally.
	Example: the start or result of a long running operation */
	Info,

	/*! Information for the developers only. Normally this level is disabled when running the release build on the
	user's machine but can be enabled so that the user can send the log file back to the developers for inspection. */
	Verbose,

	/*! Information for the developers which does not get compiled in the Release build (so it cannot be enabled on the user's machine).
	For use in performance-sensitive code.
	Note: this has no practical use (if indeed used in performance code, it slows down the Debug build too much), so this will be removed soon */
	Debug,

	//! Overall number of supported logging levels
	LevelCount
};

typedef void( *LogCallbackFunctionType )( Level, const char* message );

} }

#define LOG( level, ... ) ((void)0)
