s3 = sqrt(3);

trivec = 3;
trisize = 180;
trioff = trisize / s3;
bsize = 27;
wid = 3;
cwid = wid*s3;
height = 35;
bheight = 20;
sheight = height - 25;
cang = 10;
btol = 0.3;
bth = 5;
bbev = 3;
bwid = 2;

color("gray")
front();
color("teal") translate([9.5,-1,height-wid-0.1]) rotate([0,0,180]) batteryholder();

switches();

module switches() {
    for (n = [360/trivec:360/trivec:360]) {
        rotate([0,0,n])
        translate([0,trioff-bsize-cwid,0]) {
            *color("brown") translate([0,0,sheight-0.5]) cube([15.5,15.5,1], true);
            color("white") button();
        }
    }
}

module button() {
    bs2 = bsize-btol*s3;
    cur_prism(trivec, [
            [bs2-wid-bbev,height+bth],
            [bs2-wid,height+bth-bbev],
            [bs2-wid,sheight+wid+2],
            [bs2,sheight+2],
            [bs2,sheight],
            [3,sheight],
            [3,sheight+wid],
            [bs2-bwid-wid,sheight+wid],
            [bs2-bwid-wid,height+bth-wid] ]);            
}

module front() {
    sides = 360/cang+trivec;
    difference() {
        cur_prism(trivec,[
            [trioff-wid/2,height],
            [trioff,height-wid/2],
            [trioff,0],
            [trioff-cwid,0],
            [trioff-cwid,height-wid]]);
        for (n=[360/trivec:360/trivec:360]) {
            rotate([0,0,n])
            translate([0,trioff-bsize-cwid,0])
            polyhedron(
                points = concat(
                    cur_ngon(trivec,bsize-wid/2,height+0.5),
                    cur_ngon(trivec,bsize-wid,height-0.5),
                    cur_ngon(trivec,bsize-wid,height-wid-0.5)
                ), faces = concat(
                [[for (n=[0:sides-1]) n]],
                nquads(sides,0),
                nquads(sides,sides),
                [[for (n=[sides-1:-1:0]) n+sides*2]]
                )
            );
        }
    }
    for (n=[360/trivec:360/trivec:360]) {
        rotate([0,0,n])
        translate([0,trioff-bsize-cwid,0])
        polyhedron(
            points = concat(
                cur_ngon(trivec,bsize-wid,sheight+wid+2),
                cur_ngon(trivec,bsize-wid,height),
                cur_ngon(trivec,bsize,height),
                cur_ngon(trivec,bsize,sheight+2) ),
            faces = concat(
                nquads(sides,0),
                nquads(sides,sides),
                nquads(sides,sides*2),
                [for (i=[0:sides-1]) [(i+1)%sides,i+sides*3,i]],
                [for (i=[0:sides-1]) [(i+1)%sides,(i+1)%sides+sides*3,i+sides*3]]
            ) );
                
    }
    translate([9.5,-1,height-wid-0.1]) batteryclips();
    translate([-33,5,height-wid-0.1]) rotate([0,0,90]) wemosd1();
}

function ngon(n,s,h) = [for (an=[360/n:360/n:360]) [s*cos(an),s*sin(an),h]];

function cur_ngon(n,s,h,ct=5,cs=cang) = concat([for (sd=[0:n-1])
        for (an=[(sd-0.5)*360/n:cs:(sd+0.5)*360/n])
            [s*sin(sd*360/n)+ct*sin(an),s*cos(sd*360/n)+ct*cos(an),h]]
        );
        
function cur_ngon2(n,s,ct=5,cs=cang) = concat([for (sd=[0:n-1])
        for (an=[(sd-0.5)*360/n:cs:(sd+0.5)*360/n])
            [s*sin(sd*360/n)+ct*sin(an),s*cos(sd*360/n)+ct*cos(an)]]
        );

module batteryclips() {
    translate([-33/2,-95/2,0]) batterypin();
    translate([-33/2, 95/2,0]) batterypin();
    translate([ 33/2,-95/2,0]) batterypin();
    translate([ 33/2, 95/2,0]) batterypin();
    translate([-14,-50.6,0]) rotate([0,0,180]) batterytab(10);
    translate([-14, 50.3,0]) batterytab(10);
    translate([ 14,-50.6,0]) rotate([0,0,180]) batterytab(10);
    translate([ 14, 50.3,0]) batterytab(10);
}

module batterypin() {
    rotate([0,0,360/48]) {
        translate([0,0,-1.9]) cylinder(4.0,2.25,2.25, $fn=24);
        translate([0,0,-3.0]) cylinder(1.2,1.1,1.2, $fn=24);
        translate([0,0,-4.0]) cylinder(1.0,0.8,1.1, $fn=24);
    }
}

module batterytab(w) {
    translate([-w/2,0,0]) rotate([0,90,0]) linear_extrude(height=w) polygon([
        [-2.1,0],[3.0,0],[3.8,-0.8],[6.5,0.5],[5.5,1.5],[-2.1,1.5]
    ]);
}

module wemosd1() {
    *#translate([0,0,-2.4]) cube([34.6,25.4,4.8], true);
    // translate([-17.3,   0,0]) rotate([0,0,90]) mcutab(12);
    // translate([ 17.3,-7.2,0]) rotate([0,0,-90]) mcutab(8);
    translate([ 17.3, 0,0]) rotate([0,0,-90]) mcutab(15);
    translate([-17.3,-9.2,0]) rotate([0,0, 90]) mcutab(8);
    translate([-17.3, 9.2,0]) rotate([0,0, 90]) mcutab(8);

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
    bh=50/2-cr;
    bw=100.5/2-cr;
    difference() {
        union() {
            translate([1,0.2,-3]) linear_extrude(height=1) polygon(concat(
                [for (an=[  0:10: 90]) [ bh+sin(an)*cr, bw+cos(an)*cr]],
                [for (an=[ 90:10:180]) [ bh+sin(an)*cr,-bw+cos(an)*cr]],
                [for (an=[180:10:270]) [-bh+sin(an)*cr,-bw+cos(an)*cr]],
                [for (an=[270:10:360]) [-bh+sin(an)*cr, bw+cos(an)*cr]]
            ));
            translate([0, 6,-12]) cube([40,77,24], true);
        }
        translate([-33/2,-95/2,-4.5]) cylinder(3,1.5,1.5, $fn=24);
        translate([-33/2, 95/2,-4.5]) cylinder(3,1.5,1.5, $fn=24);
        translate([ 33/2,-95/2,-4.5]) cylinder(3,1.5,1.5, $fn=24);
        translate([ 33/2, 95/2,-4.5]) cylinder(3,1.5,1.5, $fn=24);
    }
}

function nquads(n,o) = concat(
    [for (i=[0:n-1]) [(i+1)%n+o,i+o,i+o+n]],
    [for (i=[0:n-1]) [(i+1)%n+o,i+o+n,(i+1)%n+o+n]]
);

module cur_prism(sides, points, ct = 5, cang = 10) {
    sds = 360/cang+sides;
    nm = len(points);
    polyhedron(
        points = [for (ps = points) each cur_ngon(sides, ps[0],ps[1],ct,cang)]
            ,
        faces = concat(
                [[for (n=[0:sds-1]) n]],
                [for (n=[0:nm-1]) each nquads(sds,sds*n)],
                [[for (n=[sds-1:-1:0]) n+sds*(nm-1)]]
            ));
            
}
