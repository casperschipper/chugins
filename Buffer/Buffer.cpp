// interpolated buffer tool

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <cstdlib> 

inline int round(float r) {
    return (r > 0.0) ? (int) floor(r + 0.5) : (int) ceil(r - 0.5);
}

inline float clipf(float x,float min,float max) {
    if (x < min) return min;
    else if (x > max) return max;
    else return x;
}

inline float wrapf(float x, float max) {
    while (x < 0) x += max;
    return fmod(x,max);
}

t_CKUINT g_srate;

CK_DLL_CTOR(buffer_ctor); // constructor
CK_DLL_DTOR(buffer_dtor); // deconstructor
// setter functions:
CK_DLL_MFUN(buffer_max); // setting size of delay
CK_DLL_MFUN(buffer_rate); // set current delaytime
CK_DLL_MFUN(buffer_interp); // to set interp
CK_DLL_MFUN(buffer_sync); // the sync thing
CK_DLL_MFUN(buffer_valueAt); // to set a value of the buffer directly
CK_DLL_MFUN(buffer_play); // play switch
CK_DLL_MFUN(buffer_record); // rec switch
CK_DLL_MFUN(buffer_loop); // loop playback ?
CK_DLL_MFUN(buffer_recLoop); // loop rec
CK_DLL_MFUN(buffer_frequency); // frequency
CK_DLL_MFUN(buffer_delay); // delaytime if sync = 3

// getter functions:
CK_DLL_MFUN(buffer_getMax); // setting size of delay
CK_DLL_MFUN(buffer_getRate); // set current delaytime
CK_DLL_MFUN(buffer_getInterp); // to set interp
CK_DLL_MFUN(buffer_getSync); // the sync thing
CK_DLL_MFUN(buffer_getValueAt); // to set a value of the buffer directly
CK_DLL_MFUN(buffer_getPlay); // play switch
CK_DLL_MFUN(buffer_getRecord); // rec switch
CK_DLL_MFUN(buffer_getLoop); // rec switch
CK_DLL_MFUN(buffer_getRecLoop); // rec switch
CK_DLL_MFUN(buffer_getFrequency); // frequency
CK_DLL_MFUN(buffer_getDelay); // delaytime



CK_DLL_MFUN(buffer_clear); // to clear buffer
CK_DLL_MFUN(buffer_sinewave); // set sinewave;
CK_DLL_MFUN(buffer_noise); // set noise;

CK_DLL_TICK(buffer_tick); // this is main dsp

t_CKINT buffer_data_offset = 0; // ?

struct bufferData
{
    float * buffer; // pointer to delayline
    int max; // maxsize of delay
    int wPos; // the current writing position
    int interp; // type of interpolation
    int sync; // type of connection to previous unit
    float readPos; // readposition
    float rate; // playback rate
    float frequency;
    float delay;
    bool record; // record switch
    bool play; // play "
    bool recLoop; // loop recording
    int recLeft;
    bool loop; // loop playback
};

