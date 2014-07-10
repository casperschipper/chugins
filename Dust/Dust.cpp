

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>


CK_DLL_CTOR(Dust_ctor);
CK_DLL_DTOR(Dust_dtor);

CK_DLL_MFUN(Dust_setProbability);
CK_DLL_MFUN(Dust_getProbability);

CK_DLL_TICK(Dust_tick);

t_CKINT Dust_data_offset = 0;


struct DustData
{
    float probability;
    SAMPLE currentSample;
};


CK_DLL_QUERY(Dust)
{
    QUERY->setname(QUERY, "Dust");
    
    QUERY->begin_class(QUERY, "Dust", "UGen");
    
    QUERY->add_ctor(QUERY, Dust_ctor);
    QUERY->add_dtor(QUERY, Dust_dtor);
    
    QUERY->add_ugen_func(QUERY, Dust_tick, NULL, 1, 1);
    
    QUERY->add_mfun(QUERY, Dust_setProbability, "float", "p");
    QUERY->add_arg(QUERY, "float", "p");
    
    QUERY->add_mfun(QUERY, Dust_getProbability, "float", "p");
    
    Dust_data_offset = QUERY->add_mvar(QUERY, "int", "@DustData", false);
    
    QUERY->end_class(QUERY);

    return TRUE;
}


CK_DLL_CTOR(Dust_ctor)
{
    OBJ_MEMBER_INT(SELF, Dust_data_offset) = 0;
    
    DustData * dustdata = new DustData;
    dustdata->probability = 0.01;
    
    OBJ_MEMBER_INT(SELF, Dust_data_offset) = (t_CKINT) dustdata;
}

CK_DLL_DTOR(Dust_dtor)
{
    DustData * dustdata = (DustData *) OBJ_MEMBER_INT(SELF, Dust_data_offset);
    if(dustdata)
    {
        delete dustdata;
        OBJ_MEMBER_INT(SELF, Dust_data_offset) = 0;
        dustdata = NULL;
    }
}

CK_DLL_TICK(Dust_tick)
{
    DustData * dustdata = (DustData *) OBJ_MEMBER_INT(SELF, Dust_data_offset);


    // just add the parameter controlled one, so when no input is connected, probabality can also be set manually.
    // take negative also as probability.
    SAMPLE probability = abs(in) + dustdata->probability;

    *out = ((float)rand()/RAND_MAX) * (probability > ((float)rand()/RAND_MAX));

    return TRUE;
}

CK_DLL_MFUN(Dust_setProbability)
{
    DustData * dustdata = (DustData *) OBJ_MEMBER_INT(SELF, Dust_data_offset);
    dustdata->probability = GET_NEXT_FLOAT(ARGS);
    RETURN->v_float = dustdata->probability;
}

CK_DLL_MFUN(Dust_getProbability)
{
    DustData * dustdata = (DustData *) OBJ_MEMBER_INT(SELF, Dust_data_offset);
    RETURN->v_float = dustdata->probability;
}




