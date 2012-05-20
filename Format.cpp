////////////////////////////////////////////////////////////////////////////////
//! \file   Format.cpp
//! \brief  Helper functions for parsing and formatting WMI values.
//! \author Chris Oldwood

#include "Common.hpp"
#include "Format.hpp"
#include <Core/StringUtils.hpp>
#include <WCL/DateTime.hpp>

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

	if (*it != '.')
		return false;
	else
		++it;

	digits = 6; // Time fraction digits.

	while ( (digits != 0) && (it != end) )
	{
		if (!isdigit(*it))
			return false;

		--digits;
		++it;
	}	

	if ( (*it != '+') && (*it != '-') )
		return false;
	else
		++it;

	digits = 3; // Timezone offset digits.

	while ( (digits != 0) && (it != end) )
	{
		if (!isdigit(*it))
			return false;

		--digits;
		++it;
	}	

	const int year       = Core::parse<int>(value.substr( 0, 4));
	const int month      = Core::parse<int>(value.substr( 4, 2));
	const int day        = Core::parse<int>(value.substr( 6, 2));
	const int hours      = Core::parse<int>(value.substr( 8, 2));
	const int minutes    = Core::parse<int>(value.substr(10, 2));
	const int seconds    = Core::parse<int>(value.substr(12, 2));
	const tstring offset = value.substr(21, 4);

	if ((year < CDate::MIN_YEAR) || (year > CDate::MAX_YEAR))
		return false;

	if ((month < CDate::MIN_MONTH) || (month > CDate::MAX_MONTH))
		return false;

	if ((day < CDate::MIN_DAY) || (day > CDate::MAX_DAY))
		return false;

	if ((hours < CTime::MIN_HOURS) || (hours > CTime::MAX_HOURS))
		return false;

	if ((minutes < CTime::MIN_MINS) || (minutes > CTime::MAX_MINS))
		return false;

	if ((seconds < CTime::MIN_SECS) || (seconds > CTime::MAX_SECS))
		return false;

	CDateTime parsedDateTime(day, month, year, hours, minutes, seconds);

	datetime = Core::fmt(TXT("%s %s"), parsedDateTime.ToString().c_str(), offset.c_str());

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//! Format the value. If enabled it will look for strings that appear to be
//! WMI style datetimes and reformat them as a normal datetime.

tstring formatValue(const WCL::Variant& value, bool noFormatting)
{
	tstring result;

	if ( (value.type() == VT_EMPTY) || (value.type() == VT_NULL))
	{
		result = TXT("<null>");
	}
	else if (value.type() == VT_BSTR)
	{
		result = value.format();

		if (!noFormatting)
		{
			tstring datetime;

			if (tryConvertDateTime(result, datetime))
				result = datetime;
		}
	}
	else
	{
		if (value.isArray())
		{
			VARTYPE valueType = value.valueType();

			result = Core::fmt(TXT("<array of %s>"), WCL::Variant::formatType(valueType));
		}
		else
		{
			if (!value.tryFormat(result))
				result = TXT("<conversion failed>");
		}
	}

	return result;
}
