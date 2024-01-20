adc => DelayC del => dac;

del.max(44100::samp); //init maximum delay size

SinOsc c => blackhole;   // the source for modulating the delaytime
0.3 => c.freq;
20000 => c.gain;

while(1) {
    del.delay((c.last() + 22000)*samp); // the control rate loop
    samp => now;
}
