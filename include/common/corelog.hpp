/*
quote: https://github.com/emilk/loguru.git
adjust:
1.remove streaming
2.have modified namespace
3.rename filename
*/
#include <common/typedefs.h>
#if defined(LOG_IMPLEMENTATION)
	#warning "You are defining LOG_IMPLEMENTATION. This is for older versions of Loguru. You should now instead include corelog.cpp (or build it and link with it)"
#endif

// Disable all warnings from gcc/clang:
#if defined(__clang__)
	#pragma clang system_header
#elif defined(__GNUC__)
	#pragma GCC system_header
#endif

#ifndef LOG_HAS_DECLARED_FORMAT_HEADER
#define LOG_HAS_DECLARED_FORMAT_HEADER

// Semantic versioning. Loguru version can be printed with 
// printf("%d.%d.%d", LOG_VERSION_MAJOR, LOG_VERSION_MINOR, LOG_VERSION_PATCH);
#define LOG_VERSION_MAJOR 2
#define LOG_VERSION_MINOR 0
#define LOG_VERSION_PATCH 0

#if defined(_MSC_VER)
#include <sal.h>	// Needed for _In_z_ etc annotations
#endif

// ----------------------------------------------------------------------------

#ifndef LOG_SCOPE_TEXT_SIZE
	// Maximum length of text that can be printed by a LOG_SCOPE.
	// This should be long enough to get most things, but short enough not to clutter the stack.
	#define LOG_SCOPE_TEXT_SIZE 196
#endif

#ifndef LOG_FILENAME_WIDTH
	// Width of the column containing the file name
	#define LOG_FILENAME_WIDTH 23
#endif

#ifndef LOG_THREADNAME_WIDTH
	// Width of the column containing the thread name
	#define LOG_THREADNAME_WIDTH 16
#endif

#ifndef LOG_SCOPE_TIME_PRECISION
	// Resolution of scope timers. 3=ms, 6=us, 9=ns
	#define LOG_SCOPE_TIME_PRECISION 3
#endif

#ifndef LOG_CATCH_SIGABRT
	// Should Loguru catch SIGABRT to print stack trace etc?
	#define LOG_CATCH_SIGABRT 1
#endif

#ifndef LOG_VERBOSE_SCOPE_ENDINGS
	// Show milliseconds and scope name at end of scope.
	#define LOG_VERBOSE_SCOPE_ENDINGS 1
#endif

#ifndef LOG_REDEFINE_ASSERT
	#define LOG_REDEFINE_ASSERT 0
#endif

#ifndef LOG_WITH_STREAMS
	#define LOG_WITH_STREAMS 0
#endif

#ifndef LOG_REPLACE_GLOG
	#define LOG_REPLACE_GLOG 0
#endif

#if LOG_REPLACE_GLOG
	#undef LOG_WITH_STREAMS
	#define LOG_WITH_STREAMS 1
#endif

#ifndef LOG_UNSAFE_SIGNAL_HANDLER
	#define LOG_UNSAFE_SIGNAL_HANDLER 1
#endif

#if LOG_IMPLEMENTATION
	#undef LOG_WITH_STREAMS
	#define LOG_WITH_STREAMS 1
#endif

#ifndef LOG_USE_FMTLIB
	#define LOG_USE_FMTLIB 0
#endif

#ifndef LOG_WITH_FILEABS
	#define LOG_WITH_FILEABS 0
#endif

#ifndef LOG_RTTI
#if defined(__clang__)
	#if __has_feature(cxx_rtti)
		#define LOG_RTTI 1
	#endif
#elif defined(__GNUG__)
	#if defined(__GXX_RTTI)
		#define LOG_RTTI 1
	#endif
#elif defined(_MSC_VER)
	#if defined(_CPPRTTI)
		#define LOG_RTTI 1
	#endif
#endif
#endif

// --------------------------------------------------------------------
// Utility macros

#define LOG_CONCATENATE_IMPL(s1, s2) s1 ## s2
#define LOG_CONCATENATE(s1, s2) LOG_CONCATENATE_IMPL(s1, s2)

#ifdef __COUNTER__
#   define LOG_ANONYMOUS_VARIABLE(str) LOG_CONCATENATE(str, __COUNTER__)
#else
#   define LOG_ANONYMOUS_VARIABLE(str) LOG_CONCATENATE(str, __LINE__)
#endif

#if defined(__clang__) || defined(__GNUC__)
	// Helper macro for declaring functions as having similar signature to printf.
	// This allows the compiler to catch format errors at compile-time.
	#define LOG_PRINTF_LIKE(fmtarg, firstvararg) __attribute__((__format__ (__printf__, fmtarg, firstvararg)))
	#define LOG_FORMAT_STRING_TYPE const char*
#elif defined(_MSC_VER)
	#define LOG_PRINTF_LIKE(fmtarg, firstvararg)
	#define LOG_FORMAT_STRING_TYPE _In_z_ _Printf_format_string_ const char*
#else
	#define LOG_PRINTF_LIKE(fmtarg, firstvararg)
	#define LOG_FORMAT_STRING_TYPE const char*
