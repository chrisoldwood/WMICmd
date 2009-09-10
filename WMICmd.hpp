////////////////////////////////////////////////////////////////////////////////
//! \file   WmiCmd.hpp
//! \brief  The WmiCmd class declaration.
//! \author Chris Oldwood

// Check for previous inclusion
#ifndef WMICMD_HPP
#define WMICMD_HPP

#if _MSC_VER > 1000
#pragma once
#endif

#include <WCL/ConsoleApp.hpp>
#include "Command.hpp"

////////////////////////////////////////////////////////////////////////////////
//! The application.

class WmiCmd : public WCL::ConsoleApp
{
public:
	//! Default constructor.
	WmiCmd();

	//! Destructor.
	virtual ~WmiCmd();
	
protected:
	//
	// ConsoleApp methods.
	//

	//! Run the application.
	virtual int run(int argc, tchar* argv[]);

	//! Display the program options syntax.
	virtual void showUsage();

private:
	//
	// Members.
	//
	Core::CmdLineParser m_parser;		//!< The command line parser.

	//
	// Internal methods.
	//

	//! Create the Comand object.
	CommandPtr createCommand(int argc, tchar* argv[]); // throw(CmdLineException)

	//! Display the program version.
	void showVersion();

	//! Display the manual.
	void showManual();
};

//! The application object.
extern WmiCmd g_app;

#endif // WMICMD_HPP
