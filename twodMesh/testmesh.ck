Twodmesh mesh => Gain g => dac;

g.gain(1);

8 => int c;

while(1) {
    mesh.setMeshValue(1,Math.random(1,10),cs.random(1,10));
    cs.rd(.01::second,.4::second) => now;
}