#endif

// Used to mark corelog_and_abort for the benefit of the static analyzer and optimizer.
#if defined(_MSC_VER)
#define LOG_NORETURN __declspec(noreturn)
#else
#define LOG_NORETURN __attribute__((noreturn))
#endif

#if defined(_MSC_VER)
#define LOG_PREDICT_FALSE(x) (x)
#define LOG_PREDICT_TRUE(x)  (x)
#else
#define LOG_PREDICT_FALSE(x) (__builtin_expect(x,     0))
#define LOG_PREDICT_TRUE(x)  (__builtin_expect(!!(x), 1))
#endif

#if LOG_USE_FMTLIB
	#include <fmt/format.h>
#endif

#ifdef _WIN32
	#define STRDUP(str) _strdup(str)
#else
	#define STRDUP(str) strdup(str)
#endif

// --------------------------------------------------------------------

namespace corelog
{
	// Simple RAII ownership of a char*.
	class CORE_EXPORT Text
	{
	public:
		explicit Text(char* owned_str) : _str(owned_str) {}
		~Text();
		Text(Text&& t)
		{
			_str = t._str;
			t._str = nullptr;
		}
		Text(Text& t) = delete;
		Text& operator=(Text& t) = delete;
		void operator=(Text&& t) = delete;

		const char* c_str() const { return _str; }
		bool empty() const { return _str == nullptr || *_str == '\0'; }

		char* release()
		{
			auto result = _str;
			_str = nullptr;
			return result;
		}

	private:
		char* _str;
	};

	// Like printf, but returns the formated text.
	CORE_EXPORT
	Text textprintf(LOG_FORMAT_STRING_TYPE format, ...) LOG_PRINTF_LIKE(1, 2);

	// Overloaded for variadic template matching.
	CORE_EXPORT
	Text textprintf();

	using Verbosity = int;

#undef FATAL
#undef ERROR
#undef WARNING
#undef INFO
#undef MAX

	enum NamedVerbosity : Verbosity
	{
		// Used to mark an invalid verbosity. Do not corelog to this level.
		Verbosity_INVALID = -10, // Never do LOG_F(INVALID)

		// You may use Verbosity_OFF on g_stderr_verbosity, but for nothing else!
		Verbosity_OFF     = -9, // Never do LOG_F(OFF)

		// Prefer to use ABORT_F or ABORT_S over LOG_F(FATAL) or LOG_S(FATAL).
		Verbosity_FATAL   = -3,
		Verbosity_ERROR   = -2,
		Verbosity_WARNING = -1,

		// Normal messages. By default written to stderr.
		Verbosity_INFO    =  0,

		// Same as Verbosity_INFO in every way.
		Verbosity_0       =  0,

		// Verbosity levels 1-9 are generally not written to stderr, but are written to file.
		Verbosity_1       = +1,
		Verbosity_2       = +2,
		Verbosity_3       = +3,
		Verbosity_4       = +4,
		Verbosity_5       = +5,
		Verbosity_6       = +6,
		Verbosity_7       = +7,
		Verbosity_8       = +8,
		Verbosity_9       = +9,

		// Don not use higher verbosity levels, as that will make grepping corelog files harder.
		Verbosity_MAX     = +9,
	};

	struct Message
	{
		// You would generally print a Message by just concating the buffers without spacing.
		// Optionally, ignore preamble and indentation.
		Verbosity   verbosity;   // Already part of preamble
		const char* filename;    // Already part of preamble
		unsigned    line;        // Already part of preamble
		const char* preamble;    // Date, time, uptime, thread, file:line, verbosity.
		const char* indentation; // Just a bunch of spacing.
		const char* prefix;      // Assertion failure info goes here (or "").
		const char* message;     // User message goes here.
	};

	/* Everything with a verbosity equal or greater than g_stderr_verbosity will be
	written to stderr. You can set this in code or via the -v argument.
	Set to corelog::Verbosity_OFF to write nothing to stderr.
	Default is 0, i.e. only corelog ERROR, WARNING and INFO are written to stderr.
	*/
	CORE_EXPORT extern Verbosity g_stderr_verbosity;
	CORE_EXPORT extern bool      g_colorcorelogtostderr; // True by default.
	CORE_EXPORT extern unsigned  g_flush_interval_ms; // 0 (unbuffered) by default.
	CORE_EXPORT extern bool      g_preamble; // Prefix each corelog line with date, time etc? True by default.

	/* Specify the verbosity used by corelog to corelog its info messages including the header
	corelogged when corelogged::init() is called or on exit. Default is 0 (INFO).
	*/
	CORE_EXPORT extern Verbosity g_internal_verbosity;

	// Turn off individual parts of the preamble
	CORE_EXPORT extern bool      g_preamble_date; // The date field
	CORE_EXPORT extern bool      g_preamble_time; // The time of the current day
	CORE_EXPORT extern bool      g_preamble_uptime; // The time since init call
	CORE_EXPORT extern bool      g_preamble_thread; // The corelogging thread
	CORE_EXPORT extern bool      g_preamble_file; // The file from which the corelog originates from
	CORE_EXPORT extern bool      g_preamble_verbose; // The verbosity field
	CORE_EXPORT extern bool      g_preamble_pipe; // The pipe symbol right before the message

