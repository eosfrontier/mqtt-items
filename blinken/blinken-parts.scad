s3 = sqrt(3);

studwidth = 50;
studheight = studwidth/s3/2;
holesize = 8;
holeheight = 8;
holetop = 11;
bottom = 1;
thickness = 1.3;
numstuds = 5;
studspace = 8;
sideheight = 10;
boxheight = 20;
boxthick = 2;
edge = 2;

for (i = [0:numstuds-1]) {
    color("white")
    translate([(i-(numstuds-1)/2)*(studwidth+studspace),0,0])
    lightstud();
}
color("DimGray") lightbox();
*color("Gray") translate([0,0,-boxheight-sideheight]) lightbottom();

color("Teal") translate([0,0,-boxthick-5]) rotate([0,0,90]) batteryholder();

/*
for (i = [0:numstuds-1]) {
    translate([(i-(numstuds-1)/2)*(studwidth+studspace),0,-boxthick]) {
        rotate([90,0,0]) translate([-11,0,-0.5]) clip();
        rotate([90,0,0]) translate([ 11,0,-0.5]) clip();
    }
}
*/

*rotate([0,0,40]) mirror([0,0,1]) lightbox();
*lightstud();
*rotate([0,0,40]) lightbottom();


module clip() {
    cw = 6/2;
    ch = bottom+boxthick;
    linear_extrude(height=1) polygon([
        [-cw-1,-2],[-cw-2,-1],[-cw-2,0],
        [-cw,0],[-cw,ch],[-cw-1.5,ch+1.5],[-cw+0.5,ch+3.5],
        [-cw+2,ch+2],[-cw+2,0],
        [cw-2,0],[cw-2,ch+2],
        [cw-0.5,ch+3.5],[cw+1.5,ch+1.5],[cw,ch],[cw,0],
        [cw+2,0],[cw+2,-1],[cw+1,-2]
    ]);
}

module lightbox() {
    ol = boxthick-1;
    xs2 = (studwidth*(numstuds-1) + studspace*(numstuds-1))/2;
    xs1 = xs2 + studwidth/2 + s3*edge;
    ys = studheight + edge;
    h = sideheight;
    h2 = h + boxheight;
    xb1 = xs1 + h*s3;
    xb2 = xs2;
    yb = ys + h;
    
    xo1 = xb1 - ol*s3;
    xo2 = xb2;
    yo = yb - ol;
    
    ho = h2 - boxthick;
    xi1 = xb1 - boxthick*s3;
    xi2 = xb2;
    yi = yb - boxthick;
    hi = boxthick;
    
    sxs = studwidth/2-0.4*s3;
    sys = studwidth/s3/2-0.4;
    
    difference() {
        polyhedron(
            points = concat(
                hexbox(xs1,xs2,ys,0),
                hexbox(xb1,xb2,yb,-h),
                hexbox(xb1,xb2,yb,-h2),
                hexbox(xo1,xo2,yo,-h2),
                hexbox(xo1,xo2,yo,-ho),
                hexbox(xi1,xi2,yi,-ho),
                hexbox(xi1,xi2,yi,-h),
                hexbox(xs1,xs2,ys,-hi)
            ),
            faces = concat(
                [[for(i=[0:5]) i]],
                nquads(6,0),
                nquads(6,6),
                nquads(6,12),
                nquads(6,18),
                nquads(6,24),
                nquads(6,30),
                nquads(6,36),
                [[for(i=[5:-1:0]) i+42]]
            )
        );
        for (i = [0:numstuds-1]) {
            translate([(i-(numstuds-1)/2)*(studwidth+studspace),0,-hi-0.1]) {
                linear_extrude(height=hi+0.2)
                polygon([[-sxs,0],[0,sys],[sxs,0],[0,-sys]]);
                translate([0,0,hi-0.3])
                linear_extrude(height=0.5,scale=(sys+0.5)/sys)
                polygon([[-sxs,0],[0,sys],[sxs,0],[0,-sys]]);
            }
            /*
            translate([(i-(numstuds-1)/2)*(studwidth+studspace),0,-boxthick-0.1])
            cylinder(boxthick+0.2,holesize/2,holesize/2,$fn=64);
            
            translate([(i-(numstuds-1)/2)*(studwidth+studspace),0,-boxthick/2]) {
                translate([-11,0,0]) cube([6.2,1.2,boxthick+0.2],true);
                translate([ 11,0,0]) cube([6.2,1.2,boxthick+0.2],true);
            }
            */
        }
    }
    translate([(numstuds/2-1) * (studwidth+studspace), 0, -boxthick]) wemosd1();

