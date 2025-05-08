#pragma once

struct trace_ctx
{
	const char   *fmt;
	const char   *file;
	unsigned int  line;
	unsigned char warn : 1;
};

__attribute__((cold))
void trace_impl(const struct trace_ctx *ctx, ...);

__attribute__((format(printf, 1, 2)))
static inline void __dummy_check_fmt(const char *fmt, ...) {}

#define trace(fmt, ...) \
	do { \
		static const struct trace_ctx _trace_ctx = {fmt "\n", __FILE__, __LINE__}; \
		if (0) __dummy_check_fmt(fmt, ##__VA_ARGS__); \
		trace_impl(&_trace_ctx, ##__VA_ARGS__); \
	} while (0)

#define warn(fmt, ...) \
	do { \
		static const struct trace_ctx _trace_ctx = {fmt "\n", __FILE__, __LINE__, .warn = 1}; \
		if (0) __dummy_check_fmt(fmt, ##__VA_ARGS__); \
		trace_impl(&_trace_ctx, ##__VA_ARGS__); \
	} while (0)