	// May not throw!
	typedef void (*corelog_handler_t)(void* user_data, const Message& message);
	typedef void (*close_handler_t)(void* user_data);
	typedef void (*flush_handler_t)(void* user_data);

	// May throw if that's how you'd like to handle your errors.
	typedef void (*fatal_handler_t)(const Message& message);

	// Given a verbosity level, return the level's name or nullptr.
	typedef const char* (*verbosity_to_name_t)(Verbosity verbosity);

	// Given a verbosity level name, return the verbosity level or
	// Verbosity_INVALID if name is not recognized.
	typedef Verbosity (*name_to_verbosity_t)(const char* name);

	/*  Should be called from the main thread.
		You don't *need* to call this, but if you do you get:
			* Signal handlers installed
			* Program arguments corelogged
			* Working dir corelogged
			* Optional -v verbosity flag parsed
			* Main thread name set to "main thread"
			* Explanation of the preamble (date, threanmae etc) corelogged

		corelog::init() will look for arguments meant for corelog and remove them.
		Arguments meant for corelog are:
			-v n   Set corelog::g_stderr_verbosity level. Examples:
				-v 3        Show verbosity level 3 and lower.
				-v 0        Only show INFO, WARNING, ERROR, FATAL (default).
				-v INFO     Only show INFO, WARNING, ERROR, FATAL (default).
				-v WARNING  Only show WARNING, ERROR, FATAL.
				-v ERROR    Only show ERROR, FATAL.
				-v FATAL    Only show FATAL.
				-v OFF      Turn off corelogging to stderr.

		Tip: You can set g_stderr_verbosity before calling corelog::init.
		That way you can set the default but have the user override it with the -v flag.
		Note that -v does not affect file corelogging (see corelog::add_file).

		You can use something else instead of "-v" via verbosity_flag.
		You can also set verbosity_flag to nullptr.
	*/
	CORE_EXPORT
	void init(int& argc, char* argv[], const char* verbosity_flag = "-v");

	// Will call remove_all_callbacks(). After calling this, corelogging will still go to stderr.
	// You generally don't need to call this.
	CORE_EXPORT
	void shutdown();

	// What ~ will be replaced with, e.g. "/home/your_user_name/"
	CORE_EXPORT
	const char* home_dir();

	/* Returns the name of the app as given in argv[0] but without leading path.
	   That is, if argv[0] is "../foo/app" this will return "app".
	*/
	CORE_EXPORT
	const char* argv0_filename();

	// Returns all arguments given to corelog::init(), but escaped with a single space as separator.
	CORE_EXPORT
	const char* arguments();

	// Returns the path to the current working dir when corelog::init() was called.
	CORE_EXPORT
	const char* current_dir();

	// Returns the part of the path after the last / or \ (if any).
	CORE_EXPORT
	const char* filename(const char* path);

	// e.g. "foo/bar/baz.ext" will create the directories "foo/" and "foo/bar/"
	CORE_EXPORT
	bool create_directories(const char* file_path_const);

	// Writes date and time with millisecond precision, e.g. "20151017_161503.123"
	CORE_EXPORT
	void write_date_time(char* buff, unsigned buff_size);

	// Helper: thread-safe version strerror
	CORE_EXPORT
	Text errno_as_text();

	/* Given a prefix of e.g. "~/corelog/" this might return
	   "/home/your_username/corelog/app_name/20151017_161503.123.corelog"

	   where "app_name" is a sanitized version of argv[0].
	*/
	CORE_EXPORT
	void suggest_corelog_path(const char* prefix, char* buff, unsigned buff_size);

	enum FileMode { Truncate, Append };

	/*  Will corelog to a file at the given path.
		Any corelogging message with a verbosity lower or equal to
		the given verbosity will be included.
		The function will create all directories in 'path' if needed.
		If path starts with a ~, it will be replaced with corelog::home_dir()
		To stop the file corelogging, just call corelog::remove_callback(path) with the same path.
	*/
	CORE_EXPORT
	bool add_file(const char* path, FileMode mode, Verbosity verbosity);

	/*  Will be called right before abort().
		You can for instance use this to print custom error messages, or throw an exception.
		Feel free to call LOG:ing function from this, but not FATAL ones! */
	CORE_EXPORT
	void set_fatal_handler(fatal_handler_t handler);

	// Get the current fatal handler, if any. Default value is nullptr.
	CORE_EXPORT
	fatal_handler_t get_fatal_handler();

	/*  Will be called on each corelog messages with a verbosity less or equal to the given one.
		Useful for displaying messages on-screen in a game, for example.
		The given on_close is also expected to flush (if desired).
	*/
	CORE_EXPORT
	void add_callback(
		const char*     id,
		corelog_handler_t   callback,
		void*           user_data,
		Verbosity       verbosity,
		close_handler_t on_close = nullptr,
		flush_handler_t on_flush = nullptr);

