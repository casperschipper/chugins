// v 0.3 Coded by Casper Schipper, 2011

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>

CK_DLL_CTOR(scaler_ctor); // constructor
CK_DLL_DTOR(scaler_dtor); // deconstructor

CK_DLL_MFUN(scaler_min_in); // Minimum input
CK_DLL_MFUN(scaler_max_in); // Maximum input
CK_DLL_MFUN(scaler_min_out); // Minimum output
CK_DLL_MFUN(scaler_max_out); // Maximum output
CK_DLL_MFUN(scaler_exp); // exponentialness

CK_DLL_TICK(scaler_tick); // this is the main loop

t_CKINT scaler_data_offset = 0; 

struct scalerData
{
    float minIn;
    float maxIn;
    float rangeIn;
    float minOut;
    float maxOut;
    float rangeOut;
    float exp;
};

float mini(float a,float b) {
    return (a < b) ? a : b;
}

float maxi(float a,float b) {
    return (a > b) ? a : b;
}

CK_DLL_QUERY(scaler) 
{
    QUERY->setname(QUERY, "Scaler");
    
    QUERY->begin_class(QUERY, "Scaler", "UGen");
    
    QUERY->add_ctor(QUERY, scaler_ctor); // add the constructor
    QUERY->add_dtor(QUERY, scaler_dtor); // add the deconstructor
    
    QUERY->add_ugen_func(QUERY, scaler_tick, NULL, 1, 1); 

    QUERY->add_mfun(QUERY, scaler_min_in, "void", "minIn"); // expected minimum of input
    QUERY->add_arg(QUERY, "float", "arg");
    
    QUERY->add_mfun(QUERY, scaler_max_in, "void", "maxIn"); // expected maximum of input
    QUERY->add_arg(QUERY, "float", "arg"); 
    
    QUERY->add_mfun(QUERY, scaler_min_out, "void", "minOut"); // desired minimum of output
    QUERY->add_arg(QUERY, "float", "arg");
    
    QUERY->add_mfun(QUERY, scaler_max_out, "void", "maxOut"); // desired maximum of output
    QUERY->add_arg(QUERY, "float", "arg"); 
    
    QUERY->add_mfun(QUERY, scaler_exp, "void", "exp"); // desired maximum of output
    QUERY->add_arg(QUERY, "float", "arg"); 
    
    scaler_data_offset = QUERY->add_mvar(QUERY, "int", "@data", false);
    
    QUERY->end_class(QUERY); 
    
    return TRUE;
}


CK_DLL_CTOR(scaler_ctor) // constructing and standard values.
{
    OBJ_MEMBER_INT(SELF, scaler_data_offset) = 0; //
    
    scalerData * scdata = new scalerData; // scaler data object
    
    scdata->minIn = -1;
    scdata->maxIn = 1;
    scdata->rangeIn = 2;
    scdata->minOut = 0;
    scdata->maxOut = 1;
    scdata->rangeOut = 1;
    scdata->exp = 1;
    
    OBJ_MEMBER_INT(SELF, scaler_data_offset) = (t_CKINT) scdata;
}

CK_DLL_DTOR(scaler_dtor) // deconstruction
{
    scalerData * scdata = (scalerData *) OBJ_MEMBER_INT(SELF, scaler_data_offset);
    if(scdata)
    {
        delete scdata;    
        OBJ_MEMBER_INT(SELF, scaler_data_offset) = 0;
        scdata = NULL;
    }
}

CK_DLL_TICK(scaler_tick)
{
    scalerData * scdata = (scalerData *) OBJ_MEMBER_INT(SELF, scaler_data_offset);
    
    float input = (in - scdata->minIn) / scdata->rangeIn;
       
    if (input < 0) {
        *out = mini(scdata->minOut,scdata->maxOut) + (-pow ( fabs(input) ,scdata->exp) * scdata->rangeOut);
    }
    else {
        *out = mini(scdata->minOut,scdata->maxOut) + (pow (      input  ,scdata->exp) * scdata->rangeOut);
    }
    
    return TRUE;
}

CK_DLL_MFUN(scaler_min_in)
{
    scalerData * scdata = (scalerData *) OBJ_MEMBER_INT(SELF, scaler_data_offset);
    scdata->minIn = (GET_NEXT_FLOAT(ARGS)); 
    scdata->rangeIn = fabs(scdata->maxIn - scdata->minIn);
}
                        
CK_DLL_MFUN(scaler_max_in)
{
    scalerData * scdata = (scalerData *) OBJ_MEMBER_INT(SELF, scaler_data_offset);
    scdata->maxIn = (GET_NEXT_FLOAT(ARGS));
    scdata->rangeIn = fabs(scdata->maxIn - scdata->minIn);
}
                        
CK_DLL_MFUN(scaler_min_out)
{
    scalerData * scdata = (scalerData *) OBJ_MEMBER_INT(SELF, scaler_data_offset);
    scdata->minOut = (GET_NEXT_FLOAT(ARGS));
    scdata->rangeOut = fabs(scdata->maxOut - scdata->minOut);
}
                        
CK_DLL_MFUN(scaler_max_out)
{
    scalerData * scdata = (scalerData *) OBJ_MEMBER_INT(SELF, scaler_data_offset);
    scdata->maxOut = (GET_NEXT_FLOAT(ARGS)); 
    scdata->rangeOut = fabs(scdata->maxOut - scdata->minOut);
}
                        
CK_DLL_MFUN(scaler_exp) {
    scalerData * scdata = (scalerData *) OBJ_MEMBER_INT(SELF, scaler_data_offset);
    float value = (GET_NEXT_FLOAT(ARGS));
    if (value>0) scdata->exp = value;
    else scdata->exp = 1;
}


