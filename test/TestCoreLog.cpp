#include <common/corelog.hpp>
#include <stdio.h>
void TestCoreLog()
{
	corelog::set_preamble_uptime(false);
	//corelog::set_preamble_file(false);
	//corelog::set_preamble_date(false);
	//corelog::set_preamble_time(false);
	corelog::init(3);
	corelog::add_file("log/everything.log", corelog::Append, corelog::Verbosity_INFO);

	corelog::add_file("latest_readable.log", corelog::Truncate, corelog::Verbosity_INFO);

	{
		LOG_SCOPE_F(INFO, "Will indent all corelog messages within this scope.");
		LOG_F(INFO, "I'm hungry for some %.3f!", 3.14159);
		LOG_F(4, "Will only show if verbosity is 2 or higher");
	}

	LOG_SCOPE_FUNCTION(INFO);
	LOG_F(INFO, "Doing some stuff...");
	for (int i = 0; i < 2; ++i) {
		VLOG_SCOPE_F(1, "Iteration %d", i);
		LOG_SCOPE_F(ERROR, "Iteration %d", i);
		LOG_IF_F(WARNING, i == 1, "Bad result");
		LOG_F(WARNING, "==============================");
		LOG_F(ERROR, "- - - - - - - - -");
	}

}