	/*  Set a callback that returns custom verbosity level names. If callback
		is nullptr or returns nullptr, default corelog names will be used.
	*/
	CORE_EXPORT
	void set_verbosity_to_name_callback(verbosity_to_name_t callback);

	/*  Set a callback that returns the verbosity level matching a name. The
		callback should return Verbosity_INVALID if the name is not
		recognized.
	*/
	CORE_EXPORT
	void set_name_to_verbosity_callback(name_to_verbosity_t callback);

	/*  Get a custom name for a specific verbosity, if one exists, or nullptr. */
	CORE_EXPORT
	const char* get_verbosity_name(Verbosity verbosity);

	/*  Get the verbosity enum value from a custom 4-character level name, if one exists.
		If the name does not match a custom level name, Verbosity_INVALID is returned.
	*/
	CORE_EXPORT
	Verbosity get_verbosity_from_name(const char* name);

	// Returns true iff the callback was found (and removed).
	CORE_EXPORT
	bool remove_callback(const char* id);

	// Shut down all file corelogging and any other callback hooks installed.
	CORE_EXPORT
	void remove_all_callbacks();

	// Returns the maximum of g_stderr_verbosity and all file/custom outputs.
	CORE_EXPORT
	Verbosity current_verbosity_cutoff();

#if LOG_USE_FMTLIB
	// Actual corelogging function. Use the LOG macro instead of calling this directly.
	CORE_EXPORT
	void corelog(Verbosity verbosity, const char* file, unsigned line, LOG_FORMAT_STRING_TYPE format, fmt::ArgList args);
	FMT_VARIADIC(void, corelog, Verbosity, const char*, unsigned, LOG_FORMAT_STRING_TYPE)

	// Log without any preamble or indentation.
	CORE_EXPORT
	void raw_corelog(Verbosity verbosity, const char* file, unsigned line, LOG_FORMAT_STRING_TYPE format, fmt::ArgList args);
	FMT_VARIADIC(void, raw_corelog, Verbosity, const char*, unsigned, LOG_FORMAT_STRING_TYPE)
#else // LOG_USE_FMTLIB?
	// Actual corelogging function. Use the LOG macro instead of calling this directly.
	CORE_EXPORT
	void corelog(Verbosity verbosity, const char* file, unsigned line, LOG_FORMAT_STRING_TYPE format, ...) LOG_PRINTF_LIKE(4, 5);

	// Log without any preamble or indentation.
	CORE_EXPORT
	void raw_corelog(Verbosity verbosity, const char* file, unsigned line, LOG_FORMAT_STRING_TYPE format, ...) LOG_PRINTF_LIKE(4, 5);
#endif // !LOG_USE_FMTLIB

	// Helper class for LOG_SCOPE_F
	class CORE_EXPORT LogScopeRAII
	{
	public:
		LogScopeRAII() : _file(nullptr) {} // No corelogging
		LogScopeRAII(Verbosity verbosity, const char* file, unsigned line, LOG_FORMAT_STRING_TYPE format, ...) LOG_PRINTF_LIKE(5, 6);
		~LogScopeRAII();

#if defined(_MSC_VER) && _MSC_VER > 1800
		// older MSVC default move ctors close the scope on move. See
		// issue #43
		LogScopeRAII(LogScopeRAII&& other)
			: _verbosity(other._verbosity)
			, _file(other._file)
			, _line(other._line)
			, _indent_stderr(other._indent_stderr)
			, _start_time_ns(other._start_time_ns)
		{
			// Make sure the tmp object's destruction doesn't close the scope:
			other._file = nullptr;

			for (unsigned int i = 0; i < LOG_SCOPE_TEXT_SIZE; ++i) {
				_name[i] = other._name[i];
			}
		}
#else
		LogScopeRAII(LogScopeRAII&&) = default;
#endif

	private:
		LogScopeRAII(const LogScopeRAII&) = delete;
		LogScopeRAII& operator=(const LogScopeRAII&) = delete;
		void operator=(LogScopeRAII&&) = delete;

		Verbosity   _verbosity;
		const char* _file; // Set to null if we are disabled due to verbosity
		unsigned    _line;
		bool        _indent_stderr; // Did we?
		long long   _start_time_ns;
		char        _name[LOG_SCOPE_TEXT_SIZE];
	};

	// Marked as 'noreturn' for the benefit of the static analyzer and optimizer.
	// stack_trace_skip is the number of extrace stack frames to skip above corelog_and_abort.
	CORE_EXPORT
	LOG_NORETURN void corelog_and_abort(int stack_trace_skip, const char* expr, const char* file, unsigned line, LOG_FORMAT_STRING_TYPE format, ...) LOG_PRINTF_LIKE(5, 6);
	CORE_EXPORT
	LOG_NORETURN void corelog_and_abort(int stack_trace_skip, const char* expr, const char* file, unsigned line);

	// Flush output to stderr and files.
	// If g_flush_interval_ms is set to non-zero, this will be called automatically this often.
	// If not set, you do not need to call this at all.
	CORE_EXPORT
	void flush();

