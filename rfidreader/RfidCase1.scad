s3=sqrt(3);

diamondsize=110;
hexagonsize=100;
lednum=24;
ledpos=59.8/2;
ledsize=3;
caseheight=30;

dimples=false;
sidecovers=false;
black=true;

diamondcase();
diamondbottom();
translate([0,0,caseheight]) hexfront();
translate([0,0,caseheight]) diffuser();
translate([0,0,caseheight-2]) covers();

*mirror([0,0,1]) diamondcase();
*diamondbottom();
*mirror([0,0,1]) hexfront();
*translate([0,0,-5.8]) diffuser();

module diamondcase() {
    union() {
        difference() {
            union() {
                linear_extrude(height=caseheight-0.4) diamond(diamondsize, 2);
                translate([0,0,caseheight-0.4]) linear_extrude(height=0.4, scale=0.995) diamond(diamondsize, 2);
            }
            translate([0,0,caseheight-5]) linear_extrude(height=6) hexagon(hexagonsize-10);
            if (sidecovers) {
                translate([0,0,-1]) linear_extrude(height=caseheight-3) diamond(diamondsize-4, 1);
                translate([0,0,-1]) linear_extrude(height=3) diamond(diamondsize-2, 1);
                ds1 = (diamondsize-4)/2;
                ds2 = (hexagonsize-2)/2;
                translate([0,0,caseheight-5]) linear_extrude(height=3) polygon([
                    [-ds2,-ds1/s3-(ds1-ds2)/s3],[0,-ds1*2/s3],[ ds2,-ds1/s3-(ds1-ds2)/s3],
                    [ ds2, ds1/s3+(ds1-ds2)/s3],[0, ds1*2/s3],[-ds2, ds1/s3+(ds1-ds2)/s3]
                ]);
                translate([ diamondsize*0.64,0,caseheight-2]) linear_extrude(height=3) triangle(diamondsize*0.63,[0,0,0]);
                translate([-diamondsize*0.64,0,caseheight-2]) linear_extrude(height=3) mirror([1,0,0]) triangle(diamondsize*0.63,[0,0,0]);
                translate([ diamondsize*0.64,0,caseheight-6]) linear_extrude(height=5) triangle(diamondsize/2,[1,1,1]);
                translate([-diamondsize*0.64,0,caseheight-6]) linear_extrude(height=5) mirror([1,0,0]) triangle(diamondsize/2,[1,1,1]);
            } else {
                translate([0,0,-1]) linear_extrude(height=caseheight-1) diamond(diamondsize-4, 1);
                translate([0,0,-1]) linear_extrude(height=3) diamond(diamondsize-2, 1);
            }
            if (dimples) {
                translate([-diamondsize*0.75,-diamondsize*0.24,caseheight-2]) sphere(10,$fn=64);
                translate([-diamondsize*0.75, diamondsize*0.24,caseheight-2]) sphere(10,$fn=64);
                translate([ diamondsize*0.75,-diamondsize*0.24,caseheight-2]) sphere(10,$fn=64);
                translate([ diamondsize*0.75, diamondsize*0.24,caseheight-2]) sphere(10,$fn=64);
            }
            for (a=[30:60:330]) {
                rotate([0,0,a]) casetabcut();
            }

            for (a=[0:60:300]) rotate([0,0,a]) casepinhole();
        }
        rotate([0,0, 30]) bottomtablip(-55);
        rotate([0,0,-30]) bottomtablip( 55);
        rotate([0,0,150]) bottomtablip( 55);
        rotate([0,0,210]) bottomtablip(-55);
        rotate([0,0, 30]) bottomtablip(  5);
        rotate([0,0,-30]) bottomtablip( -5);
        rotate([0,0,150]) bottomtablip( -5);
        rotate([0,0,210]) bottomtablip(  5);
        translate([-66, 0, caseheight-2]) wemosd1();

        *#translate([0,0,caseheight-2-1.5]) rotate([0,0,30]) batteryholder();
        translate([0,0,caseheight-2-1.5]) rotate([0,0,30]) batteryclips();
    }
}

