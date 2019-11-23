/* Fine
    cang = 1;
    bang = 5;
    dang = 2.5;
    dstp = 5;
    pang = 10;
    pbev = 15;
    centersteps = 60;
    coang=82;
// */
 
///* Coarse
    cang = 5;
    bang = 15;
    dang = 5;
    dstp = 3;
    pang = 15;
    pbev = 30;
    centersteps = 20;
    coang=80;
// */

width = 170;
height = 170;
breadth = 100;
thick = 15;
wall = 2;
exof = 10;
edgebev = 3;

indof = 20;
cutof = 14;

butwid = 50;
buthi = 50;
knobr = 25;
butsz = 140;


basehi = 9.5;

tabhi = 18;
mtabhi = 13;
stabhi = 7.5;

boxheight = 51;
ribheight = 30;
boxang = 30;
boxsl = tan(boxang)*sin(45);

centerwidth = (width+thick+exof-butsz-0.1)*2;
centerthick = 32;

buttonsp = 75;

complete = false;

if (complete) {

    rotate([90,0,0]) front_l();
    rotate([90,0,0]) front_r();

    buttons_r();
    mirror([1,0,0]) buttons_r();

    hinge_r();
    mirror([1,0,0]) hinge_r();

    center_p();

    *color("teal") translate([0,125,27]) rotate([-90,90,0]) batteryholder();
    *color("teal") translate([118,68,0]) rotate([180,0,45]) batteryholder();
    *color("teal") translate([2.5,113.6,0]) rotate([180,0,90]) batteryholder_1();

    translate([0, 0, -0.1]) center_bottom();

    color("teal") translate([0,butsz-buthi+5.5+wall,50.0]) rotate([-90,0,180]) batteryholder_1();

    *color("teal") translate([130,75.1,11]) rotate([180+boxang,0,135]) atmega();
} else {

    *buttons_r();

    // To print
    *rotate([90,-45,0]) sidebox();
    *rbutton();
    *rotate([90,0,0]) hinge_r();
    *front_r();
    *front_l();

    *center_p();
    *translate([0,0,-50.1]) center_bottom();

    *translate([0,0.1,0]) sidebox();
    *translate([0,0.1,-50.1]) sidebox_bottom();

    hinge_r();
    translate([0,0, -0.1]) hinge_bottom();
}

// Bottom covers

module hinge_bottom(w = breadth, hsw = 20, t=wall) {
    csds = (45/dang+135/bang+5);
    bsds = 6;
    tsds = csds*bsds;
    difference() {
        polyhedron(
            points = concat(
                hinge_c_curve1(0, -t, t=0),
                hinge_c_curve1(0, 0, t=0),
                hinge_c_curve1(0, 0, t=t+0.1),
                hinge_c_curve1(0, 2, t=t+0.1),
                hinge_c_curve1(0, 2, t=t*2),
                hinge_c_curve1(0, 0, t=t*2)
            ), faces = concat(
                [for (s=[0:bsds-2]) each cquads(csds, csds*s, csds*bsds)],
                [[for (s=[0:csds-1]) s],
                 [for (s=[tsds-1:-1:tsds-csds]) s]]
            ));
        b_hingeholes(o=-1);
    }
    x1=width+thick-5.5;
    x2=width+thick+hsw+7.4;
    translate([x1, -17, 0]) rotate([0,0,90]) box_tab(w=16, h=stabhi+0.1);
    translate([x1, -51, 0]) rotate([0,0,90]) box_tab(w=16, h=stabhi+0.1);
    translate([x1, -85, 0]) rotate([0,0,90]) box_tab(w=16, h=stabhi+0.1);

    translate([x2, -17, 0]) rotate([0,0,-90]) box_tab(w=16, h=stabhi+0.1);
    translate([x2, -51, 0]) rotate([0,0,-90]) box_tab(w=16, h=stabhi+0.1);

    translate([x2-12, -90, 0]) rotate([0,0,-140]) box_tab(w=16, h=stabhi+0.1);
}

module center_bottom(w = centerwidth, b = centerthick, t = wall, tol=0.1) {
    csds = 4;
    bsds = 6;
    tsds = csds*bsds;
    y = butsz-buthi+5.1;
    difference() {
        polyhedron(
        points = concat(
            cb_curve(w-tol*2, b, -t, y),
            cb_curve(w-tol*2, b, 0, y),
            cb_curve(w-t*2-tol*2, b-t*2-tol*2, 0, y+t+tol),
            cb_curve(w-t*2-tol*2, b-t*2-tol*2, 5, y+t+tol),
            cb_curve(w-t*4-tol*4, b-t*4-tol*4, 5, y+t*2+tol*2),
            cb_curve(w-t*4-tol*4, b-t*4-tol*4, 0, y+t*2+tol*2)),
        faces = concat(
            [for (s=[0:bsds-2]) each cquads(csds, csds*s, tsds)],
            [[for (s=[0:csds-1]) s],
             [for (s=[tsds-1:-1:tsds-csds]) s]]
             ));

        translate([-w/2+18, y+b-t-t/2, 4.5]) cube([14, t+1, 3], true);
        translate([-w/2+18, y+b-t-t/2, 3]) cube([8, t+1, 6], true);

        b_centerholes();
        b_centerholes(f=-1);
    }
    translate([-w/2+18, y+b-t-8.5, 0]) {
        translate([0,0,1.45]) cube([12, 4, 3.1], true);
        translate([-5,3,1.45]) cube([2, 8, 3.1], true);
        translate([ 5,3,1.45]) cube([2, 8, 3.1], true);
        translate([ 4.2,0,3]) cylinder(2, 1.4, 1.4, $fn=360/pang);
        translate([-4.2,0,3]) cylinder(2, 1.4, 1.4, $fn=360/pang);
        translate([ 4.2,0,5]) cylinder(0.5, 1.4, 0.9, $fn=360/pang);
        translate([-4.2,0,5]) cylinder(0.5, 1.4, 0.9, $fn=360/pang);

        translate([-7, 0, 0]) musb_tab();
        translate([ 7, 0, 0]) mirror([1,0,0]) musb_tab();
    }

    translate([0, y+b-t-0.6, 0]) box_tab(w=40);
    translate([-w/2+20, y+t+0.6, 0]) mirror([0,1,0]) box_tab();
    translate([ w/2-20, y+t+0.6, 0]) mirror([0,1,0]) box_tab();
}

