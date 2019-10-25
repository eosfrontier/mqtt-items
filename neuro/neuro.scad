cang = 5;
bang = 15;

difference() {
    translate([-10,0,0]) quarterpipe();
    translate([5,110,50]) rotate([-90,0,0])
    #cylinder(35,35,35,$fn=60);
}

union() {
    difference() {
        translate([10,0,0]) mirror([1,0,0]) quarterpipe();
        translate([10,0,0]) {
            cpoint(5, 15);
            cline(5,15,15,15);
            cpoint(15, 15);
            cline(15,15,25,35);
            cpoint(25, 35);
            cline(25,35,25,55);
            cpoint(25, 55);
            cline(25,55,45,55);
            cpoint(45,55);
            cline(45,55,75,35);
            cpoint(75,35);
            cline(75,35,85,55);
            cpoint(5,85);
            cline(5,85,20,75);
            cpoint(20,75);
            cline(20,75,30,80);
            cpoint(30,80);
            cline(30,80,40,70);
            cpoint(40,70);
            cline(40,70,45,55);
        }
    }
    translate([5,120,50]) disc();
}


module cpoint(a, z, t=5, d=2, rw=120+15, rh=120+15) {
    translate([rw*sin(a),rh*cos(a), z])
    rotate([0,0,-a])
    translate([0,-d,0])
    disc(t, d*2, d*1.5, 0);
}

module cline(a1, z1, a2, z2, t=2, rw=120+15, rh=120+15, a=cang) {
    if (a1 > a2) {
        cline_lr(a2, z2, a1, z1, t, rw, rh, a);
    } else {
        cline_lr(a1, z1, a2, z2, t, rw, rh, a);
    }
}

module cline_lr(a1, z1, a2, z2, t, rw, rh, a) {
    st = a2-a1;
    abst = st>0.1?st:0.1;
    zdis = (z2-z1);
    zf = zdis/abst;
    bsds = 360/bang;
    csds = ceil(abst/a)+1;
    // Afstand is een hoek, moet echte afstand worden (ongeveer)
    adis = st*rw*PI/180;
    tnorm = sqrt(zdis*zdis+adis*adis);
    t1 = t*adis / tnorm;
    t2 = t*zdis / tnorm;
    aend = (st>0.1)?(st-0.1):0;
    polyhedron(
        points = [for (ba=[bang:bang:360]) each concat(
            [for (an=[0:a:aend])
             [(rw+t*sin(ba))*sin(an+a1)-t2*cos(ba)*cos(an+a1),
              (rh+t*sin(ba))*cos(an+a1)+t2*cos(ba)*sin(an+a1),
              z1+an*zf+t1*cos(ba)]
            ],
            [[(rw+t*sin(ba))*sin(a2)-t2*cos(ba)*cos(a2),
              (rh+t*sin(ba))*cos(a2)+t2*cos(ba)*sin(a2),
              z2+t1*cos(ba)]])],
        faces = concat(
            [for (s=[0:bsds-1]) each cquads(csds, csds*s, csds*bsds, 1)],
            [[for (s=[bsds-1:-1:0]) s*csds]],
            [[for (s=[bsds-1:-1:0]) (bsds-s)*csds-1]]
            ));
}

module disc(r=30, t=20, bbv=3, tbv=10) {
    csds = 360/cang;
    bsds = 180/bang+2;
    polyhedron(
        points = concat(
            [for (an=[-90:bang:0]) each circle(
                r-tbv*(1-cos(an)), t-tbv*(1+sin(an)))],
            [for (an=[0:bang:90]) each circle(
                r-bbv*(1-cos(an)), bbv*(1-sin(an)))]
        ),
        faces = concat(
            [for (s=[0:bsds-2]) each cquads(csds, csds*s)],
            [[for (s=[0:csds-1]) s]],
            [[for (s=[csds-1:-1:0]) s+csds*(bsds-1)]]
        ));
}

function circle(r, h, a=cang) = 
    [for (an=[a:a:360]) [r*cos(an),h,r*sin(an)]];

module quarterpipe(san=-90, rw=120, rh=120, s=100, t=15, bv=3, eb=5) {
    csds = 180/cang+2+(180/bang+2);
    bsds = 180/bang+2;
    tsds = csds*(bsds-1);
    polyhedron(
        points = concat(
            [for (an=[-90:bang:0]) each concat(
                qcircle(
                rw+t-bv*(1+sin(an)), rh+t-bv*(1+sin(an)),
                bv*(1-cos(an)), san
                ),
                bcircle(eb-bv*(1-cos(an)),
                1, rh+t-bv*(1+sin(an)), eb,
                san=-90
                ),
                bcircle(eb-bv*(1-cos(an)),
                1, rh+t-bv*(1+sin(180-an)), s-eb,
                san=0
                ),
                qcircle(
                rw+t-bv*(1+sin(180-an)), rh+t-bv*(1+sin(180-an)),
                s-bv*(1+cos(180-an)), san, a=-cang
                )
            )],
            [for (an=[0:bang:90]) each concat(
                qcircle(
                rw+bv*(1-sin(an)), rh+bv*(1-sin(an)),
                bv*(1-cos(an)), san
                ),
                bcircle(eb-bv*(1-cos(an)),
                1, rh+bv*(1-sin(an)), eb,
                san=-90
                ),
                bcircle(eb-bv*(1-cos(an)),
                1, rh+bv*(1-sin(180-an)), s-eb,
                san=0
                ),
                qcircle(
                rw+bv*(1-sin(180-an)), rh+bv*(1-sin(180-an)),
                s-bv*(1+cos(180-an)), san, a=-cang
                )
            )]

        ),
        faces = concat(
            [for (s=[0:bsds-2]) each cquads(csds, csds*s, csds*bsds)],
            [for (s=[1:csds/2-1]) each [
                [s-1,s,csds-s],[s,csds-s-1,csds-s],
                [tsds+s,tsds+s-1,tsds+csds-s],[tsds+s,tsds+csds-s,tsds+csds-s-1]]]
        ));        
}

function bcircle(r, x, y, z, san=0, a=bang) =
    [for (an=[san+(a>0?0:90):a:san+(a>0?90:0)]) [x+r*cos(an),y,z+r*sin(an)]];

function qcircle(rw, rh, s, san=0, a=cang) =
    [for (an=[san+(a>0?0:90):a:san+(a>0?90:0)]) [rw*sin(an),rh*cos(an),s]];

function cquads(n,o,s=9999999,ex=0) = concat(
    [for (i=[0:n-1-ex]) [(i+1)%n+o,i+o,(i+o+n)%s]],
    [for (i=[0:n-1-ex]) [(i+1)%n+o,(i+o+n)%s,((i+1)%n+o+n)%s]]
);
