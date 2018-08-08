#pragma once
/*
pragma once is a gcc- and clang-specific extension, which makes handling include
guards faster.
The DEBUG_H check is the normal include guard for other compilers.
*/
#ifndef DEBUG_H
#define DEBUG_H

/*
We need stdio for fprintf and FILE, but it has a different name in C++
*/
#ifdef __cplusplus
#	include <cstdio>
#else
#	include <stdio.h>
#endif

FILE *debug_stream=NULL;

/*
Set debug_level to a sane default of 1;
This will not hide any specified with no level.
*/
#ifdef DEBUG_LEVEL
	unsigned int debug_level=(unsigned int)DEBUG_LEVEL;
#else
	unsigned int debug_level=(unsigned int)1;
#endif

#ifdef DEBUG

/*
We want to choose the best pretty-printed version of the function name.
GCC has a variable called __PRETTY_FUNCTION__ (which isn't a macro even though
it looks like one). This was introduced in GCC 2.4, but was broken g++ before
version 2.6.
C++ has a variable called __func__, which gives the mangled function name.
This isn't as easy for people to read (although some people might prefer it,
I'll make that an easier to configure option in a later version), but we can 
fall back to that if we don't have anything else.

To add support for another compiler, we can add an extra #elif which tests the
compiler-specific macros (like the line marked (1)) and defines FUNCTION
appropriately.
For a different C++ version, you can add a test line after (2)
*/
#ifdef __cplusplus
#	ifdef __GNUC__ && ( ( __GNUC__ >= 3) || ( __GNUC__ == 2 && __GNUC_MINOR__ >=6 ) )
#		define FUNCTION __PRETTY_FUNCTION__ /* (2) */
#	else
#		define FUNCTION __func__
#	endif /*C++ version */
#elif defined __GNUC__ && ( ( __GNUC__ >= 3) || ( __GNUC__ == 2 && __GNUC_MINOR__ >= 4 ) ) /* (1) */
#	define FUNCTION __PRETTY_FUNCTION__
#else
#	define FUNCTION debug_quote( )
#endif

/*
sets the default stream for debugging statements to be stderr.
This is a horrible hack needed to work around the fact that stderr isn't
a constant, so it can't be used to initialise a global.
*/
#define DEBUG_DEFAULT_STREAM stderr

/*
 We will let the programmer specify the maximum buffer length if he wants to
 Otherwise, just use 60 chars because it seems like a good idea.
 We usually want the debug line to fit on an 80-column screen, and this size 
 doesn't count the file name, function name, or line number.
*/
#ifndef DEBUG_MAX_LENGTH
#	define DEBUG_MAX_LENGTH 60
#endif

#define LINE_LOCATION __FILE__ " " debug_quote(__LINE__) " "

/*
We have a special C++ version, to take advantage of stream inserters.
*/
#ifdef __cplusplus
#	include <iostream>
#	define debug_cpp(string) \
		do {\
			std::cerr<< LINE_LOCATION << " " << FUNCTION << ": " << string << std::endl; \
			std::cout.flush(); \
		} while(0)
#endif /*cpp*/

/*
Check that the specified file isn't NULL.
We could assume that NULL is 0, but that isn't the case in some of the more 
unusual C dialects, and we want to make this file as portable as possible.

WARNING: This macro is unsafe.
*/
#define DEBUG_CHECK_FILE(file) \
	(file != NULL) ? file : DEBUG_DEFAULT_STREAM

/*
This is the code used to check the debug level.
If we are using GCC, we can also make sure that the programmer hasn't passed a 
negative value, otherwise we just have to assume he has a bit of common sense.

If your compiler can support typeof, add a test for it to the first line
*/
#if defined __GNUC__ && __GNUC__ >= 2
#	define DEBUG_CHECK_LEVEL(level) \
		__typeof__((level)) DEBUG_LEVEL_TEMP = level; \
		if(  \
			DEBUG_LEVEL_TEMP <= debug_level \
			&& DEBUG_LEVEL_TEMP > 0 )
