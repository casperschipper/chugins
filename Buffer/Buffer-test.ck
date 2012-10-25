adc => Buffer buf => dac;

buf.interp(2);

float dura;

buf.max(100::ms);

while(1) {
    Std.rand2f(-20,50) => Std.mtof => buf.freq;
    Std.rand2f(-50,30) => Std.mtof =>  dura;
    (1 / dura) * second => now;
    buf.clear();
    buf.record(1);
    100::ms => now;
    buf.record(0);
    
}


