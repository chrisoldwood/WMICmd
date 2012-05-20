////////////////////////////////////////////////////////////////////////////////
//! \file   Format.hpp
//! \brief  Helper functions for parsing and formatting WMI values.
//! \author Chris Oldwood

// Check for previous inclusion
#ifndef APP_FORMAT_HPP
#define APP_FORMAT_HPP

#if _MSC_VER > 1000
#pragma once
#endif

#include <WCL/Variant.hpp>

////////////////////////////////////////////////////////////////////////////////
// Try and convert a string into a datetime. The format of a WMI datetime is:-
// YYYYMMDDHHMMSS.FFFFFF+TZO e.g. 20101008181758.546000+060

bool tryConvertDateTime(const tstring& value, tstring& datetime);

////////////////////////////////////////////////////////////////////////////////
// Format the value. If enabled it will look for strings that appear to be
// WMI style datetimes and reformat them as a normal datetime.

tstring formatValue(const WCL::Variant& value, bool detectDates);

#endif // APP_FORMAT_HPP
