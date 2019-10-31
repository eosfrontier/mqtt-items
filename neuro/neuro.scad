
/* Fine
    cang = 1;
    bang = 5;
    dang = 2.5;
    dstp = 5;
    pang = 10;
    pbev = 15;
    coang=82;
// */
 
// /* Coarse
    cang = 5;
    bang = 15;
    dang = 5;
    dstp = 3;
    pang = 15;
    pbev = 30;
    coang=80;
// */

width = 180;
height = 180;
breadth = 100;
thick = 15;

difference() {
    translate([-10,0,0]) quarterpipe_h();
    translate([-10,0,0]) mirror([1,0,0]) {
        cpoint(10, 15);
        cline(10,15,15,20);
        cpoint(15, 20);
        cline(15,20,25,35);
        cpoint(25, 35);
        cline(25,35,35,20);
        cpoint(30, 55);
        cline(30,55,45,55);

        cpoint(55,50);
        cline(55,50,75,65);
        cpoint(75,65);
        cline(75,65,85,55);
        cpoint(5,88);
        cline(5,88,20,75);
        cpoint(20,75);
        cline(20,75,30,80);
        cpoint(30,80);
        cline(30,80,30,55);
        cpoint(40,70);
        cline(40,70,45,55);
        
        cpoint(15,50);
        cline(15,50,25,35);
        
        cpoint(45,55);
        cline(45,55,55,50);
        cline(55,50,40,20);
        cpoint(40,20);
        cline(40,20,35,20);
        cpoint(35,20);
    }
}

*union() {
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
            cline(45,55,55,50);
            cpoint(55,50);
            cline(55,50,75,35);
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
            
            cpoint(15,45);
            cline(15,45,25,35);
            
            cline(55,50,40,20);
            cpoint(40,20);
            cline(40,20,35,25);
            cpoint(35,25);
        }
    }
    translate([5,height,breadth/2]) disc();
}


module cpoint(a, z, t=5, d=1.5, rw=width+thick, rh=height+thick) {
    translate([rw*sin(a),rh*cos(a), z])
    rotate([0,0,-a])
    translate([0,-d,0])
    disc(t, d*2, d*1.5, 0, ca=pang, ba=pbev);
}