module b_centerholes(o=-1, f=1, t=wall, r=10/2) {
    y = butsz-buthi+5.1 + centerthick/2;
    x = f*(centerwidth/2+o*(t+3-0.5));
    translate([x, y, 25]) {
        translate([0,-10,-18]) rotate([0,90,0]) cylinder(7, r, r, true, $fn=360/pang);
        translate([0, 10,-18]) rotate([0,90,0]) cylinder(7, r, r, true, $fn=360/pang);
    }
}

module b_hingeholes(o=1, t=wall, r=10/2) {
    translate([width+thick/2+ 7, o*(t+3), 6]) rotate([90,0,0]) cylinder(7, r, r, true, $fn=360/pang);
    translate([width+thick/2+32, o*(t+3), 6]) rotate([90,0,0]) cylinder(7, r, r, true, $fn=360/pang);
}

module musb_tab(h=4.6, w=8) {
    rotate([0,-90,-90]) translate([0,0,-w/2]) linear_extrude(height=w) polygon([
        [-1,0],[h,0],[h+0.4,0.4],[h+2,-0.4],[h+1, -1.4],[-1,-1.4]
    ]);
}

module box_tab(h=tabhi+0.1, w=20) {
    rotate([0,-90,0]) translate([0,0,-w/2]) linear_extrude(height=w) polygon([
        [-1,0],[h,0],[h+0.6,0.6],[h+2,-0.5],[h+1, -1.5],[-1,-1.5]
    ]);
}

function cb_curve(w, b, h, y) = [[-w/2,y,h],[w/2,y,h],[w/2,y+b,h],[-w/2,y+b,h]];


module sidebox_bottom(t = wall) {
    csds = (225/bang+5)+(45/dang+1);
    bsds = 6;
    tsds = bsds*csds;
    tw = width+thick+exof;
    supr = 64/sqrt(2);
    difference() {
        union() {
            polyhedron(points = concat(
                sbb_curve(-t, 0),
                sbb_curve(0, 0),
                sbb_curve(0, t+0.1),
                sbb_curve(5, t+0.1),
                sbb_curve(5, t*2),
                sbb_curve(0, t*2)
                ), faces = concat(
                    [for (s=[0:bsds-2]) each cquads(csds, csds*s, tsds)],
                    [[for (s=[0:csds-1]) s],
                     [for (s=[tsds-1:-1:tsds-csds]) s]]
                ));

            translate([tw-butsz/2-21.65+30, butsz/2-21.65-30, 0]) rotate([0,0,135]) box_tab(w=40);
            translate([tw-butsz/2-21.65-30, butsz/2-21.65+30, 0]) rotate([0,0,135]) box_tab(w=40);
            translate([tw-butsz/2+21.65, butsz/2+21.65, 0]) rotate([0,0,-45]) box_tab(h=mtabhi+0.1, w=127);
        }
        b_centerholes(o=1);
        b_hingeholes();

        // Rib cutouts
        translate([tw-butsz/2, butsz/2, 0]) rotate([0,0,45]) sb_support(th=1.2);
        translate([tw-butsz/2+supr, butsz/2-supr, 0]) rotate([0,0,45]) sb_support(th=1.2);
        translate([tw-butsz/2-supr, butsz/2+supr, 0]) rotate([0,0,45]) sb_support(th=1.2);
        for (s=[-supr:supr/4:supr]) {
            translate([tw-butsz/2+s, butsz/2-s, 0]) rotate([0,0,45]) sb_support_2(th=0.6);
        }
        translate([tw-butsz/2+supr*1.25, butsz/2-supr*1.25, 0]) rotate([0,0,45]) sb_support_2(12, th=0.6);
    }
}

function sbb_curve(z, t, eb=5, hb=20, w=width+thick+exof, bw=butwid, bh=buthi, bs=butsz, hsw=20) = concat(
// I don't know where 1.213 comes from; trial and error
    [[w-t+hsw, t, z]],
    bcircle_b(hb-t, w-hb+hsw, hsw-1.213, z, 90, 135, a=dang),
    bcircle_b(eb-t, w-bs+bh-eb, bs-eb, z, 135, 180),
    bcircle_b(eb-t, w-bs+eb, bs-eb, z, 180, 270),
    bcircle_b(eb-t, w-bs+eb, bs-bw+eb, z, 270, 315),
    bcircle_b(eb-t, w-bw+eb, eb, z, 315, 360)
    );

function bcircle_b(r, x, y, z, san, ean, a=bang) = 
    [for (an=[san:(san<ean?a:-a):ean])
        [x+r*sin(an),y-r*cos(an),z]];

// The modules

module center_p(w = centerwidth, b = centerthick, h = 40, eh=40, t=wall, st=centersteps) {
    csds = 2*st+2;
    bsds = (180/bang+7);
    tsds = csds*bsds;
    y = butsz-buthi+5.1;
    difference() {
        union() {
            polyhedron(points = concat( 
                [for (an=[-90:bang:0]) each center_curve(an, y, w, b, h, eh+30, st)],
                center_curve(0, y, w, b, 0, 0, st),
                center_curve(0, y+t, w-t*2, b-t*2, 0, 0, st, bv=edgebev-t),
                center_curve(0, y+t, w-t*2, b-t*2, tabhi-1.2, 0, st, bv=edgebev-t),
                center_curve(0, y+t+0.6, w-t*2, b-t*2-1.2, tabhi, 0, st, bv=edgebev-t-0.6),
                center_curve(0, y+t, w-t*2, b-t*2, tabhi+0.6, 0, st, bv=edgebev-t),
                [for (an=[0:-bang:-90]) each center_curve(an, y+t, w-t*2, b-t*2, h, eh+30, st, bv=edgebev-t)]
                ), faces = concat(
                [for (s=[0:bsds-2]) each cquads(csds, csds*s, tsds)],
                [for (s=[0:csds/2-1]) each [[s, s+1, csds-s-1],[s, csds-s-1, csds-s]]],
                [for (s=[0:csds/2-1]) each [[tsds-s-1, tsds-s-2, tsds-csds+s+1],[tsds-s-1, tsds-csds+s+1, tsds-csds+s]]]
                ));
            translate([0, y, h+eh+10]) disc(r=40, t=b+5);
        }
        translate([0, y+t, h+eh+10]) disc(r=38, t=b+5-t*2);
        translate([0, y+b/2, h+eh-10]) cube([51.6, b-t*2, 40], true);

        centerholes(f =  1);
        centerholes(f = -1);

        translate([-w/2+14, y+b-t/2, 25]) switchhole();
        #translate([-w/2+18, y+b-t/2, 1.9]) cube([9, t+1, 4.2], true);
    }
    translate([0, y+t, 0]) battery_slot();
    translate([0, y+t, 0]) mirror([1,0,0]) battery_slot();
}