CK_DLL_QUERY(buffer) 
{
    g_srate = QUERY->srate;
    
    QUERY->setname(QUERY, "Buffer");
    
    QUERY->begin_class(QUERY, "Buffer", "UGen");
    
    QUERY->add_ctor(QUERY, buffer_ctor); // add the constructor
    QUERY->add_dtor(QUERY, buffer_dtor); // add the deconstructor
    
    QUERY->add_ugen_func(QUERY, buffer_tick, NULL, 1, 1); 

    QUERY->add_mfun(QUERY, buffer_max, "dur", "max"); // dur argument specifying the length of the buffer
    QUERY->add_arg(QUERY, "dur", "arg");
    QUERY->add_mfun(QUERY, buffer_getMax, "dur", "max"); 

    
    QUERY->add_mfun(QUERY, buffer_play, "int", "play"); // a 1 starts playback, 0 stops it.
    QUERY->add_arg(QUERY, "int", "arg");
    QUERY->add_mfun(QUERY, buffer_getPlay, "int", "play"); 
   
    QUERY->add_mfun(QUERY, buffer_record, "int", "record"); // a 1 starts recording, 0 stops it.
    QUERY->add_arg(QUERY, "int", "arg");
    QUERY->add_mfun(QUERY, buffer_getRecord, "int", "record"); 

    
    QUERY->add_mfun(QUERY, buffer_loop, "int", "loop"); // a 1 switches on looping, 0 switches off.
    QUERY->add_arg(QUERY, "int", "arg");
    QUERY->add_mfun(QUERY, buffer_getLoop, "int", "loop"); 

    
    QUERY->add_mfun(QUERY, buffer_recLoop, "int", "recLoop"); // a 1 switches on loop recording, 0 switches off.
    QUERY->add_arg(QUERY, "int", "arg");
    QUERY->add_mfun(QUERY, buffer_getRecLoop, "int", "recLoop"); 

    
    QUERY->add_mfun(QUERY, buffer_rate, "float", "rate"); // playback speed 
    QUERY->add_arg(QUERY, "float", "arg"); 
    QUERY->add_mfun(QUERY, buffer_getRate, "float", "rate");  

    QUERY->add_mfun(QUERY, buffer_frequency, "float", "freq"); // playback speed 
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->add_mfun(QUERY, buffer_getFrequency, "float", "freq"); 

    QUERY->add_mfun(QUERY, buffer_sync, "int", "sync"); // 0 = record, 1 = frequency ctrl, 2 = phase ctrl
    QUERY->add_arg(QUERY, "int", "arg");
    QUERY->add_mfun(QUERY, buffer_getSync, "int", "sync"); 
    
    
    QUERY->add_mfun(QUERY, buffer_interp, "int", "interp"); // 0 = no interpolation, 1 = linear, 2 = cubic spline
    QUERY->add_arg(QUERY, "int", "arg"); 
    QUERY->add_mfun(QUERY, buffer_getInterp, "int", "interp"); // 0 = no interpolation, 1 = linear, 2 = cubic spline

    QUERY->add_mfun(QUERY, buffer_delay, "dur", "delay"); // duration of buffer ( <= than dur specified at max parameter)
    QUERY->add_arg(QUERY, "dur", "arg"); 
    QUERY->add_mfun(QUERY, buffer_getDelay, "dur", "delay"); // 
    
    QUERY->add_mfun(QUERY, buffer_valueAt, "float", "valueAt"); // two arguments location and value
    QUERY->add_arg(QUERY, "int", "index"); 
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->add_mfun(QUERY, buffer_getValueAt, "float", "valueAt"); 
    QUERY->add_arg(QUERY, "int", "index");
    
    QUERY->add_mfun(QUERY, buffer_clear, "void", "clear"); // clears buffer
    
    QUERY->add_mfun(QUERY, buffer_sinewave, "void", "sinewave"); // sets waveform to sinewave
    QUERY->add_mfun(QUERY, buffer_noise, "void", "noise"); // sets waveform to sinewave
    
    buffer_data_offset = QUERY->add_mvar(QUERY, "int", "@data", false);
    
    QUERY->end_class(QUERY); 
    
    return TRUE;
}


CK_DLL_CTOR(buffer_ctor) // constructing the Buffer chugin
{
    OBJ_MEMBER_INT(SELF, buffer_data_offset) = 0; //
    
    bufferData * bfdata = new bufferData; // allocate a buffer data object
    bfdata->max = 4096;                // default values
    bfdata->buffer= NULL;             
    bfdata->rate = 1;
    bfdata->frequency = 0;
    bfdata->delay = 2;
    bfdata->wPos = 0;                   // setting writing position to 0, very important!
    bfdata->readPos = 0;                // set readpos to 0
    bfdata->interp = 1;                 // using linear interpolation
    bfdata->play = true;                // playback switched on
    bfdata->record = true;
    bfdata->loop = true;
    bfdata->recLoop = false;
    bfdata->recLeft = bfdata->max;
    bfdata->sync = 0;
    
    if (bfdata->buffer) delete [] bfdata->buffer; // delete if present.
    bfdata->buffer = new float[bfdata->max];      // allocate new memory
    
    for (int i = 0;i<bfdata->max;i++) bfdata->buffer[i] = 0; // clear the allocated memory
    
    OBJ_MEMBER_INT(SELF, buffer_data_offset) = (t_CKINT) bfdata;
}

CK_DLL_DTOR(buffer_dtor) // deconstruction
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    if(bfdata)
    {
        if (bfdata->buffer) delete [] bfdata->buffer; // free the allocated memory
        delete bfdata;   // delete object data
        OBJ_MEMBER_INT(SELF, buffer_data_offset) = 0;  
        bfdata = NULL;
    }
}

