#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

static struct config s_config = {
	.frame_size = 4, // short+short
};

const struct config *const config = &s_config;

static void fatal(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	abort();
}

static void init_latency(void);

__attribute__((constructor))
static void _init_config(void)
{
	if (getenv("TRACE_ALL"))
		s_config.trace_all = 1;

	if (getenv("TRACE_TIMESTAMPS"))
		s_config.trace_timestamps = 1;

	if (getenv("CONSTANT_LATENCY"))
		s_config.test_return_constant_latency_value = 1;

	init_latency();
}

__attribute__((constructor))
static void _init_lib(void)
{
	if (config->trace_all)
		trace("init");
}

__attribute__((destructor))
static void _fini_lib(void)
{
	if (config->trace_all)
		trace("fini");
}

static void init_latency(void)
{
	const char *var;
	int n;

	// get the main value

	if ((var = getenv("LATENCY_FRAMES")) != NULL)
	{
		if (
			sscanf(var, "%d%n", &s_config.latency_frames, &n) != 1 ||
			s_config.latency_frames <= 0 ||
			(long)n != (long)strlen(var))
		{
			fatal("bad latency value");
		}
	}
	else if ((var = getenv("LATENCY_MSEC")) != NULL)
	{
		if (
			sscanf(var, "%d%n", &s_config.latency_msec, &n) != 1 ||
			s_config.latency_msec <= 0 ||
			(long)n != (long)strlen(var))
		{
			fatal("bad latency value");
		}
	}
	else
	{
		// default
		s_config.latency_msec = 16;
	}

	// fill the other field

	if (s_config.latency_frames)
	{
		s_config.latency_msec = s_config.latency_frames/44.100;
	}
	else if (s_config.latency_msec)
	{
		s_config.latency_frames = s_config.latency_msec*44.100;
	}
	s_config.latency_bytes = s_config.latency_frames*s_config.frame_size;
}
