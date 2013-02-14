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
#include <WCL/ConsoleCmd.hpp>

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
	virtual int run(int argc, tchar* argv[], tistream& in, tostream& out, tostream& err);

	//! Get the name of the application.
	virtual tstring applicationName() const;

	//! Display the program options syntax.
	virtual void showUsage(tostream& out) const;

private:
	//
	// Members.
	//
	Core::CmdLineParser m_parser;		//!< The command line parser.

	//
	// Internal methods.
	//

	//! Create the Comand object.
	WCL::ConsoleCmdPtr createCommand(int argc, tchar* argv[]); // throw(CmdLineException)
};

//! The application object.
extern WmiCmd g_app;

#endif // WMICMD_HPP