	template<class T> inline Text format_value(const T&)                    { return textprintf("N/A");     }
	template<>        inline Text format_value(const char& v)               { return textprintf("%c",   v); }
	template<>        inline Text format_value(const int& v)                { return textprintf("%d",   v); }
	template<>        inline Text format_value(const unsigned int& v)       { return textprintf("%u",   v); }
	template<>        inline Text format_value(const long& v)               { return textprintf("%lu",  v); }
	template<>        inline Text format_value(const unsigned long& v)      { return textprintf("%ld",  v); }
	template<>        inline Text format_value(const long long& v)          { return textprintf("%llu", v); }
	template<>        inline Text format_value(const unsigned long long& v) { return textprintf("%lld", v); }
	template<>        inline Text format_value(const float& v)              { return textprintf("%f",   v); }
	template<>        inline Text format_value(const double& v)             { return textprintf("%f",   v); }

	/* Thread names can be set for the benefit of readable corelogs.
	   If you do not set the thread name, a hex id will be shown instead.
	   These thread names may or may not be the same as the system thread names,
	   depending on the system.
	   Try to limit the thread name to 15 characters or less. */
	CORE_EXPORT
	void set_thread_name(const char* name);

	/* Returns the thread name for this thread.
	   On OSX this will return the system thread name (settable from both within and without Loguru).
	   On other systems it will return whatever you set in set_thread_name();
	   If no thread name is set, this will return a hexadecimal thread id.
	   length should be the number of bytes available in the buffer.
	   17 is a good number for length.
	   right_align_hext_id means any hexadecimal thread id will be written to the end of buffer.
	*/
	CORE_EXPORT
	void get_thread_name(char* buffer, unsigned long long length, bool right_align_hext_id);

	/* Generates a readable stacktrace as a string.
	   'skip' specifies how many stack frames to skip.
	   For instance, the default skip (1) means:
	   don't include the call to corelog::stacktrace in the stack trace. */
	CORE_EXPORT
	Text stacktrace(int skip = 1);

	/*  Add a string to be replaced with something else in the stack output.

		For instance, instead of having a stack trace look like this:
			0x41f541 some_function(std::basic_ofstream<char, std::char_traits<char> >&)
		You can clean it up with:
			auto verbose_type_name = corelog::demangle(typeid(std::ofstream).name());
			corelog::add_stack_cleanup(verbose_type_name.c_str(); "std::ofstream");
		So the next time you will instead see:
			0x41f541 some_function(std::ofstream&)

		`replace_with_this` must be shorter than `find_this`.
	*/
	CORE_EXPORT
	void add_stack_cleanup(const char* find_this, const char* replace_with_this);

	// Example: demangle(typeid(std::ofstream).name()) -> "std::basic_ofstream<char, std::char_traits<char> >"
	CORE_EXPORT
	Text demangle(const char* name);

	// ------------------------------------------------------------------------
	/*
	Not all terminals support colors, but if they do, and g_colorcorelogtostderr
	is set, Loguru will write them to stderr to make errors in red, etc.

	You also have the option to manually use them, via the function below.

	Note, however, that if you do, the color codes could end up in your corelogfile!

	This means if you intend to use them functions you should either:
		* Use them on the stderr/stdout directly (bypass Loguru).
		* Don't add file outputs to Loguru.
		* Expect some \e[1m things in your corelogfile.

	Usage:
		printf("%sRed%sGreen%sBold green%sClear again\n",
			   corelog::terminal_red(), corelog::terminal_green(),
			   corelog::terminal_bold(), corelog::terminal_reset());

	If the terminal at hand does not support colors the above output
	will just not have funky \e[1m things showing.
	*/

	// Do the output terminal support colors?
	CORE_EXPORT
	bool terminal_has_color();

	// Colors
	CORE_EXPORT const char* terminal_black();
	CORE_EXPORT const char* terminal_red();
	CORE_EXPORT const char* terminal_green();
	CORE_EXPORT const char* terminal_yellow();
	CORE_EXPORT const char* terminal_blue();
	CORE_EXPORT const char* terminal_purple();
	CORE_EXPORT const char* terminal_cyan();
	CORE_EXPORT const char* terminal_light_gray();
	CORE_EXPORT const char* terminal_light_red();
	CORE_EXPORT const char* terminal_white();

	// Formating
	CORE_EXPORT const char* terminal_bold();
	CORE_EXPORT const char* terminal_underline();

	// You should end each line with this!
	CORE_EXPORT const char* terminal_reset();

	// --------------------------------------------------------------------
	// Error context related:

	struct StringStream;

	// Use this in your EcEntryBase::print_value overload.
	CORE_EXPORT
	void stream_print(StringStream& out_string_stream, const char* text);

	class CORE_EXPORT EcEntryBase
	{
	public:
		EcEntryBase(const char* file, unsigned line, const char* descr);
		~EcEntryBase();
		EcEntryBase(const EcEntryBase&) = delete;
		EcEntryBase(EcEntryBase&&) = delete;
		EcEntryBase& operator=(const EcEntryBase&) = delete;
		EcEntryBase& operator=(EcEntryBase&&) = delete;

		virtual void print_value(StringStream& out_string_stream) const = 0;