module battery_slot(l=100, l2=15, w=29.2, h=4.5, t=1, s=2.5, e=0.5) {
    translate([w/2,0,l2+h+t]) rotate([0,90,0]) linear_extrude(height=s) polygon([
        [0,0],[0,h+t],[e,h+t],[e+h+t,0]
    ]);
    translate([w/2,h,l2+h+t]) rotate([-90,0,0]) linear_extrude(height=t) polygon([
        [-e,0],[0,0],[0,e]
    ]);
    translate([w/2,0,l2+h+t]) linear_extrude(height=l-l2-h-t) polygon([
        [0, -e],[0, h],[-e, h],[-e, h+t],[s,h+t],[s,-e]
    ]);
}

module switchhole(t=wall) {
    rotate([90,0,0]) cylinder(t+1, 10, 10, true, $fn=360/dang);
    translate([-10,0,0]) cube([1, t+1, 2], true);
    translate([0,0, 10]) cube([4, t+1, 1], true);
    translate([0,0,-10]) cube([4, t+1, 1], true);
}

function center_curve(an, y, w, b, h, eh, st, bv=edgebev) = concat(
    [for (x=[ 1:-2/st:-1]) [ x*w/2, y  +bv-bv*cos(an), h-bv*sin(an) + eh*((1-abs(x))*(1-abs(x))) ]],
    [for (x=[-1: 2/st: 1]) [ x*w/2, y+b-bv+bv*cos(an), h-bv*sin(an) + eh*((1-abs(x))*(1-abs(x))) ]]
);

/*
function center_curve(an, y, w, b, h, eb=5, bv=edgebev) = concat(
    qcircle(
        eb-bv+bv*cos(an), eb-bv+bv*cos(an),
        h-bv-bv*sin(an),  w/2-eb, y+eb, san=90, a=bang),
    qcircle(
        eb-bv+bv*cos(an), eb-bv+bv*cos(an),
        h-bv-bv*sin(an), -w/2+eb, y+eb, san=180, a=bang),
    qcircle(
        eb-bv+bv*cos(an), eb-bv+bv*cos(an),
        h-bv-bv*sin(an), -w/2+eb, y+b-eb, san=270, a=bang),
    qcircle(
        eb-bv+bv*cos(an), eb-bv+bv*cos(an),
        h-bv-bv*sin(an),  w/2-eb, y+b-eb, san=0, a=bang)
);
*/

module hinge_r(w = breadth, bv=edgebev) {
    csds = 360/bang+5;
    bsds = 90/bang+2;
    bsh = bsds/2-1;
    tsds = csds*bsds;

    bsds2 = 180/bang+2;
    tsds2 = csds*bsds2;

    exsds = (90/cang-1);

    translate([exof,0,basehi]) difference() {
        union() {
            polyhedron(
            points = concat(
                hinge_curve(0,0,0,bv),
                [for (an=[0:bang:90]) each
                    hinge_curve(an,-w,1, bv)]
                    ),
            faces = concat(
                [for (s=[0:bsds-2]) each cquads(csds, csds*s, tsds)],
                [[for (s=[0:csds-1]) s],
                 [for (s=[tsds-1:-1:tsds-csds]) s]]
             ));
            polyhedron(
                points = concat(
                    [for (an=[-90:bang:0]) each
                        hinge_curve(an,-w+indof-0.1,-1, bv, sho=cutof+2.5, eb=5, oc=8)],
                    [for (an=[0:bang:90]) each
                        hinge_curve(an,-w,1, bv, sho=cutof+2.5, eb=5, oc=8)]
                        ),
                faces = concat(
                    [for (s=[0:bsds2-2]) each cquads(csds, csds*s, tsds2)],
                    [[for (s=[0:csds-1]) s],
                     [for (s=[tsds2-1:-1:tsds2-csds]) s]]
             ));
            translate([width+thick/2,-w+indof,10.5]) rotate([90,0,0]) cylinder(7.5, 4.4, 4.4, true, $fn=360/dang);
            translate([width+thick/2,-w+indof+4.25,10.5]) rotate([90,0,0]) cylinder(1, 3.4, 4.4, true, $fn=360/dang);
            hinge_side1();
        }
        translate([-exof,0,-basehi]) hinge_cutout();
        translate([-exof, 0, -basehi]) hingeholes();
    }
}

module hinge_side1(hsw = 20, w = breadth, x = width+thick, bv=edgebev, tol=0.1) {
    csds = (45/bang+45/dang+6);
    bsds = (90/bang+2);
    tsds = bsds*csds;
    polyhedron(
        points = concat(
            hinge_s_curve1(0, x, hsw, w, -basehi, bv),
            [for (an=[0:bang:90]) each hinge_s_curve1(an, x, hsw, w, -tol, bv)]
            ),
        faces = concat(
            [for (s=[0:bsds-2]) each cquads(csds, csds*s)],
            [[for (s=[0:csds-1]) s],
             [for (s=[tsds-1:-1:tsds-csds]) s]]
        )
    );
}

module hinge_side2(hsw = 20, w = butwid, x = width+thick, bv=edgebev, eb=5, tol=0.1) {
    csds = (45/dang+4);
    bsds = (90/bang+2);
    tsds = bsds*csds;
    polyhedron(
        points = concat(
            hinge_s_curve2(0, x, hsw, w, -basehi, bv, eb),
            [for (an=[0:bang:90]) each hinge_s_curve2(an, x, hsw, w, -tol, bv, eb)]
            ),
        faces = concat(
            [for (s=[0:bsds-2]) each cquads(csds, csds*s)],
            [[for (s=[0:csds-1]) s],
             [for (s=[tsds-1:-1:tsds-csds]) s]]
        )
    );
}

function hinge_s_curve1(an, x, hsw, w, z, bv, hb=20) = concat(
    qcircle(bv*cos(an), bv*cos(an),
        z+bv*sin(an), x-bv, -w+bv, san=135, ean=45, a=-bang),
    qcircle(hb-bv+bv*cos(an), hb-bv+bv*cos(an),
        z+bv*sin(an), x+hsw-hb, -w+hsw+hb*0.4, san=90, ean=45, a=-dang),
    [[x+hsw-bv+bv*cos(an), 0, z+bv*sin(an)],
     [x-bv-12, 0, z+bv*sin(an)],
     [x-bv-12, -10, z+bv*sin(an)],
     [x-bv, -10, z+bv*sin(an)] ]
);
    
