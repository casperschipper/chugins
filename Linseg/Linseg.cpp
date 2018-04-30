

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>


CK_DLL_CTOR(linseg_ctor);
CK_DLL_DTOR(linseg_dtor);

CK_DLL_MFUN(linseg_duration);
CK_DLL_MFUN(linseg_target);
CK_DLL_MFUN(linseg_durtar);
CK_DLL_MFUN(linseg_value);
CK_DLL_MFUN(linseg_getduration);
CK_DLL_MFUN(linseg_getvalue);



CK_DLL_TICK(linseg_tick);

t_CKINT linseg_data_offset = 0;


struct linsegData
{
    float target;
    float rate;
    float duration;
    float value;
    bool state;
};


CK_DLL_QUERY(linseg)
{
    QUERY->setname(QUERY, "Linseg");
    
    QUERY->begin_class(QUERY, "Linseg", "UGen");
    
    QUERY->add_ctor(QUERY, linseg_ctor);
    QUERY->add_dtor(QUERY, linseg_dtor);
    
    QUERY->add_ugen_func(QUERY, linseg_tick, NULL, 1, 1);
    
    QUERY->add_mfun(QUERY, linseg_duration, "dur", "duration"); // amount of time to take to get to next
    QUERY->add_arg(QUERY, "dur", "arg");
    
    QUERY->add_mfun(QUERY, linseg_target, "float", "target"); // the value to be reached, providing it equals reaching it
    QUERY->add_arg(QUERY, "float", "arg");
    
    /*
    QUERY->add_mfun(QUERY, linseg_durtar, "void", "target"); // set duration and time in one call.
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->add_arg(QUERY, "dur","arg");
    */
     
    QUERY->add_mfun(QUERY, linseg_value, "float", "value"); // set to a value immediately
    QUERY->add_arg(QUERY, "float", "arg");
    
    QUERY->add_mfun(QUERY, linseg_getduration, "dur", "duration"); //getter function
        
    QUERY->add_mfun(QUERY, linseg_getvalue, "float", "value"); // getter function
    
    linseg_data_offset = QUERY->add_mvar(QUERY, "int", "@data", false);
    
    QUERY->end_class(QUERY);

    return TRUE;
}


CK_DLL_CTOR(linseg_ctor)
{
    OBJ_MEMBER_INT(SELF, linseg_data_offset) = 0;
    
    linsegData * lsdata = new linsegData;

    lsdata->rate = 0;
    lsdata->value = 0;
    lsdata->target = 0;
    lsdata->duration = 0;
    OBJ_MEMBER_INT(SELF, linseg_data_offset) = (t_CKINT) lsdata;
}

CK_DLL_DTOR(linseg_dtor)
{
    linsegData * lsdata = (linsegData *) OBJ_MEMBER_INT(SELF, linseg_data_offset);
    if(lsdata)
    {
        delete lsdata;
        OBJ_MEMBER_INT(SELF, linseg_data_offset) = 0;
        lsdata = NULL;
    }
}

CK_DLL_TICK(linseg_tick)
{
    linsegData * lsdata = (linsegData *) OBJ_MEMBER_INT(SELF, linseg_data_offset);
    if (lsdata->state) {
        if(lsdata->value < lsdata->target) {
            lsdata->value += lsdata->rate;
            if(lsdata->value >= lsdata->target) {
                lsdata->state = 0;
                lsdata->value = lsdata->target;
                lsdata->rate = 0;
            }
        }
        if(lsdata->value > lsdata->target) {
            lsdata->value -= lsdata->rate;
            if(lsdata->value <= lsdata->target) {
                lsdata->value = lsdata->target;
                lsdata->state = 0;
                lsdata->rate = 0;
            }
        }
    }
    *out = lsdata->value;
    
    return TRUE;
}

CK_DLL_MFUN(linseg_target)
{
    linsegData * lsdata = (linsegData *) OBJ_MEMBER_INT(SELF, linseg_data_offset);
    lsdata->target = GET_NEXT_FLOAT(ARGS);
    lsdata->rate = fabs((lsdata->value-lsdata->target) / lsdata->duration);
    lsdata->state = 1;
    RETURN->v_float = (t_CKFLOAT) lsdata->target;
}

CK_DLL_MFUN(linseg_value)
{
    linsegData * lsdata = (linsegData *) OBJ_MEMBER_INT(SELF, linseg_data_offset);
    lsdata->value = GET_NEXT_FLOAT(ARGS);
    lsdata->state = 0;
    RETURN->v_float = (t_CKFLOAT) lsdata->value;
}

CK_DLL_MFUN(linseg_duration)
{
    linsegData * lsdata = (linsegData *) OBJ_MEMBER_INT(SELF, linseg_data_offset);
    lsdata->duration = (float) GET_NEXT_DUR(ARGS);
    if (lsdata->duration == 0) {
        lsdata->state = 0; // jump to value
        lsdata->value = lsdata->target;
    }
    if (lsdata->state == 1) lsdata->rate = fabs((lsdata->value-lsdata->target) / lsdata->duration);
    RETURN->v_dur = (t_CKDUR) lsdata->duration;
}

CK_DLL_MFUN(linseg_getduration)
{
    linsegData * lsdata = (linsegData *) OBJ_MEMBER_INT(SELF, linseg_data_offset);
    RETURN->v_dur = (t_CKDUR) lsdata->duration;
}

CK_DLL_MFUN(linseg_getvalue)
{
    linsegData * lsdata = (linsegData *) OBJ_MEMBER_INT(SELF, linseg_data_offset);
    RETURN->v_float = (t_CKDUR) lsdata->value;
}