module cline(a1, z1, a2, z2, t=1.5, rw=width+thick, rh=height+thick, a=cang) {
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
    bsds = 360/pbev;
    csds = ceil(abst/a)+1;
    // Afstand is een hoek, moet echte afstand worden (ongeveer)
    adis = st*rw*PI/180;
    tnorm = sqrt(zdis*zdis+adis*adis);
    t1 = t*adis / tnorm;
    t2 = t*zdis / tnorm;
    aend = (st>0.1)?(st-0.1):0;
    polyhedron(
        points = [for (ba=[pbev:pbev:360]) each concat(
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

module disc(r=30, t=20, bbv=3, tbv=10, ca=cang, ba=bang) {
    csds = 360/ca;
    bsds = 180/ba+2;
    polyhedron(
        points = concat(
            [for (an=[-90:ba:0]) each circle(
                r-tbv*(1-cos(an)), t-tbv*(1+sin(an)), ca)],
            [for (an=[0:ba:90]) each circle(
                r-bbv*(1-cos(an)), bbv*(1-sin(an)), ca)]
        ),
        faces = concat(
            [for (s=[0:bsds-2]) each cquads(csds, csds*s)],
            [[for (s=[0:csds-1]) s]],
            [[for (s=[csds-1:-1:0]) s+csds*(bsds-1)]]
        ));
}

function circle(r, h, a=cang) = 
    [for (an=[a:a:360]) [r*cos(an),h,r*sin(an)]];

module quarterpipe(san=-90, rw=width, rh=height, s=breadth, t=thick, bv=3, eb=5) {
    csds = 180/cang+2+(180/bang+2); // curve + 2 corners
    bsds = 180/bang+2; // bevel sides
    tsds = csds*(bsds-1); // total (-1 for inside inxdex)
    polyhedron(
        points = concat(
            // Outside
            [for (an=[-90:bang:0]) each 
                qpipe_curve(an, san, rw+t-bv, rh+t-bv, s, bv, eb)],
            // Inside
            [for (an=[0:bang:90]) each
                qpipe_curve(an, san, rw+bv, rh+bv, s, bv, eb)]

        ),
        faces = concat(
            // sides
            [for (s=[0:bsds-2]) each cquads(csds, csds*s, csds*bsds)],
            [for (s=[1:csds/2-1]) each [
                // outside
                [s-1,s,csds-s],[s,csds-s-1,csds-s],
                // inside
                [tsds+s,tsds+s-1,tsds+csds-s],[tsds+s,tsds+csds-s,tsds+csds-s-1]]]
        ));        
}

function qpipe_curve(an, san, rw, rh, s, bv, eb) =
    concat(
        qcircle( // lower curve
            rw-bv*sin(an), rh-bv*sin(an),
            bv*(1-cos(an)), san
        ),
        bcircle( // lower corner
            eb-bv*(1-cos(an)),
            1, rh-bv*sin(an), eb,
            san=-90
        ),
        bcircle( // upper corner
            eb-bv*(1-cos(an)),
            1, rh-bv*sin(an), s-eb,
            san=0
        ),
        qcircle( // upper curve
            rw-bv*sin(an), rh-bv*sin(an),
            s-bv*(1-cos(an)), san, a=-cang
        )
    );

module quarterpipe_h(san=-90, rw=width, rh=height, s=breadth, t=thick, bv=3, eb=5, cs=10, bth=2, ics=30) {
    // just the curve
    jsds = (180/bang+2)+(90/bang+2)+(100/dang+1);
    // curve + 2 corners
    csds = (coang*2)/cang+2+jsds+(180/dang+2)+(180/bang+2)+dstp*4-4+(100/dang+2);
    bsds = 180/bang+6; // bevel sides
    tsds = csds*(bsds-1); // total (-1 for inside inxdex)
    osds = csds*(bsds-4); // Filling back edge
    isds = csds*(bsds-3); // Filling back edge
    cursds = jsds/2+1+dstp*2-2+(50/dang+1);
    offsds = (50/dang)+dstp;
    botsds = 90/bang+90/dang+2;
    ofoff = ((50/dang)+dstp+(45/bang)+(90/bang)+dstp+(50/dang));
    ovoff = ceil(2/cang);
    ios = ics+t;
    polyhedron(
        points = concat(
            // Outside
            [for (an=[-90:bang:0]) each
                qpipe_h_curve(an, san, rw+t-bv, rh+t-bv, s, bv, eb, cs, ics)],
            // Inside
            [for (an=[0:bang:90]) each
                qpipe_h_curve(an, san, rw+bv, rh+bv, s, bv, eb, cs, ics, ios, t)],
            qpipe_h_curve(90, san, rw+bv, rh+bv, s, bv, eb, cs, ics, ios, t, o=10),
            qpipe_h_curve(90, san, rw+bv+bth, rh+bv+bth, s, bv, eb, cs, ics, ios, t, bth, o=10),
            qpipe_h_curve(90, san, rw+bv+bth, rh+bv+bth, s, bv, eb, cs, ics, ios, t, bth),

            qpipe_h_curve(-90, san, rw+t-bv-bv, rh+t-bv-bv, s, bv, eb, cs, ics, bth=bv)

        ),
        faces = concat(
            // sides
            [for (s=[0:bsds-5]) each cquads(csds, csds*s, csds*bsds, ex=1)],
            cquads(csds, csds*(bsds-4), csds*bsds, ex=csds/2+cursds-ovoff),
            cquads(csds, csds*(bsds-4)+csds/2+cursds-1+ovoff, csds*bsds, ex=csds/2+cursds-ovoff+2),
            [[csds*(bsds-4)+csds/2+cursds-1+ovoff, csds*(bsds-4)+csds/2-cursds-ovoff, csds*(bsds-3)+csds/2-cursds-ovoff],
             [csds*(bsds-4)+csds/2+cursds-1+ovoff, csds*(bsds-3)+csds/2-cursds-ovoff, csds*(bsds-3)+csds/2+cursds-1+ovoff]],
            [for (s=[bsds-3:bsds-2]) each cquads(csds, csds*s, csds*bsds, ex=1)],
            [[isds-1,0,csds-1],[isds-1,isds-csds,0],
             [isds-csds-1,isds-1,csds-1],[isds-csds,isds-csds*2,0],
             [isds+csds-1,isds+csds*2-1,isds+csds*3-1],[isds+csds*2,isds+csds,isds],
             [isds,isds+csds-1,isds+csds*2],[isds+csds-1,isds+csds*3-1,isds+csds*2]],
            [for (s=[0:floor((bsds-6)/2)-1]) each [
                [csds*s,osds-(csds*(s+1)),csds*(s+1)],
                [csds*(s+1),osds-(csds*(s+1)),osds-(csds*(s+2))],
                [osds-(csds*(s+1))+csds-1,csds*s+csds-1,csds*(s+1)+csds-1],
                [osds-(csds*(s+1))+csds-1,csds*(s+1)+csds-1,osds-(csds*(s+2))+csds-1]
                ]],
            [for (s=[0:botsds]) each [
                // inside
                [isds+s-1,isds+s,isds+csds-s],[isds+s,isds+csds-s-1,isds+csds-s],
                // outside
                [osds+s,osds+s-1,osds+csds-s],[osds+s,osds+csds-s,osds+csds-s-1]]],
            [[osds+botsds,isds+csds-botsds-1,isds+botsds],
             [osds+botsds,osds+csds-botsds-1,isds+csds-botsds-1]],
            [for (s=[1:csds/2-cursds]) each [
                // outside
                [s-1,s,csds-s],[s,csds-s-1,csds-s],
                // inside
                [tsds+s,tsds+s-1,tsds+csds-s],[tsds+s,tsds+csds-s,tsds+csds-s-1]]],
            ovoff=0?[]:[for (s=[csds/2-cursds-ovoff+1:csds/2-cursds]) each [
                // inside
                [isds+s-1,isds+s,isds+csds-s],[isds+s,isds+csds-s-1,isds+csds-s],
                // outside
                [osds+s,osds+s-1,osds+csds-s],[osds+s,osds+csds-s,osds+csds-s-1]]],
            [[csds/2-ofoff-2,csds/2-ofoff-1,csds/2],
             [csds/2+ofoff+1,csds/2+ofoff+2,csds/2],
             [csds/2+ofoff+2,csds/2-ofoff-2,csds/2],
             [osds+csds/2-ofoff-1,osds+csds/2-ofoff-2,osds+csds/2],
             [osds+csds/2+ofoff+2,osds+csds/2+ofoff+1,osds+csds/2],
             [osds+csds/2-ofoff-2,osds+csds/2+ofoff+2,osds+csds/2],
             [isds+csds/2-ofoff-2,isds+csds/2-ofoff-1,isds+csds/2],
             [isds+csds/2+ofoff+1,isds+csds/2+ofoff+2,isds+csds/2],
             [isds+csds/2+ofoff+2,isds+csds/2-ofoff-2,isds+csds/2],
             [tsds+csds/2-ofoff-1,tsds+csds/2-ofoff-2,tsds+csds/2],
             [tsds+csds/2+ofoff+2,tsds+csds/2+ofoff+1,tsds+csds/2],
             [tsds+csds/2-ofoff-2,tsds+csds/2+ofoff+2,tsds+csds/2]
            ],
            [for (s=[0:offsds]) each [
                [csds/2-s-1,csds/2-s,csds/2-ofoff+s],
                [csds/2-s,csds/2-ofoff+s-1,csds/2-ofoff+s],
                [csds/2+s,csds/2+s+1,csds/2+ofoff-s],
                [csds/2+s,csds/2+ofoff-s,csds/2+ofoff-s+1],

                [osds+csds/2-s,osds+csds/2-s-1,osds+csds/2-ofoff+s],
                [osds+csds/2-s,osds+csds/2-ofoff+s,osds+csds/2-ofoff+s-1],
                [osds+csds/2+s+1,osds+csds/2+s,osds+csds/2+ofoff-s],
                [osds+csds/2+s,osds+csds/2+ofoff-s+1,osds+csds/2+ofoff-s],
            
                [isds+csds/2-s-1,isds+csds/2-s,isds+csds/2-ofoff+s],
                [isds+csds/2-s,isds+csds/2-ofoff+s-1,isds+csds/2-ofoff+s],
                [isds+csds/2+s,isds+csds/2+s+1,isds+csds/2+ofoff-s],
                [isds+csds/2+s,isds+csds/2+ofoff-s,isds+csds/2+ofoff-s+1],

                
                [tsds+csds/2-s,tsds+csds/2-s-1,tsds+csds/2-ofoff+s],
                [tsds+csds/2-s,tsds+csds/2-ofoff+s,tsds+csds/2-ofoff+s-1],
                [tsds+csds/2+s+1,tsds+csds/2+s,tsds+csds/2+ofoff-s],
                [tsds+csds/2+s,tsds+csds/2+ofoff-s+1,tsds+csds/2+ofoff-s]
            ]],
            [[for (s=[csds/2-ofoff+offsds:csds/2-offsds-1]) s]],
            [[for (s=[csds/2+offsds+1:csds/2+ofoff-offsds]) s]],
            [[for (s=[csds/2-offsds-1:-1:csds/2-ofoff+offsds]) osds+s]],
            [[for (s=[csds/2+ofoff-offsds:-1:csds/2+offsds+1]) osds+s]],
            [[for (s=[csds/2-ofoff+offsds:csds/2-offsds-1]) isds+s]],
            [[for (s=[csds/2+offsds+1:csds/2+ofoff-offsds]) isds+s]],
            [[for (s=[csds/2-offsds-1:-1:csds/2-ofoff+offsds]) tsds+s]],
            [[for (s=[csds/2+ofoff-offsds:-1:csds/2+offsds+1]) tsds+s]]
        ));        
}

function qpipe_h_curve(an, san, rw, rh, s, bv, eb, cs, ics, io=0, th=0, bth=0, o=0) =
    concat(
        io ? qcircle( // lower base edge inside
            eb-bth-bv*sin(an+180), eb-bth-bv*sin(an+180),
            bv*(1-cos(an))+o, (rw+ics+th-bth-bv*2)*sin(san), -io, san=90-san, a=-bang
            ) : qcircle( // lower base edge outside
            eb-bv*sin(180-an)-bth, eb-bv*sin(180-an)-bth,
            bv*(1-cos(an))+o, (rw+ics+bth)*sin(san), -ics-eb, san=san, a=bang
        ),
        qcircle( // lower base curve
            (io?eb-bth:ics)-bv*sin(an+180), (io?eb-bth:ics+bth)-bv*sin(an+180),
            bv*(1-cos(an))+o, (rw+(io?eb-bth:ics))*sin(san), -io, san=-san, a=-dang
        ),
        qcircle( // lower curve
            rw-bv*sin(an), rh-bv*sin(an),
            bv*(1-cos(an))+o, san=san, ean=coang
        ),
        bcircle_m( // matching inside curve
            (s-cs*2-eb*2)/2+bv,
            15, rh-bv*sin(an),
            bv*(1-cos(an)),
            san=130, ean=50, a=-dang
        ),
        bline_c( // matching lower straight
            rh-bv*sin(an),
            15 - sin(140)*((s-cs*2-eb*2)/2+bv),
            bv*(1-cos(an)),
            1  + sin( 45)*(eb-bv),
            bv*(1-cos(an)),
            stp = dstp
        ),
        bcircle( // lower corner
            eb-bv*(1-cos(an)),
            1, rh-bv*sin(an), eb,
            san=-90
        ),
        bcircle( // lower inside corner
            eb-bv*(1-cos(an)),
            1, rh-bv*sin(an), cs,
            san=0, ean=45
        ),
        bline_c( // lower straight
            rh-bv*sin(an),
            1  + sin( 45)*(eb-bv*(1-cos(an))),
            cs + cos( 45)*(eb-bv*(1-cos(an))),
            15 - sin(140)*((s-cs*2-eb*2)/2+bv*(1-cos(an))),
            s/2+ cos(140)*((s-cs*2-eb*2)/2+bv*(1-cos(an))),
            stp = dstp
        ),
        bcircle_c( // inside curve
            (s-cs*2-eb*2)/2+bv*(1-cos(an)),
            15, rh-bv*sin(an), s/2,
            san=130, ean=100, a=-dang
        ),
        bline_c( // upper straight
            rh-bv*sin(an),
            15 - sin( 40)*((s-cs*2-eb*2)/2+bv*(1-cos(an))),
            s/2+ cos( 40)*((s-cs*2-eb*2)/2+bv*(1-cos(an))),
            1  + sin(135)*(eb-bv*(1-cos(an))),
            s-cs + cos(135)*(eb-bv*(1-cos(an))),
            stp = dstp
        ),
        bcircle( // upper inside corner
            eb-bv*(1-cos(an)),
            1, rh-bv*sin(an), s-cs,
            san=-45, ean=45
        ),
        bcircle( // upper corner
            eb-bv*(1-cos(an)),
            1, rh-bv*sin(an), s-eb,
            san=0
        ),
        bline_c( // matching upper straight
            rh-bv*sin(an),
            1  + sin( 40)*(eb-bv),
            s-bv*(1-cos(an)),
            15 - sin(135)*((s-cs*2-eb*2)/2+bv),
            s-bv*(1-cos(an)),
            stp = dstp
        ),
        bcircle_m( // matching inside curve
            (s-cs*2-eb*2)/2+bv,
            15, rh-bv*sin(an),
            s-bv*(1-cos(an)),
            san=180, ean=50, a=-dang
        ),
        qcircle( // upper curve
            rw-bv*sin(an), rh-bv*sin(an),
            s-bv*(1-cos(an))-o, san=san, a=-cang, ean=coang
        ),
        qcircle( // upper base curve
            (io?eb-bth:ics)-bv*sin(an+180), (io?eb-bth:ics+bth)-bv*sin(an+180),
            s-bv*(1-cos(an))-o, (rw+(io?eb-bth:ics))*sin(san), -io, san=-san, a=dang
        ),
        io ? qcircle( // upper base edge inside
            eb-bth-bv*sin(an+180), eb-bth-bv*sin(an+180),
            s-bv*(1-cos(an))-o, (rw+ics+th-bth-bv*2)*sin(san), -io, san=90-san, a=bang
            ) : qcircle( // upper base edge outside
            eb-bv*sin(180-an)-bth, eb-bv*sin(180-an)-bth,
            s-bv*(1-cos(an))-o, (rw+ics+bth)*sin(san), -ics-eb, san=san, a=-bang
        )
    );

function bline_c(y, x1,z1,x2,z2,stp=dstp) =
    [for (s=[1:stp-1]) [x1+(x2-x1)*s/stp,
                sqrt(pow(y,2)-pow(x1+(x2-x1)*s/stp,2)),
                z1+(z2-z1)*s/stp]];

function bcircle(r, x, y, z, san=0, ean=90, a=bang) =
    [for (an=[san+(a>0?0:ean):a:san+(a>0?ean:0)])
        [x+r*cos(an),y,z+r*sin(an)]];
    
function bcircle_c(r, x, y, z, san=0, ean=90, a=bang) =
    [for (an=[san+(a>0?0:ean):a:san+(a>0?ean:0)])
        [x+r*cos(an),sqrt(pow(y,2)-pow(x+r*cos(an),2)),z+r*sin(an)]];

function bcircle_m(r, x, y, z, san=0, ean=90, a=bang) =
    [for (an=[san+(a>0?0:ean):a:san+(a>0?ean:0)])
        [x+r*cos(an),sqrt(pow(y,2)-pow(x+r*cos(an),2)),z]];

function qcircle(rw, rh, z, x=0, y=0, san=0, ean=90, a=cang) =
    [for (an=[san+(a>0?0:ean):a:san+(a>0?ean:0)])
        [x+rw*sin(an),y+rh*cos(an),z]];

function cquads(n,o,s=9999999,ex=0) = concat(
    [for (i=[0:n-1-ex]) [(i+1)%n+o,i+o,(i+o+n)%s]],
    [for (i=[0:n-1-ex]) [(i+1)%n+o,(i+o+n)%s,((i+1)%n+o+n)%s]]
);
