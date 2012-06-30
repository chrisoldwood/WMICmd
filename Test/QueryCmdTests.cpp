////////////////////////////////////////////////////////////////////////////////
//! \file   QueryCmdTests.cpp
//! \brief  The unit tests for the QueryCmd class.
//! \author Chris Oldwood

#include "Common.hpp"
#include <Core/UnitTest.hpp>
#include "QueryCmd.hpp"
#include <sstream>
#include <WCL/AutoCom.hpp>

TEST_SET(QueryCmd)
{
	// These aren't stricly unit tests because they invoke the genuine WMI layer
	// via COM. However the WMI queries are fast and they are determistic to a
	// a large degree and so we save ourselves the effort of mocking the WMI API.
	WCL::AutoCom com(COINIT_APARTMENTTHREADED);

TEST_CASE("execute with no arguments should use the default settings")
{
	tchar*    argv[] = { TXT("Test.exe"), TXT("query"), TXT("select LastBootUpTime from Win32_OperatingSystem") };
	const int argc = ARRAY_SIZE(argv);

	QueryCmd       command(argc, argv);
	tostringstream out, err;

	int result = command.execute(out, err);

	TEST_TRUE(result == 0);
	TEST_TRUE(tstrstr(out.str().c_str(), TXT("LastBootUpTime")) != nullptr);
}
TEST_CASE_END

TEST_CASE("execute with --noformat should elide spacing between objects")
{
	tchar*    argv[] = { TXT("Test.exe"), TXT("query"), TXT("select Name from Win32_Service"), TXT("--noformat") };
	const int argc = ARRAY_SIZE(argv);

	QueryCmd       command(argc, argv);
	tostringstream out, err;

	int result = command.execute(out, err);

	TEST_TRUE(result == 0);
	TEST_TRUE(tstrstr(out.str().c_str(), TXT("\n\n")) == nullptr);
}
TEST_CASE_END

}
TEST_SET_END