function hinge_s_curve2(an, x, hsw, w, z, bv, eb, hb=20) = concat(
    [[x+2-eb, w-2-eb+(eb-bv+bv*cos(an))*sqrt(2), z+bv*sin(an)],
     [x+2-eb, 0, z+bv*sin(an)],
     [x+hsw-bv+bv*cos(an), 0, z+bv*sin(an)]],
    qcircle(hb-bv+bv*cos(an), hb-bv+bv*cos(an),
        z+bv*sin(an), x+hsw-hb, w-hsw-eb+(eb-hb)*(sqrt(2)-1), san=45, ean=45, a=-dang)
);

function hinge_curve(an, o, bs, bv, sho=0, oc=3, tc=1.5, bh=basehi, bw=thick, th=32.5, eb=10.5, rw=width, tol=0.1) = concat(
    [[rw+bw-bv*(1-cos(an)),o+bs*bv-bv*sin(an),-bh]],
    bcircle(
        oc-bv+bv*cos(an),
        rw+bw-oc, o+bs*bv-bv*sin(an), sho-tol+bv-oc, a=bang),
    bcircle_e(
        eb-bv+tc-tc*cos(an)+tol, eb-bv*cos(an)+tol,
        rw+eb-bv, o+bs*bv-bv*sin(an), sho+eb, san=-180,a=-bang),
    bcircle(
        tc*cos(an)+0.05,
        rw-tc-tol-0.05, o+bs*bv-bv*sin(an), th, ean=180, a=bang),
    [[rw-tc-0.05-tc*cos(an)-tol,o+bs*bv-bv*sin(an),-bh]]
);

module hinge_cutout() {
    csds = (45/dang+135/bang+5);
    bsds = (90/bang+5);
    tsds = csds*bsds;
    polyhedron(
        points = concat(
            hinge_c_curve1(0, -0.1),
            hinge_c_curve1(0, stabhi-1),
            hinge_c_curve1(0, stabhi, t=wall+0.5, t2=0.5),
            hinge_c_curve1(0, stabhi+0.5),
            [for (an=[0:bang:90]) each hinge_c_curve1(an, 9.5)]
        ), faces = concat(
            [for (s=[0:bsds-2]) each cquads(csds, csds*s, csds*bsds)],
            [[for (s=[0:csds-1]) s],
             [for (s=[tsds-1:-1:tsds-csds]) s]]
        ));
}

module sidebox_cutout() {
    csds = (45/dang+4);
    bsds = (90/bang+2);
    tsds = csds*bsds;
    polyhedron(
        points = concat(
            hinge_c_curve2(0, -0.1),
            [for (an=[0:bang:90]) each hinge_c_curve2(an, 9.5)]
        ), faces = concat(
            [for (s=[0:bsds-2]) each cquads(csds, csds*s, csds*bsds)],
            [[for (s=[0:csds-1]) s],
             [for (s=[tsds-1:-1:tsds-csds]) s]]
        ));
}

function hinge_c_curve1(an, z, hsw=20, hb=20, w=breadth, x=width+thick, t=wall, bv=edgebev-wall, tc=1.5, t2=0) = concat(
    [ [x+hsw+9-t+bv*cos(an), -t+t2, z+bv*sin(an)], [x-8+t, -t+t2, z+bv*sin(an)]],
    qcircle(tc, bv+wall,
        z+bv*sin(an), x-8+wall-0.65+t, -w+bv+wall+t, san=180, ean=90, a=-bang),
    qcircle(wall+bv*cos(an), wall+bv*cos(an),
        z+bv*sin(an), x+hsw-hb+9-wall-t/2, -w+bv+wall+t, san=135, ean=45, a=-bang),
    qcircle(hb-bv+bv*cos(an)-t, hb-bv+bv*cos(an)-t,
        z+bv*sin(an), x+hsw-hb+10, -w+hsw+8, san=90, ean=45, a=-dang)
);

function hinge_c_curve2(an, z, hsw=20, hb=20, w=buthi, x=width+thick, t=wall, bv=edgebev-wall) = concat(
    [[x+2.8, w+bv*cos(an)*sqrt(2), z+bv*sin(an)], [x+2.8, t, z+bv*sin(an)],
     [x+hsw+9-t+bv*cos(an), t, z+bv*sin(an)]],
    qcircle(hb-bv+bv*cos(an), hb-bv+bv*cos(an),
        z+bv*sin(an), x+hsw-hb+10-t, w-hsw-12, san=45, ean=45, a=-dang)
);

module buttons_r() {
    translate([0,0.1,0]) {
        sidebox();
        boxbutton(-buttonsp/2);
        boxbutton( buttonsp/2);
        boxbutton_hw(-buttonsp/2);
        boxbutton_hw( buttonsp/2);
        translate([0,0,-0.1]) sidebox_bottom();
    }
}

module boxbutton(off=0) {
    tw = width+thick+exof;
    z = boxheight+tw*boxsl;
    ht = wall * sqrt(2);
    btr = off/sqrt(2);
    translate([tw-butsz/2+btr, butsz/2-btr, boxheight-ht-0.5])
    rotate([0,boxang,45]) rbutton();
}

module boxbutton_hw(off=0) {
    tw = width+thick+exof;
    z = boxheight+tw*boxsl;
    ht = wall * sqrt(2);
    btr = off/sqrt(2);
    color("teal")
    translate([tw-butsz/2+btr, butsz/2-btr, boxheight-ht-0.5])
    rotate([0,boxang,45]) rbutton_hw();
}

module rbutton(r=knobr-0.5, ca=dang, t=15, b=10, ba=bang, tbv=5, bbv=5) {
    csds = 360/ca;
    bsds = 180/ba+9;
    rotate([90,0,0]) translate([0,-t+7,0]) difference() {
        polyhedron(
            points = concat(
                [for (an=[-90:ba:0]) each circle(
                    r-tbv*(1-cos(an)), t-tbv*(1+sin(an)), ca)],
                [for (an=[0:ba:45]) each circle(
                    r-bbv*(1-cos(an)), bbv*(1-sin(an)), ca)],
                [for (an=[45:ba:90]) each circle(
                    r-b-bbv*(1-cos(an)), -b+bbv*(1-sin(an)), ca)],
                circle(2, -b, ca),
                circle(2, 0, ca),
                circle(r-5, 0, ca),
                circle(r-3, 2, ca),
                circle(r-3, t-4, ca),
                circle(r-5, t-2, ca)
            ),
            faces = concat(
                [for (s=[0:bsds-2]) each cquads(csds, csds*s)],
                [[for (s=[0:csds-1]) s],
                 [for (s=[csds-1:-1:0]) s+csds*(bsds-1)]]
            ));
        translate([0,-b-1,0]) rotate([-90,0,0]) rotspline();
        translate([12,t-1,0]) rotate([-90,0,0]) cylinder(1.5, 5,7, $fn=360/dang);     
    }
}