    translate([0,0,-boxheight-sideheight]) {
        for (i = [0:numstuds-2]) {
            bottomtablip((i-(numstuds-2)/2)*(studwidth+studspace));
            mirror([0,1,0]) bottomtablip((i-(numstuds-2)/2)*(studwidth+studspace));
        }
        rpos = (studheight+edge+sideheight-boxthick-0.8) * s3/2;
        translate([-(studwidth+studspace)*(numstuds-1)/2,0,0]) rotate([0,0,30]) bottomtablip(-studwidth/4, rpos);
        translate([-(studwidth+studspace)*(numstuds-1)/2,0,0]) rotate([0,0,150]) bottomtablip(studwidth/4, rpos);
        translate([(studwidth+studspace)*(numstuds-1)/2,0,0]) rotate([0,0,-30]) bottomtablip(studwidth/4, rpos);
        translate([(studwidth+studspace)*(numstuds-1)/2,0,0]) rotate([0,0,-150]) bottomtablip(-studwidth/4, rpos);
    }
}

module lightbottom() {
    tol=0.2;
    ol = boxthick-1;
    
    xs2 = (studwidth*(numstuds-1) + studspace*(numstuds-1))/2;
    xs1 = xs2 + studwidth/2 + s3*edge;
    ys = studheight + edge;
    h = sideheight;
    xb1 = xs1 + h*s3;
    xb2 = xs2;
    yb = ys + h;
    
    xi1 = xb1 - ol*s3 - tol*s3;
    xi2 = xb2;
    yi = yb - ol - tol;
    
    difference() {
        linear_extrude(height=boxthick) polygon([
            [-xi1,0],[-xi2,yi],[xi2,yi],[xi1,0],[xi2,-yi],[-xi2,-yi]
        ]);
        for (i = [-numstuds+1:numstuds-1]) {
            translate([i*(studwidth+studspace)/2,0,-0.1])
            rotate([0,0,30]) cylinder(boxthick+0.2,10,10,$fn=6);
        }
    }
    for (i = [0:numstuds-2]) {
        bottomtab((i-(numstuds-2)/2)*(studwidth+studspace));
        mirror([0,1,0]) bottomtab((i-(numstuds-2)/2)*(studwidth+studspace));
    }
    rpos = (studheight+edge+sideheight-boxthick-0.9) * s3/2;
    translate([-(studwidth+studspace)*(numstuds-1)/2,0,0]) rotate([0,0,30]) bottomtab(-studwidth/4, rpos);
    translate([-(studwidth+studspace)*(numstuds-1)/2,0,0]) rotate([0,0,150]) bottomtab(studwidth/4, rpos);
    translate([(studwidth+studspace)*(numstuds-1)/2,0,0]) rotate([0,0,-30]) bottomtab(studwidth/4, rpos);
    translate([(studwidth+studspace)*(numstuds-1)/2,0,0]) rotate([0,0,-150]) bottomtab(-studwidth/4, rpos);
}

module lightstud() {
    tol=0;
    xs = studwidth/2-tol;
    ys = xs/s3;
    zs = studheight;
    cs = holesize/2;
    
    xse = xs - 0.6*s3;
    yse = ys - 0.6;
    
    //ch = holeheight;
    ch = 0;
    tt = thickness;
    zs2 = zs - tt;
    xs2 = xs - (ch+tt)*s3;
    ys2 = ys - (ch+tt);
    sides = 64;
    ld = 22;
    hi = boxthick;
    bv = 0.5;
    xs3 = xse - bv*s3;
    ys3 = yse - bv;

