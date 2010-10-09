////////////////////////////////////////////////////////////////////////////////
//! \file   QueryCmd.cpp
//! \brief  The QueryCmd class definition.
//! \author Chris Oldwood

#include "Common.hpp"
#include "QueryCmd.hpp"
#include "CmdLineArgs.hpp"
#include <Core/CmdLineException.hpp>
#include <Core/tiostream.hpp>
#include <Core/StringUtils.hpp>
#include <WCL/VariantBool.hpp>
#include <WMI/Connection.hpp>
#include <WMI/ObjectIterator.hpp>

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
	{ DETECT_DATES,	TXT("dd"),	TXT("detectdates"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,				TXT("Detect date strings and display appropriately")	},
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
	tstring		user     = m_parser.getSwitchValue(USER);
	tstring		password = m_parser.getSwitchValue(PASSWORD);
	bool		showHost = m_parser.isSwitchSet(SHOW_HOST);
	bool		showTypes = m_parser.isSwitchSet(SHOW_TYPES);
	bool		detectDates = m_parser.isSwitchSet(DETECT_DATES);
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
			tcout << std::endl;

		// For all objects...
		for (; objectIter != objectEnd; ++objectIter)
		{
			if (showHost)
			{
				tcout << TXT("Host");
				tcout << TXT(": ");
				tcout << host;
				tcout << std::endl;
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

				tcout << name;

				if (showTypes)
				{
					tcout << TXT(" [") << WCL::Variant::formatFullType(value) << TXT("]");
				}

				tcout << TXT(": ");
				tcout << formatValue(value, detectDates);
				tcout << std::endl;
			}

			tcout << std::endl;
		}
	}

	return EXIT_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//! Try and convert a string into a datetime. The format of a WMI datetime is:-
//! YYYYMMDDHHMMSS.FFFFFF+TZO e.g. 20101008181758.546000+060

bool tryConvertDateTime(const tstring& value, tstring& datetime)
{
	if (value.size() != 25)
		return false;

	tstring::const_iterator it = value.begin();
	tstring::const_iterator end = value.end();

	size_t digits = 8 + 6; // Date + Time digits.

	while ( (digits != 0) && (it != end) )
	{
		if (!isdigit(*it))
			return false;

		--digits;
		++it;
	}	

	if (*it++ != '.')
		return false;

	digits = 6; // Time fraction digits.

	while ( (digits != 0) && (it != end) )
	{
		if (!isdigit(*it))
			return false;

		--digits;
		++it;
	}	

	if (*it++ != '+')
		return false;

	digits = 3; // Timezone offset digits.

	while ( (digits != 0) && (it != end) )
	{
		if (!isdigit(*it))
			return false;

		--digits;
		++it;
	}	

	const tstring year     = value.substr( 0, 4);
	const tstring month    = value.substr( 4, 2);
	const tstring day      = value.substr( 6, 2);
	const tstring hours    = value.substr( 8, 2);
	const tstring minutes  = value.substr(10, 2);
	const tstring seconds  = value.substr(12, 2);
	const tstring fraction = value.substr(15, 6);
	const tstring offset   = value.substr(22, 3);

	datetime = Core::fmt(TXT("%s/%s/%s %s:%s:%s +%s"),
						 day.c_str(), month.c_str(), year.c_str(),
						 hours.c_str(), minutes.c_str(), seconds.c_str(),
						 offset.c_str());

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//! Format the value. If enabled it will look for strings that appear to be
//! WMI style datetimes and reformat them as a normal datetime.

tstring QueryCmd::formatValue(const WCL::Variant& value, bool detectDates)
{
	tstring result;

	if (value.IsArray())
	{
		result = TXT("<array>");
	}
	else
	{
		result = value.format();

		if (detectDates)
		{
			tstring datetime;

			if (tryConvertDateTime(result, datetime))
				return datetime;
		}
	}

	return result;
}