module rbutton_hw() {
    translate([0,0,-26]) cylinder(33.01, 3, 3);
    translate([0,0,-26]) cylinder(6.5, 4.4, 4.4);
    translate([0,0,-26-7.3/2]) cube([14, 15, 7.3], true);
}

module rotspline() {
    outr = 6.3/2;
    inr = 5.3/2;
    h1 = 6;
    h2 = 6.5;
    h3 = 10;
    csds = 18*2;
    bsds = 5;
    polyhedron(
        points = concat(
            rstar(1, 1, h3+2),
            rstar(outr, inr, h3),
            rstar(outr, inr, h3-h1),
            rstar(outr, outr, h3-h2),
            rstar(outr, outr, 0)
        ), faces = concat(
            [for (s=[0:bsds-2]) each cquads_r(csds, csds*s, csds*bsds)],
            [[for (s=[0:csds-1]) s],
             [for (s=[0:csds-1]) csds*bsds-s-1]]
        ));
}

function rstar(r1, r2, z, n=18) =
    [for (an = [360/n:360/n:360]) each
        [[r1*sin(an-180/n),r1*cos(an-180/n),z],
         [r2*sin(an),r2*cos(an),z]]];
    
function cquads_r(n,o,s=9999999,ex=0) = concat(
    [for (i=[0:2:n-1-ex]) [(i+1)%n+o,i+o,(i+o+n)%s]],
    [for (i=[0:2:n-1-ex]) [(i+1)%n+o,(i+o+n)%s,((i+1)%n+o+n)%s]],
    [for (i=[1:2:n-1-ex]) [(i+1)%n+o,i+o,((i+1)%n+o+n)%s]],
    [for (i=[1:2:n-1-ex]) [i+o,(i+o+n)%s,((i+1)%n+o+n)%s]]
);

module sidebox() {
    tw = width+thick+exof;
    z = boxheight+tw*boxsl;
    t = wall;
    ht = wall * sqrt(2);
    csds = 360/bang+6;
    bsds = 90/bang+7;
    eb = 5;
    bv = 3;
    btr = (buttonsp/2)/sqrt(2);
    supr = 64/sqrt(2);
    difference() {
        union() {
            polyhedron(
            points = concat(
                sb_curve(0, z-ht, boxsl, eb, 0, tw, t),
                sb_curve(0, tabhi+0.5, 0, eb, 0, tw, t),
                sb_curve(0, tabhi, 0, eb, 0, tw, t+0.5),
                sb_curve(0, tabhi-1, 0, eb, 0, tw, t),
                sb_curve(0, 0, 0, eb, 0, tw, t),
                sb_curve(0, 0, 0, eb, 0, tw),
                [for (an=[0:bang:90]) each sb_curve(an, z, boxsl, eb, bv, tw)]
            ), faces = concat(
                [for (s=[0:bsds-2]) each cquads(csds,csds*s,csds*bsds)],
                [[for (s=[0:csds-1]) s],
                 [for (s=[csds-1:-1:0]) s+(csds*(bsds-1))]]
            ));
            translate([width+thick/2+exof,-1,20]) rotate([90,0,0]) cylinder(6, 4.9, 4.9, true, $fn=360/dang);
            translate([width+thick/2+exof,-4,20]) rotate([90,0,0]) cylinder(0.5, 4.9, 4.4, $fn=360/dang);
            translate([exof,0,basehi]) hinge_side2();
        }
        translate([tw-butsz/2+btr, butsz/2-btr, boxheight-ht-0.5])
        rotate([0,boxang,45]) translate([0,0,0.83]) cylinder(wall+1, knobr, knobr, $fn=360/dang);
        translate([tw-butsz/2-btr, butsz/2+btr, boxheight-ht-0.5])
        rotate([0,boxang,45]) translate([0,0,0.83]) cylinder(wall+1, knobr, knobr, $fn=360/dang);
        translate([width+thick/2+exof, 0,20]) rotate([90,0,0]) cylinder(11.5, 3.9, 3.9, true, $fn=360/dang);
        translate([width+thick/2+exof,-4,20]) rotate([90,0,0]) cylinder(0.6, 3.6, 4.2, $fn=360/dang);
        sidebox_cutout();

        centerholes(t+1, +1);
        hingeholes(1);
    }
    translate([tw-butsz/2, butsz/2, boxheight-ht-0.5]) rotate([0,boxang, 45]) difference() {
        translate([-2.02, 0, -24.2]) cube([42.04, 130, 3], true);
        translate([0,  buttonsp/2, -23.8]) cylinder(3, 4.5, 4.5, true, $fn=360/pang);
        translate([0, -buttonsp/2, -23.8]) cylinder(3, 4.5, 4.5, true, $fn=360/pang);
    }
    translate([tw-butsz/2, butsz/2, boxheight-33.585]) rotate([0,0,45]) {
        translate([17.8, 0, 0]) cube([28.4, 130, 3], true);
    }
    translate([tw-butsz/2, butsz/2, 0]) rotate([0,0,45]) sb_support();
    translate([tw-butsz/2+supr, butsz/2-supr, 0]) rotate([0,0,45]) sb_support();
    translate([tw-butsz/2-supr, butsz/2+supr, 0]) rotate([0,0,45]) sb_support();
    for (s=[-supr:supr/4:supr]) {
        translate([tw-butsz/2+s, butsz/2-s, 0]) rotate([0,0,45]) sb_support_2();
    }
    translate([tw-butsz/2+supr*1.25, butsz/2-supr*1.25, 0]) rotate([0,0,45]) sb_support_2(12);

    translate([tw-butsz/2+22.1, butsz/2+22.1, mtabhi]) rotate([0,0,45]) sb_tablip();
}

module sb_tablip(w=butsz-11) {
    rotate([90,0,0]) translate([0,0,-w/2]) linear_extrude(height=w) polygon([
        [-0.6, 0],[0.1, 0.7],[0.1,-2.9],[-0.6,-0.8]
    ]);
}

module sb_support(th=1) {
    pw = (buthi+butwid-11.5)/sqrt(2);
    ph = ribheight;
    phs = tan(boxang)*pw;
    translate([0, 0, boxheight-ph-21])
    polyhedron(
        points= [[pw/2,-th, 0],[pw/2,-th, ph],[-pw/2,-th, ph+phs],[-pw/2,-th, phs],
                 [pw/2, th, 0],[pw/2, th, ph],[-pw/2, th, ph+phs],[-pw/2, th, phs]],
        faces = [[3,2,1,0],[0,1,5,4],[1,2,6,5],[2,3,7,6],[3,0,4,7],[6,7,4,5]]);
}

