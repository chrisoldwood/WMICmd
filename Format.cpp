////////////////////////////////////////////////////////////////////////////////
//! \file   Format.cpp
//! \brief  Helper functions for parsing and formatting WMI values.
//! \author Chris Oldwood

#include "Common.hpp"
#include "Format.hpp"
#include <Core/StringUtils.hpp>
#include <WMI/DateTime.hpp>
#include <WCL/VariantVector.hpp>
#include <Core/AnsiWide.hpp>
#include <malloc.h.>

static tstring formatIntegerValue(const WCL::Variant& value, bool applyFormatting);

////////////////////////////////////////////////////////////////////////////////
//! Get the string used to separate groups of digits in a number.

static tstring getGroupSeparator()
{
	int    count = ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SMONTHOUSANDSEP, NULL, 0);
	tchar* buffer = static_cast<tchar*>(_alloca(Core::numBytes<tchar>(count+1)));

	buffer[0] = TXT('\0');

	if (::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SMONTHOUSANDSEP, buffer, count+1) == 0)
		return TXT(",");

	return buffer;
}

////////////////////////////////////////////////////////////////////////////////
//! Try and convert a string into a datetime. The format of a WMI datetime is:-
//! YYYYMMDDHHMMSS.FFFFFF+TZO e.g. 20101008181758.546000+060

bool tryConvertDateTime(const tstring& value, tstring& datetime)
{
	CDateTime parsed;
	tstring   offset;

	if (!WMI::tryParseDateTime(value, parsed, offset))
		return false;

	datetime = Core::fmt(TXT("%s %s"), parsed.ToString().c_str(), offset.c_str());

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//! Try and convert a string into a 64-bit integer. WMI appears to return sint64
//! and uint64 values as strings (VT_BSTR).

bool tryConvert64BitInteger(const tstring& value, tstring& integer)
{
	tstring::const_iterator it = value.begin();
	tstring::const_iterator end = value.end();
	bool isSigned = false;

	if (it == end)
		return false;

	if (*it == TXT('-'))
	{
		isSigned = true;
		++it;
	}

	while (it != end)
	{
		if (!isdigit(*it))
			return false;

		++it;
	}	

	if (isSigned)
		integer = formatIntegerValue(WCL::Variant(Core::parse<int64>(value)), true);
	else
		integer = formatIntegerValue(WCL::Variant(Core::parse<uint64>(value)), true);

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//! Format an empty/null value.

static tstring formatEmptyValue(const WCL::Variant& value, bool applyFormatting)
{
	tstring result;

	if (applyFormatting)
	{
		ASSERT((value.type() == VT_EMPTY) || (value.type() == VT_NULL));

		if (value.type() == VT_EMPTY)
		{
			result = TXT("<empty>");
		}
		else if (value.type() == VT_NULL)
		{
			result = TXT("<null>");
		}
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////
//! Format a string value. If enabled it will look for strings that appear to be
//! WMI style datetimes and reformat them as a normal datetime.

static tstring formatStringValue(const WCL::Variant& value, bool applyFormatting)
{
	tstring result = value.format();

	if (applyFormatting)
	{
		tstring converted;

		if (tryConvertDateTime(result, converted))
		{
			result = converted;
		}
		else if (tryConvert64BitInteger(result, converted))
		{
			result = converted;
		}
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////
//! Format an integer value.

static tstring formatIntegerValue(const WCL::Variant& value, bool applyFormatting)
{
	tstring result;

	if (applyFormatting)
	{
		typedef tstring::reverse_iterator rev_iter;
		typedef tstring::const_reverse_iterator c_rev_iter;

		const tstring rawResult = value.format();

		const size_t  numDigits = (rawResult[0] != TXT('-')) ? rawResult.length() : (rawResult.length()-1);
		const tstring separator = getGroupSeparator();
		const size_t  numSeps = (numDigits-1) / 3;
		const size_t  total = rawResult.length() + (numSeps * separator.length());

		result = tstring(total, TXT(' '));

		c_rev_iter it = rawResult.rbegin();
		c_rev_iter end = rawResult.rend();

		size_t   digits = 0;
		size_t   seps = 0;
		rev_iter output = result.rbegin();

		while (it != end)
		{
			*output++ = *it++;

			if ( ((++digits % 3) == 0) && (seps++ != numSeps) )
				output = std::copy(separator.rbegin(), separator.rend(), output);
		}
	}
	else
	{
		result = value.format();
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////
//! Format a VARIANT type value.

tstring formatValue(const WCL::Variant& value, bool applyFormatting)
{
	tstring result;
	VARTYPE type = value.type();

	if ( (type == VT_EMPTY) || (type == VT_NULL))
	{
		result = formatEmptyValue(value, applyFormatting);
	}
	else if (type == VT_BSTR)
	{
		result = formatStringValue(value, applyFormatting);
	}
	else if ( (type == VT_I1 ) || (type == VT_I2 ) || (type == VT_I4 ) || (type == VT_I8 )
		   || (type == VT_UI1) || (type == VT_UI2) || (type == VT_UI4) || (type == VT_UI8) )
	{
		result = formatIntegerValue(value, applyFormatting);
	}
	else
	{
		if (value.isArray())
		{
			VARTYPE valueType = value.valueType();

			if (valueType == VT_BSTR)
			{
				typedef WCL::VariantVector<BSTR>::const_iterator c_iter;

				SAFEARRAY*               safeArray = V_ARRAY(&value);
				WCL::VariantVector<BSTR> array(safeArray, VT_BSTR, false);

				for (c_iter it = array.begin(); it != array.end(); ++it)
				{
					if (it != array.begin())
						result += TXT(',');

					result += W2T(*it);
				}
			}
			else
			{
				result = Core::fmt(TXT("<array of %s>"), WCL::Variant::formatType(valueType));
			}
		}
		else
		{
			if (!value.tryFormat(result))
				result = TXT("<conversion failed>");
		}
	}

	return result;
}
