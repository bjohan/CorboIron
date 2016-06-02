
mainHousingInsideX = 110;
mainHousingInsideY = 110;
mainHousingInsideZ = 210;
t = 3;


module powerSupply()
{
    cube([200, 100, 42]);
}

module iecHoles(t)
{
    hs = 55.5-3.5;
    lc = 23;
    
    rotate(-90, [0, 0, 1]){
    translate([0, lc-hs/2, 10])
        rotate(90,[0,1,0])
            cylinder(t, 3/2,3/2, $fn=10);
    translate([0, lc+hs/2, 10])
        rotate(90,[0,1,0])
            cylinder(t, 3/2,3/2, $fn=10);
    translate()
        cube([t, 46, 20]);
    }
}

module fanHoles(t)
{
    
    //translate([0, lc-hs/2, 10])
        rotate(90,[1,0,0]){
            cylinder(t, 24,24);
            translate([20, 20, 0])
                cylinder(t, 3/2, 3/2, $fn=10);
            translate([20, -20, 0])
                cylinder(t, 3/2, 3/2, $fn=10);
            translate([-20, 20, 0])
                cylinder(t, 3/2, 3/2, $fn=10);
            translate([-20, -20, 0])
                cylinder(t, 3/2, 3/2, $fn=10);
        }
}


module displayHoles(t, r)
{
    $fn = 10;
    y = 55;
    x = 93;
    xo = (98-x)/2;
    yo = (60-y)/2;
    translate( [xo,yo, 0])
        cylinder(t, r, r);
    translate( [xo+x,yo, 0])
        cylinder(t, r, r);
    translate( [xo+x,yo+y, 0])
        cylinder(t, r, r);
    translate( [xo,yo+y, 0])
        cylinder(t, r, r);
}

module displayVolumes(){
    cube([98, 60, 2]);
    translate([0, 10, -2])
        cube([98, 40, 14]);
    translate([8, 60-22, -12])
        cube([52, 22, 12]);
}





module display()
{
    difference(){
        displayVolumes();
        translate([0,0,-1])
        displayHoles(5);
    }
    
}


module pcb(){
    cube([160, 100, 2]);
    translate([-20, 4, 2])
        cube([20, 92, 17]);
}


module mainHousing(sx, sy, sz, t){
    fanHoleY=70;
    fanHoleZ=30;
    exhaustSeparation = 10;
    difference(){
        cube([sx, sy, sz]);
        translate([t, t, t])
            cube([sx-2*t, sy-t+1, sz-2*t]);
        translate([-1, t+fanHoleY, t+fanHoleZ])
            rotate([0, 0, 90])
                fanHoles(t+2);    
        for(n = [0:15]){
            translate([sx-t-1, 20+t, 20+t+exhaustSeparation*n])
                rotate([0, 90, 0])
                    cylinder(t+2, 2.5, 2.5);
        }
    }
    
}

module placedPowerSupply(){
    translate([t, t, t])
        rotate([0, -90, -90])
            powerSupply();
}


mainHousing(mainHousingInsideX+2*t, mainHousingInsideY+2*t ,mainHousingInsideZ+2*t, t);

placedPowerSupply();



display();