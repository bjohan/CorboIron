//$fn=200;
mainHousingInsideX = 120;
mainHousingInsideY = 120;
mainHousingInsideZ = 210;
t = 3;

dispTiltOffs = 20;
dispProjHeight = 50;
dispRecess = 40;
topFoldHeight = 20;


module iecHoles(t){
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

module fanHoles(t){
    
    //translate([0, lc-hs/2, 10])
        //rotate(90,[1,0,0]){
            cylinder(t, 24,24);
            translate([20, 20, 0])
                cylinder(t, 3/2, 3/2, $fn=10);
            translate([20, -20, 0])
                cylinder(t, 3/2, 3/2, $fn=10);
            translate([-20, 20, 0])
                cylinder(t, 3/2, 3/2, $fn=10);
            translate([-20, -20, 0])
                cylinder(t, 3/2, 3/2, $fn=10);
        //}
}


module displayHoles(t, r){
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




module display(){
    difference(){
        displayVolumes();
        translate([0,0,-1])
        displayHoles(5);
    }
    
}


module pcb(){
    cube([100, 80, 2]);
    //Iron 0 iron 1 connector
    translate([1, -5, 2])
        cube([31, 12, 11]);
    
    //Iron 2 and iron 3 connector
    translate([100-1-31, -5, 2])
        cube([31, 12, 11]);
    
    //Power in at bottom connector
    translate([43, 3, -20])
        cube([12, 25, 20]);
    
    //Filter and voltage regulator volume
    translate([37, 2, 2])
        cube([30, 30, 20]);
    
    //Arduino volume
    translate([41, 27, 2])
        cube([18, 45, 22]);
    
    //Pin headers
    translate([35, 42, 2])
        cube([3, 30, 20]);
    translate([53, 70, 2])
        cube([11, 3, 20]);
        
    //Capacitors
    translate([12, 66, 2])    
        cylinder(13,4,4);
    translate([23, 66, 2])    
        cylinder(13,4,4);
    translate([77, 66, 2])    
        cylinder(13,4,4);
    translate([89, 66, 2])    
        cylinder(13,4,4);
    
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

module powerSupply(){
    cube([200, 100, 42]);
}


module placedPowerSupply(t){
    translate([10, 10, 3+200])
        rotate([0,90,0])
            powerSupply();
}


module penHolderMainBody(frontAngle){
    length = 110;
    height = 35;
    width = (mainHousingInsideX-2)/2;
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
        
        translate([(mainHousingInsideX-2)/2-dia, 90+2.1, dia/2])
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
        
