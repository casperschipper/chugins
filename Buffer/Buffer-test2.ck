Buffer mod => Buffer buf; PanFour p;
p.connect(buf);
Step offset => buf;

mod.max(30::samp);
mod.interp(0);
mod.record(0);
mod.noise();

mod.freq(20);
mod.gain(200);

offset.next(1*Std.rand2f(0.8,1.2));

buf.max(50::samp); // set the size of the internal memory

// buf.valueAt([integer],[float]) works as well, but as of yet doesn't return values.

p.pan(Std.rand2(-1,1)); 

buf.noise(); // fill the buf with noise. 

buf.record(0); // turn of recording
buf.interp(2); // type of interpolation, 2 stands for cubic spline, 1 would be linear and 0 non at all.

buf.sync(1); // use input as FM control.

buf.gain(0.1); // gain as usual







hour => now;