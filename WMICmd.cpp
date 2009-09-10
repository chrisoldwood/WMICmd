////////////////////////////////////////////////////////////////////////////////
//! \file   TheApp.cpp
//! \brief  The TheApp class definition.
//! \author Chris Oldwood

#include "Common.hpp"
#include "TheApp.hpp"
#include <Core/tiostream.hpp>
#include <WCL/Path.hpp>
#include <WCL/VerInfoReader.hpp>

////////////////////////////////////////////////////////////////////////////////
// Local variables.

static tstring s_appName(TXT("Application"));

////////////////////////////////////////////////////////////////////////////////
// The table of command line switches.

enum
{
	USAGE	= 0,	//!< Show the program options syntax.
	VERSION	= 1,	//!< Show the program version and copyright.
};

static Core::CmdLineSwitch s_switches[] = 
{
	{ USAGE,	TXT("?"),	NULL,			Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,	NULL,	TXT("Display the program options syntax")	},
	{ VERSION,	TXT("v"),	TXT("version"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,	NULL,	TXT("Display the program version")			},
};
static size_t s_switchCount = ARRAY_SIZE(s_switches);

////////////////////////////////////////////////////////////////////////////////
//! Default constructor.

TheApp::TheApp()
	: m_parser(s_switches, s_switches+s_switchCount)
{
}

////////////////////////////////////////////////////////////////////////////////
//! Destructor.

TheApp::~TheApp()
{
}

////////////////////////////////////////////////////////////////////////////////
//! Run the application.

int TheApp::run(int argc, tchar* argv[])
{
	m_parser.parse(argc, argv, Core::CmdLineParser::ALLOW_UNIX_FORMAT);

	// Request for help?
	if (m_parser.isSwitchSet(USAGE))
	{
		showUsage();
		return EXIT_SUCCESS;
	}
	// Request for version?
	else if (m_parser.isSwitchSet(VERSION))
	{
		showVersion();
		return EXIT_SUCCESS;
	}

	return EXIT_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//! Display the program options syntax.

void TheApp::showUsage()
{
	tcout << std::endl;
	tcout << TXT("USAGE: ") << s_appName << (" [options] ...") << std::endl;
	tcout << std::endl;

	tcout << m_parser.formatSwitches(Core::CmdLineParser::UNIX);
}

////////////////////////////////////////////////////////////////////////////////
//! Display the program version.

void TheApp::showVersion()
{
	// Extract details from the resources.
	tstring filename  = CPath::Application();
	tstring version   = WCL::VerInfoReader::GetStringValue(filename, WCL::VerInfoReader::PRODUCT_VERSION);
	tstring copyright = WCL::VerInfoReader::GetStringValue(filename, WCL::VerInfoReader::LEGAL_COPYRIGHT);

#ifdef _DEBUG
	version += TXT(" [Debug]");
#endif

	// Display version etc.
	tcout << std::endl;
	tcout << s_appName << TXT(" v") << version << std::endl;
	tcout << copyright << std::endl;
}
