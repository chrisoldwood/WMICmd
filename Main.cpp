////////////////////////////////////////////////////////////////////////////////
//! \file   Main.cpp
//! \brief  The console application entry point.
//! \author Chris Oldwood

#include "Common.hpp"
#include <tchar.h>
#include "WmiCmd.hpp"

////////////////////////////////////////////////////////////////////////////////
//! The process entry point.

int _tmain(int argc, tchar* argv[])
{
	return g_app.main(argc, argv);
}