module wemosd1() {
    *#translate([0,0,-2.4]) cube([34.6,25.4,4.8], true);
    translate([-17.3,   0,0]) rotate([0,0,90]) mcutab(12);
    translate([ 17.3,-9.2,0]) rotate([0,0,-90]) mcutab(7);
    translate([ 17.3, 9.2,0]) rotate([0,0,-90]) mcutab(7);

    translate([ 10,-12.7-2/2,-7/2+0.1]) cube([7,2,7],true);
    translate([ 10, 12.7+2/2,-7/2+0.1]) cube([7,2,7],true);
    translate([-10,-12.7-2/2,-7/2+0.1]) cube([7,2,7],true);
    translate([-10, 12.7+2/2,-7/2+0.1]) cube([7,2,7],true);
}

module batteryclips() {
    translate([-33/2,-94.8/2,0]) batterypin();
    translate([-33/2, 94.8/2,0]) batterypin();
    translate([ 33/2,-94.8/2,0]) batterypin();
    translate([ 33/2, 94.8/2,0]) batterypin();
    translate([-14,-50.2,0]) rotate([0,0,180]) batterytab(10);
    translate([-14, 50.2,0]) batterytab(10);
    translate([ 14,-50.2,0]) rotate([0,0,180]) batterytab(10);
    translate([ 14, 50.2,0]) batterytab(10);
}

module batterypin() {
    rotate([0,0,360/48]) {
        translate([0,0,-1.9]) cylinder(3.4,2.25,2.25, $fn=24);
        translate([0,0,-3.5]) cylinder(1.7,1.2,1.2, $fn=24);
        translate([0,0,-4.0]) cylinder(0.5,0.8,1.2, $fn=24);
    }
}

module batterytab(w) {
    translate([-w/2,0,0]) rotate([0,90,0]) linear_extrude(height=w) polygon([
        [-2,0],[3.0,0],[3.7,-0.7],[5.7,0.5],[4.5,1.5],[-2,1.5]
    ]);
}

module batteryholder() {
    cr=5;
    bh=48/2-cr;
    bw=100/2-cr;
    difference() {
        union() {
            translate([-1,0,-3]) linear_extrude(height=1) polygon(concat(
                [for (an=[  0:10: 90]) [ bh+sin(an)*cr, bw+cos(an)*cr]],
                [for (an=[ 90:10:180]) [ bh+sin(an)*cr,-bw+cos(an)*cr]],
                [for (an=[180:10:270]) [-bh+sin(an)*cr,-bw+cos(an)*cr]],
                [for (an=[270:10:360]) [-bh+sin(an)*cr, bw+cos(an)*cr]]
            ));
            translate([-1, 6,-12]) cube([40,77,24], true);
        }
        translate([-33/2,-94.5/2,-4.5]) cylinder(3,1.25,1.25, $fn=24);
        translate([-33/2, 94.5/2,-4.5]) cylinder(3,1.25,1.25, $fn=24);
        translate([ 33/2,-94.5/2,-4.5]) cylinder(3,1.25,1.25, $fn=24);
        translate([ 33/2, 94.5/2,-4.5]) cylinder(3,1.25,1.25, $fn=24);
    }
}

module mcutab(w) {
    translate([-w/2,0,0]) rotate([0,90,0]) linear_extrude(height=w) polygon([
        [0,0],[4.8,0],[5.4,-0.5],[7,0.5],[5.4,1.5],[0,1.5]
    ]);
}

module diamondbottom() {
    difference() {
        union() {
            linear_extrude(height=1.8) diamond(diamondsize-2-0.2, 1);
            translate([0,0,1.8]) linear_extrude(height=0.1,scale=0.998) diamond(diamondsize-2-0.2, 1);
            rotate([0,0, 30]) bottomtab(-55);
            rotate([0,0,-30]) bottomtab( 55);
            rotate([0,0,150]) bottomtab( 55);
            rotate([0,0,210]) bottomtab(-55);
            rotate([0,0, 30]) bottomtab(  5);
            rotate([0,0,-30]) bottomtab( -5);
            rotate([0,0,150]) bottomtab( -5);
            rotate([0,0,210]) bottomtab(  5);
        }
        translate([-diamondsize/2.5,0,-0.5]) linear_extrude(height=3) diamond(diamondsize/4, 1);
        translate([ diamondsize/2.5,0,-0.5]) linear_extrude(height=3) diamond(diamondsize/4, 1);
        translate([0,-diamondsize/s3/2.5,-0.5]) linear_extrude(height=3) diamond(diamondsize/4, 1);
        translate([0, diamondsize/s3/2.5,-0.5]) linear_extrude(height=3) diamond(diamondsize/4, 1);
    }
}

