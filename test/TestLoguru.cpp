#include "loguru.hpp"
#include <stdio.h>

void TestLogurur()
{
	// Optional, but useful to time-stamp the start of the log.
// Will also detect verbosity level on command line as -v.

	// Put every log message in "everything.log":
	loguru::set_thread_name("main");
	loguru::add_file("everything.log", loguru::Append, loguru::Verbosity_9);

	// Only log INFO, WARNING, ERROR and FATAL to "latest_readable.log":
	loguru::add_file("latest_readable.log", loguru::Truncate, loguru::Verbosity_INFO);

	// Only show most relevant things on stderr:
	loguru::g_stderr_verbosity = 1;

	{
		LOG_SCOPE_F(INFO, "Will indent all log messages within this scope.");
		LOG_F(INFO, "I'm hungry for some %.3f!", 3.14159);
		LOG_F(2, "Will only show if verbosity is 2 or higher");
	}
	LOG_F(INFO, "%sRed%sGreen%sBold green%sClear again\n",
		   loguru::terminal_red(), loguru::terminal_green(),
		   loguru::terminal_bold(), loguru::terminal_reset()); 
}