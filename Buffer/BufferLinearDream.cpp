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

t_CKUINT g_srate;

CK_DLL_CTOR(buffer_ctor); // constructor
CK_DLL_DTOR(buffer_dtor); // deconstructor

CK_DLL_MFUN(buffer_max); // setting size of delay
CK_DLL_MFUN(buffer_rate); // set current delaytime
//CK_DLL_MFUN(buffer_read); 
CK_DLL_MFUN(buffer_clear); // to clear buffer
CK_DLL_MFUN(buffer_interp); // to set interp
CK_DLL_MFUN(buffer_valueAt); // to set a value of the buffer directly
CK_DLL_MFUN(buffer_play); // play switch
CK_DLL_MFUN(buffer_record); // rec switch
CK_DLL_MFUN(buffer_loop); // loop playback ?
CK_DLL_MFUN(buffer_recLoop); // loop rec
CK_DLL_MFUN(buffer_frequency); // loop rec
CK_DLL_MFUN(buffer_sinewave); // set sinewave;


CK_DLL_TICK(buffer_tick); // this is main dsp

t_CKINT buffer_data_offset = 0; // ?

struct bufferData
{
    float * buffer; // pointer to delayline
    int max; // maxsize of delay
    int wPos; // the current writing position
    int interp;
    float readPos; // readposition
    float rate; // playback rate
    float frequency;
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
    
    QUERY->add_mfun(QUERY, buffer_max, "void", "max"); // dur argument specifying the length of the buffer
    QUERY->add_arg(QUERY, "dur", "arg");
    
    QUERY->add_mfun(QUERY, buffer_play, "void", "play"); // a 1 starts playback, 0 stops it.
    QUERY->add_arg(QUERY, "int", "arg");
   
    QUERY->add_mfun(QUERY, buffer_record, "void", "record"); // a 1 starts recording, 0 stops it.
    QUERY->add_arg(QUERY, "int", "arg");
    
    QUERY->add_mfun(QUERY, buffer_loop, "void", "loop"); // a 1 switches on looping, 0 switches off.
    QUERY->add_arg(QUERY, "int", "arg");
    
    QUERY->add_mfun(QUERY, buffer_loop, "void", "recLoop"); // a 1 switches on loop recording, 0 switches off.
    QUERY->add_arg(QUERY, "int", "arg");
    
    QUERY->add_mfun(QUERY, buffer_rate, "void", "rate"); // playback speed 
    QUERY->add_arg(QUERY, "float", "arg"); 
    
    QUERY->add_mfun(QUERY, buffer_frequency, "void", "freq"); // playback speed 
    QUERY->add_arg(QUERY, "float", "arg");
    
    //QUERY->add_mfun(QUERY, buffer_read, "string", "read"); // reading a soundfile
    //QUERY->add_arg(QUERY, "string", "arg" );
    
    QUERY->add_mfun(QUERY, buffer_clear, "void", "clear"); // clears buffer
    
    QUERY->add_mfun(QUERY, buffer_sinewave, "void", "sinewave"); // sets waveform to sinewave
    
    QUERY->add_mfun(QUERY, buffer_interp, "void", "interp"); // 0 = no interpolation, 1 = linear, 2 = cubic spline
    QUERY->add_arg(QUERY, "int", "arg"); 
    
    QUERY->add_mfun(QUERY, buffer_valueAt, "void", "valueAt"); // two arguments location and value
    QUERY->add_arg(QUERY, "int", "index"); 
    QUERY->add_arg(QUERY, "float", "arg");
    
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
    bfdata->wPos = 0;                   // setting writing position to 0, very important!
    bfdata->readPos = 0;                // set readpos to 0
    bfdata->interp = 1;                 // using linear interpolation
    bfdata->play = true;     //playback switched on
    bfdata->record = true;
    bfdata->loop = true;
    bfdata->recLoop = false;
    bfdata->recLeft = bfdata->max;
    
    
    if (bfdata->buffer) delete [] bfdata->buffer; // delete if present.
    bfdata->buffer = new float[bfdata->max];      // allocate new memory
    
