////////////////////////////////////////////////////////////////////////////////
//! \file   FormatTests.cpp
//! \brief  The unit tests for the formatting and parsing helper functions.
//! \author Chris Oldwood

#include "Common.hpp"
#include <Core/UnitTest.hpp>
#include "Format.hpp"

TEST_SET(Format)
{

TEST_CASE("tryConvertDateTime should convert a well formed WMI datetime")
{
	const size_t count = 2;

	const tchar* cases[count][2] = 
	{
		//     YYYYMMDDHHMMSS.FFFFFF+TZO
		{ TXT("20010203040506.123456+060"), TXT("03/02/2001 04:05:06 +060") },
		{ TXT("20010203040506.123456-060"), TXT("03/02/2001 04:05:06 -060") },
	};

	for (size_t i = 0; i != count; ++i)
	{
		tstring	actual;

		bool suceeded = tryConvertDateTime(cases[i][0], actual);

		TEST_TRUE(suceeded);
		TEST_TRUE(actual == cases[i][1]);
	}
}
TEST_CASE_END

TEST_CASE("tryConvertDateTime should fail when input invalid")
{
	const tchar* cases[] = 
	{
		//   YYYYMMDDHHMMSS.FFFFFF+TZO

		// Invalid format.
		TXT("20010101000000#000000+000"),
		TXT("20010101000000.000000#000"),

		// Invalid year, month, day, etc.
		TXT("99990101000000.000000+000"),
		TXT("20019901000000.000000+000"),
		TXT("20010199000000.000000+000"),
		TXT("20010101990000.000000+000"),
		TXT("20010101009900.000000+000"),
		TXT("20010101000099.000000+000"),
	}; 

	const size_t count = ARRAY_SIZE(cases);

	for (size_t i = 0; i != count; ++i)
	{
		tstring	actual;

		TEST_FALSE(tryConvertDateTime(cases[i], actual));
	}
}
TEST_CASE_END

}
TEST_SET_END
