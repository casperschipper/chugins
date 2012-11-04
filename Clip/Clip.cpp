// v 0.1 Coded by Casper Schipper, 2011

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>

CK_DLL_TICK(clip_tick); // this is main dsp

CK_DLL_QUERY(clip) 
{
    QUERY->setname(QUERY, "Clip");
    
    QUERY->begin_class(QUERY, "Clip", "UGen");
    
    QUERY->add_ugen_func(QUERY, clip_tick, NULL, 1, 1); 
    
    // no args and not data
        
    QUERY->end_class(QUERY); 
    
    return TRUE;
}


CK_DLL_TICK(clip_tick)
{
    float value = in;
    if (value < -1) value = -1;
    else if (value > 1) value = 1;
    *out = value;
    
    return TRUE;
}