module sb_support_2(ph = boxheight-32, th=0.4) {
    pw1 = (buthi+butwid-11.5)/sqrt(2)/2;
    pw = ph / tan(50);
    pw2 = pw1 - pw;
    polyhedron(
        points= [[pw1,-th, 0],[pw1,-th, ph],[pw2,-th, ph],
                 [pw1, th, 0],[pw1, th, ph],[pw2, th, ph]],
        faces = [[2,1,0],[0,1,4,3],[1,2,5,4],[2,0,3,5],[3,4,5]]);
}

function sb_curve(an, z, zsl, eb, bv, w, t=0, bw=butwid, bh=buthi, bs=butsz) = concat(
    bcircle_s(
        3-bv*(1-cos(an))-t, w-3, 3, z-bv*(1-sin(an)), zsl, 0, 90),
    bcircle_s(
        eb-bv*(1-cos(an))-t, w-eb, bh-eb, z-bv*(1-sin(an)), zsl, 90, 135),
    bcircle_s(
        eb-bv*(1-cos(an))-t, w-bs+bh-eb, bs-eb, z-bv*(1-sin(an)), zsl, 135, 180),
    bcircle_s(
        eb-bv*(1-cos(an))-t, w-bs+eb, bs-eb, z-bv*(1-sin(an)), zsl, 180, 270),
    bcircle_s(
        eb-bv*(1-cos(an))-t, w-bs+eb, bs-bw+eb, z-bv*(1-sin(an)), zsl, 270, 315),
    bcircle_s(
        eb-bv*(1-cos(an))-t, w-bw+eb, eb, z-bv*(1-sin(an)), zsl, 315, 360)
    );

function bcircle_s(r, x, y, z, zsl, san, ean, a=bang) = 
    [for (an=[san:(san<ean?a:-a):ean])
        [x+r*sin(an),y-r*cos(an),(z-(x+y+r*(sin(an)-cos(an)))*zsl)]];

