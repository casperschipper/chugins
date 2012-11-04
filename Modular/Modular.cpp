// v 0.1 Coded by Casper Schipper, 2011

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>

CK_DLL_TICK(Modular_tick); // this is main dsp
 
inline float Mod(float x, float y)
{
    if (0 == y)
        return x;
    
    return x - y * floor(x/y);
}

CK_DLL_QUERY(Modular) 
{
    QUERY->setname(QUERY, "Modular");
    
    QUERY->begin_class(QUERY, "Modular", "UGen");
    
    QUERY->add_ugen_func(QUERY, Modular_tick, NULL, 1, 1); 
        
    QUERY->end_class(QUERY); 
    
    return TRUE;
}


CK_DLL_TICK(Modular_tick)
{
    *out = Mod(in,1.0);
    
    return TRUE;
}




