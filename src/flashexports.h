#pragma once

struct FPX_Functions
{
	int   count;
	void *funcs[27];
};

const void *FPX_Init(void *api);
