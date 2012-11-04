// v 0.1 Coded by Casper Schipper, 2011

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>

CK_DLL_TICK(tanh_tick); // this is main dsp

CK_DLL_QUERY(tanh) 
{
    QUERY->setname(QUERY, "Tanh");
    
    QUERY->begin_class(QUERY, "Tanh", "UGen");
    
    QUERY->add_ugen_func(QUERY, tanh_tick, NULL, 1, 1); 
    
    // no args and not data
        
    QUERY->end_class(QUERY); 
    
    return TRUE;
}


CK_DLL_TICK(tanh_tick)
{
    *out = tanh(in);
    
    return TRUE;
}