		EcEntryBase* previous() const { return _previous; }

	// private:
		const char*  _file;
		unsigned     _line;
		const char*  _descr;
		EcEntryBase* _previous;
	};

	template<typename T>
	class EcEntryData : public EcEntryBase
	{
	public:
		using Printer = Text(*)(T data);

		EcEntryData(const char* file, unsigned line, const char* descr, T data, Printer&& printer)
			: EcEntryBase(file, line, descr), _data(data), _printer(printer) {}

		virtual void print_value(StringStream& out_string_stream) const override
		{
			const auto str = _printer(_data);
			stream_print(out_string_stream, str.c_str());
		}

	private:
		T       _data;
		Printer _printer;
	};

	// template<typename Printer>
	// class EcEntryLambda : public EcEntryBase
	// {
	// public:
	// 	EcEntryLambda(const char* file, unsigned line, const char* descr, Printer&& printer)
	// 		: EcEntryBase(file, line, descr), _printer(std::move(printer)) {}

	// 	virtual void print_value(StringStream& out_string_stream) const override
	// 	{
	// 		const auto str = _printer();
	// 		stream_print(out_string_stream, str.c_str());
	// 	}

	// private:
	// 	Printer _printer;
	// };

	// template<typename Printer>
	// EcEntryLambda<Printer> make_ec_entry_lambda(const char* file, unsigned line, const char* descr, Printer&& printer)
	// {
	// 	return {file, line, descr, std::move(printer)};
	// }

	template <class T>
	struct decay_char_array { using type = T; };

	template <unsigned long long  N>
	struct decay_char_array<const char(&)[N]> { using type = const char*; };

	template <class T>
	struct make_const_ptr { using type = T; };

	template <class T>
	struct make_const_ptr<T*> { using type = const T*; };

	template <class T>
	struct make_ec_type { using type = typename make_const_ptr<typename decay_char_array<T>::type>::type; };

	/* 	A stack trace gives you the names of the function at the point of a crash.
		With ERROR_CONTEXT, you can also get the values of select local variables.
		Usage:

		void process_customers(const std::string& filename)
		{
			ERROR_CONTEXT("Processing file", filename.c_str());
			for (int customer_index : ...)
			{
				ERROR_CONTEXT("Customer index", customer_index);
				...
			}
		}

		The context is in effect during the scope of the ERROR_CONTEXT.
		Use corelog::get_error_context() to get the contents of the active error contexts.

		Example result:

		------------------------------------------------
		[ErrorContext]                main.cpp:416   Processing file:    "customers.json"
		[ErrorContext]                main.cpp:417   Customer index:     42
		------------------------------------------------

		Error contexts are printed automatically on crashes, and only on crashes.
		This makes them much faster than corelogging the value of a variable.
	*/
	#define ERROR_CONTEXT(descr, data)                                             \
		const corelog::EcEntryData<corelog::make_ec_type<decltype(data)>::type>      \
			LOG_ANONYMOUS_VARIABLE(error_context_scope_)(                       \
				__FILE__, __LINE__, descr, data,                                   \
				static_cast<corelog::EcEntryData<corelog::make_ec_type<decltype(data)>::type>::Printer>(corelog::ec_to_text) ) // For better error messages

/*
	#define ERROR_CONTEXT(descr, data)                                 \
		const auto LOG_ANONYMOUS_VARIABLE(error_context_scope_)(    \
			corelog::make_ec_entry_lambda(__FILE__, __LINE__, descr,    \
				[=](){ return corelog::ec_to_text(data); }))
*/

	using EcHandle = const EcEntryBase*;

	/*
		Get a light-weight handle to the error context stack on this thread.
		The handle is valid as long as the current thread has no changes to its error context stack.
		You can pass the handle to corelog::get_error_context on another thread.
		This can be very useful for when you have a parent thread spawning several working threads,
		and you want the error context of the parent thread to get printed (too) when there is an
		error on the child thread. You can accomplish this thusly:

		void foo(const char* parameter)
		{
			ERROR_CONTEXT("parameter", parameter)
			const auto parent_ec_handle = corelog::get_thread_ec_handle();

			std::thread([=]{
				corelog::set_thread_name("child thread");
				ERROR_CONTEXT("parent context", parent_ec_handle);
				dangerous_code();
			}.join();
		}

	*/
	CORE_EXPORT
	EcHandle get_thread_ec_handle();

	// Get a string describing the current stack of error context. Empty string if there is none.
	CORE_EXPORT
	Text get_error_context();

	// Get a string describing the error context of the given thread handle.
	CORE_EXPORT
	Text get_error_context_for(EcHandle ec_handle);

	// ------------------------------------------------------------------------

	CORE_EXPORT Text ec_to_text(const char* data);
	CORE_EXPORT Text ec_to_text(char data);
	CORE_EXPORT Text ec_to_text(int data);
	CORE_EXPORT Text ec_to_text(unsigned int data);
	CORE_EXPORT Text ec_to_text(long data);
	CORE_EXPORT Text ec_to_text(unsigned long data);
	CORE_EXPORT Text ec_to_text(long long data);
	CORE_EXPORT Text ec_to_text(unsigned long long data);
	CORE_EXPORT Text ec_to_text(float data);
	CORE_EXPORT Text ec_to_text(double data);
	CORE_EXPORT Text ec_to_text(long double data);
	CORE_EXPORT Text ec_to_text(EcHandle);

