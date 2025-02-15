Linseg line => SinOsc c => dac;

while(1) {
    line.target(Std.rand2f(400,800));
    line.duration(.1::second) => now;
}



hour => now;