module covers() {
    if (sidecovers) {
        difference() {
            translate([diamondsize*0.64,0,0]) linear_extrude(height=2) triangle(diamondsize*0.6235,[0,2,0]);
            if (dimples) {
                translate([diamondsize*0.746,-diamondsize*0.233,-4]) sphere(10,$fn=64);
                translate([diamondsize*0.746, diamondsize*0.233,-4]) sphere(10,$fn=64);
            }
        }
        difference() {
            translate([-diamondsize*0.64,0,0]) linear_extrude(height=2) mirror([1,0,0]) triangle(diamondsize*0.6235,[0,2,0]);
            if (dimples) {
                translate([-diamondsize*0.746,-diamondsize*0.233,-4]) sphere(10,$fn=64);
                translate([-diamondsize*0.746, diamondsize*0.233,-4]) sphere(10,$fn=64);
            }
        }
    }
}

module hexfront() {
    difference() {
        union() {
            hexdia = hexagonsize*2/s3;
            hexcover(10, hexdia-15, hexdia, 1.5, 2);

            translate([0,0,3.3]) rotate([0,0,360/48]) cylinder(5, 53.6/2, 53.6/2, $fn=24);
            if (black) {
                translate([0,0,7.3]) rotate([0,0,360/48]) cylinder(1, 68/2, 68/2, $fn=24);
            } else {
                translate([0,0,5.8]) rotate([0,0,360/48]) cylinder(2.5, 68/2, 68/2, $fn=24);
            }
            rotate([0,0,360/48]) translate([-5,-33,3.3]) rotate([0,90,0])
                linear_extrude(height=10) polygon([
                    [-5,-3],[-5,-1],[-1,-1],[0.5,0.5],[2,-0.5],[-0.5,-3]
                ]);
            rotate([0,0,360/48]) translate([5,33,3.3]) rotate([0,90,180])
                linear_extrude(height=10) polygon([
                    [-5,-3],[-5,-1],[-1,-1],[0.5,0.5],[2,-0.5],[-0.5,-3]
                ]);

            rotate([0,0, 30]) casetab();
            rotate([0,0, 90]) casetab();
            rotate([0,0,150]) casetab();
            rotate([0,0,210]) casetab();
            rotate([0,0,270]) casetab();
            rotate([0,0,330]) casetab();

            for (a=[0:60:300]) rotate([0,0,a]) casepin();
        }
        if (black) {
            for (an = [360/lednum:360/lednum:360]) {
                rotate([0,0,an]) {
                    translate([ledpos, 0, 6.7]) ledhole2(height=3);
                }
            }
        } else {
            for (an = [360/lednum:360/lednum:360]) {
                rotate([0,0,an]) {
                    translate([ledpos, 0, 6.7]) ledhole();
                    translate([ledpos, 0, 5.8]) cube([5.5,5.5,2],true);
                }
            }
        }
        translate([0,0,2.7]) rotate([0,0,360/48]) cylinder(5.6, 53.6/2-2, 53.6/2-2, $fn=24);
        translate([0,0,5.3]) rotate([0,0,7.5]) cube([41.8,43.2,6],true);
        translate([ 53.6/2-2,0,3.7]) cube([5,10,4.2],true);
        translate([-53.6/2+2,0,3.7]) cube([5,10,4.2],true);
    }
}

