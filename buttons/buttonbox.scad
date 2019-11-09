s3 = sqrt(3);
s2 = sqrt(2);

trivec = 3;
trisize = 170;
trioff = trisize / s3;
bsize = 25;
wid = 3;
cwid = wid*s3/2;
height = 30;
bheight = 20;
bott = 2;
tabheight = 7.5;
sheight = 10;
cang = 3;
btol = 0.3;
tol = 0.1;
bth = 6;
bbev = 4;
bwid = 2;

crv = 10;

taboff = trioff-bsize-cwid-1.114-tol;

*boxsq();
boxtri();

module boxsq() {
    bw = 110;
    bh = 30;
    eh = tabheight;
    bt = 2;
    btw = 3.65;
    
    color("gray")
    sqfront(bw,bh,bt,btw*s2);
    
    *color("gray")
    translate([0,0,-0.1])
    sqback(bw,bt,3,eh);
    
    *color("white")
    for (n = [0:3]) {
        translate([-41.4,n*(82.8/3)-41.4,0]) sqbutton(bh, btw*s2);
    }
    
    *color("teal") translate([26,-4.6,bh-bt-1.1]) rotate([0,0,180])batteryholder();
}

*rotate([0,180,0]) union() {
    sqbutton(30,3.65*s2);
    // Sacrificial layer to bridge hole
    #translate([0,0,sheight+wid-0.1]) cube([11,11,0.2],true);
}

module boxtri() {
    color("gray")
    front2();
    *color("gray")
    *translate([0,0,-tol]) bottom2();
    *switches(180);
    *color("teal") translate([0,-19.8,height-wid-1.1]) rotate([0,0,180]) batteryholder();
}

*rotate([0,180,0]) union() {
    button(180);
    // Sacrificial layer to bridge hole
    #translate([0,0,sheight+wid-0.1]) cube([11,11,0.2],true);
}

module sqfront(bw,bh,bt,btw) {
    w = bw/s2-crv-bt;
    t = bt;
    bev = 2;
    bv = bev*s2/2;
    bs2 = btw;
    
    difference() {
        rotate([0,0,45]) cur_prism(4, [
            [w-t,bh,t-bv],
            [w-t,bh-bev,t],
            [w-t,0,t],
            [w-t,0,0],
            [w-t,bh-bt,0]]);
        
            #for (n=[0:3]) {
                translate([-41.4,n*(82.8/3)-41.4,0])
                rotate([0,0,45])
                cur_prism(4, [
                    [bs2,bh+0.5,1-cwid],
                    [bs2,bh-0.5,-cwid],
                    [bs2,bh-bt-0.5,-cwid]]);
        
            #translate([26,-4.6,bh-bt-0.1])
            translate([-0.5,-95/2-4,-5.8]) cube([8.5,3,2.6],true);
        
        }
    }
    
    for (n=[0:3]) {
        translate([-41.4,n*(82.8/3)-41.4,0])
        rotate([0,0,45])
        cur_donut(4, [
            [bs2,sheight+2+wid,-cwid],
            [bs2,bh-1,-cwid],
            [bs2,bh-1,0.2],
            [bs2,sheight+2,0.2],
            [bs2,sheight+2,0]]);                
    }
    
    bsw = bs2/s2;
    bssz = (bsw+(crv*cos(cang/2))+0.2);
    for (n=[0:3]) {
        translate([-41.4,n*(82.8/3)-41.4,sheight+2])
        linear_extrude(height=bh-(sheight+2)-1)
        polygon(concat(
            [[bsw,-13.9],[bssz,-13.9],[bssz,13.9],[bsw,13.9]],
            [for (an = [cang/2:cang:90-cang/2]) [bsw+sin(an)*crv,bsw+cos(an)*crv]],
            [for (an = [90+cang/2:cang:180-cang/2]) [bsw+sin(an)*crv,-bsw+cos(an)*crv]]
    ));
    }
    for (n=[0:2]) {
        translate([-41.4,n*(82.8/3)-41.4,sheight+2])
        linear_extrude(height=bh-(sheight+2)-1)
        polygon(concat(
            [[-bssz,bsw],[-bssz,27.6-bsw]],
            [for (an = [90+cang/2:cang:180-cang/2]) [-bsw-sin(an)*crv,27.6-bsw+cos(an)*crv]],
            [for (an = [cang/2:cang:90-cang/2]) [-bsw-sin(an)*crv,bsw+cos(an)*crv]]
    ));
    }
    
    translate([26,-4.6,bh-bt-1.1]) batteryclips_w();
    
    translate([-14.5,-30,bh-bt-0.1]) rotate([0,0,-90]) wemosd1(soff=1);
    
    translate([35,-bw/2+bt/2-tol,tabheight-tol])
    rotate([0,0,180])
    bottomtablip();
    translate([-15,-bw/2+bt/2-tol,tabheight-tol])
    rotate([0,0,180])
    bottomtablip();
    
    translate([35,bw/2-bt/2+tol,tabheight-tol])
    bottomtablip();
    translate([-15,bw/2-bt/2+tol,tabheight-tol])
    bottomtablip();
    
    translate([bw/2-bt/2+tol,30,tabheight-tol])
    rotate([0,0,-90])
    bottomtablip();
    translate([bw/2-bt/2+tol,-30,tabheight-tol])
    rotate([0,0,-90])
    bottomtablip();
    
    translate([-bw/2+bt/2-tol,30,-3])
    rotate([0,0,90])
    bottomtablip();
    translate([-bw/2+bt/2-tol,-30,-3])
    rotate([0,0,90])
    bottomtablip();
}

