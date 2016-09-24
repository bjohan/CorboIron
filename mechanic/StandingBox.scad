//$fn=200;
mainHousingInsideX = 120;
mainHousingInsideY = 120;
mainHousingInsideZ = 210;
t = 3;

dispTiltOffs = 20;
dispProjHeight = 50;
dispRecess = 40;
topFoldHeight = 20;

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
        translate([sx/2, sy/2+10, sz+1])
            rotate([90, 0, 0])
                fanHoles(t+2);    
        for(n = [0:15]){
            translate([sx-t-1, 20+t, 20+t+exhaustSeparation*n])
                rotate([0, 90, 0])
                    cylinder(t+2, 2.5, 2.5);
        }
    }
    
}

module mainHousingFrontPanelScrewHoles(sx, sy, sz, t){
    translate([t+10, sy+t, t+5]){
        for(n = [0:5]){
            translate([20*n, 0, 0])
                rotate([90, 0, 0])
                    cylinder(25, 1.5, 1.5);
        }
    }
    
        translate([t+10, sy+t, sz-t-5]){
        for(n = [0:5]){
            translate([20*n, 0, 0])
                rotate([90, 0, 0])
                    cylinder(25, 1.5, 1.5);
        }
    }
}


module mainHousingWithFrontPanelHoles(sx, sy, sz, t){
    difference(){
        union(){
            mainHousing(sx, sy, sz, t);
            translate([t, sy-10-t, t])
                cube([sx-2*t, 10,10]);
            translate([t, sy-10-t, sz-10-t])
                cube([sx-2*t, 10,10]);
            
        }
        mainHousingFrontPanelScrewHoles(sx, sy, sz, t);
    }
}

function getDispAngle() =
    atan2(dispProjHeight, dispRecess);

module frontPanelOnSide(height, width, t){
    //dispTiltOffs = 10;
    //dispProjHeight = 50;
    //dispRecess = 30;
    //topFoldHeight = 20;http://www.di.se/artiklar/2016/6/5/schweiz-rostar-om-medborgarlon/
    dto = dispTiltOffs;
    poly = [    [0,0],
                [dispTiltOffs,0],
                [dispProjHeight+dto,dispRecess],
                //[height,dispRecess],
                [height-topFoldHeight, dispRecess],
                [height-topFoldHeight, 0],
                [height, 0],
                [height, t],
                [height-topFoldHeight+t, t],
                
                [height-topFoldHeight+t,dispRecess+t],
    
                //[height,dispRecess+t],
                [dispProjHeight+dto,dispRecess+t],
                [dispTiltOffs,+t],
                [0,t]];
    linear_extrude(width)
        polygon(points=poly);
    
}

module placedFrontPanel(height, width, depth, t){
    /*t = 3;

dispTiltOffs = 20;
dispProjHeight = 50;
dispRecess = 40;
topFoldHeight = 20;*/
    translate([t, depth+t, t])
    
    rotate([0, -90, 180]){
        frontPanelOnSide(height, width, t);
        translate([dispProjHeight+dispTiltOffs, dispRecess+t,0])
            cube([10, 10, width]);
        translate([height-topFoldHeight-10+t, dispRecess+t,0])
            cube([10, 10, width]);
    }
}

module placedDisplay(){
translate([t+(mainHousingInsideX-98)/2, 78, 68])
    rotate(-getDispAngle(),[1, 0, 0])
        rotate([0, 0, 180])
            translate([-98,-60, 0]){
                display();
               
            }
}

module placedDisplayHoles(){
translate([t+(mainHousingInsideX-98)/2, 78, 68])
    rotate(-getDispAngle(),[1, 0, 0])
        rotate([0, 0, 180])
            translate([-98,-60, 0]){
                displayHoles(10, 3/2);
            }
}
        
module placedFrontPanelWithHoles(height, width, depth, t){
    difference(){
        placedFrontPanel(height, width, depth, t);
        placedDisplay();
        placedDisplayHoles();
        mainHousingFrontPanelScrewHoles(mainHousingInsideX+2*t, mainHousingInsideY+t, mainHousingInsideZ+2*t,t);
    }
}


module placedPowerSupply(t){
    translate([t, t, t])
        rotate([0,-90,-90])
            powerSupply();
}


module penHolderMainBody(frontAngle){
    length = 110;
    height = 35;
    width = mainHousingInsideX/2;
    //frontAngle = 40;
    difference(){
    cube([width, length, height]);
    translate([0, length, 0])
        rotate([frontAngle, 0, 0])
            cube(2*[height, height, height]);
    }
}

module penHolderWithPrintHoles(angle){
    dia = 11.8;
    difference(){
        penHolderMainBody(angle);
        translate([dia, 90+2.1, dia/2])
            rotate([angle, 0, 0])
                cylinder(100 , dia/2, dia/2);
        
        translate([mainHousingInsideX/2-dia, 90+2.1, dia/2])
            rotate([angle, 0, 0])
                cylinder(100 , dia/2, dia/2);
    }
}


module penHolderDryClean(angle){
    h = 35;
    dia = 56;
    d=55;
    dt = d+h;
    angle=20;
    hd = (dia/2+2)/sqrt(2);
    difference(){
        penHolderMainBody(angle);
        
        translate([mainHousingInsideX/4, dia/2+5, 10])
            rotate([angle, 0, 0]){
                cylinder(100 , dia/2-2, dia/2-2);
                translate([0,0,13])
                    cylinder(100 , dia/2, dia/2);
                translate([hd, hd, 0])
                    cylinder(100, 3/2, 3/2);
                translate([hd, -hd, 0])
                    cylinder(100, 3/2, 3/2);
                translate([-hd, hd, 0])
                    cylinder(100, 3/2, 3/2);
                translate([-hd, -hd, 0])
                    cylinder(100, 3/2, 3/2);
                
            }

    translate([0, d, h])
        rotate([angle, 0, 0])
            translate([0, -dt, 0]){
                cube([mainHousingInsideX/2, dt, 30]);
            }

    }
    
}


module penHolder(){
  
   
    //translate([0, 0, 30])
    
    intersection(){
        penHolderDryClean(0);
        penHolderWithPrintHoles(40);
    }
}


module printFrontPanel(){
    rotate([0, 90, 0])
    placedFrontPanelWithHoles(mainHousingInsideZ, mainHousingInsideX, mainHousingInsideY, t);
    translate([10, 120, -t-2])
        cylinder(2,20,20);
    translate([210, 120, -t-2])
        cylinder(2,20,20);
    translate([80, 120-40, -t-2])
        cylinder(2,20,20);
    translate([180, 120-40, -t-2])
        cylinder(2,20,20);
}

//printFrontPanel();

/*translate([0, 120, 0])
    penHolder();

translate([0, 73, 185])
    rotate([-90,0,0])
        penHolder();*/
mainHousingWithFrontPanelHoles(mainHousingInsideX+2*t, mainHousingInsideY+t, mainHousingInsideZ+2*t,t);

placedFrontPanelWithHoles(mainHousingInsideZ, mainHousingInsideX, mainHousingInsideY, t);

//placedPowerSupply(t);

//placedDisplay();