    for (int i = 0;i<bfdata->max;i++) bfdata->buffer[i] = 0; // clear the allocated memory
    //for (int i;i<bfdata->max;i++) bfdata->buffer[i] = ((rand()/RAND_MAX) * 2.0) - 1.0; // testnoise

    
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
    
    if (bfdata->recLeft>0 && bfdata->record) {
        bfdata->buffer[bfdata->wPos++] = in; // write a sample if recording is switched on, increase write position.
        bfdata->recLeft--;
    }
        
    else if (bfdata->recLoop) bfdata->recLeft = bfdata->max;

    if (bfdata->wPos >= bfdata->max) bfdata->wPos = 0;
    
    if (bfdata->play) {
        if (bfdata->interp == 0) {                          // no interpolation, my favourate kind !
            *out = bfdata->buffer[round(bfdata->readPos)];
        }
        
        if (bfdata->interp == 1) {                      // linear interpolation
            int index = (int) floor(bfdata->readPos);   // finding the integer index, for safety use floor.
            float frac = bfdata->readPos - index;       // the fraction for the calculation
         
            *out = (bfdata->buffer[(index+1)%bfdata->max] - bfdata->buffer[index]) * frac;
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
        (bfdata->readPos < 0) ? bfdata->readPos = bfdata->readPos + bfdata->max : bfdata->readPos = fmod(bfdata->readPos,bfdata->max); //wrap
    }
    else *out = 0;
    
    return TRUE;
}

/*
CK_DLL_CTRL( sndbuf_ctrl_read )
{
    sndbuf_data * d = (sndbuf_data *)OBJ_MEMBER_UINT(SELF, sndbuf_offset_data);
    const char * filename = GET_CK_STRING(ARGS)->str.c_str();
    
    if( bfdata->buffer )
    {
        delete [] bfdata->buffer;
        bfdata->buffer = NULL;
    }
    
    if( bfdata->chunk_table )
    {
        delete [] bfdata->chunk_table;
        bfdata->chunk_table = NULL;
    }
    
    if( bfdata->fd )
    {
        sf_close( bfdata->fd );
        bfdata->fd = NULL;
    }
    
    // log
    EM_log( CK_LOG_INFO, "(sndbuf): reading '%s'...", filename );
    
    {
        // stat the file first
        struct stat s;
        if( stat( filename, &s ) )
        {
            fprintf( stderr, "[chuck](via SndBuf): cannot stat file '%s'...\n", filename );
            return;
        }
        
        // open it
        SF_INFO info;
        info.format = 0;
        const char * format = (const char *)strrchr( filename, '.');
        if( format && strcmp( format, ".raw" ) == 0 )
        { 
            fprintf( stderr, "[chuck](via SndBuf) %s :: type is '.raw'...\n    assuming 16 bit signed mono (PCM)\n", filename );
            info.format = SF_FORMAT_RAW | SF_FORMAT_PCM_16 | SF_ENDIAN_CPU ;
            info.channels = 1;
            info.samplerate = 44100;
        }
        
        // open the handle
        bfdata->fd = sf_open( filename, SFM_READ, &info );
        t_CKINT er = sf_error( bfdata->fd );
        if( er )
        {
            fprintf( stderr, "[chuck](via SndBuf): sndfile error '%li' opening '%s'...\n", er, filename );
            fprintf( stderr, "[chuck](via SndBuf): (reason: %s)\n", sf_strerror( bfdata->fd ) );
            if( bfdata->fd ) sf_close( bfdata->fd );
            // escape
            return;
        }
        
        // allocate
        t_CKINT size = info.channels * info.frames;
        bfdata->buffer = new SAMPLE[size+info.channels];
        memset( bfdata->buffer, 0, (size+info.channels)*sizeof(SAMPLE) );
        bfdata->chan = 0;
        bfdata->num_frames = info.frames;
        bfdata->num_channels = info.channels;
        bfdata->samplerate = info.samplerate;
        bfdata->num_samples = size;
        
        // log
        EM_pushlog();
        EM_log( CK_LOG_INFO, "channels: %d", bfdata->num_channels );
        EM_log( CK_LOG_INFO, "frames: %d", bfdata->num_frames );
        EM_log( CK_LOG_INFO, "srate: %d", bfdata->samplerate );
        EM_log( CK_LOG_INFO, "chunks: %d", bfdata->chunks );
        EM_poplog();
        
        // read
        sf_seek( bfdata->fd, 0, SEEK_SET );
        bfdata->chunks_read = 0;
        
        // no chunk
        if( !bfdata->chunks )
        {
            // read all
            t_CKUINT f = sndbuf_read( d, 0, bfdata->num_frames );
            // check
            if( f != (t_CKUINT)bfdata->num_frames )
            {
                fprintf( stderr, "[chuck](via SndBuf): read %lu rather than %ld frames from %s\n",
                        f, size, filename );
                sf_close( bfdata->fd ); bfdata->fd = NULL;
                return;
            }
            
            assert( bfdata->fd == NULL );
        }
        else
        {
            // reset
            bfdata->chunks_size = bfdata->chunks;
            bfdata->chunks_total = bfdata->num_frames / bfdata->chunks;
            bfdata->chunks_total += bfdata->num_frames % bfdata->chunks ? 1 : 0;
            bfdata->chunks_read = 0;
            bfdata->chunk_table = new bool[bfdata->chunks_total];
            memset( bfdata->chunk_table, 0, bfdata->chunks_total * sizeof(bool) );
            
            // read chunk
            // sndbuf_load( d, 0 );
        }
    }
    
    // bfdata->interp = SNDBUF_INTERP;
    bfdata->sampleratio = (double)bfdata->samplerate / (double)g_srate;
    // set the rate
    bfdata->rate = bfdata->sampleratio * bfdata->rate_factor;
    bfdata->curr = bfdata->buffer;
    bfdata->curf = 0;
    bfdata->eob = bfdata->buffer + bfdata->num_samples;
}
*/



CK_DLL_MFUN(buffer_max)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    int size =  (int) floor(GET_NEXT_DUR(ARGS)); // int size of the delay translated from dur into samples
    if (bfdata->buffer) delete [] bfdata->buffer;   //delete old delay memory
    bfdata->buffer = new float[size];              // allocate the new size
    for (int i = 0;i<size;i++) bfdata->buffer[i] = 0;  // zero the delay line
    bfdata->max = size;                             // set the max in the buffer struct
}

