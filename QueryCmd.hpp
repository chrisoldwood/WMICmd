////////////////////////////////////////////////////////////////////////////////
//! \file   QueryCmd.hpp
//! \brief  The QueryCmd class declaration.
//! \author Chris Oldwood

// Check for previous inclusion
#ifndef APP_QUERYCMD_HPP
#define APP_QUERYCMD_HPP

#if _MSC_VER > 1000
#pragma once
#endif

#include <WCL/ConsoleCmd.hpp>

////////////////////////////////////////////////////////////////////////////////
//! The command used to list the running servers and topics.

class QueryCmd : public WCL::ConsoleCmd
{
public:
	//! Constructor.
	QueryCmd(int argc, tchar* argv[]);

	//! Destructor.
	virtual ~QueryCmd();
	
private:
	//
	// Command methods.
	//

	//! Get the description of the command.
	virtual const tchar* getDescription();

	//! Get the expected command usage.
	virtual const tchar* getUsage();

	//! The implementation of the command.
	virtual int doExecute(tostream& out, tostream& err);

	//! Read the list of hostnames from a text file.
	Core::CmdLineParser::StringVector readHostsFile(const tstring& filename);
};

#endif // APP_QUERYCMD_HPP
