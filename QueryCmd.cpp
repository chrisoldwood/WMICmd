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
#include <Core/TextFileIterator.hpp>
#include <sstream>
#include <iomanip>
#include <Core/StringUtils.hpp>
#include <limits>
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
//! The table of command specific command line switches.

static Core::CmdLineSwitch s_switches[] = 
{
	{ USAGE,		TXT("?"),	NULL,				Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,				TXT("Display the command syntax")						},
	{ USAGE,		NULL,		TXT("help"),		Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,				TXT("Display the command syntax")						},
	{ HOSTNAMES,	TXT("h"),	TXT("hosts"),		Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::MULTIPLE,	TXT("hostname"),	TXT("Remote machines to query")							},
	{ HOSTSFILE,	TXT("hf"),	TXT("hostsfile"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::SINGLE,	TXT("file"),		TXT("File with remote machines to query")				},
	{ USER,			TXT("u"),	TXT("user"),		Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::SINGLE,	TXT("login"),		TXT("The login name for remote machines")				},
	{ PASSWORD,		TXT("p"),	TXT("password"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::SINGLE,	TXT("password"),	TXT("The password for remote machines")					},
	{ SHOW_HOST,	TXT("sh"),	TXT("showhost"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,				TXT("Display the hostname in the output")				},
	{ SHOW_TYPES,	TXT("st"),	TXT("showtypes"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,				TXT("Display the type of each property value")			},
	{ NO_FORMAT,	TXT("nf"),	TXT("noformat"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,				TXT("Display raw values instead")						},
	{ ALIGN,		TXT("a"),	TXT("align"),		Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,				TXT("Align the output")									},
	{ TOP,			TXT("t"),	TXT("top"),			Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::SINGLE,	TXT("count"),		TXT("Limit results to first N items")					},
};
static size_t s_switchCount = ARRAY_SIZE(s_switches);

////////////////////////////////////////////////////////////////////////////////
//! Constructor.

QueryCmd::QueryCmd(int argc, tchar* argv[])
	: WCL::ConsoleCmd(s_switches, s_switches+s_switchCount, argc, argv, USAGE)
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
	typedef WMI::Object::PropertyNames::const_iterator PropNameIter;

	// Validate and extract the command line arguments.
	if (m_parser.getUnnamedArgs().size() < 2)
		throw Core::CmdLineException(TXT("No WMI query text specified"));

	if ( (m_parser.isSwitchSet(USER) && !m_parser.isSwitchSet(PASSWORD))
	  || (m_parser.isSwitchSet(PASSWORD) && !m_parser.isSwitchSet(USER)) )
		throw Core::CmdLineException(TXT("Both --user and --password must be specified together"));

	if ( (m_parser.isSwitchSet(SHOW_TYPES) && m_parser.isSwitchSet(ALIGN)) )
		throw Core::CmdLineException(TXT("Cannot specify --showtypes and --align together"));

	tstring		query    = m_parser.getUnnamedArgs().at(1);
	tstring		user     = m_parser.getSwitchValue(USER);
	tstring		password = m_parser.getSwitchValue(PASSWORD);
	bool		showHost = m_parser.isSwitchSet(SHOW_HOST);
	bool		showTypes = m_parser.isSwitchSet(SHOW_TYPES);
	bool		applyFormatting = !m_parser.isSwitchSet(NO_FORMAT);
	bool		align    = m_parser.isSwitchSet(ALIGN);
	Hostnames	hostnames;

	if (m_parser.isSwitchSet(HOSTNAMES))
	{
		const Core::CmdLineParser::StringVector& args = m_parser.getNamedArgs().find(HOSTNAMES)->second;

		hostnames.insert(hostnames.end(), args.begin(), args.end());
	}

	if (m_parser.isSwitchSet(HOSTSFILE))
	{
		tstring hostsFile = m_parser.getSwitchValue(HOSTSFILE);
		Core::CmdLineParser::StringVector args = readHostsFile(hostsFile);

		hostnames.insert(hostnames.end(), args.begin(), args.end());
	}

	if (hostnames.empty())
		hostnames.push_back(WMI::Connection::LOCALHOST);

	size_t maxItems = std::numeric_limits<size_t>::max();

	if (m_parser.isSwitchSet(TOP))
		maxItems = Core::parse<size_t>(m_parser.getSwitchValue(TOP));

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

		if (showHost)
		{
			if (applyFormatting)
				out << std::endl;

			out << TXT("Host");
			out << TXT(": ");
			out << host;
			out << std::endl;
		}

		// For all objects...
		for (size_t count = 0; (objectIter != objectEnd) && (count != maxItems); ++objectIter, ++count)
		{
			if (applyFormatting)
				out << std::endl;

			WMI::Object					object = *objectIter;
			WMI::Object::PropertyNames	names;

			object.getPropertyNames(names);

			// For all properties...
			PropNameIter nameIter = names.begin();
			PropNameIter nameEnd  = names.end();

			size_t maxNameLength = 0;

			for (; nameIter != nameEnd; ++nameIter)
			{
				const tstring& name = *nameIter;

				maxNameLength = std::max(name.length(), maxNameLength);
			}

			size_t nameWidth = (align) ? maxNameLength : 0;

			nameIter = names.begin();
			nameEnd  = names.end();

			for (; nameIter != nameEnd; ++nameIter)
			{
				const tstring& name = *nameIter;

				WCL::Variant value;

				object.getProperty(name, value);

				tstring formattedType = TXT(" [") + WCL::Variant::formatFullType(value) + TXT("]");
				tstring formattedValue = formatValue(value, applyFormatting);

				out << std::setiosflags(std::ios_base::left) << std::setw(nameWidth) << name;

				if (showTypes)
					out << formattedType;

				out << TXT(": ");
				out << formattedValue;
				out << std::endl;
			}
		}
	}

	return EXIT_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//! Read the list of hostnames from a text file. Empty lines are ignored as are
//! comments which start with the # character.

Core::CmdLineParser::StringVector QueryCmd::readHostsFile(const tstring& filename)
{
	Core::CmdLineParser::StringVector hosts;

	Core::TextFileIterator end;
	Core::TextFileIterator it(filename);

	for (; it != end; ++it)
	{
		tstring line(*it);

		size_t pos = line.find_first_of(TXT('#'));

		if (pos != tstring::npos)
			line.erase(pos);

		Core::trim(line);

		if (line.empty())
			continue;

		hosts.push_back(line);
	}

	return hosts;
}