	/*
	You can add ERROR_CONTEXT support for your own types by overloading ec_to_text. Here's how:

	some.hpp:
		namespace corelog {
			Text ec_to_text(MySmallType data)
			Text ec_to_text(const MyBigType* data)
		} // namespace corelog

	some.cpp:
		namespace corelog {
			Text ec_to_text(MySmallType small_value)
			{
				// Called only when needed, i.e. on a crash.
				std::string str = small_value.as_string(); // Format 'small_value' here somehow.
				return Text{STRDUP(str.c_str())};
			}

			Text ec_to_text(const MyBigType* big_value)
			{
				// Called only when needed, i.e. on a crash.
				std::string str = big_value->as_string(); // Format 'big_value' here somehow.
				return Text{STRDUP(str.c_str())};
			}
		} // namespace corelog

	Any file that include some.hpp:
		void foo(MySmallType small, const MyBigType& big)
		{
			ERROR_CONTEXT("Small", small); // Copy ´small` by value.
			ERROR_CONTEXT("Big",   &big);  // `big` should not change during this scope!
			....
		}
	*/
} // namespace corelog

// --------------------------------------------------------------------
// Logging macros

// LOG_F(2, "Only corelogged if verbosity is 2 or higher: %d", some_number);
#define VLOG_F(verbosity, ...)                                                                     \
	((verbosity) > corelog::current_verbosity_cutoff()) ? (void)0                                   \
									  : corelog::corelog(verbosity, __FILE__, __LINE__, __VA_ARGS__)

// LOG_F(INFO, "Foo: %d", some_number);
#define LOG_F(verbosity_name, ...) VLOG_F(corelog::Verbosity_ ## verbosity_name, __VA_ARGS__)

#define VLOG_IF_F(verbosity, cond, ...)                                                            \
	((verbosity) > corelog::current_verbosity_cutoff() || (cond) == false)                          \
		? (void)0                                                                                  \
		: corelog::corelog(verbosity, __FILE__, __LINE__, __VA_ARGS__)