CK_DLL_MFUN(buffer_sinewave)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    for (int i = 0;i < bfdata->max;i++) {
        bfdata->buffer[i] = sin(2 * 3.141592653589793 * (((float) i)/(bfdata->max+2)));
    }
}

CK_DLL_MFUN(buffer_clear)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    for (int i = 0;i < bfdata->max ;i++) {
        bfdata->buffer[i] = 0;
    }
}

CK_DLL_MFUN(buffer_rate)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    bfdata->rate = GET_NEXT_FLOAT(ARGS);
}

CK_DLL_MFUN(buffer_frequency)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    bfdata->frequency = GET_NEXT_FLOAT(ARGS);
    bfdata->rate = (float) ((bfdata->frequency * bfdata->max) / g_srate);
}

CK_DLL_MFUN(buffer_play)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    bfdata->play = (bool) GET_NEXT_INT(ARGS);
}

CK_DLL_MFUN(buffer_interp)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    bfdata->interp = GET_NEXT_INT(ARGS);
}


CK_DLL_MFUN(buffer_record)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    bfdata->recLeft = bfdata->max;
    bfdata->record = (bool) GET_NEXT_INT(ARGS);
}

CK_DLL_MFUN(buffer_loop)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    bfdata->loop = (bool) GET_NEXT_INT(ARGS);
}

CK_DLL_MFUN(buffer_recLoop)
{
    bufferData * bfdata = (bufferData *) OBJ_MEMBER_INT(SELF, buffer_data_offset);
    bfdata->recLoop = (bool) GET_NEXT_INT(ARGS);
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
}