#else
#	define DEBUG_CHECK_LEVEL(level) \
		unsigned int DEBUG_LEVEL_TEMP = (unsigned int) level\
		if(  \
			DEBUG_LEVEL_TEMP <= debug_level \
			&& DEBUG_LEVEL_TEMP > 0)
#endif

/*
Prints a single string to the file specified by debug_stream, at debug level 1.
Each string is printed on its own line, and prefixed by the file name, line 
number, and the function name.
*/
#define debug(string) \
	do{\
		fdebugl(1, debug_stream, string); \
	}while(0)

/*
Prints a single string to the file specified by the first argument.
THis is equivalent to fdebugl(1, file, string).
*/
#define fdebug(file, string)\
	do {\
		fdebugl(1, file, string); \
	} while(0)

/*
This is equivalent to fdebugl(level, debug_stream,string)
*/
#define debugl(level, string) \
	do { \
		fdebugl(level, debug_stream, string); \
	}while(0)

/*
This prints a single string to the specified file, but only if level < debug_level.
That is, a bigger debug_level is more verbose.
This is safe, and checks that file is non-null.
If file is null, it prints to whatever is specified by DEBUG_DEFAULT_STREAM
If that is null, it will segfault.
*/
#define fdebugl(level, file, string) \
	do { \
		DEBUG_CHECK_LEVEL(level) { \
			FILE *DEBUG_FILE = file; \
			DEBUG_FILE = DEBUG_CHECK_FILE( DEBUG_FILE ); \
			fprintf( DEBUG_CHECK_FILE(DEBUG_FILE),  LINE_LOCATION "%s: %s\n", FUNCTION, string ); \
			fflush( DEBUG_FILE ); \
		} \
		}while(0)

#define debug_quote(x)	_debug_quote(x)
#define _debug_quote(x)	#x

/*
WARNING: The following versions depend on a C99/C++0x feature: variadic macros.
There is no guard for that implemented because many C89/C++90 compilers
also support this feature.
If it doesn't work for you, you should probably add guards.
*/

/*
This is similar to printf, but adds location information to the start 
of the line and a newline at the end.
*/
#define debugf(...) \
	do {\
		fdebuglf( 1, debug_stream, __VA_ARGS__); \
	}while(0)

/*
Prints debugf-like output to the specified file.
This macro is safe.
If file is NULL, it prints to DEBUG_DEFAULT_STREAM.
If that is NULL, the program will segfault. That shouldn't happen unless someone
overrides the default value, since the default is stderr.
*/
#define fdebugf(file, ...) \
	do {\
		fdebuglf(1, file, __VA_ARGS__ ); \
	}while(0)

/*
This is eqivalent to  fdebuglf
*/
#define debuglf(level, ...) \
	do {\
		fdebuglf(level, debug_stream, __VA_ARGS__ ); \
	}while(0)

/*
This acts like fprintf, but only prints if level <= debug_level, and prepends
location information to the start of the line, and appends a newline.
This macro is safe.

If file is NULL, it prints to DEBUG_DEFAULT_STREAM.
If that is NULL or inaccessible, the program will segfault. That shouldn't 
happen unless someone overrides the default value, since the default is stderr.
*/
#define fdebuglf(level, file, ...) \
	do {\
		DEBUG_CHECK_LEVEL(level) {\
			FILE *DEBUG_FILE = file; \
			DEBUG_FILE = DEBUG_CHECK_FILE( DEBUG_FILE ); \
			fprintf( DEBUG_FILE,  LINE_LOCATION "%s: ", FUNCTION); \
			fprintf( DEBUG_FILE, __VA_ARGS__ ); \
			fprintf( DEBUG_FILE, "\n" ); \
			fflush( DEBUG_FILE ); \
		} \
	} while(0)

#else /* DEBUG is undefined, i.e. debugging is turned off */

/*
We need dummy definitions to prevent problems with undefined macros.
*/

#define debug(string)   do{}while(0)
#define debugl(level, string)   do{}while(0)
#define debugf(...)  do{}while(0)
#define fdebug(file, string)   do{}while(0)
#define fdebugf(file, ...)  do{}while(0)
#define debuglf(...) do{}while(0)
#define fdebuglf(...) do{}while(0)

#endif /*DEBUG*/

#endif /*DEBUG_H*/