module sqback(bw,bt,et,eh) {
    w = bw/s2-crv-bt;
    t = bt;
    
    difference() {
        rotate([0,0,45]) cur_prism(4, [
            [w-t,0,-et],
            [w-t,eh,-et],
            [w-t,eh,-tol],
            [w-t,0,-tol],
            [w-t,0,t],
            [w-t,-bt,t]
        ]);
        
            
        translate([-bw/2+bt/2+tol,30,-3])
        rotate([0,0,90])
        bottomtablip(wid=21);
        translate([-bw/2+bt/2+tol,-30,-3])
        rotate([0,0,90])
        bottomtablip(wid=21);
        
        #for (n = [3:6]) {
            translate([(n-2)*4*s2,(n+2)*4*s2,0])
            rotate([0,0,45])
            cur_prism(2, [[(6-n)*8,0.5,-8],[(6-n)*8,-2.5,-8]]);
            translate([-(n-2)*4*s2,-(n+2)*4*s2,0])
            rotate([0,0,45])
            cur_prism(2, [[(6-n)*8,0.5,-8],[(6-n)*8,-2.5,-8]]);
        }
        #for (n = [-2:2]) {
            translate([0,n*8*s2,0])
            rotate([0,0,45])
            cur_prism(2, [[32,0.5,-8],[32,-2.5,-8]]);
        }
    }

    
    translate([35,-bw/2+bt/2,tabheight])
    rotate([0,0,180])
    bottomtab();
    translate([-15,-bw/2+bt/2,tabheight])
    rotate([0,0,180])
    bottomtab();
    
    translate([35,bw/2-bt/2,tabheight])
    bottomtab();
    translate([-15,bw/2-bt/2,tabheight])
    bottomtab();
    
    translate([bw/2-bt/2,30,tabheight])
    rotate([0,0,-90])
    bottomtab();
    translate([bw/2-bt/2,-30,tabheight])
    rotate([0,0,-90])
    bottomtab();

}

module bottom2() {
    to2 = trioff-bsize-cwid;
    difference() {
        cur_prism(trivec, [
            [to2,0,-bwid*2/s3-tol],
            [to2,sheight-2,-bwid*2/s3-tol],
            [to2,sheight-2,-tol],
            [to2,0,-tol],
            [to2,0,cwid],
            [to2,-bott,cwid]
        ],cw=bsize);
        #for (n = [360/trivec:360/trivec:360]) rotate([0,0,n]) {
            translate([0,0,-bott-0.5]) slitgroup();
        }
    }
    for (n = [360/trivec:360/trivec:360]) {
        rotate([0,0,n]) {
            translate([40,-taboff,sheight-2])
            rotate([0,0,180])
            bottomtab();
            translate([-40,-taboff,sheight-2])
            rotate([0,0,180])
            bottomtab();
        }
    }
}

