Random rnd;

class DustBurst extends Chubgraph {
    Step unit => Envelope l => CurveTable curve => Dust d => Clip c => outlet;
    unit.next(true);
    
    6.0 => float _curve;
    
    [0.,1.,-_curve,0.95,0.0,-_curve,1.0,0.0] => curve.coefs;
       
   
       
   fun void loop() {
       while(1) {
           l.value(0);
           l.target(0.95);
           0 => Std.srand;
           .2*second => l.duration => now;
       }
   }
   
   spork ~ loop();
}

DustBurst bah => dac.left;
DustBurst bah2 => dac.right;

minute => now;
