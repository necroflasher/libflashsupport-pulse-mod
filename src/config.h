#pragma once

struct config
{
	int latency_bytes;
	int latency_frames;
	int latency_msec;

	int frame_size; // size of audio frame in bytes (constant value)

	int trace_all; // trace entry/exit of functions

	int trace_timestamps; // include timestamps in trace output
	int trace_context;  // include file and line in trace output

	// set PA_STREAM_ADJUST_LATENCY in the pulseaudio stream flags
	// this matches what pulse normally does when PULSE_LATENCY_MSEC= is used
	// off by default because
	// 1. it can inconsistently assign a different (higher) value than requested
	// 2. the adjustment *might* be unnecessary in a low latency setup?
	int test_use_adjust_latency_flag;

	// test: instead of properly implementing FPX_SoundOutput_Latency(), just
	//  return the constant value from config->latency_frames
	// (does this make any difference???)
	int test_return_constant_latency_value;
};

extern const struct config *const config;