module slitgroup(w=5, s=5, n=3, e=1, h=3) {
    for (i = [0:n-1]) {
        translate([(w+s)/s3,s/2+i*(s+w),0])
        slit((2*w+s+6*i*(w+s))/s3, w, h);
    }
    if (e > 0)
    for (i = [n:n+e-1]) {
        translate([0,s/2+i*(s+w)+w,0])
        rotate([0,0,180]) slit((2*w+s+6*(i-1)*(w+s)+2*w)/s3, w, h);
    }
}

module slit(w, h=10, t=3) {
    linear_extrude(height=t)
    polygon([[-w/2,0],[w/2,0],[w/2+h/s3,h],[-w/2-h/s3,h]]);
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

module sqbutton(height, bsize, btol=0.2, bbev=bbev) {
    bs2 = bsize-btol*s3;
    rotate([0,0,45])
    difference() {
        cur_prism(4, [
            [bs2,height+bth-1,-bbev-cwid+1],
            [bs2,height+bth-bbev,-cwid],
            [bs2,sheight+wid+2,-cwid],
            [bs2,sheight+2,0],
            [bs2,sheight,0],
            [0,sheight,5-crv],
            [0,sheight+wid,5-crv],
            [bs2,sheight+wid,-cwid-bwid],
            [bs2,height+bth-bbev,-cwid-bwid],
            [bs2,height+bth-bwid,-cwid-bbev] ]);
        *for (n=[360/trivec:360/trivec:360]) rotate([0,0,n+180])
        translate([0,0,height+bth-bwid-0.1]) slitgroup(3,2,2,1,1.1);
    }
    rotate([0,0,90]) swtabs();
}

module button(rot) {
    bs2 = bsize-btol*s3;
    difference() {
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
        for (n=[360/trivec:360/trivec:360]) rotate([0,0,n+180])
        translate([0,0,height+bth-bwid-0.1]) slitgroup(3,2,2,1,1.1);
    }
    rotate([0,0,rot]) swtabs();
}

module swtabs() {
    rotate([0,0,-90]) translate([0,15.5/2,sheight]) swtab();
    rotate([0,0,-90]) mirror([0,1,0]) translate([0,15.5/2,sheight]) swtab();
    
