#include <chuck_dl.h>
#include <chuck_def.h>

CK_DLL_TICK(clipper_tick);

CK_DLL_QUERY(Clipper)
{
	QUERY->setname(QUERY, "Clipper");

	QUERY->begin_class(QUERY, "Clipper", "UGen");
	QUERY->add_ugen_func(QUERY, clipper_tick, NULL, 1, 1);
	QUERY->end_class(QUERY);

	return TRUE;
}

CK_DLL_TICK(clipper_tick)
{
	*out = in > 1. ? 1. : (in < -1. ? -1. : in);

	return TRUE;
}