module diffuser() {
    if (black) {
        difference() {
            union() {
                translate([0,0,5.8]) rotate([0,0,360/48]) cylinder(1.5, 68/2, 68/2, $fn=24);
                for (an = [360/lednum:360/lednum:360]) {
                    rotate([0,0,an]) {
                        translate([ledpos, 0, 7.2]) ledhole2(size=2.8, height=0.4);
                        translate([ledpos, 0, 7.2]) ledhole2(size=2.0, height=1.0);
                    }
                }
            }
            translate([0,0,5.7]) rotate([0,0,360/48]) cylinder(1.7, 53.6/2+0.1, 53.6/2+0.1, $fn=24);
            translate([0,0,5.3]) rotate([0,0,7.5]) cube([41.8,43.2,6],true);
            
            rotate([0,0,360/48]) translate([0,-35,  4]) cube([10.2,4.7,10],true);
            rotate([0,0,360/48]) translate([0, 35,  4]) cube([10.2,4.7,10],true);
            *rotate([0,0,360/48]) translate([0,-35,1.8]) cube([10.2,5.5,10],true);
            *rotate([0,0,360/48]) translate([0, 35,1.8]) cube([10.2,5.5,10],true);

            for (an = [360/lednum:360/lednum:360]) {
                rotate([0,0,an]) {
                    translate([ledpos, 0, 5.8]) cube([5.5,5.5,2],true);
                    *translate([ledpos, 0, 6.8]) ledhole2(size=2, height=2.3, scale=0.05);
                    translate([ledpos, 0, 7.6]) ledhole2(size=1, height=0.7);
                }
            }
        }
    }
}

module casepin() {
    sides=24;
    h=4;
    slant=1;
    r=1;
    taper=0.7;
    translate([0, hexagonsize/s3-2.5,-2]) polyhedron(
        points = concat(
            [for (a=[0:360/sides:360-360/sides]) [sin(a)*r,cos(a)*r,h-slant*cos(a)*r] ],
            [for (a=[0:360/sides:360-360/sides]) [sin(a)*r,cos(a)*r,-0.5] ],
            [for (a=[0:360/sides:360-360/sides]) [sin(a)*r*taper,cos(a)*r*taper,-1] ]
        ),
        faces = concat(
            [[for(i=[0:sides-1]) i]],
            nquads(sides,0),
            nquads(sides,sides),
            [[for(i=[sides*3-1:-1:sides*2]) i]]
        )
    );
}

module casepinhole() {
    sides=24;
    h=3;
    r=1.1;
    translate([0, hexagonsize/s3-2.5,caseheight-2.5]) polyhedron(
        points = concat(
            [for (a=[0:360/sides:360-360/sides]) [sin(a)*r,cos(a)*r,0] ],
            [for (a=[0:360/sides:360-360/sides]) [sin(a)*r,cos(a)*r,h] ]
        ),
        faces = concat(
            [[for(i=[0:sides-1]) i]],
            nquads(sides,0),
            [[for(i=[sides*2-1:-1:sides]) i]]
        )
    );
    
    // cylinder(4,1,1, $fn=sides);
}

module casetabcut() {
    translate([-10.5,(hexagonsize-10)/2+0.1,caseheight-2])
        rotate([0,90,0])
            linear_extrude(height=21) polygon([
                [-2.5,-1],[-2.5,1],[-1,0],[-0.5,0],[0.5,1],[0.5,-1]
            ]);
}

module casetab() {
    translate([-10,(hexagonsize-10)/2,-2])
        rotate([0,90,0])
            linear_extrude(height=20) polygon([
                [-11,-1.5],[-9,0],[-0.5,0],[0.3,0.8],[1.7,-0.2],[0.3,-1.5]
            ]);
}

module bottomtab(off=0) {
    translate([-10+off,(diamondsize-4)/2-0.9,0])
        rotate([0,90,0])
            linear_extrude(height=20) polygon([
                [0,-1.5],[0,0],[-8.2,0],[-9,0.8],[-11,-0.5],[-9.5,-1.5]
            ]);
}

module bottomtablip(off=0) {
    translate([-10+off,(diamondsize-4)/2-0.8,0])
        rotate([0,90,0])
            linear_extrude(height=20) polygon([
                [-6,1.3],[-8.2,0],[-9.5,1.3]
            ]);
}