    difference() {
        polyhedron(
            points = concat(
                [[0,0,zs2],[xs2,0,ch],[0,-ys2,ch],[-xs2,0,ch],[0,ys2,ch]],
                [[0,0,zs],[xs,0,0],[0,-ys,0],[-xs,0,0],[0,ys,0]],
                [[xse,0,-0.4],[0,-yse,-0.4],[-xse,0,-0.4],[0,yse,-0.4]],
                [[xse,0,-hi+bv],[0,-yse,-hi+bv],[-xse,0,-hi+bv],[0,yse,-hi+bv]],
                [[xs3,0,-hi],[0,-ys3,-hi],[-xs3,0,-hi],[0,ys3,-hi]],
                [for (an=[0:360/sides:359]) [sin(an)*cs,cos(an)*cs,-hi]],
                [for (an=[0:360/sides:359]) [sin(an)*cs,cos(an)*cs,ch]]
            ),
            faces = concat(
                [[2,1,0],[3,2,0],[4,3,0],[1,4,0]],
                [for (i=[0:3]) for (j=[0:sides/4-1])
                    [ i+ld-4, ld+((i*sides/4)+j+sides/8)%sides, ld+((i*sides/4)+j+sides/8+1)%sides ]],
                [for (i=[0:3]) [i+ld-4,(i+3)%4+ld-4, ld+(i*sides/4+sides/8)%sides]],
                nquads(sides,ld),
                [for (i=[0:3]) for (j=[0:sides/4-1])
                    [ i+1, ld+sides+((i*sides/4)+j+sides/8+1)%sides, ld+sides+((i*sides/4)+j+sides/8)%sides ]],
                [for (i=[0:3]) [(i+3)%4+1,i+1, ld+sides+(i*sides/4+sides/8)%sides]],
                [[6,7,5],[7,8,5],[8,9,5],[9,6,5]],
                nquads(4,6),
                nquads(4,10),
                nquads(4,14)
            )
        );
        /*
        translate([-11,0,bottom/2]) cube([6.2,1.2,bottom+0.2],true);
        translate([ 11,0,bottom/2]) cube([6.2,1.2,bottom+0.2],true);
        */
    }
}

module wemosd1() {
    *#translate([0,0,-2.4]) cube([34.6,25.4,4.8], true);
    // translate([-17.3,   0,0]) rotate([0,0,90]) mcutab(12);
    translate([ 17.3,-11.2,0]) rotate([0,0,-90]) mcutab(6);
    translate([ 17.3, 11.2,0]) rotate([0,0,-90]) mcutab(6);
    translate([-17.3,-11.2,0]) rotate([0,0, 90]) mcutab(6);
    translate([-17.3, 11.2,0]) rotate([0,0, 90]) mcutab(6);

    translate([ 10,-12.7-2/2,-7/2+0.1]) cube([7,2,7],true);
    translate([ 10, 12.7+2/2,-7/2+0.1]) cube([7,2,7],true);
    translate([-10,-12.7-2/2,-7/2+0.1]) cube([7,2,7],true);
    translate([-10, 12.7+2/2,-7/2+0.1]) cube([7,2,7],true);
}

module mcutab(w) {
    translate([-w/2,0,0]) rotate([0,90,0]) linear_extrude(height=w) polygon([
        [0,0],[4.8,0],[5.4,-0.5],[7,0.5],[5.4,1.5],[0,1.5]
    ]);
}

module bottomtab(off=0, pos=studheight+edge+sideheight-boxthick-0.9) {
    translate([-10+off,pos,0])
    rotate([0,90,0])
    linear_extrude(height=20) polygon([
        [0,-1.5],[0,0],[-8.2,0],[-9,0.8],[-11,-0.5],[-9.5,-1.5]
    ]);
}

module bottomtablip(off=0, pos=studheight+edge+sideheight-boxthick-0.8) {
    translate([-10+off,pos,0])
    rotate([0,90,0])
    linear_extrude(height=20) polygon([
        [-6,1.3],[-8.2,0],[-9.5,1.3]
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

function nquads(n,o) = concat(
    [for (i=[0:n-1]) [(i+1)%n+o,i+o,i+o+n]],
    [for (i=[0:n-1]) [(i+1)%n+o,i+o+n,(i+1)%n+o+n]]
);
function hexbox(x1,x2,y,z) = [[-x1,0,z],[-x2,y,z],[x2,y,z],[x1,0,z],[x2,-y,z],[-x2,-y,z]];
