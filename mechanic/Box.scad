module powerSupply()
{
    cube([200, 100, 42]);
}

module bottomHousing(ix, iy, iz, t)
{
    difference(){
        translate([-t,-t,-t])
            cube([ix+2*t, iy+2*t, iz+t]);
    
        cube([ix, iy, iz+1]);
    }
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


module exhaustHoles(t)
{
    r = 2.5;
    sep = 10;
    for(x = [0:5])
        for (z = [0:2]){
            translate([x*sep, 0, z*sep])
                rotate(90,[1,0,0])
                    cylinder(t, r, r);
        }
        /*rotate(90,[1,0,0]){
            cylinder(t, 24,24);
            translate([20, 20, 0])
                cylinder(t, 3/2, 3/2);
            translate([20, -20, 0])
                cylinder(t, 3/2, 3/2);
            translate([-20, 20, 0])
                cylinder(t, 3/2, 3/2);
            translate([-20, -20, 0])
                cylinder(t, 3/2, 3/2);
        }*/
}



module bottomSectionHoles(ix, iy, iz, t,r){
    translate([-t-1, 10, iz-5])
        rotate(90, [0, 1, 0] )
            cylinder(ix+2*t+2, r);
    
    translate([-t-1, iy+2*t-10, iz-5])
        rotate(90, [0, 1, 0] )
            cylinder(ix+2*t+2, r);
    
    translate([-t-1, (iy+2*t)/2, iz-5])
        rotate(90, [0, 1, 0] )
            cylinder(ix+2*t+2, r);
}


module bottomSection(ix, iy, iz, t){ 
    difference(){
        bottomHousing(ix, iy, iz,t);
        translate([ix-30, iy+2*t-1, 24+2])
            fanHoles(6);
        translate([ix-120, iy+2*t-1, 0])
            iecHoles(6);
        translate([10, 1, 10])
            exhaustHoles(5, $fn=10);
         translate([ix-135, -4, -t]){
             displayCavity(55, 55, 135+t, t, 6);    
         }
         
          translate([75, -40, 5])
            rotate(45,[1,0,0]) 
                       translate([0,0,0])
            displayHoles(10, 3);
         
         
        /*translate([-3, 10, 40])
            iecHoles(3);
        translate([-4, 85, 60])
            fanHoles(6);*/
         bottomSectionHoles(ix, iy, iz, t, 3/2, $fn=10);

    }
   
   difference(){ 
    translate([ix-135, 0, -t]){
        displaySection(55, 55, 135+t, t);
    }
    
    translate([75, -40, 5])
        rotate(45,[1,0,0]){ 
            display();
            translate([0,0,0])
            displayHoles(10, 3/2);
            translate([115, 30, 0])
            cylinder(10, 7/2, 7/2);
        }
         translate([ix-135-1, -60, -3])
            cube([135+t+2,15,100]);
    }
    translate([ix-135, -45, -t])
            cube([135+t,t,10]);
   
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


module displaySection(ox, oy,oz, t)
{
    oPoints = [[0, 0], [ox, 0], [0, oy]];
    iPoints = [[t, 0], [ox-t, 0], [t, oy-2*t]];
    rotate(180, [0, 0, 1])
    rotate(-90, [0, 1, 0])
    difference(){
        linear_extrude(oz)
            polygon(oPoints);
    
        translate([0,0,t])
            linear_extrude(oz-2*t)
                polygon(iPoints);
    }
}

module displayCavity(ox, oy, oz, td, t)
{
    x = ox-2*td;
    y = oy -2*td;
    z = oz -2*td;
    translate([td, 0, td])
    cube([z,t, x]);
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

module topHousing(ix, iy, iz, t, tol, overlap, height){
 translate([t+tol/2, t+tol/2, iz-overlap+t])
    bottomHousing(ix-tol-2*t, iy-tol-2*t, overlap-t, t);
    
 translate([0, 0, iz]){
     //difference(){
        translate([tol/2, tol/2, 0]){
            difference(){
            bottomHousing(ix-tol, iy-tol, height, t+tol/2);
            translate([0,0,-t-tol])
                cube([ix-tol, iy-tol, t+tol]);
            }
        }
     //}
     translate([1,7,-8])
     difference(){
        cube([10,15,10]);
         translate([5, 5, 0])
            cylinder(10, 1.5, 1.5, $fn=10);
     }
     translate([150,7,-8])
     difference(){
        cube([15,15,10]);
         translate([7, 5, 0])
            cylinder(10, 1.5, 1.5, $fn=10);
     }
     translate([1, 100, 0]){
         translate([0,5,-8])
             difference(){
                cube([10,15,10]);
                 translate([5, 10, 0])
                    cylinder(10, 1.5, 1.5, $fn=10);
             }
         translate([150,5,-8])
             difference(){
                cube([15,15,10]);
                 translate([7, 10, 0])
                    cylinder(10, 1.5, 1.5, $fn=10);
             }
     }
 }
    }

module topSection(ix, iy, iz, t, tol, overlap, height){

    difference(){
    topHousing(ix, iy, iz, t, tol, overlap, height);
        translate([t+tol/2, t+tol/2, iz-overlap+t]){
    translate([-t,10,8]){
        pcb();
        translate([151-t-tol/2, 113+t-5, -overlap-t])
            cube([50, 20, overlap+t]);
    }
     
    }bottomSectionHoles(ix, iy, iz, t, 3/2, $fn=10);
    }
    //translate to bottom plane
    translate([t+tol/2, t+tol/2, iz-overlap+t]){
        translate([-t-tol/2,0,8]){
            //pcb();
        }
        translate([-t*2-tol/2, -t*2-tol/2, -t])
            difference(){
                cube([ix+t*2, iy+t*2, overlap+t]);
                    translate([0.5, 0.5, 0])
                        cube([ix+t*2-1, iy+t*2-1, overlap+t]);
            }
    }
    
}


//displayHoles(20, 3/2);
ix = 201;
iy = 145;
iz = 55;
t = 3;
tol = 1;
overlap = 10;
height = 20;

//bottomSection(ix, iy, iz, t);
topSection(ix, iy, iz, t, tol, overlap, height);
module bottomSupports(){
translate([100, -8, 0])
cylinder(42.2,3,3);
translate([140, -8, 0])
cylinder(42.2,3,3);
translate([105, iy, 0])
cylinder(19.9,3,3);
}
//translate([75, -40, 5])
//        rotate(45,[1,0,0]) 
//            display();

//translate([-3, 10, 40])
//        iecHoles(3);
//translate([-4, iy+2*t, 60])
//        fanHoles(6);
//translate([0,15,0])
//    powerSupply();

