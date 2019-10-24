
cang = 1;
bang = 10;

translate([-5,0,0]) difference() {
    quarterpipe(san=-90);
    translate([10,110,50]) rotate([-90,0,0])
    #cylinder(30,40,40,$fn=180);
}

translate([ 5,0,0]) union() {
    quarterpipe(san=0);
    translate([0,120,50]) disc();
}

module disc(r=30, t=18, bv=3) {
    csds = 360/cang;
    bsds = 180/bang+1;
    polyhedron(
        points = concat(
            [for (an=[-90:bang:0]) each circle(
                r-bv*(1-cos(an)), t-bv*(1+sin(an)))],
            [for (an=[0:bang:90]) each circle(
                r-bv*(1-cos(an)), bv*(1-sin(an)))]
        ),
        faces = concat(
            [for (s=[0:bsds-2]) each cquads(csds, csds*s)],
            [[for (s=[0:csds-1]) s]],
            [[for (s=[csds-1:-1:0]) s+csds*(bsds-1)]]
        ));
}

function circle(r, h, a=cang) = 
    [for (an=[a:a:360]) [r*cos(an),h,r*sin(an)]];

module quarterpipe(rw=120, rh=120, s=100, t=15, bv=3, san=0) {
    csds = 90/cang+1;
    bsds = 360/bang+4;
    polyhedron(
        points = concat(
            [for (an=[0:bang:90]) each qcircle(
                rw+bv*(1-sin(an)), rh+bv*(1-sin(an)),
                bv*(1-cos(an)), san
                )],
            [for (an=[90:bang:180]) each qcircle(
                rw+bv*(1-sin(an)), rh+bv*(1-sin(an)),
                s-bv*(1+cos(an)), san
                )],
            [for (an=[180:bang:270]) each qcircle(
                rw+t-bv*(1+sin(an)), rh+t-bv*(1+sin(an)),
                s-bv*(1+cos(an)), san
                )],
            [for (an=[270:bang:360]) each qcircle(
                rw+t-bv*(1+sin(an)), rh+t-bv*(1+sin(an)),
                bv*(1-cos(an)), san
                )]
        ),
        faces = concat(
            [for (s=[0:bsds-1]) each cquads(csds, csds*s, csds*bsds, 1)],
            [[for (s=[0:bsds-1]) s*csds+csds-1]],
            [[for (s=[bsds-1:-1:0]) s*csds]]
        ));        
}

function qcircle(rw, rh, s, san=0, a=cang) =
    [for (an=[san:a:san+90]) [rw*sin(an),rh*cos(an),s]];

function cquads(n,o,s=9999999,ex=0) = concat(
    [for (i=[0:n-1-ex]) [(i+1)%n+o,i+o,(i+o+n)%s]],
    [for (i=[0:n-1-ex]) [(i+1)%n+o,(i+o+n)%s,((i+1)%n+o+n)%s]]
);
