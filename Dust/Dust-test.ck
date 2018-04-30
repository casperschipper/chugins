

Dust dust[50];
Pan2 p[50];

for (int i;i<dust.cap();i++) {
    dust[i] =>  p[i] => dac;
    p[i].pan(Math.random2f(-1,1));
    dust[i].p(1.0/cs.rv(100,10000));
    dust[i].gain(0.1);
}

hour => now;