        translate([(mainHousingInsideX-2)/4, dia/2+5, 10])
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
                cube([(mainHousingInsideX-2)/2, dt, 30]);
            }
    }
    
}


module penHolder(){
    intersection(){
        penHolderDryClean(0);
        penHolderWithPrintHoles(40);
    }
}


module penHolderKnob(s){
    cylinder(5, s*30/2, s*35/2);
}

module penHolderKnobKeyHole(){
    hull(){
        penHolderKnob(1);
        translate([0,10,0])
            penHolderKnob(1.3);
    }
}

module truncatedPenHolder(){
    difference(){
    penHolder();
    translate([0,100,0])
        cube([100, 20, 20]);
    translate([0,85,0])
        cube([11,15,25]);
    translate([(mainHousingInsideX-2)/2-11,85,0])
        cube([11,15,25]);
    translate([(mainHousingInsideX-2)/4,40,0])
        penHolderKnobKeyHole();
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



module multiPanelp(z){

    x = 120;
    y = x;
//    z = 3;
    ca = 15;
    cb = 15;
    b = 10;
    r = 3/2;
    t = 3;
    difference(){
        union(){
            cube([x,y,z]);
            translate([0, ca, z])
                cube([b, y-ca-cb, b]);
            translate([x-b, ca, z])
                cube([b, y-ca-cb, b]);
        }
        
        ay = ca+b/2;
        by = y/2;
        cy = y-cb-b/2;
        hy = t+b/2;
        //echo(hy)
        
        translate([-1, ay, hy])
            rotate([0, 90, 0])
                cylinder(b+2, r, r);
        translate([-1, by, hy])
            rotate([0, 90, 0])
                cylinder(b+2, r, r);
        translate([-1, cy, hy])
            rotate([0, 90, 0])
                cylinder(b+2, r, r);
        translate([x-b, 0, 0]){
            translate([-1, ay, hy])
                rotate([0, 90, 0])
                    cylinder(b+2, r, r);
            translate([-1, by, hy])
                rotate([0, 90, 0])
                    cylinder(b+2, r, r);
            translate([-1, cy, hy])
                rotate([0, 90, 0])
                    cylinder(b+2, r, r);
        }
        ax = ca+b/2;
        bx = x/2;
        cx = x -cb -b/2;
        
        /*translate([ax, hy, -1])
            cylinder(z+t+2, r, r);
        translate([bx, hy, -1])
            cylinder(z+t+2, r, r);
        translate([cx, hy, -1])
            cylinder(z+t+2, r, r);
        */
        /*translate([0, x-hy*2, 0]){
            translate([ax, hy, -1])
                cylinder(z+t+2, r, r);
            translate([bx, hy, -1])
                cylinder(z+t+2, r, r);
            translate([cx, hy, -1])
                cylinder(z+t+2, r, r);
        }*/
        
        translate([ax, hy-t, -1])
            cylinder(z+t+2, r, r);
        translate([bx, hy-t, -1])
            cylinder(z+t+2, r, r);
        translate([cx, hy-t, -1])
            cylinder(z+t+2, r, r);
        
        translate([0, x-(hy)*2+t, 0]){
            translate([ax, hy, -1])
                cylinder(z+t+2, r, r);
            translate([bx, hy, -1])
                cylinder(z+t+2, r, r);
            translate([cx, hy, -1])
                cylinder(z+t+2, r, r);
        }

    }
}

module multiPanel(){
    multiPanelp(3);
}
module multiPanelIecHole(){
    difference(){
        multiPanel();
        translate([55,43,-1])
        rotate([0,0,-17])
        rotate([-90,0,0])
        iecHoles(5);
    }
}
module wheelPanel(){
    difference(){
        multiPanel();
        translate([55,43,-1]){
            cylinder(5,8.5/2, 8.5/2);
            translate([6,0,2])
            cylinder(5,3/2, 3/2);
        }
    }
}
module backPanel(){
    
    difference(){
        multiPanel();
        translate([20,11+27,-1])
        xzCylinders(2,2, 5, 80);
    }
    
}

module fanPanel(){
    difference(){
        multiPanel();
        translate([60,60,0])
        fanHoles(3);
    }
}

module bottomPanel(){
    difference(){
        union(){
            translate([0,0,-10])
            multiPanelp(13);
            translate([35,35,3])
                cube([50,50,5]);
            //cube([120,120,10]);
        }
        
        for(i=[10:0]){
            translate([10+i*10,0,-5+3/2])
                rotate([-90,0,0])
                    cylinder(120,3,3);
        }
        
        for(i=[10:0]){
            translate([0, 10+10*i,-5+3/2])
                rotate([0,90,0])
                    cylinder(120,3,3);
        }
        //for(y=[4:0])
        //    for(x=[4:0])
        //        translate([40+10*x, 40+y*10,-5+3/2])
        //            cylinder(20,3,3);
        translate([-1, 20, 5+3])
        yxCylinders(3, 3/2, 12,80);
        translate([-1+110, 20, 5+3])
        yxCylinders(3, 3/2, 12,80);
        translate([20,5,-10])
            xzCylinders(3, 3, 10, 80);
        translate([20,115,-10])
            xzCylinders(3, 3, 10, 80);
        //translate([35+2,35+2,3])
        //        cube([46,46,50]);
        translate([60,60,-6.5])
            fanHoles(20);
    }
   
    
}



module xCylinder(h, r1, r2){
    rotate([0,90,0])
        cylinder(h,r1,r2);
}
module yCylinder(h, r1, r2){
    rotate([-90,0,0])
        cylinder(h,r1,r2);
}
module zCylinder(h, r1, r2){
    cylinder(h,r1,r2);
}

module xzCylinders(n, r, h,l){
    for(i = [0:n-1]){
        echo(i*l/(n-1));
        translate([i*l/(n-1),0,0])
            zCylinder(h,r,r);
    }
}

module xyCylinders(n, r, h,l){
    for(i = [0:n-1]){
        echo(i*l/(n-1));
        translate([i*l/(n-1),0,0])
            yCylinder(h,r,r);
    }
}

module yxCylinders(n, r, h,l){
    for(i = [0:n-1]){
        echo(i*l/(n-1));
        translate([0, i*l/(n-1),0])
            xCylinder(h,r,r);
    }
}



module frontSmall(){

    x = 120;
    y = 120;
    z = 3;
    ca = 15;
    cb = 0;
    b = 10;
    r = 3/2;
    t = 3;
    //xzCylinders(5,3,10,200);
    
    difference(){
            union(){
                cube([120,40,3]);
                translate([0, ca, z])
                    cube([b, 10, b]);
                translate([x-b, ca, z])
                    cube([b, 10, b]);
            }
            
            translate([20,5,0])
                xzCylinders(3,r, 10, 80);
            
            translate([20,35,0])
                xzCylinders(3,r, 10, 80);
            
            translate([0,20,8])
                xCylinder(10,r,r);
       
            translate([110,20,8])
                xCylinder(10,r,r);
        }
        
}


module threeHoleBarXy(){
    difference(){
        cube([90,10,10]);
        translate([5,0,5])
        xyCylinders(3,3/2, 10, 80);
    }
}

module displayPanel(){

    x = 120;
    y = 120;
    z = 3;
    ca = 15;
    cb = 0;
    b = 10;
    r = 3/2;
    t = 3;
    //xzCylinders(5,3,10,200);
    
    difference(){
            union(){
                cube([120,80,3]);
                rotate([17,0,0])
                    cube([120,10,10]);
                translate([0,71,0])
                    rotate([17,0,0])
                        cube([120,10,10]);
                /*translate([15,0,0])
                    rotate([90,0,0])
                        threeHoleBarXy();
                translate([15,70,3])
                    rotate([-17,0,0])
                        threeHoleBarXy()*/;
                //translate([0, ca, z])
                //    cube([b, 10, b]);
                //translate([x-b, ca, z])
                //    cube([b, 10, b]);
            }
            translate([15,0,0])
                rotate([17,0,0])
                    translate([5,5,-5])
                        xzCylinders(3,3/2,20,80);
            translate([15,71,0])
                rotate([17,0,0])
                    translate([5,-0.1,5])
                        xyCylinders(3,3/2,20,80);
   translate([10,10,-6]){
        displayVolumes();
            displayHoles(10,3/2);
   }
    translate([10,10,3]){
    
            displayHoles(10,4);
   }             
            
        }
 
}


module displayPanelWithDisplay(){
    displayPanel();
     translate([10,10,-6])
        displayVolumes();
}
module largeFrontPanel(){
    translate([61,(mainHousingInsideX-2)/4+1-0.5,3])
        penHolderKnob(1);
    translate([61,(mainHousingInsideX-2)/4*3+2-0.5,3])
        penHolderKnob(1);
    difference(){
        union(){
            cube([132,120,3]);
                translate([132, 15, 3])
                rotate([0,0,90])
                    threeHoleBarXy();    

            translate([0, 15, 0])
                cube([10, 90, 24]);
                //rotate([0,0,90])
                //    rotate([90,0,0])
                //    threeHoleBarXy();
        }
        translate([132-25,0,0])
            cube([25, 11, 4]);
        translate([132-25,120-11,0])
            cube([25, 11, 4]);
        cube([11,11,4]);
        translate([0,120-11,0])
            cube([11,11,4]);
    translate([5,20,0])
        rotate([0,0,90])
        xzCylinders(3,3/2,35,80);

    }

}

module pcbPillar(){
    difference(){
        union(){
            translate([18+42+5-20,10,3])
                cube([10,10,90]);
            translate([18+42-15+5,10,73])
                cube([10,10,20]);
            translate([18+42-20+5,10,73-10])
                rotate([0,22,0])
                    cube([10,10,15]);
        }
        translate([18+42-20,15,73+12])
            xCylinder(30, 3/2, 3/2);
    }
}

module holeGrid(h,r, xn, yn, xd, yd){
        for(i=[0:xn-1]){
            for(j=[0:yn-1]){
                translate([i*xd, j*yd, 0]){
                    cylinder(h,r,r);
                }
            }
        }
}

module pcbPanel(){
    difference(){
    multiPanel();
    translate([18+42,10,3])
    rotate([0,0,90])
        rotate([90,0,0])
            pcb();
        translate([50,25,-1])
            holeGrid(5,2.5,2,5,6,6);
        translate([50,25+46,-1])
            holeGrid(5,2.5,2,5,6,6);
        
        translate([20,60,-1])
            cylinder(5,14/2,14/2);
    }
    
    pcbPillar();
    translate([0,90,0])
        pcbPillar();
    
}

module pcbPanelWithPcb(){
    
    pcbPanel();
    translate([18+42,10,3])
    rotate([0,0,90])
        rotate([90,0,0])
            pcb();
    
}


module connector(){
    cube([30,12.5,9]);
    translate([0,0.5,-9])
    cube([30,6,9]);
}
module zTube(l, ri, ro){
    difference(){
        zCylinder(l,ro,ro);
        zCylinder(l, ri, ri);
    }
}

module zCone(l, ril, riu, wt){
    difference(){
        zCylinder(l,ril+wt,riu+wt);
        zCylinder(l, ril, riu);
    }
}
/*module crossOver(){
    hull(){
    cube([30,10,1]);
    translate([15,5,20]){
        zTube(1,3,4);
        translate([10,0,0])
            zTube(1,3,4);
    }}
}
*/
module connectorStrainRelief(){
    connSX = 30;
    connSY = 12.5;
    connSZ = 9;
    wall = 2.5;
    stop = 0.75;
    benReliefHeght = 20;
    transitionHeight = 20;
    cableRadius = 3.5;
    //connector();
    difference(){
        translate([-wall, -wall, -wall]){
            cube([connSX+2*wall, connSY+2*wall, connSZ+wall]);
            
        }
            cube([connSX, connSY, connSZ]);
            translate([stop*0,0*stop,-wall])
            cube([connSX-0*2*stop, connSY-0.5*2*stop, connSZ]);
        }
    //zTube(20, 6/2, 6/2+wall);

    difference(){
        //Outer Hull
        hull(){
            translate([-wall, -wall, connSZ])
                cube([connSX+2*wall, connSY+2*wall, 1]);
            translate([-wall+(connSX+2*wall)/3, /*-wall+*/connSY/2, connSZ+transitionHeight])
                cylinder(1,cableRadius+wall,cableRadius+wall);
            translate([-wall+2*(connSX+2*wall)/3, /*-wall+*/connSY/2, connSZ+transitionHeight])
                cylinder(1,cableRadius+wall,cableRadius+wall);
        }
    
        //Inner Hull
        hull(){
            translate([0, 0, connSZ])
                cube([connSX, connSY, 1]);
            translate([-wall+(connSX+2*wall)/3, /*-wall+*/connSY/2, connSZ+transitionHeight-wall])
                cylinder(1,cableRadius,cableRadius);
            translate([-wall+2*(connSX+2*wall)/3, /*-wall+*/connSY/2, connSZ+transitionHeight-wall])
                cylinder(1,cableRadius,cableRadius);
        }
        
        translate([-wall+2*(connSX+2*wall)/3, /*-wall+*/connSY/2, connSZ+transitionHeight-wall])
            cylinder(10,cableRadius,cableRadius);
        translate([-wall+(connSX+2*wall)/3, /*-wall+*/connSY/2, connSZ+transitionHeight-wall])
            cylinder(10,cableRadius,cableRadius);
    }
    
    translate([-wall+(connSX+2*wall)/3, /*-wall+*/connSY/2, connSZ+transitionHeight])
        zTube(benReliefHeght,cableRadius,cableRadius+wall-0.5);
    translate([-wall+2*(connSX+2*wall)/3, /*-wall+*/connSY/2, connSZ+transitionHeight])
        zTube(benReliefHeght,cableRadius,cableRadius+wall-0.5);
}




module solderPenMainBody(){
    zTube(3.5,9.25/2,8.5/2+1.5);
    translate([0,0,3.5])
        zTube(50,8.5/2,8.5/2+1.5);
}

module solderPenTpuSleeveLower(){
    $fn=100;
    zTube(12, 9/2, 15/2);
    translate([0,0,12])
        zTube(20, 11/2, 15/2);
}
module solderPenTpuSleeveUpper(){
    $fn=100;
    zTube(35, 11/2, 15/2);
    translate([0,0,35])
        zCone(15, 11/2, 3.5, 2);
    translate([0,0,50])
        zCone(25, 3.5, 3.5, 2);
}
//solderPenTpuSleeveLower();
//translate([0,0,40])
    solderPenTpuSleeveUpper();
//solderPenMainBody();
//connectorStrainRelief();
//crossOver();
//backPanel();
//pcbPanel();
//pcbPanelWithPcb();
//multiPanelIecHole();
//largeFrontPanel();
//frontSmall();

//truncatedPenHolder();
//penHolderWithPrintHoles(40);
//bottomPanel();
//wheelPanel();

//multiPanelp(10);
//fanPanel();

//
//
//displayPanel();

//xyCylinder(10,2,2);

//Bottom
/*bottomPanel();

//Bottom Back
translate([-3, 0, 3])
    rotate([90, 0, 90])
        backPanel();

//Bottom right
translate([120, 123, 3])
    rotate([90, -90, 0])
        wheelPanel();

//Bottom left 
//translate([120, 0-3, 123])
//    rotate([-90, 90, 0])
//        multiPanelIecHole();


//Top Back
translate([-3, 0, 3+120])
    rotate([90, 0, 90])
        backPanel();

//Top right
//translate([120, 123, 3+120])
//    rotate([90, -90, 0])
//        multiPanel();

//Top left 
translate([120, 0-3, 123+120])
    rotate([-90, 90, 0])
        multiPanel();



//Top
translate([0, 120, 123+123])
    rotate([180, 0, 0])
        pcbPanel();
        
//Small panel on front at the bottom            
translate([123,0,43])
    rotate([0,-90,0])
        rotate([0,0,90])
          frontSmall();

translate([110,0,33])
    rotate([0,90-17,0])
    rotate([0,0,90])
        displayPanelWithDisplay();
        
translate([90-4,0,243])
    rotate([0,90,0])
    largeFrontPanel();

translate([89, 60-0.5, 130+100-8])
    rotate([0,90,0])
    rotate([0,0,-90])
        truncatedPenHolder();
translate([89, 120-0.5, 130+100-8])
    rotate([0,90,0])
    rotate([0,0,-90])
        truncatedPenHolder();*/

//truncatedPenHolder();
//largeFrontPanel();
//placedPowerSupply();

/*translate([-3, 0, 120])
    rotate([0, 90, 0])
        multiPanel();

//printFrontPanel();


translate([0, 73, 185])
    rotate([-90,0,0])
        penHolder();*/
//mainHousingWithFrontPanelHoles(mainHousingInsideX+2*t, mainHousingInsideY+t, mainHousingInsideZ+2*t,t);
//rotate([0, 0, -90])
//    placedFrontPanelWithHoles(mainHousingInsideZ, mainHousingInsideX, mainHousingInsideY, t);

//placedPowerSupply(t);

//placedDisplay();
