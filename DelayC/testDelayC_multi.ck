fun void oneVoice() {
    adc => Delay del => dac; // connect one delay between adc and dac
    
    .5 => del.gain;
    
    del.max(881000::samp); // set the max size of the delay;
    
    SinOsc c => blackhole; // the modulator for the delaytime
    
    c.freq(Std.rand2f(0.01,0.5)); // set the freq
    c.gain(Std.rand2f(200,44000)); // set the depth
    
    while(1) {      // control rate loop
        del.delay((c.last() + c.gain() + 3)*samp); // set delaylength with output of SinOsc c.
        samp => now;
    }
}

for (int i;i<20;i++) { // spork twenty
    spork ~ oneVoice();
    .1::second => now;
}

hour => now;
