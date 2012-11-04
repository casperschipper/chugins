// v 0.3 Coded by Casper Schipper, 2011

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>

CK_DLL_TICK(mtof_tick); // this is main dsp

CK_DLL_QUERY(mtof) 
{
    QUERY->setname(QUERY, "Mtof");
    
    QUERY->begin_class(QUERY, "Mtof", "UGen");
    
    QUERY->add_ugen_func(QUERY, mtof_tick, NULL, 1, 1); 
    
    // no args and not data
        
    QUERY->end_class(QUERY); 
    
    return TRUE;
}


CK_DLL_TICK(mtof_tick)
{
    *out = 440.0 * pow(2.0, ((in - 69.0) / 12.0));
    
    return TRUE;
}




