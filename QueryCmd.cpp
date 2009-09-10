////////////////////////////////////////////////////////////////////////////////
//! \file   QueryCmd.cpp
//! \brief  The QueryCmd class definition.
//! \author Chris Oldwood

#include "Common.hpp"
#include "QueryCmd.hpp"
#include <Core/tiostream.hpp>
#include "CmdLineArgs.hpp"
#include <Core/CmdLineException.hpp>
#include <WMI/Connection.hpp>
#include <WMI/ObjectIterator.hpp>
#include <WCL/VariantBool.hpp>

////////////////////////////////////////////////////////////////////////////////
//! The table of command specific command line switches.

static Core::CmdLineSwitch s_switches[] = 
{
	{ USAGE,		TXT("?"),	NULL,				Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,				TXT("Display the command syntax")			},
	{ HOSTNAMES,	TXT("h"),	TXT("hosts"),		Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::MULTIPLE,	TXT("hostname"),	TXT("Remote machines to query")				},
	{ USER,			TXT("u"),	TXT("user"),		Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::SINGLE,	TXT("login"),		TXT("The login name for remote machines")	},
	{ PASSWORD,		TXT("p"),	TXT("password"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::SINGLE,	TXT("password"),	TXT("The password for remote machines")		},
};
static size_t s_switchCount = ARRAY_SIZE(s_switches);

////////////////////////////////////////////////////////////////////////////////
//! Constructor.

QueryCmd::QueryCmd(int argc, tchar* argv[])
	: Command(s_switches, s_switches+s_switchCount, argc, argv)
{
}

////////////////////////////////////////////////////////////////////////////////
//! Destructor.

QueryCmd::~QueryCmd()
{
}

////////////////////////////////////////////////////////////////////////////////
//! Get the description of the command.

const tchar* QueryCmd::getDescription()
{
	return TXT("Execute a WMI query and output the results");
}

////////////////////////////////////////////////////////////////////////////////
//! Get the expected command usage.

const tchar* QueryCmd::getUsage()
{
	return TXT("USAGE: WMICmd query <query> [--hosts <hostname> ...] [--user <login> --password <password>]");
}

////////////////////////////////////////////////////////////////////////////////
//! The implementation of the command.

int QueryCmd::doExecute()
{
	ASSERT(m_parser.getUnnamedArgs().at(0) == TXT("query"));

	typedef Core::CmdLineParser::StringVector Hostnames;
	typedef Hostnames::const_iterator HostIter;

	// Validate and extract the command line arguments.
	if (m_parser.getUnnamedArgs().size() < 2)
		throw Core::CmdLineException(TXT("No WMI query text specified"));

	if ( (m_parser.isSwitchSet(USER) && !m_parser.isSwitchSet(PASSWORD))
	  || (m_parser.isSwitchSet(PASSWORD) && !m_parser.isSwitchSet(USER)) )
		throw Core::CmdLineException(TXT("Both --user and --password must be specified together"));

	tstring		query    = m_parser.getUnnamedArgs().at(1);
	Hostnames	hostnames;
	tstring		user     = m_parser.getSwitchValue(USER);
	tstring		password = m_parser.getSwitchValue(PASSWORD);

	if (m_parser.isSwitchSet(HOSTNAMES))
		hostnames = m_parser.getNamedArgs().find(HOSTNAMES)->second;

	if (hostnames.empty())
		hostnames.push_back(WMI::Connection::LOCALHOST);

	// For all hosts to query...
	for (HostIter it = hostnames.begin(); it != hostnames.end(); ++it)
	{
		const tstring& host = *it;

		// Open a connection.
		WMI::Connection connection;

		if (host == WMI::Connection::LOCALHOST)
			connection.open();
		else
			connection.open(host, user, password);

		// Execute the query.
		WMI::ObjectIterator objectIter = connection.execQuery(query.c_str());
		WMI::ObjectIterator objectEnd;

		if (objectIter != objectEnd)
			tcout << std::endl;

		// For all objects...
		for (; objectIter != objectEnd; ++objectIter)
		{
			WMI::Object					object = *objectIter;
			WMI::Object::PropertyNames	names;

			object.getPropertyNames(names);

			// For all properties...
			WMI::Object::PropertyNames::const_iterator nameIter = names.begin();
			WMI::Object::PropertyNames::const_iterator nameEnd  = names.end();

			for (; nameIter != nameEnd; ++nameIter)
			{
				const tstring& name = *nameIter;

				WCL::Variant value;

				object.getProperty(name, value);

				tcout << name << TXT(": ") << value.format() << std::endl;
			}

			tcout << std::endl;
		}
	}

	return EXIT_SUCCESS;
}
