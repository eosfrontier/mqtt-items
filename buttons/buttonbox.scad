s3 = sqrt(3);

trivec = 3;
trisize = 170;
trioff = trisize / s3;
bsize = 25;
wid = 3;
cwid = wid*s3/2;
height = 35;
bheight = 20;
bott = 2;
sheight = 9.5;
cang = 5;
btol = 0.3;
tol = 0.1;
bth = 6;
bbev = 4;
bwid = 2;

crv = 10;

*color("gray")
front();
*color("teal") translate([9.5,-1,height-wid-0.1]) rotate([0,0,180]) batteryholder();

*switches();

color("gray")
front2();
color("gray")
translate([0,0,-tol]) bottom2();
switches(180);
color("teal") translate([0,-17,height-wid-0.1]) rotate([0,0,180]) batteryholder();

*button(180);

module bottom2() {
    to2 = trioff-bsize-cwid;
    difference() {
        cur_prism(trivec, [
            [to2,0,-bwid],
            [to2,sheight-2,-bwid],
            [to2,sheight-2,-tol],
            [to2,0,-tol],
            [to2,0,cwid],
            [to2,-bott,cwid]
        ],cw=bsize);
        #for (n = [360/trivec:360/trivec:360]) {
            rotate([0,0,n])
            translate([35,5,0])
            cur_prism(trivec, [
                [20,1,-5],
                [20,-bott-1,-5]
            ]);
        }
    }
}

module switches(rot=0) {
    for (n = [360/trivec:360/trivec:360]) {
        rotate([0,0,n])
        translate([0,trioff-bsize-cwid,0]) {
            #color("brown") translate([0,-1.25,sheight-0.75]) cube([15.5,15.5,1.5], true);
            color("white") rotate([0,0,rot]) button(rot);
        }
    }
}

module swtab(w=10) {
    rotate([0,90,0])
    translate([0,0,-w/2])
    linear_extrude(height=w)
    polygon([
        [-0.1,0.1],[1.6,0.1],[2.0,-0.3],
        [3.5,0.6],[2.8,1.2],[-0.1,1.2]
    ]);
}

module button(rot) {
    bs2 = bsize-btol*s3;
    cur_prism(trivec, [
            [bs2,height+bth,-bbev-cwid],
            [bs2,height+bth-bbev,-cwid],
            [bs2,sheight+wid+2,-cwid],
            [bs2,sheight+2,0],
            [bs2,sheight,0],
            [0,sheight,5-crv],
            [0,sheight+wid,5-crv],
            [bs2,sheight+wid,-cwid-bwid],
            [bs2,height+bth-bbev,-cwid-bwid],
            [bs2,height+bth-bwid,-cwid-bbev] ]);    
    rotate([0,0,90]) translate([0,15.5/2,sheight]) swtab();
    rotate([0,0,-90]) translate([0,15.5/2,sheight]) swtab();
    rotate([0,0,rot]) {
        translate([0,7.2,sheight-1]) cube([10,1,2],true);
        translate([-8,-9.6,sheight-1]) cube([3,1,2],true);
        translate([ 8,-9.6,sheight-1]) cube([3,1,2],true); 
    }
}

module front2() {
    sides = 360/cang;
    to2 = trioff-bsize-cwid;
    difference() {
        cur_prism(trivec,[
            [to2,height,cwid-cwid],
            [to2,height-cwid,cwid],
            [to2,0,cwid],
            [to2,0,-0.01],
            [to2,height-wid,-0.01]],cw=bsize);
        for (n=[360/trivec:360/trivec:360]) {
            rotate([0,0,n])
            translate([0,trioff-bsize-cwid,0])
            rotate([0,0,180])
            polyhedron(
                points = concat(
                    cur_ngon(trivec,bsize,height+0.5,crv-cwid+s3/2),
                    cur_ngon(trivec,bsize,height-0.5,crv-cwid),
                    cur_ngon(trivec,bsize,height-wid-0.5,crv-cwid)
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
        rotate([0,0,180])
        polyhedron(
            points = concat(
                cur_ngon(trivec,bsize,sheight+wid+2,crv-cwid),
                cur_ngon(trivec,bsize,height-1,crv-cwid),
                cur_ngon(trivec,bsize,height-1,crv),
                cur_ngon(trivec,bsize,sheight+2,crv) ),
            faces = concat(
                nquads(sides,0),
                nquads(sides,sides),
                nquads(sides,sides*2),
                [for (i=[0:sides-1]) [(i+1)%sides,i+sides*3,i]],
                [for (i=[0:sides-1]) [(i+1)%sides,(i+1)%sides+sides*3,i+sides*3]]
            ) );
                
    }
    translate([0,-17,height-wid-0.1]) batteryclips();
    translate([-50,25,height-wid-0.1]) rotate([0,0,-120]) wemosd1();
}


module front() {
    sides = 360/cang;
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

    translate([ 10,-12.7-2/2,-7/2+0.1]) cube([7,2,7.2],true);
    translate([ 10, 12.7+2/2,-7/2+0.1]) cube([7,2,7.2],true);
    translate([-10,-12.7-2/2,-7/2+0.1]) cube([7,2,7.2],true);
    translate([ -6, 12.7+2/2,-7/2+0.1]) cube([7,2,7.2],true);
}

module mcutab(w) {
    translate([-w/2,0,0]) rotate([0,90,0]) linear_extrude(height=w) polygon([
        [-0.2,0],[4.8,0],[5.4,-0.5],[7,0.5],[5.4,1.5],[-0.2,1.5]
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

module cur_prism(sides, points, ct = crv, cs = 10, cw=0) {
    sds = 360/cs;
    nm = len(points);
    polyhedron(
        points = [for (ps = points) each cur_ngon(sides, ps[0],ps[1],ct+(ps[2]?ps[2]:0),cs,cw)]
            ,
        faces = concat(
                [[for (n=[0:sds-1]) n]],
                [for (n=[0:nm-2]) each nquads(sds,sds*n)],
                [[for (n=[sds-1:-1:0]) n+sds*(nm-1)]]
            ));
            
}

function cur_ngon(n,s,h,ct=crv,cs=cang,cw=0) = concat([for (sd=[0:n-1]) each concat(
        [for (an=[(sd-0.5)*360/n+cs/2:cs:(sd)*360/n-cs/2])
            [s*sin(sd*360/n)+ct*sin(an)+cw*sin((sd-0.5)*360/n),
             s*cos(sd*360/n)+ct*cos(an)+cw*cos((sd-0.5)*360/n),h]],
        [for (an=[(sd)*360/n+cs/2:cs:(sd+0.5)*360/n-cs/2])
            [s*sin(sd*360/n)+ct*sin(an)+cw*sin((sd+0.5)*360/n),
             s*cos(sd*360/n)+ct*cos(an)+cw*cos((sd+0.5)*360/n),h]])
        ]
        );
