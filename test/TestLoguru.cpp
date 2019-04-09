#include <src/common/corelog.cpp>
#include <stdio.h>

void TestLogurur()
{
	// Optional, but useful to time-stamp the start of the corelog.
// Will also detect verbosity level on command line as -v.

	// Put every corelog message in "everything.corelog":
	corelog::set_thread_name("main");
	corelog::add_file("log/everything.log", corelog::Append, corelog::Verbosity_INFO);

	// Only corelog INFO, WARNING, ERROR and FATAL to "latest_readable.corelog":
	corelog::add_file("latest_readable.log", corelog::Truncate, corelog::Verbosity_INFO);

	// Only show most relevant things on stderr:
	corelog::g_stderr_verbosity = 1;

	{
		LOG_SCOPE_F(INFO, "Will indent all corelog messages within this scope.");
		LOG_F(INFO, "I'm hungry for some %.3f!", 3.14159);
		LOG_F(4, "Will only show if verbosity is 2 or higher");
	}

    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "Doing some stuff...");
    for (int i = 0; i < 2; ++i) {
        //VLOG_SCOPE_F(1, "Iteration %d", i);
        //LOG_SCOPE_F(ERROR, "Iteration %d", i);
        //LOG_IF_F(WARNING, i == 1, "Bad result");
        RAW_LOG_F(WARNING, "==============================");
    }

}