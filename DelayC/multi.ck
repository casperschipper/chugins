fun void oneVoice() {
    adc => DelayC del => Gain g => dac;
    g => Gain fb => del;
    
    .75 => del.gain;
    
    .5 => del.gain;
    
    del.max(88100::samp);
    
    SinOsc c => blackhole;
    
    c.freq(Std.rand2f(0.01,0.1));
    c.gain(Std.rand2f(200,20400));
    
    int count;    
    while(1) {
        del.delay((c.last() + (c.gain()+100))*samp);
        samp => now;
    }
}

for (int i;i<20;i++) {
    spork ~ oneVoice();
    .1::second => now;
}

1::hour => now;