function linepoints(x1, y1, x2, y2, z, n) = [
    for (p = [0:n-1]) [x1+(x2-x1)*(p/n), y1+(y2-y1)*(p/n), z]
];

function ntris(n,n1,n2,o,f) = concat(
    [for (i=[n1:n2-1]) [(i+f)%n+o,i+o+n,(i+1)%n+o+n]],
    [for (i=[n1:n2-1]) [(i+1)%n+o,i+o,(i+1-f)%n+o+n]]
    );


module ledhole2(height=3.1, size=3, scale=1) {
    translate([0.2,0,0]) linear_extrude(height=height,scale=scale) polygon([[size,0],[0,size/s3],[-size,0],[0,-size/s3]]);
}

module ledhole(height=2.7, size=3, sides=48) {
    polyhedron(
        points = concat(
            [for (a=[0:360/sides:360-360/sides]) [size*cos(a)*0.6,size*sin(a)*0.6,0]],
            linepoints( size, 0, 0,  size/s3, height, sides/4),
            linepoints(0,  size/s3, -size, 0, height, sides/4),
            linepoints(-size, 0, 0, -size/s3, height, sides/4),
            linepoints(0, -size/s3,  size, 0, height, sides/4)
        ),
        faces = concat(
            [[for(i=[0:sides-1]) i]],
            ntris(sides,      0  ,sides/4  ,0,1),
            ntris(sides,sides/4  ,sides/2  ,0,0),
            ntris(sides,sides/2  ,sides*3/4,0,1),
            ntris(sides,sides*3/4,sides    ,0,0),
            [[for(i=[sides*2-1:-1:sides]) i]]
            )
    );
}

module hexagon(hw) {
    c1=hw/2;
    c2=c1/s3;
    polygon([[-c1,-c2],[0,-c2*2],[c1,-c2],[c1,c2],[0,c2*2],[-c1,c2]]);
}

module diamond(dw, rd=2) {
    polygon(concat(
        [for (an = [-60:10:60]) [-dw+rd*2-rd*cos(an),rd*sin(an)] ],
        [[0,dw/s3]],
        [for (an = [-60:10:60]) [dw-rd*2+rd*cos(an),-rd*sin(an)] ],
        [[0,-dw/s3]]
    ));
}

module triangle(tw, rd) {
    polygon(concat(
        [for (an = [  0:10:120]) [-(tw-rd[0]*6/s3)/2/s3-rd[0]*cos(an), (tw-rd[0]*6/s3)/2+rd[0]*sin(an)] ],
        [for (an = [120:10:240]) [ (tw-rd[1]*6/s3)/s3-rd[1]*cos(an),rd[1]*sin(an)] ],
        [for (an = [240:10:360]) [-(tw-rd[2]*6/s3)/2/s3-rd[2]*cos(an),-(tw-rd[2]*6/s3)/2+rd[2]*sin(an)] ]
        /* [[tw/s3,0],[-tw/2/s3,tw/2],[-tw/2/s3,-tw/2]] */
    ));
}

function ngon(n,w,h) = [for (a=[360/n:360/n:360]) [sin(a)*w,cos(a)*w,h]];

function nquads(n,o) = [for (i=[0:n-1]) [(i+1)%n+o,i+o,i+o+n,(i+1)%n+o+n]];

module hexcover(height, topwidth, bottomwidth, wallthickness, topthickness, sides=6) {
    slope = (bottomwidth-topwidth)/2/height;
    wtxo = sqrt(pow(wallthickness,2)+pow(wallthickness*slope,2));
    wtxo2 = wtxo-topthickness*slope;
    polyhedron(
        points = concat(
            ngon(sides,topwidth/2,height),
            ngon(sides,bottomwidth/2,0),
            ngon(sides,bottomwidth/2-wtxo,0),
            ngon(sides,topwidth/2-wtxo2,height-topthickness)),
        faces = concat(
            [[for(i=[0:sides-1]) i]],
            nquads(sides,0),
            nquads(sides,sides),
            nquads(sides,sides*2),
            [[for(i=[sides*4-1:-1:sides*3]) i]]),
        convexity=10);
}