module front_l() {
    translate([-exof,50,0]) difference() {
        union() {
            quarterpipe_h();
            for (an=[12.5:5:87.5]) rib(an);
            translate([-width-thick/2,-30,0]) rotate([0,0,-90]) cylinder(edgebev, 5.5, 5.5, $fn=360/dang);
        }
        translate([-width-thick/2,-30,breadth-indof-edgebev+0.2]) rotate([0,0,90]) cylinder(edgebev, 4.5, 4.5, $fn=360/dang);
        translate([-width-thick/2,-30,-0.5]) rotate([0,0,-90]) cylinder(edgebev+1, 5.0, 5.0, $fn=360/dang);
        mirror([1,0,0]) {
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
            cline(75,65,90,55);
            cpoint(90,55);
            
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
}

module front_r() {
    translate([exof,50,0]) difference() {
        union() {
            mirror([1,0,0]) quarterpipe();
            mirror([1,0,0]) rib(2.5);
            mirror([1,0,0]) for (an=[2.5:5:87.5]) rib(an);
            translate([-5,height,breadth/2]) topdisc();
            translate([width+thick/2,-30,0]) rotate([0,0,-90]) cylinder(edgebev, 5.5, 5.5, $fn=360/dang);
        }
        translate([width+thick/2,-30,breadth-indof-edgebev+0.2]) rotate([0,0,90]) cylinder(edgebev, 4.5, 4.5, $fn=360/dang);
        translate([width+thick/2,-30,-0.5]) rotate([0,0,-90]) cylinder(edgebev+1, 5.0, 5.0, $fn=360/dang);
        translate([-5,height+wall,breadth/2]) disc(r=28, t=16);
        union() {
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
            cpoint(85,55);
            
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
}

module topdisc(r=30, t=20) {
    h = thick - 3;
    difference() {
        disc(r, t);
        #translate([r/2+2,h/2-0.1,0]) cube([r,h+0.1,2*r], true);
    }
}

module rib(an, th=thick-2, rh=height+0.1, w=wall, z=breadth, zo=3, o=10) {
    rotate([0,0,an]) translate([-w/2, rh+th, z-zo])
    rotate([0,90,0]) linear_extrude(height=w)
        polygon([
            [0,0],[0,-th],[o,-th],[o+th,0]]);
    rotate([0,0,an]) translate([w/2, rh+th, zo])
    rotate([0,-90,0]) linear_extrude(height=w)
        polygon([
            [0,0],[0,-th],[o,-th],[o+th,0]]);
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
        //cline_lr(a2, z2, a1, z1, t-1, rw-2, rh-2, a);
    } else {
        cline_lr(a1, z1, a2, z2, t, rw, rh, a);
        //cline_lr(a1, z1, a2, z2, t-1, rw-2, rh-2, a);
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

module quarterpipe(rw=width, rh=height, s=breadth, t=thick, bv=3, eb=5, ics=30, bth=2) {
    // just the curve
    jsds = (180/bang+2)+(90/bang+2)+(100/dang+1);

    csds = 180/cang+2+(180/bang+2)+(180/bang+2)+(360/bang+4); // curve + 2 corners
    cs2 = csds/2;

    bsds = 180/bang+6; // bevel sides
    
    tsds = csds*(bsds-1); // total (-1 for inside index)
    osds = csds*(bsds-4); // Filling back edge
    isds = csds*(bsds-3); // Filling back edge
    
    botsds = 90/bang+1+(180/bang+2);
    
    // closed off end segments
    ovoff = ceil(2/cang);
    
    cursds = (90/bang+1);

    polyhedron(
        points = concat(
            // Outside
            [for (an=[-90:bang:0]) each 
                qpipe_curve(an, rw+t-bv, rh+t-bv, s, t, bv, eb, ics, 0)],
            // Inside
            [for (an=[0:bang:90]) each
                qpipe_curve(an, rw+bv, rh+bv, s, t, bv, eb, ics, ics)],
            qpipe_curve(90, rw+bv, rh+bv, s, t, bv, eb, ics, ics, o=10),
            qpipe_curve(90, rw+bv+bth, rh+bv+bth, s, t, bv, eb, ics, ics, o=10, bth=bth),
            qpipe_curve(90, rw+bv+bth, rh+bv+bth, s, t, bv, eb, ics, ics, bth=bth),
            qpipe_curve(-90, rw+t-bv-bth, rh+t-bv-bth, s, t, bv, eb, ics, bth=bth)
        ),
        faces = concat(
            // sides
            [for (s=[0:bsds-5]) each cquads(csds, csds*s, csds*bsds, ex=1)],
            cquads(csds, csds*(bsds-4)+botsds, csds*bsds, ex=cs2+cursds+botsds+ovoff),
            cquads(csds, csds*(bsds-4)+cs2+cursds-1+ovoff, csds*bsds, ex=cs2+cursds+botsds+ovoff),
            [[csds*(bsds-4)+cs2+cursds-1+ovoff, csds*(bsds-4)+cs2-cursds-ovoff, csds*(bsds-3)+cs2-cursds-ovoff],
             [csds*(bsds-4)+cs2+cursds-1+ovoff, csds*(bsds-3)+cs2-cursds-ovoff, csds*(bsds-3)+cs2+cursds-1+ovoff]],
            cquads(csds, csds*(bsds-3), csds*bsds, ex=1),
            cquads(csds, csds*(bsds-2), csds*bsds, ex=1),

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
            [for (s=[1:botsds]) each [
                // inside
                [isds+s-1,isds+s,isds+csds-s],[isds+s,isds+csds-s-1,isds+csds-s],
                // outside
                [osds+s,osds+s-1,osds+csds-s],[osds+s,osds+csds-s,osds+csds-s-1]]],
            [for (s=[cs2-cursds-ovoff+1:cs2-1]) each [
                // inside
                [isds+s-1,isds+s,isds+csds-s],[isds+s,isds+csds-s-1,isds+csds-s],
                // outside
                [osds+s,osds+s-1,osds+csds-s],[osds+s,osds+csds-s,osds+csds-s-1]]],

            [[osds+botsds,isds+csds-botsds-1,isds+botsds],
             [osds+botsds,osds+csds-botsds-1,isds+csds-botsds-1]],
            [for (s=[1:cs2-1]) each [
                // outside
                [s-1,s,csds-s],[s,csds-s-1,csds-s],
                // inside
                [tsds+s,tsds+s-1,tsds+csds-s],[tsds+s,tsds+csds-s,tsds+csds-s-1]]]
        ));        
}

function qpipe_curve(an, rw, rh, s, t, bv, eb, ics, io=0, o=0, bth=0) =
    concat(
        curve_lower(an,rw,rh,s,t,bv,eb,ics,io,o,bth),
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
        curve_upper(an,rw,rh,s,t,bv,eb,ics,io,o,bth)
    );

function curve_lower(an, rw, rh, s, t, bv, eb, ics, io, o, bth, coang=90) = concat(
        io ? qcircle( // lower base curve
            (t/2-bv)-bv*sin(an+180)-bth, (t/2-bv)-bv*sin(an+180)-bth,
            bv*(1-cos(an))+o, -(rw+((t/2-bv)-bth))+0.01, -ics, san=90, a=-bang
        ) : qcircle( // lower base curve
            (t/2-bv)-bv*sin(an)-bth, (t/2-bv)-bv*sin(an)-bth,
            bv*(1-cos(an))+o, -(rw-((t/2-bv)-bth))-0.01, -ics, san=180, a=bang
        ),
        scircle( // outcut incorner
            0,
            -rh+bv*sin(an), cutof-ics-eb, bv-bv*cos(an),
            san=-90
        ),
        scircle( // outcut corner
            0,
            -rh+bv*sin(an), cutof-ics, bv-bv*cos(an),
            san=-90
        ),
        qcircle( // lower curve
            rw-bv*sin(an), rh-bv*sin(an),
            bv*(1-cos(an))+o, san=-90, ean=coang
        )
    );

function curve_upper(an, rw, rh, s, t, bv, eb, ics, io, o, bth, coang=90) = concat(
        qcircle( // upper curve
            rw-bv*sin(an), rh-bv*sin(an),
            s-bv*(1-cos(an))-o, san=-90, ean=coang, a=-cang
        ),
        scircle( // outcut corner
            eb-bv*(1-cos(an)),
            -rh+bv*sin(an), cutof-ics, s-eb,
            san=90
        ),
        scircle( // outcut incorner
            eb-bv*(cos(an)),
            -rh+bv*sin(an), cutof-ics-eb-eb+bv, s-indof-bv+eb,
            san=-90, a=-bang
        ),
        io ? qcircle( // upper base curve
            (t/2-bv)-bv*sin(an+180)-bth, (t/2-bv)-bv*sin(an+180)-bth,
            s-bv*(1-cos(an))-o-indof, -(rw+((t/2-bv)-bth))+0.01, -ics, san=90, a=bang
        ) : qcircle( // upper base curve
            (t/2-bv)-bv*sin(an)-bth, (t/2-bv)-bv*sin(an)-bth,
            s-bv*(1-cos(an))-o-indof, -(rw-((t/2-bv)-bth))-0.01, -ics, san=180, a=-bang
        )
    );

module quarterpipe_h(rw=width, rh=height, s=breadth, t=thick, bv=3, eb=5, cs=10, bth=2, ics=30) {
    

    // just the curve
    jsds = (180/bang+2)+(90/bang+2)+(100/dang+1);
    // curve + 2 corners
    csds = (coang*2)/cang+2+jsds+(180/bang+2)+dstp*4-4+(100/dang+2)+(360/bang+4);
    bsds = 180/bang+6; // bevel sides
    tsds = csds*(bsds-1); // total (-1 for inside inxdex)
    osds = csds*(bsds-4); // Filling back edge
    isds = csds*(bsds-3); // Filling back edge
    
    // closed off end segments
    ovoff = ceil(2/cang);
    
    cursds = jsds/2+1+dstp*2-2+(50/dang+1);
    offsds = (50/dang)+dstp;
    botsds = 90/bang+1+(180/bang+2);
    ofoff = ((50/dang)+dstp+(45/bang)+(90/bang)+dstp+(50/dang));
    ios = ics+6;
    polyhedron(
        points = concat(
            // Outside
            [for (an=[-90:bang:0]) each
                qpipe_h_curve(an, rw+t-bv, rh+t-bv, s, t, bv, eb, cs, ics)],
            // Inside
            [for (an=[0:bang:90]) each
                qpipe_h_curve(an, rw+bv, rh+bv, s, t, bv, eb, cs, ics, ios, t)],
            qpipe_h_curve(90, rw+bv, rh+bv, s, t, bv, eb, cs, ics, ios, t, o=10),
            qpipe_h_curve(90, rw+bv+bth, rh+bv+bth, s, t, bv, eb, cs, ics, ios, t, bth, o=10),
            qpipe_h_curve(90, rw+bv+bth, rh+bv+bth, s, t, bv, eb, cs, ics, ios, t, bth),

            qpipe_h_curve(-90, rw+t-bv-bth, rh+t-bv-bth, s, t, bv, eb, cs, ics, bth=bth)

        ),
        faces = concat(
            // sides
            [for (s=[0:bsds-5]) each cquads(csds, csds*s, csds*bsds, ex=1)],
            cquads(csds, csds*(bsds-4)+botsds, csds*bsds, ex=csds/2+cursds+botsds+ovoff),
            cquads(csds, csds*(bsds-4)+csds/2+cursds-1+ovoff, csds*bsds, ex=csds/2+cursds+botsds+ovoff),
            [[csds*(bsds-4)+csds/2+cursds-1+ovoff, csds*(bsds-4)+csds/2-cursds-ovoff, csds*(bsds-3)+csds/2-cursds-ovoff],
             [csds*(bsds-4)+csds/2+cursds-1+ovoff, csds*(bsds-3)+csds/2-cursds-ovoff, csds*(bsds-3)+csds/2+cursds-1+ovoff]],
            cquads(csds, csds*(bsds-3), csds*bsds, ex=1),
            cquads(csds, csds*(bsds-2), csds*bsds, ex=1),
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
            [for (s=[1:botsds]) each [
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

function qpipe_h_curve(an, rw, rh, s, t, bv, eb, cs, ics, io=0, th=0, bth=0, o=0) =
    concat(
        curve_lower(an,rw,rh,s,t,bv,eb,ics,io,o,bth,coang),
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
        curve_upper(an,rw,rh,s,t,bv,eb,ics,io,o,bth,coang)
    );

function bline_c(y, x1,z1,x2,z2,stp=dstp) =
    [for (s=[1:stp-1]) [x1+(x2-x1)*s/stp,
                sqrt(pow(y,2)-pow(x1+(x2-x1)*s/stp,2)),
                z1+(z2-z1)*s/stp]];

function scircle(r, x, y, z, san=0, ean=90, a=bang) =
    [for (an=[san+(a>0?0:ean):a:san+(a>0?ean:0)])
        [x,y+r*cos(an),z+r*sin(an)]];
    
function bcircle(r, x, y, z, san=0, ean=90, a=bang) =
    [for (an=[san+(a>0?0:ean):a:san+(a>0?ean:0)])
        [x+r*cos(an),y,z+r*sin(an)]];
    
function bcircle_e(rx, rz, x, y, z, san=0, ean=90, a=bang) =
    [for (an=[san+(a>0?0:ean):a:san+(a>0?ean:0)])
        [x+rx*cos(an),y,z+rz*sin(an)]];
    
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

module atmega() {
    cube([18, 33, 3.2], true);
    translate([0, 15, 0.4]) cube([7.5, 5, 4], true);
}

module batteryholder() {
    cr=5;
    bh=50/2-cr;
    bw=100.5/2-cr;
    difference() {
        union() {
            translate([1,0.2,-3.5]) linear_extrude(height=1.5) polygon(concat(
                [for (an=[  0:10: 90]) [ bh+sin(an)*cr, bw+cos(an)*cr]],
                [for (an=[ 90:10:180]) [ bh+sin(an)*cr,-bw+cos(an)*cr]],
                [for (an=[180:10:270]) [-bh+sin(an)*cr,-bw+cos(an)*cr]],
                [for (an=[270:10:360]) [-bh+sin(an)*cr, bw+cos(an)*cr]]
            ));
            translate([0, 6,-12]) cube([40,77,24], true);
            translate([0.5,95/2,-4.5]) cube([7.5,6,2],true);
        }
        translate([-33/2,-95/2,-4.5]) cylinder(3,1.5,1.5, $fn=24);
        translate([-33/2, 95/2,-4.5]) cylinder(3,1.5,1.5, $fn=24);
        translate([ 33/2,-95/2,-4.5]) cylinder(3,1.5,1.5, $fn=24);
        translate([ 33/2, 95/2,-4.5]) cylinder(3,1.5,1.5, $fn=24);
    }
}

module batteryholder_1() {
    cr=2;
    bh=29.2/2-cr;
    bw=99.2/2-cr;
    difference() {
        union() {
            translate([0,0.2,-3.5]) linear_extrude(height=1.5) polygon(concat(
                [for (an=[  0:10: 90]) [ bh+sin(an)*cr, bw+cos(an)*cr]],
                [for (an=[ 90:10:180]) [ bh+sin(an)*cr,-bw+cos(an)*cr]],
                [for (an=[180:10:270]) [-bh+sin(an)*cr,-bw+cos(an)*cr]],
                [for (an=[270:10:360]) [-bh+sin(an)*cr, bw+cos(an)*cr]]
            ));
            translate([0, 6,-12]) cube([20,77,24], true);
            translate([10.2,38.9,-1.0]) cube([6,7.5,2],true);
            translate([0.5,-95/2,-7]) cube([13.5,14,7],true);
        }
        translate([-22/2,-94.5/2,-4.5]) cylinder(3,1.5,1.5, $fn=24);
        translate([-22/2, 94.5/2,-4.5]) cylinder(3,1.5,1.5, $fn=24);
        translate([ 22/2,-94.5/2,-4.5]) cylinder(3,1.5,1.5, $fn=24);
        translate([ 22/2, 94.5/2,-4.5]) cylinder(3,1.5,1.5, $fn=24);
    }
}

module centerholes(t=wall, o=-1, f=1) {
    y = butsz-buthi+5.1 + centerthick/2;
    x = f*(centerwidth/2+o*t/2);
    translate([x, y, 25]) {
        cube([t+1, 10, 25], true);
        translate([0, -10, -18]) rotate([0,90,0]) cylinder(t+1, 2, 2, true, $fn=360/pang);
        translate([0,  10, -18]) rotate([0,90,0]) cylinder(t+1, 2, 2, true, $fn=360/pang);
        translate([0, -10,   8]) rotate([0,90,0]) cylinder(t+1, 2, 2, true, $fn=360/pang);
        translate([0,  10,   8]) rotate([0,90,0]) cylinder(t+1, 2, 2, true, $fn=360/pang);
    }
}

module hingeholes(o=-1, t=wall) {
    translate([width+thick/2+7, o*t/2, 6]) rotate([90,0,0]) cylinder(t+1, 2, 2, true, $fn=360/pang);
    translate([width+thick/2+32, o*t/2, 6]) rotate([90,0,0]) cylinder(t+1, 2, 2, true, $fn=360/pang);
}
