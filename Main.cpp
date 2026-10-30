#include "DemoFile.h"
#include "DemoReader.h"
#include <cstdlib>
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN

#if defined _WIN64
#error Build target has to be set to x86
#endif

bool g_bPrintVerbose{ false };
bool g_bPrintEntities{ false };
bool g_bPrintStringTables{ false };
bool g_bPrintUserCmds{ false };
bool g_bPrintNoDefaults{ false };

int main( int argc, char *argv[] )
{
	SetConsoleTitle( TEXT( "CS:S v34 Demo Parser" ) );
	SetConsoleOutputCP( CP_UTF8 );

#ifdef _DEBUG
	system( "pause" );
#endif

	std::string demoArg{ "" };

	for ( int nArg = 1; nArg < argc; ++nArg )
	{
		std::string arg{ argv[ nArg ] };

		if ( arg.size() > 4 && !_stricmp( arg.substr( arg.size() - 4 ).c_str(), ".dem" ) )
			demoArg = arg;
		else if ( !_stricmp( arg.c_str(), "-verbose" ) )
		{
			g_bPrintVerbose = true;
			g_bPrintEntities = true;
			g_bPrintStringTables = true;
			g_bPrintUserCmds = true;
		}
		else if ( !_stricmp( arg.c_str(), "-entities" ) )
			g_bPrintEntities = true;
		else if ( !_stricmp( arg.c_str(), "-stringtables" ) )
			g_bPrintStringTables = true;
		else if ( !_stricmp( arg.c_str(), "-usercmds" ) )
			g_bPrintUserCmds = true;
		else if ( !_stricmp( arg.c_str(), "-nodefaults" ) )
			g_bPrintNoDefaults = true;
		else
			std::cout << "Unrecognized argument: \"" << arg << "\"\n";
	}

	if ( demoArg.empty() )
	{
		std::cout << "Usage: cssv34_demo_parser.exe <demofile> [arguments]\n";
		std::cout << "\nOptional arguments:\n"
			<< " -verbose : print everything\n"
			<< " -entities : print server classes and entity updates\n"
			<< " -stringtables : print string tables\n"
			<< " -usercmds : print user commands on pov demos\n"
			<< " -nodefaults : do not print the default data (game events, chat messages, server info)\n";

		system( "pause" );
		return EXIT_FAILURE;
	}

	DemoFile demo( demoArg );

	if ( !demo.IsValid() )
	{
		std::cout << "Failed to read demo: " << demo.GetFilename() << " is " << demo.GetErrorString() << '\n';
		system( "pause" );
		return EXIT_FAILURE;
	}

	DemoReader reader( &demo );

	try
	{
		if ( reader.ReadDemo() )
			std::cout << "\nDemo successfully read until the end.\n";
		else
			std::cout << "\nProcess aborted by user.\n";
	}
	catch ( std::exception e )
	{
		std::cout << "\nFailed to read demo until the end (" << e.what() << ").\n";
	}

	system( "pause" );
	return EXIT_SUCCESS;
}