CK_DLL_TICK(buffer_tick)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    
    if ((bfdata->recLeft>0 && bfdata->record && (bfdata->sync == 0)) || (bfdata->sync == 3)) {
        bfdata->buffer[bfdata->wPos++] = in; // write a sample if recording is switched on, increase write position.
        bfdata->recLeft--;
    }
        
    else if (bfdata->recLeft == 0 && bfdata->recLoop) bfdata->recLeft = bfdata->max;

    if (bfdata->wPos >= bfdata->max) bfdata->wPos = 0;
    
    if (bfdata->play) {
        if (bfdata->sync == 1) bfdata->rate = (bfdata->max * clipf(in,-(g_srate/2.0),g_srate/2.0) / g_srate); // sync frequency to input
        
        if (bfdata->sync == 2) bfdata->readPos = wrapf((float) in,bfdata->max);  // sync readPos to input
        
        if (bfdata->sync == 3) bfdata->readPos = wrapf(((float) bfdata->wPos - bfdata->delay),bfdata->max);

        if (bfdata->interp == 0) {                          // no interpolation, my favourate kind !
            *out = bfdata->buffer[round(bfdata->readPos)];
        }
        
        if (bfdata->interp == 1) {                      // linear interpolation
            int index = (int) floor(bfdata->readPos);   // finding the integer index, for safety use floor.
            float frac = bfdata->readPos - index;       // the fraction for the calculation
         
            *out = bfdata->buffer[index%bfdata->max] + (bfdata->buffer[(index+1)%bfdata->max] - bfdata->buffer[index%bfdata->max]) * frac;
        }
        
        if (bfdata->interp == 2) {
            int index = (int) floor(bfdata->readPos);   // finding the integer index, for safety use floor.
            float frac = bfdata->readPos - index;       // the fraction for the calculation
            
            // reading 4 adjecent samples with some checks to wrap around in the buffer
            float L1 = bfdata->buffer[((index-1)<0) ? index-1 + bfdata->max   : (index - 1) % bfdata->max];
            float L0 = bfdata->buffer[(index<0) ? index + bfdata->max         : index % bfdata->max];
            float H0 = bfdata->buffer[((index+1)<0) ? index + 1 + bfdata->max : (index + 1) % bfdata->max];
            float H1 = bfdata->buffer[((index+2)<0) ? index + 2 + bfdata->max : (index + 2) % bfdata->max];
            
            // a formula found on http://www.musicdsp.org/showArchiveComment.php?ArchiveID=62
            *out = L0 + .5*
            frac*(H0-L1+
                  frac*(H0 + L0*(-2) + L1 +
                        frac*((H0 - L0)*9 + (L1 - H1)*3 +
                              frac*((L0 - H0)*15 + (H1 - L1)*5 +
                                    frac*((H0 - L0)*6 + (L1 - H1)*2 )))));
        }
        // readpos
        bfdata->readPos = bfdata->readPos + bfdata->rate; // increase the reading position based on frequency.
        while (bfdata->readPos < 0) bfdata->readPos = bfdata->readPos + bfdata->max;
        bfdata->readPos = fmod(bfdata->readPos,bfdata->max);
    }
    else *out = 0;
    
    return TRUE;
}

CK_DLL_MFUN(buffer_max)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    int size =  (int) floor(GET_NEXT_DUR(ARGS)); // int size of the delay translated from dur into samples
    if (bfdata->buffer) delete [] bfdata->buffer;   //delete old delay memory
    bfdata->buffer = new float[size];              // allocate the new size
    for (int i = 0;i<size;i++) bfdata->buffer[i] = 0;  // zero the allocated memory
    bfdata->max = size;                             // set the max in the buffer struct
    RETURN->v_dur = (t_CKDUR) size;
}

CK_DLL_MFUN(buffer_getMax) 
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    RETURN->v_dur = (t_CKDUR) bfdata->max;
}

CK_DLL_MFUN(buffer_sinewave)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    for (int i = 0; i < bfdata->max; i++) {
        bfdata->buffer[i] = sin(TWO_PI * (((float) i)/(bfdata->max+2)));
    }
}

CK_DLL_MFUN(buffer_noise)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    for (int i = 0; i < bfdata->max; i++) {
        bfdata->buffer[i] = (((float)rand()/RAND_MAX) * 2.0) - 1.0;
    }
}