#define LOG_IF_F(verbosity_name, cond, ...)                                                        \
	VLOG_IF_F(corelog::Verbosity_ ## verbosity_name, cond, __VA_ARGS__)

#define VLOG_SCOPE_F(verbosity, ...)                                                               \
	corelog::LogScopeRAII LOG_ANONYMOUS_VARIABLE(error_context_RAII_) =                          \
	((verbosity) > corelog::current_verbosity_cutoff()) ? corelog::LogScopeRAII() :                  \
	corelog::LogScopeRAII(verbosity, __FILE__, __LINE__, __VA_ARGS__)

// Raw corelogging - no preamble, no indentation. Slightly faster than full corelogging.
#define RAW_VLOG_F(verbosity, ...)                                                                 \
	((verbosity) > corelog::current_verbosity_cutoff()) ? (void)0                                   \
									  : corelog::raw_corelog(verbosity, __FILE__, __LINE__, __VA_ARGS__)

#define RAW_LOG_F(verbosity_name, ...) RAW_VLOG_F(corelog::Verbosity_ ## verbosity_name, __VA_ARGS__)

// Use to book-end a scope. Affects corelogging on all threads.
#define LOG_SCOPE_F(verbosity_name, ...)                                                           \
	VLOG_SCOPE_F(corelog::Verbosity_ ## verbosity_name, __VA_ARGS__)

#define LOG_SCOPE_FUNCTION(verbosity_name) LOG_SCOPE_F(verbosity_name, __func__)

// -----------------------------------------------
// ABORT_F macro. Usage:  ABORT_F("Cause of error: %s", error_str);

// Message is optional
#define ABORT_F(...) corelog::corelog_and_abort(0, "ABORT: ", __FILE__, __LINE__, __VA_ARGS__)

// --------------------------------------------------------------------
// CHECK_F macros:

#define CHECK_WITH_INFO_F(test, info, ...)                                                         \
	LOG_PREDICT_TRUE((test) == true) ? (void)0 : corelog::corelog_and_abort(0, "CHECK FAILED:  " info "  ", __FILE__,      \
													   __LINE__, ##__VA_ARGS__)

/* Checked at runtime too. Will print error, then call fatal_handler (if any), then 'abort'.
   Note that the test must be boolean.
   CHECK_F(ptr); will not compile, but CHECK_F(ptr != nullptr); will. */
#define CHECK_F(test, ...) CHECK_WITH_INFO_F(test, #test, ##__VA_ARGS__)

#define CHECK_NOTNULL_F(x, ...) CHECK_WITH_INFO_F((x) != nullptr, #x " != nullptr", ##__VA_ARGS__)

#define CHECK_OP_F(expr_left, expr_right, op, ...)                                                 \
	do                                                                                             \
	{                                                                                              \
		auto val_left = expr_left;                                                                 \
		auto val_right = expr_right;                                                               \
		if (! LOG_PREDICT_TRUE(val_left op val_right))                                          \
		{                                                                                          \
			auto str_left = corelog::format_value(val_left);                                        \
			auto str_right = corelog::format_value(val_right);                                      \
			auto fail_info = corelog::textprintf("CHECK FAILED:  %s %s %s  (%s %s %s)  ",           \
				#expr_left, #op, #expr_right, str_left.c_str(), #op, str_right.c_str());           \
			auto user_msg = corelog::textprintf(__VA_ARGS__);                                       \
			corelog::corelog_and_abort(0, fail_info.c_str(), __FILE__, __LINE__,                        \
			                      "%s", user_msg.c_str());                                         \
		}                                                                                          \
	} while (false)

#ifndef LOG_DEBUG_LOGGING
	#ifndef NDEBUG
		#define LOG_DEBUG_LOGGING 1
	#else
		#define LOG_DEBUG_LOGGING 0
	#endif
#endif

#if LOG_DEBUG_LOGGING
	// Debug corelogging enabled:
	#define DLOG_F(verbosity_name, ...)     LOG_F(verbosity_name, __VA_ARGS__)
	#define DVLOG_F(verbosity, ...)         VLOG_F(verbosity, __VA_ARGS__)
	#define DLOG_IF_F(verbosity_name, ...)  LOG_IF_F(verbosity_name, __VA_ARGS__)
	#define DVLOG_IF_F(verbosity, ...)      VLOG_IF_F(verbosity, __VA_ARGS__)
	#define DRAW_LOG_F(verbosity_name, ...) RAW_LOG_F(verbosity_name, __VA_ARGS__)
	#define DRAW_VLOG_F(verbosity, ...)     RAW_VLOG_F(verbosity, __VA_ARGS__)
#else
	// Debug corelogging disabled:
	#define DLOG_F(verbosity_name, ...)
	#define DVLOG_F(verbosity, ...)
	#define DLOG_IF_F(verbosity_name, ...)
	#define DVLOG_IF_F(verbosity, ...)
	#define DRAW_LOG_F(verbosity_name, ...)
	#define DRAW_VLOG_F(verbosity, ...)
#endif

#define CHECK_EQ_F(a, b, ...) CHECK_OP_F(a, b, ==, ##__VA_ARGS__)
#define CHECK_NE_F(a, b, ...) CHECK_OP_F(a, b, !=, ##__VA_ARGS__)
#define CHECK_LT_F(a, b, ...) CHECK_OP_F(a, b, < , ##__VA_ARGS__)
#define CHECK_GT_F(a, b, ...) CHECK_OP_F(a, b, > , ##__VA_ARGS__)
#define CHECK_LE_F(a, b, ...) CHECK_OP_F(a, b, <=, ##__VA_ARGS__)
#define CHECK_GE_F(a, b, ...) CHECK_OP_F(a, b, >=, ##__VA_ARGS__)

#ifndef LOG_DEBUG_CHECKS
	#ifndef NDEBUG
		#define LOG_DEBUG_CHECKS 1
	#else
		#define LOG_DEBUG_CHECKS 0
	#endif
#endif

#if LOG_DEBUG_CHECKS
	// Debug checks enabled:
	#define DCHECK_F(test, ...)             CHECK_F(test, ##__VA_ARGS__)
	#define DCHECK_NOTNULL_F(x, ...)        CHECK_NOTNULL_F(x, ##__VA_ARGS__)
	#define DCHECK_EQ_F(a, b, ...)          CHECK_EQ_F(a, b, ##__VA_ARGS__)
	#define DCHECK_NE_F(a, b, ...)          CHECK_NE_F(a, b, ##__VA_ARGS__)
	#define DCHECK_LT_F(a, b, ...)          CHECK_LT_F(a, b, ##__VA_ARGS__)
	#define DCHECK_LE_F(a, b, ...)          CHECK_LE_F(a, b, ##__VA_ARGS__)
	#define DCHECK_GT_F(a, b, ...)          CHECK_GT_F(a, b, ##__VA_ARGS__)
	#define DCHECK_GE_F(a, b, ...)          CHECK_GE_F(a, b, ##__VA_ARGS__)
#else
	// Debug checks disabled:
	#define DCHECK_F(test, ...)
	#define DCHECK_NOTNULL_F(x, ...)
	#define DCHECK_EQ_F(a, b, ...)
	#define DCHECK_NE_F(a, b, ...)
	#define DCHECK_LT_F(a, b, ...)
	#define DCHECK_LE_F(a, b, ...)
	#define DCHECK_GT_F(a, b, ...)
	#define DCHECK_GE_F(a, b, ...)
#endif // NDEBUG


#if LOG_REDEFINE_ASSERT
	#undef assert
	#ifndef NDEBUG
		// Debug:
		#define assert(test) CHECK_WITH_INFO_F(!!(test), #test) // HACK
	#else
		#define assert(test)
	#endif
#endif // LOG_REDEFINE_ASSERT

#endif // LOG_HAS_DECLARED_FORMAT_HEADER