    translate([-5.4,7.7,sheight-1]) cube([5,2,3],true);
    translate([ 5.4,7.7,sheight-1]) cube([5,2,3],true);
    translate([ 6.6,-10.1,sheight-1]) cube([2.6,2,3],true); 
    translate([-7.2,-10.1,sheight-0.3]) cube([1.5,2,1.5],true);         

}

module swtab(w=10) {
    rotate([0,90,0])
    translate([0,0,-w/2+1.5])
    linear_extrude(height=w)
    polygon([
        [0.8,0.1],[1.6,0.1],[2.3,-0.6],
        [4.4,0.6],[3.0,1.6],[0.8,1.6]
    ]);
    translate([1.5,0.85,-1]) cube([17,1.5,1.2],true);
    translate([10.1,0.85,-0.6]) cube([2,1.5,2],true);
    translate([-7.7,0.85,-0.6]) cube([2,1.5,2],true);
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
        for (n=[360/trivec:360/trivec:360]) rotate([0,0,n]) {
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
            #translate([0,0,height-0.2]) slitgroup(0.5,4.5,6);
        }
        #translate([0,-20,height-wid-1.1])
        translate([-0.5,-95/2-4,-4.8]) cube([8.5,3,2.6],true);
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
    translate([0,-19.7,height-wid-1.1]) batteryclips_w();
    translate([-50,25,height-wid-0.1]) rotate([0,0,-120]) wemosd1();
    
    
    for (n = [360/trivec:360/trivec:360]) {
        rotate([0,0,n]) {
            translate([40,-taboff-tol,sheight-2-tol])
            rotate([0,0,180])
            bottomtablip();
            translate([-40,-taboff-tol,sheight-2-tol])
            rotate([0,0,180])
            bottomtablip();
        }
    }
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
    translate([-15,-50.6,0]) rotate([0,0,180]) batterytab(15);
    translate([-15, 50.3,0]) batterytab(15);
    translate([ 15,-50.6,0]) rotate([0,0,180]) batterytab(15);
    translate([ 15, 50.3,0]) batterytab(15);
}

module batteryclips_w() {
    translate([-33/2,-95/2,0]) batterypin_w();
    translate([-33/2, 95/2,0]) batterypin();
    translate([ 33/2,-95/2,0]) batterypin_w();
    translate([ 33/2, 95/2,0]) batterypin();
    translate([-15,-50.6,0]) rotate([0,0,180]) batterytab_w(15);
    translate([-15, 50.3,0]) batterytab(15);
    translate([ 15,-50.6,0]) rotate([0,0,180]) batterytab_w(15);
    translate([ 15, 50.3,0]) batterytab(15);
}

module batterypin() {
    rotate([0,0,360/48]) {
        translate([0,0,-1.9]) cylinder(4.0,2.25,2.25, $fn=24);
        translate([0,0,-3.0]) cylinder(1.2,1.1,1.2, $fn=24);
        translate([0,0,-4.0]) cylinder(1.0,0.8,1.1, $fn=24);
    }
}

module batterypin_w() {
    rotate([0,0,360/48]) {
        translate([0,0,-1.9]) cylinder(4.0,2.25,2.25, $fn=24);
        translate([0,0,-2.5]) cylinder(1.2,1.1,1.1, $fn=24);
    }
}

module batterytab(w) {
    translate([-w/2,0,0]) rotate([0,90,0]) linear_extrude(height=w) polygon([
        [-2.1,0],[3.5,0],[4.2,-0.8],[7,0.5],[6,1.5],[-2.1,1.5]
    ]);
}

module batterytab_w(w) {
    translate([-w/2,0,0]) rotate([0,90,0]) linear_extrude(height=w) polygon([
        [-0.1,0],[3.5,0],[4.2,-0.8],[7,0.5],[6,1.5],[-0.1,1.5]
    ]);
}

module wemosd1(soff=0) {
    *#translate([0,0,-2.4]) cube([34.6,25.4,4.8], true);
    // translate([-17.3,   0,0]) rotate([0,0,90]) mcutab(12);
    // translate([ 17.3,-7.2,0]) rotate([0,0,-90]) mcutab(8);
    translate([ 17.3, 0,0]) rotate([0,0,-90]) mcutab(15);
    translate([-17.3,-9.2+soff/2,0]) rotate([0,0, 90]) mcutab(8-soff);
    translate([-17.3, 9.2,0]) rotate([0,0, 90]) mcutab(8);

    translate([ 10,-12.8-2/2,-7/2+0.1]) cube([7,2,7.2],true);
    translate([ 10, 12.8+2/2,-7/2+0.1]) cube([7,2,7.2],true);
    translate([-10,-12.8-2/2,-7/2+0.1]) cube([7,2,7.2],true);
    translate([ -6, 12.8+2/2,-7/2+0.1]) cube([7,2,7.2],true);
}

module mcutab(w) {
    translate([-w/2,0,0]) rotate([0,90,0]) linear_extrude(height=w) polygon([
        [-0.2,0],[4.8,0],[5.4,-0.5],[7,0.5],[5.4,1.5],[-0.2,1.5]
    ]);
}

module bottomtab(off=0, pos=0) {
    translate([-10+off,pos,0])
    rotate([0,90,0])
    linear_extrude(height=20) polygon([
        [0,-1.5],[0,0],[-8.2,0],[-9,0.8],[-11,-0.5],[-9.5,-1.5]
    ]);
}

module bottomtablip(off=0, pos=0, wid=20) {
    translate([-wid/2+off,pos,0])
    rotate([0,90,0])
    linear_extrude(height=wid) polygon([
        [-6,1.3],[-8.2,0],[-9.5,1.3]
    ]);
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

function nquads(n,o,s=9999999) = concat(
    [for (i=[0:n-1]) [(i+1)%n+o,i+o,(i+o+n)%s]],
    [for (i=[0:n-1]) [(i+1)%n+o,(i+o+n)%s,((i+1)%n+o+n)%s]]
);

module cur_prism(sides, points, ct = crv, cs = cang, cw=0) {
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

module cur_donut(sides, points, ct = crv, cs = cang, cw=0) {
    sds = 360/cs;
    nm = len(points);
    polyhedron(
        points = [for (ps = points) each cur_ngon(sides, ps[0],ps[1],ct+(ps[2]?ps[2]:0),cs,cw)]
            ,
        faces = concat(
                [for (n=[0:nm-1]) each nquads(sds,sds*n,sds*nm)]
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
