

speaker();

module speaker() {
    height = 35;
    backdia = 44.5;
    backhi = 11.5;
    coneb = 57;
    conet = 70;
    conehi = 15;
    cone2hi = 5.5;
    
    plate = 0.5;
    ring = 1.5;
    frontdia = 77;
    frontindia = 71;

    platexy = 78;
    platedia = 95;
    
    fn=80;

    difference() {
        union() {
            translate([0,0,-height])
            cylinder(backhi, backdia/2, backdia/2, $fn=fn);
            translate([0,0,-height+backhi])
            cylinder(cone2hi, backdia/2, coneb/2, $fn=fn);
            translate([0,0,-height+backhi+cone2hi])
            cylinder(conehi, coneb/2, conet/2, $fn=fn);
            translate([0,0,-height+backhi+cone2hi+conehi])
            cylinder(height-backhi-cone2hi-conehi,
                frontdia/2, frontdia/2, $fn=fn);
            translate([0,0,-ring-plate])
            // cube([platexy, platexy, plate], true);
            plate(platexy/2, platedia/2, plate);
        }
        translate([0,0,-plate-ring])
        cylinder(plate+ring,
            frontindia/2, frontindia/2, $fn=fn);
        for (i=[45:90:315]) {
            rotate([0,0,i])
            union() {
                translate([41.4,0,-plate-ring-0.1])
                cylinder(plate+0.2, 2.2, 2.2, $fn=16);
                translate([42,0,-plate/2-ring])
                cube([1.2,4.4,plate+0.2], true);
                translate([42.6,0,-plate-ring-0.1])
                cylinder(plate+0.2, 2.2, 2.2, $fn=16);
            }
        }
        translate([0,0,-plate-ring])
        cylinder(plate+ring,
            frontindia/2, frontindia/2, $fn=fn);
    }
}

module plate(d1, d2, h, stp = 10) {
    sd2 = d2 / sqrt(2);
    off = ((d1*d1)/2 - (sd2*sd2)) / (sd2 - d1);
    ang = asin(sd2/(off+d1));
    linear_extrude(height = h) polygon(squaroid(off, off+d1, ang, stp));
}

function nquads(n,o) = concat(
    [for (i=[0:n-1]) [(i+1)%n+o,i+o,i+o+n]],
    [for (i=[0:n-1]) [(i+1)%n+o,i+o+n,(i+1)%n+o+n]]
);

function squaroid(off, dia, ao, stp=10) = concat(
    [for (a=[   -ao:(2*ao/stp):    ao+0.01]) [     dia*sin(a), -off+dia*cos(a)]],
    [for (a=[ 90-ao:(2*ao/stp): 90+ao+0.01]) [-off+dia*sin(a),      dia*cos(a)]],
    [for (a=[180-ao:(2*ao/stp):180+ao+0.01]) [     dia*sin(a),  off+dia*cos(a)]],
    [for (a=[270-ao:(2*ao/stp):270+ao+0.01]) [ off+dia*sin(a),      dia*cos(a)]]
);
