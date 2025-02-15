// v 0.3 Coded by Casper Schipper, 2011

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>

CK_DLL_CTOR(delayc_ctor); // constructor
CK_DLL_DTOR(delayc_dtor); // deconstructor

CK_DLL_MFUN(delayc_max); // setting size of delay
CK_DLL_MFUN(delayc_delay); // set current delaytime

CK_DLL_TICK(delayc_tick); // this is main dsp

t_CKINT delayc_data_offset = 0; // ?

struct delaycData
{
    float * buffer; // pointer to delayline
    int max; // maxsize of delay
    float delay; // the current delaytime in float samples.
    int wPos; // the current writing position
};

CK_DLL_QUERY(delayc) 
{
    QUERY->setname(QUERY, "DelayC");
    
    QUERY->begin_class(QUERY, "DelayC", "UGen");
    
    QUERY->add_ctor(QUERY, delayc_ctor); // add the constructor
    QUERY->add_dtor(QUERY, delayc_dtor); // add the deconstructor
    
    QUERY->add_ugen_func(QUERY, delayc_tick, NULL, 1, 1); 
    QUERY->add_mfun(QUERY, delayc_max, "void", "max"); // A dur argument specifying the maximum length of the delay
    QUERY->add_arg(QUERY, "dur", "arg");
    
    QUERY->add_mfun(QUERY, delayc_delay, "void", "delay"); // Delay time argument (also dur)
    QUERY->add_arg(QUERY, "dur", "arg"); 
    
    delayc_data_offset = QUERY->add_mvar(QUERY, "int", "@data", false);
    
    QUERY->end_class(QUERY); 
    
    return TRUE;
}


CK_DLL_CTOR(delayc_ctor) // constructing the delay chugin.
{
    OBJ_MEMBER_INT(SELF, delayc_data_offset) = 0; //
    
    delaycData * dcdata = new delaycData; // delayc data object
    dcdata->max = 4098;                // default values
    dcdata->delay = 4096;              // standard delay time = 4096 samples
    dcdata->buffer= NULL;             
    dcdata->wPos = 0;                   // setting writing position to 0, very important!
    
    if (dcdata->buffer) delete [] dcdata->buffer; // delete if present.
    dcdata->buffer = new float[dcdata->max];      // allocate new memory
    for (int i;i<dcdata->max;i++) dcdata->buffer[i] = 0; // wash the allocated memory
    
    OBJ_MEMBER_INT(SELF, delayc_data_offset) = (t_CKINT) dcdata;
}

CK_DLL_DTOR(delayc_dtor) // deconstruction
{
    delaycData * dcdata = (delaycData *) OBJ_MEMBER_INT(SELF, delayc_data_offset);
    if(dcdata)
    {
        if (dcdata->buffer) delete [] dcdata->buffer; // free the allocated memory
        delete dcdata;    
        OBJ_MEMBER_INT(SELF, delayc_data_offset) = 0;
        dcdata = NULL;
    }
}

CK_DLL_TICK(delayc_tick)
{
    delaycData * dcdata = (delaycData *) OBJ_MEMBER_INT(SELF, delayc_data_offset);
    
    dcdata->buffer[dcdata->wPos] = in; // write a sample
    float readPos = dcdata->wPos++ - dcdata->delay; // reading head follows behind writinghead, increase wPos after
    
    if (dcdata->wPos >= dcdata->max) dcdata->wPos = 0; // wrap writing head.
    
    int index = (int) floor(readPos);   // finding the integer index, for safety use floor.
    float frac = readPos - index;       // the fraction for the calculation
    
    // reading 4 adjecent samples with some checks to wrap around in the buffer
    float L1 = dcdata->buffer[((index-1)<0) ? index-1 + dcdata->max   : (index - 1) % dcdata->max];
    float L0 = dcdata->buffer[(index<0) ? index + dcdata->max         : index % dcdata->max];
    float H0 = dcdata->buffer[((index+1)<0) ? index + 1 + dcdata->max : (index + 1) % dcdata->max];
    float H1 = dcdata->buffer[((index+2)<0) ? index + 2 + dcdata->max : (index + 2) % dcdata->max];
    
    // a formula found on http://www.musicdsp.org/showArchiveComment.php?ArchiveID=62
    *out = L0 + .5*
    frac*(H0-L1+
          frac*(H0 + L0*(-2) + L1 +
                frac*((H0 - L0)*9 + (L1 - H1)*3 +
                      frac*((L0 - H0)*15 + (H1 - L1)*5 +
                            frac*((H0 - L0)*6 + (L1 - H1)*2 )))));
    
    
    return TRUE;
}

CK_DLL_MFUN(delayc_max)
{
    delaycData * dcdata = (delaycData *) OBJ_MEMBER_INT(SELF, delayc_data_offset);
    int size = 2 + (int) floor(GET_NEXT_DUR(ARGS)); // int size of the delay translated from dur into samples
    if (dcdata->buffer) delete [] dcdata->buffer;   //delete old delay memory
    dcdata->buffer = new float[size];              // allocate the new size
    for (int i;i<size;i++) dcdata->buffer[i] = 0;  // zero the delay line
    dcdata->max = size;                             // set the max in the delayc struct
}

CK_DLL_MFUN(delayc_delay)
{
    delaycData * dcdata = (delaycData *) OBJ_MEMBER_INT(SELF, delayc_data_offset);
    float delay = (float) GET_NEXT_DUR(ARGS);
    // sanity check...
    if (delay < 2) delay = 2;                               // because of interpolation minimum time = 2 samples
    if (delay > (dcdata->max-2)) delay = dcdata->max-2;     
    dcdata->delay = delay;
}


