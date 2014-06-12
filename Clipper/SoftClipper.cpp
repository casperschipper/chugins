#include <math.h>

#include <chuck_dl.h>
#include <chuck_def.h>

CK_DLL_TICK(clipper_tick);

CK_DLL_QUERY(SoftClipper)
{
	QUERY->setname(QUERY, "SoftClipper");

	QUERY->begin_class(QUERY, "SoftClipper", "UGen");
	QUERY->add_ugen_func(QUERY, clipper_tick, NULL, 1, 1);
	QUERY->end_class(QUERY);

	return TRUE;
}

CK_DLL_TICK(clipper_tick)
{
#ifdef __CHUCK_USE_64_BIT_SAMPLE__
	*out = atan(in * 2.)/M_PI_2;
#else
	*out = atanf(in * 2.)/M_PI_2;
#endif

	return TRUE;
}
