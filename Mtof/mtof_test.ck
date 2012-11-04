Step n => Mtof f => SinOsc c => dac;

c.sync(2);

while(1) {
    second => now;
    Std.rand2f(40,80) => n.next;
}