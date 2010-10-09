////////////////////////////////////////////////////////////////////////////////
//! \file   CmdLineArgs.hpp
//! \brief  The set of all command line switches.
//! \author Chris Oldwood

// Check for previous inclusion
#ifndef APP_CMDLINEARGS_HPP
#define APP_CMDLINEARGS_HPP

#if _MSC_VER > 1000
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////
//! The command line switches.

enum CmdLineArgType
{
	USAGE			= 0,	//!< Show the program options syntax.
	VERSION			= 1,	//!< Show the program version and copyright.
	HOSTNAMES		= 2,	//!< The hostnames to run the query on.
	USER			= 3,	//!< The login name to use for remote hosts.
	PASSWORD		= 4,	//!< The password to use for remote hosts.
	SHOW_HOST		= 5,	//!< Repeat the hostname in the output.
	SHOW_TYPES		= 6,	//!< Display the COM type of the property values.
	DETECT_DATES	= 7,	//!< Display datetime type strings in datetime format.
	HELP			= 99,	//!< Show the manual.
};

#endif // APP_CMDLINEARGS_HPP