CK_DLL_MFUN(buffer_clear)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    for (int i = 0; i < bfdata->max; i++) {
        bfdata->buffer[i] = 0;
    }
}

CK_DLL_MFUN(buffer_rate)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    bfdata->rate = GET_NEXT_FLOAT(ARGS);
    RETURN->v_float = bfdata->rate;
}

CK_DLL_MFUN(buffer_getRate)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    RETURN->v_float = bfdata->rate;
}

CK_DLL_MFUN(buffer_frequency)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    bfdata->frequency = GET_NEXT_FLOAT(ARGS);
    bfdata->rate = (float) ((bfdata->frequency * bfdata->max) / g_srate);
    RETURN->v_float = bfdata->frequency;
}

CK_DLL_MFUN(buffer_getFrequency)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    RETURN->v_float = bfdata->frequency;
}

CK_DLL_MFUN(buffer_play)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    bfdata->play = (bool) GET_NEXT_INT(ARGS);
    RETURN->v_int = bfdata->play;
}

CK_DLL_MFUN(buffer_getPlay)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    RETURN->v_int = bfdata->play;
}

CK_DLL_MFUN(buffer_interp)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    bfdata->interp = GET_NEXT_INT(ARGS);
    RETURN->v_int = bfdata->interp;
}

CK_DLL_MFUN(buffer_getInterp)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    RETURN->v_int = bfdata->interp;
}

CK_DLL_MFUN(buffer_sync)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    bfdata->sync = GET_NEXT_INT(ARGS);
    if (bfdata->sync == 1 || bfdata->sync == 2) { // switch of recording if input is used as sync.
        bfdata->record = false;
        bfdata->recLeft = 0;
    } 
    if (bfdata->sync == 3) {
        bfdata->recLoop = true;
        bfdata->record = 1;
    }
    RETURN->v_int = bfdata->sync;
}

CK_DLL_MFUN(buffer_getSync)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    RETURN->v_int = bfdata->sync;
}


CK_DLL_MFUN(buffer_record)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    bfdata->recLeft = bfdata->max;
    bfdata->record = (bool) GET_NEXT_INT(ARGS);
    RETURN->v_int = bfdata->record;
}

CK_DLL_MFUN(buffer_getRecord)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    RETURN->v_int = bfdata->record;
}

CK_DLL_MFUN(buffer_loop)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    bfdata->loop = (bool) GET_NEXT_INT(ARGS);
    RETURN->v_int = bfdata->loop;
}

CK_DLL_MFUN(buffer_getLoop)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    RETURN->v_int = bfdata->loop;
}

CK_DLL_MFUN(buffer_recLoop)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    bfdata->recLoop = (bool) GET_NEXT_INT(ARGS);
    RETURN->v_int = bfdata->recLoop;
}

CK_DLL_MFUN(buffer_getRecLoop)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    RETURN->v_int = bfdata->recLoop;
}

CK_DLL_MFUN(buffer_valueAt)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    int index = GET_NEXT_INT(ARGS);
    if (index >= bfdata->max || index < 0) {
        fprintf(stderr, "Buffer error: trying to write to index %i, not a valid position",index);
        index = index % bfdata->max;
        while(index < 0 && index < bfdata->max) index = index + bfdata->max;
    }
    bfdata->buffer[index] = GET_NEXT_FLOAT(ARGS);
    RETURN->v_float = bfdata->buffer[index];
}

CK_DLL_MFUN(buffer_getValueAt)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    int index = GET_NEXT_INT(ARGS);
    if (index >= bfdata->max || index < 0) {
        fprintf(stderr, "Buffer error: trying to write to index %i, not a valid position",index);
        while(index < 0 && index < bfdata->max) index = index + bfdata->max;
        index = index % bfdata->max;
    }
    RETURN->v_float = bfdata->buffer[index];
}

CK_DLL_MFUN(buffer_delay)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    float delay = (float) GET_NEXT_DUR(ARGS);
    // sanity check...
    if (delay < 2) delay = 2;                               // because of interpolation minimum time = 2 samples
    if (delay > (bfdata->max-2)) delay = bfdata->max-2;     
    bfdata->delay = delay;
    RETURN->v_dur = (t_CKDUR) delay;
}

CK_DLL_MFUN(buffer_getDelay)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    RETURN->v_dur = (t_CKDUR) bfdata->delay;
}

