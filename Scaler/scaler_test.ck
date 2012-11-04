SinOsc m => Scaler s => Mtof f => SinOsc c => dac;


m.freq(2);
c.sync(2);

s.minOut(40);
s.maxOut(80);

hour => now;