////////////////////////////////////////////////////////////////////////////////
//! \file   QueryCmd.cpp
//! \brief  The QueryCmd class definition.
//! \author Chris Oldwood

#include "Common.hpp"
#include "QueryCmd.hpp"
#include "CmdLineArgs.hpp"
#include <Core/CmdLineException.hpp>
#include <Core/tiostream.hpp>
#include <WMI/Connection.hpp>
#include <WMI/ObjectIterator.hpp>
#include "Format.hpp"

////////////////////////////////////////////////////////////////////////////////
//! The table of command specific command line switches.

static Core::CmdLineSwitch s_switches[] = 
{
	{ USAGE,		TXT("?"),	NULL,				Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,				TXT("Display the command syntax")						},
	{ HOSTNAMES,	TXT("h"),	TXT("hosts"),		Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::MULTIPLE,	TXT("hostname"),	TXT("Remote machines to query")							},
	{ USER,			TXT("u"),	TXT("user"),		Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::SINGLE,	TXT("login"),		TXT("The login name for remote machines")				},
	{ PASSWORD,		TXT("p"),	TXT("password"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::SINGLE,	TXT("password"),	TXT("The password for remote machines")					},
	{ SHOW_HOST,	TXT("sh"),	TXT("showhost"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,				TXT("Display the hostname in the output")				},
	{ SHOW_TYPES,	TXT("st"),	TXT("showtypes"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,				TXT("Display the type of each property value")			},
	{ NO_FORMAT,	TXT("nf"),	TXT("no-format"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,				TXT("Display raw values instead")						},
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

int QueryCmd::doExecute(tostream& out, tostream& /*err*/)
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
	tstring		user     = m_parser.getSwitchValue(USER);
	tstring		password = m_parser.getSwitchValue(PASSWORD);
	bool		showHost = m_parser.isSwitchSet(SHOW_HOST);
	bool		showTypes = m_parser.isSwitchSet(SHOW_TYPES);
	bool		noFormatting = m_parser.isSwitchSet(NO_FORMAT);
	Hostnames	hostnames;

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
			out << std::endl;

		// For all objects...
		for (; objectIter != objectEnd; ++objectIter)
		{
			if (showHost)
			{
				out << TXT("Host");
				out << TXT(": ");
				out << host;
				out << std::endl;
			}

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

				out << name;

				if (showTypes)
				{
					out << TXT(" [") << WCL::Variant::formatFullType(value) << TXT("]");
				}

				out << TXT(": ");
				out << formatValue(value, noFormatting);
				out << std::endl;
			}

			out << std::endl;
		}
	}

	return EXIT_SUCCESS;
}
