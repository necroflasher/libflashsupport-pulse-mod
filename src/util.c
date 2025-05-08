#define _GNU_SOURCE /* gettid */
#include "util.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "config.h"

#define COLOR_START "\x1b[2m"
#define COLOR_START2 "\x1b[33m"
#define COLOR_END "\x1b[0m"

void trace_impl(const struct trace_ctx *ctx, ...)
{
	va_list ap;

	if (config->trace_timestamps)
	{
		struct timespec ts;
		const char *color;

		clock_gettime(CLOCK_REALTIME, &ts);

		color = COLOR_START;
		if (gettid() != getpid())
			color = COLOR_START2;

		fprintf(stderr, "%s[%4ld.%03ld]" COLOR_END " ",
			color,
			ts.tv_sec % (60*60),
			(ts.tv_nsec / 1000000) % 1000
			);
	}

	if (ctx->warn || config->trace_context)
		fprintf(stderr, "%s:%u: ", ctx->file, ctx->line);

	va_start(ap, ctx);
	vfprintf(stderr, ctx->fmt, ap);
	va_end(ap);
}
