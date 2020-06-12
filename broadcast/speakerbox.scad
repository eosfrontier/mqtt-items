
gap=0;

ampoff=3.37;

holeoff = 20;

*rotate([0,0,0]) translate([-gap,0,0]) {
    #if ($preview) {
        color("grey") translate([-65,0,40]) rotate([0,-60,0])    speaker();
    }
    hexbox();
}

*rotate([0,0,180]) translate([-gap,0,0]) {
    #if ($preview) {
        color("grey") translate([-65,0,40]) rotate([0,-60,0])    speaker();
    }
    hexbox();
}

translate([0,0,0]) hexbox(true);

#if ($preview) {
    midh = 60;
    toph = 54.5;
    midt = 0;
    topt = 60;
    midsidesl = ((midh-toph)/2)/(topt-midt);
    midsidean = atan(midsidesl);

    translate([0,0,0]) powercon();

    translate([-15,0,18.5]) rotate([90,0,-90]) esp_ttgo();

    translate([15,ampoff,9.5]) rotate([90,0,-90]) amp_tda();

    translate([0,21.8,57]) rotate([-90+midsidean,0,0]) conv_3v();
}

*botcover();


//speaker_grille();
//    dome(39,100,1.5);


module hexbox(middle = false, right = false) {
    toph = 54.5;
    topw = 40;
    topt = 60;
    thick = 2;

    firsth = 70;
    firstw = 90;
    firstt = 76;

    secw = 150;
    sech = firsth + (firsth-toph)*((secw-firstw)/(firstw-topw));
    sect = firstt + (firstt-topt)*((secw-firstw)/(firstw-topw));
    
    halfw = 275;
    halfh = 160;
    halft = 48;
    
    midh = 60;
    midt = 0;
    
    botw = 400;
    both = 80;
    bott = 0;
    
    holedia = 10;
    midsidesl = ((midh-toph)/2)/(topt-midt);
    midsidean = atan(midsidesl);
    
    faces_r = concat(
        [[3,2,1,0],[0,1,16],[2,3,17]],
        [[1,2,6,5]],
        [[1,5,9],[2,10,6]],
        [[1,9,16],[10,2,17]],
        [[9,13,16],[14,10,17]],

        [[13,14,17,16],[34,33,18,19]],
        
        [[20,21,22,23],[21,20,18],[23,22,19]],
        [[22,21,25,26]],
        [[29,21,18],[22,30,19]],
        [[33,29,18],[30,34,19]],
        [[25,21,29],[30,22,26]],
        
        [[5,6,26],[5,26,25],[14,13,34],[13,33,34]],
        [[9,5,29],[5,25,29],[13,9,33],[9,29,33]],
        [[6,10,30],[6,30,26],[10,14,34],[10,34,30]],
        
        [[3,0,20],[3,20,23],[17,18,16],[17,19,18]],
        [[16,20,0],[16,18,20],[23,19,17],[3,23,17]],
        
        []
    );
    faces_m = concat(
        [[3,2,1,0],[0,1,16],[2,3,17]],
        [[20,21,22,23],[21,20,18],[23,22,19]],
        
        [[3,0,20],[3,20,23]],
        [[16,20,0],[16,18,20],[23,19,17],[3,23,17]],
        
        [[1,2,22],[1,22,21]],
        [[16,1,21],[16,21,18],[22,17,19],[2,17,22]],

        
        []
    );
    faces_l = concat(
        [[12,16,17,15],[18,32,35,19]],
        [[3,0,4,7],[20,23,27,24]],
        [[0,8,4],[3,7,11]],
        [[8,0,16],[3,11,17]],
        [[12,8,16],[11,15,17]],
        
        [[28,20,24],[27,23,31]],
        [[20,28,18],[31,23,19]],
        [[28,32,18],[35,31,19]],
        [[7,4,24],[7,24,27],[12,15,35],[12,35,32]],
        [[4,8,28],[4,28,24],[8,12,32],[8,32,28]],
        [[11,7,31],[7,27,31],[15,11,35],[11,31,35]],
        
        [[0,3,20],[3,23,20],[18,17,16],[17,18,19]],
        [[20,16,0],[16,20,18],[23,17,19],[3,17,23]],
        
        []
    );
    
    difference() {
        polyhedron(points = concat(
            h_rect(topw,toph,topt),            //  0
            h_rect(secw,sech,sect),            //  4
            h_rect(halfw,halfh,halft),         //  8
            h_rect(botw,both,bott),            // 12
            [[0,-midh/2,midt],[0,midh/2,midt]],// 16
            [[0,-midh/2+thick,midt+thick],
             [0,midh/2-thick,midt+thick]],     // 18
            h_rect(topw,toph-thick*2,topt-thick),      // 20
            h_rect(secw+thick*2,sech-thick,sect-thick),      // 24
            h_rect(halfw,halfh-thick*2,halft),  // 28
            h_rect(botw-thick*2,both-thick,bott+thick)       // 32
          ), faces = middle?faces_m:right?faces_r:faces_l,
            convexity=4);
        if (right) {
            translate([-0.1,-(midh-4)/2,-0.1])
            cube([86.1,midh-4,thick+0.101]);
        } else if (!middle) {
            translate([-86,-(midh-4)/2,-0.1])
            cube([86.1,midh-4,thick+0.101]);
            
            translate([-86+5,-(midh-4)/2,-0.1])
            cylinder(thick+0.101,5,5, $fn=60);
            translate([-86+5,(midh-4)/2,-0.1])
            cylinder(thick+0.101,5,5, $fn=60);
        }
        if (right || middle) {
            translate([9.5,-3.37+ampoff,topt-thick])
            rotate([90,0,90])
            linear_extrude(height=3)
            polygon([[-8,-0.1],[-6,2.1],[6,2.1],[8,-0.1]]);
            
            translate([9.5,-3.37+ampoff,topt-thick])
            rotate([90,0,90])
            linear_extrude(height=4)
            polygon([[-8,-0.1],[-7,1],[7,1],[8,-0.1]]);
            
            translate([13,-17+ampoff,topt-thick-0.1])
            cube([2.5,34,1.1]);
            
            // Hole for power connector
            translate([0,-midh/2+thick+holeoff*midsidesl+0.1,holeoff])
            rotate([-midsidean+90,0,0])
            cylinder(thick+0.2, holedia/2, holedia/2, $fn=60);
        }
    }
    
    g=3.4/thick;
    if (middle) {
        // Connector to side
        translate([-topw/2,-toph/2+thick,topt]) rotate([-90,0,0])
        linear_extrude(height=toph-thick*2) polygon(thick*[
            [-1-g,0],[-0.6-g,1.4],[-g,2],[0.36,2],[1.3,1],[-g+0.3,1],[-g,0]
        ]);
        // Gap filler
        translate([-topw/2,-toph/2+thick-0.1,topt]) rotate([-90,0,0])
        linear_extrude(height=toph-thick*2+0.2) polygon(thick*[
            [0.36,2],[1.3,1],[0.02,1]
        ]);
        // Connector to other side
        translate([topw/2,toph/2-thick,topt]) rotate([-90,0,180])
        linear_extrude(height=toph-thick*2) polygon(thick*[
            [-1-g,0],[-0.6-g,1.4],[-g,2],[0.36,2],[1.3,1],[-g+0.3,1],[-g,0]
        ]);
        // Gap filler
        translate([topw/2,toph/2-thick+0.1,topt]) rotate([-90,0,180])
        linear_extrude(height=toph-thick*2+0.2) polygon(thick*[
            [0.36,2],[1.3,1],[0.02,1]
        ]);
        
        // Lips for bottom cover
        translate([0,midh/2-thick*2-0.3,0.1]) rotate([90,0,180])
            lip_mid(thick);
        translate([0,-midh/2+thick*2+0.3,0.1]) rotate([90,0,0])
            lip_mid(thick);
            
        // Supports for amp
        translate([15,ampoff-17,topt-0.5])
            amp_lip();
        translate([15,ampoff+17,topt-0.5])
            mirror([0,1,0]) amp_lip();
            
        // Supports for esp32
        translate([-15,-15.7,topt-1.5])
            esp_lip();
        translate([-15,15.7,topt-1.5])
            mirror([0,1,0]) esp_lip();
           
        // Supports for 3v3 converter
        translate([0,21.8,57]) rotate([-90+midsidean,0,0])
            lip_3v();
        translate([0,21.8,57]) rotate([-90+midsidean,0,0])
            mirror([1,0,0]) lip_3v();

        // Reinforcement for powercon hole
        translate([0,-midh/2+thick+holeoff*midsidesl+0.1,holeoff])
        rotate([-midsidean+90,0,0]) difference() {
            union() {
                translate([0,0,-1.5]) cylinder(1.5, 15, 15, $fn=60);
                translate([0,0,0]) linear_extrude(height=1)
                polygon([[0,-17.8],[-11.7,15],[11.7,15]]);
            }
            translate([0,0,-1.6]) cylinder(3, holedia/2, holedia/2, $fn=60);
            #translate([0,15,-1.6]) rotate([45,0,0]) cube([30,4,4], true);
        }
    } else if (right) {
        translate([ 65,0,40]) rotate([0, 60,0]) 
            speaker_grille();
    } else {
        translate([-65,0,40]) rotate([0,60,180]) 
            speaker_grille();
        
        translate([-topw/2,-toph/2,topt]) rotate([-90,0,0])
        linear_extrude(height=toph) polygon(thick*[
            [-g+0.4,1],[-g+0.1,0],[0,0.9],[0,1]
        ]);
        
        translate([-topw/2,-toph/2,topt]) rotate([-90,0,0])
        linear_extrude(height=1.9) polygon(thick*[
            [-g+0.4,1],[-g+0.7,2],[0.34,2],[0,1]
        ]);
        translate([-topw/2-2,-toph/2+1.9,topt-4])
        rotate([135,0,0]) cube([2.66,0.85,2.1]);

        
        translate([-topw/2,toph/2-1.9,topt]) rotate([-90,0,0])
        linear_extrude(height=1.9) polygon(thick*[
            [-g+0.4,1],[-g+0.7,2],[0.34,2],[0,1]
        ]);
        translate([-topw/2-2,toph/2-1.9,topt-4])
        rotate([-45,0,0]) cube([2.66,2.1,0.85]);
        
        translate([0,-midh/2+thick+0.2,0.1]) rotate([90,0,0])
        difference() {
            linear_extrude(height=1.3) polygon([
                [-1.7,thick+3.3],
                [-0.3,thick-0.5],
                [-3,thick-0.5],[-3,thick+2]]);
            translate([-1.8,1.7,-0.1]) rotate([45,0,0]) cube([3,1,1], true);
        }

        translate([0,midh/2-thick+1.1,0.1]) rotate([90,0,0])
        difference() {
            linear_extrude(height=1.3) polygon([
                [-1.7,thick+3.3],
                [-0.3,thick-0.5],
                [-3,thick-0.5],[-3,thick+2]]);
            translate([-1.8,1.7,1.4]) rotate([45,0,0]) cube([3,1,1], true);

        }
        
        translate([-87,-(midh-4)/2,thick]) union() {
            difference() {
              union() {
                linear_extrude(height=3.6) polygon([
                  [-0.1,3.6],[10,3.6],[20,-7.1],[-0.1,-8.1]
                ]);
                translate([0,0,3.6]) linear_extrude(height=0.6) polygon([
                  [-0.1,3.6],[10,3.6],[21,-7.1-1.07],[-0.1,-9.1]
                ]);
              }
              translate([6,0.1,2.1]) cube([7.2,7.4,2.2], true);
              translate([6,0,-2.1]) cylinder(8,2,2, $fn=60);
            }
            // Cutaway layer for support
            #linear_extrude(height=0.2) polygon([
                [-0.1,-8.1],[20,-7.1],[-0.5,3.6+(1.05*10.7)]
            ]);
            #translate([6,0,3.2]) cylinder(0.2,2.1,2.1, $fn=60);
        }

        translate([-87,(midh-4)/2,thick]) union() {
            difference() {
              union() {
                linear_extrude(height=3.6) polygon([
                  [-0.1,-3.6],[10,-3.6],[20,7.1],[-0.1,8.1]
                ]);
                translate([0,0,3.6]) linear_extrude(height=0.6) polygon([
                  [-0.1,-3.6],[10,-3.6],[21,7.1+1.07],[-0.1,9.1]
                ]);
              }
              translate([6,-0.1,2.1]) cube([7.2,7.4,2.2], true);
              translate([6,0,-2.1]) cylinder(8,2,2, $fn=60);
            }
            // Cutaway layer for support
            #linear_extrude(height=0.2) polygon([
                [-0.1,8.1],[20,7.1],[-0.5,-3.6-(1.05*10.7)]
            ]);
            #translate([6,0,3.2]) cylinder(0.2,2.1,2.1, $fn=60);
        }
    }
}

module lip_3v() {
    tol=0.2;
    wid=17.1;
    hi=5;
    thi = 1.5;
    h1 = 4;
    h0 = 2;
    
    w = wid/2+tol;
    w1 = w-h0;
    w2 = wid/2+tol+thi;
    t = -tol;
    t2 = -tol-thi;
    h2 = h1+hi;
    of = 11;

    n1 = of-h2;
    n2 = of-h1;
    n3 = of-(h1-h0);
    n4 = of+(h1-h0);
    n5 = of+h1;
    n6 = of+h2;
    polyhedron(points = concat(
        [[w,n1,hi],[w,n2,t],[w1,n3,t],
         [w1,n4,t],[w,n5,t],[w,n6,hi]],
        [[w2,n1,hi],[w2,n2+thi,t2],[w1,of,t2],
         [w2,n5-thi,t2],[w2,n6,hi]]
    ), faces = concat(
        [[0,1,4,5],[1,2,3,4],[0,5,10,6]],
        [[10,9,7,6],[1,0,6,7],[5,4,9,10]],
        [[2,1,8],[4,3,8],[3,2,8],
         [8,1,7],[4,8,9],[9,8,7]]
    ), convexity = 4);
}

module lip_mid(thick) {
    difference() {
        union() {
            linear_extrude(height=thick) polygon([
                [0,thick+5],[3,thick+2],[3,thick],
                [-3,thick],[-3,thick+2]]);
            linear_extrude(height=thick+0.3) polygon([
                [0,thick+5],[1.32,thick+3.68],[0,thick],
                [-1.32,thick+3.68]]);
        }
        translate([0,thick+5.65,-0.1]) rotate([45,0,0]) cube([5,4,4], true);
    }
}

function h_rect(w, h, t) = [[-w/2,-h/2,t],[w/2,-h/2,t],[w/2,h/2,t],[-w/2,h/2,t]];

module esp_lip(bot=4, top=4.5, height=40.2) {
    th = 3;
    cp = 2;
    it = 1.1;
    tt = 4.5;
    
    polyhedron(points = concat(
        u_shape(0, it),
        u_shape(-bot, it),
        u_shape(-bot-th, it),
        u_shape(-height+top+th, tt),
        u_shape(-height+top, tt),
        u_shape(-height, tt),
        u_shape(-height-cp, tt)
    ), faces = concat(
        [[0,1,2,3,4,5,6,7,8,9]],
        nquads(10,0),
        [[10,20,11],[20,23,12,11],[23,13,12],
         [23,24,14,13],[24,25,15,14],[25,26,16,15],
         [26,17,16],[26,29,18,17],[29,19,18],[29,20,10,19]],
        [[20,30,33,23],[23,33,34,24],[24,34,35,25],
         [25,35,36,26],[26,36,39,29],[29,39,30,20]],
        [[40,41,30],[30,41,42,33],[33,42,43],
         [33,43,44,34],[34,44,45,35],[35,45,46,36],
         [36,46,47],[36,47,48,39],[39,48,49],[39,49,40,30]],
        nquads(10,40),
        [[51,61,62,52],[52,62,63,53],[53,63,64,54],
         [54,64,65,55],[55,65,66,56],[56,66,67,57],
         [57,67,68,58],[58,68,61,51],[51,50,59,58]],
        [[68,67,66,65,64,63,62,61]]
    ), convexity=4);
}

module amp_lip(bot=6.5, top=12, height=46.2) {
    th = 3;
    cp = 2;
    
    polyhedron(points = concat(
        u_shape(0),
        u_shape(-bot),
        u_shape(-bot-th),
        u_shape(-height+top+th),
        u_shape(-height+top),
        u_shape(-height),
        u_shape(-height-cp)
    ), faces = concat(
        [[0,1,2,3,4,5,6,7,8,9]],
        nquads(10,0),
        [[10,20,11],[20,23,12,11],[23,13,12],
         [23,24,14,13],[24,25,15,14],[25,26,16,15],
         [26,17,16],[26,29,18,17],[29,19,18],[29,20,10,19]],
        [[20,30,33,23],[23,33,34,24],[24,34,35,25],
         [25,35,36,26],[26,36,39,29],[29,39,30,20]],
        [[40,41,30],[30,41,42,33],[33,42,43],
         [33,43,44,34],[34,44,45,35],[35,45,46,36],
         [36,46,47],[36,47,48,39],[39,48,49],[39,49,40,30]],
        nquads(10,40),
        [[51,61,62,52],[52,62,63,53],[53,63,64,54],
         [54,64,65,55],[55,65,66,56],[56,66,67,57],
         [57,67,68,58],[58,68,61,51],[51,50,59,58]],
        [[68,67,66,65,64,63,62,61]]
    ), convexity=4);
}

function u_shape(z=0, it=1.5, ov=3, th=2.5, st=2, tl=0.2) = [
    [tl,0,z],
    [tl,ov,z],
    [th,ov,z],
    [th,0,z],
    [th,-st,z],
    [-it-th,-st,z],
    [-it-th,0,z],
    [-it-th,ov,z],
    [-it-tl,ov,z],
    [-it-tl,0,z]
];

module botcover() {
    midh = 60;
    thick = 2;

    difference() {
        union() {
            translate([-85.9,-(midh-4)/2-0.1,0])
                cube([85.9*2,midh-4-0.2,thick]);
            translate([-81,-(midh-4)/2,0])
                cylinder(thick,4.9,4.9, $fn=60);
            translate([-81,(midh-4)/2,0])
                cylinder(thick,4.9,4.9, $fn=60);
            translate([81,-(midh-4)/2,0])
                cylinder(thick,4.9,4.9, $fn=60);
            translate([81,(midh-4)/2,0])
                cylinder(thick,4.9,4.9, $fn=60);
        }
        for (i=[-7:7]) {
            translate([i*8,0,-0.1])
            rline(min(midh-15,(midh-15)*(7.6-abs(i))/5), 5, thick+0.2);
        }
        translate([-81,-(midh-4)/2,-0.1])
            cylinder(thick+0.2,2,2, $fn=60);
        translate([-81,(midh-4)/2,-0.1])
            cylinder(thick+0.2,2,2, $fn=60);
        translate([81,-(midh-4)/2,-0.1])
            cylinder(thick+0.2,2,2, $fn=60);
        translate([81,(midh-4)/2,-0.1])
            cylinder(thick+0.2,2,2, $fn=60);

    }
    translate([0,midh/2-4.4,2]) rotate([90,0,0])
    linear_extrude(height=2) polygon([
        [-75,0],[-70,5],[-62,8],[62,8],[70,5],[75,0]
    ]);
    translate([0,-midh/2+6.4,2]) rotate([90,0,0])
    linear_extrude(height=2) polygon([
        [-75,0],[-70,5],[-62,8],[62,8],[70,5],[75,0]
    ]);
}

module rline(h, w, t, st=60) {
    linear_extrude(height=t) polygon(
        concat([[-w/2,h-w]],
            [for (a=[-90:360/st:90])
                [sin(a)*w/2,h/2-w/2+cos(a)*w/2]],
            [[w/2,-h-w]],
            [for (a=[90:360/st:270])
                [sin(a)*w/2,-h/2+w/2+cos(a)*w/2]]
            ));
}

module speaker_grille(thick=2) {
    
    rw1 = 70/2;
    rw2 = 111/2;
    rw3 = 66/2;
    rh1 = -40;
    rh2 = 16;
    rh3 = 46;
    difference() {
        translate([0,0,0.2]) linear_extrude(height=thick)
        polygon(
            [[rh1,-rw1],[rh2,-rw2],[rh3,-rw3],
            [rh3,rw3],[rh2,rw2],[rh1,rw1]]);
        translate([0,0,0.1])
        cylinder(thick+0.2, 36,36, $fn=80);
        for (i=[45:90:315]) {
            rotate([0,0,i])
            union() {
                translate([42,0,0.1])
                cylinder(thick+0.2, 2.2, 2.2, $fn=32);
            }
        }
    }
    /*
    difference() {
        translate([0,0,-91]) sphere(100, $fn=200);
        translate([0,0,-91]) sphere(98, $fn=200);
        translate([0,0,-98.1]) cube([200,200,200], true);
    }*/
    dome(37.5,100,1.5);
}

module dome(rad, off, thick, stp=20, rs=96) {
    ho = off*cos(20)-2;
    mh = rs*(stp-2);
    mi = rs*(stp*2-2);
    hs = 2;
    
    hsst = floor(rad/hs/2.4);
    
    difference() {
        polyhedron(
        points = concat(
            [for (c=[rad/stp:rad/stp:rad],
                  a=[0:360/rs:360-360/rs])
                [c*sin(a),c*cos(a),off*cos(c/rad*20)-ho]],
            [for (c=[rad:-rad/stp:rad/stp-0.1],
                  a=[0:360/rs:360-360/rs])
                [c*sin(a),c*cos(a),off*cos(c/rad*20)-ho-thick]]
        ),
        faces = concat(
            [for (i=[0:rs:mi],j=[0:rs-1]) each
                [[i+((j+1)%rs),i+j,i+rs+((j+1)%rs)],
                 [i+rs+((j+1)%rs),i+j,i+rs+j]]],
            [for (j=[0:rs-1]) each
                [[mi+rs+((j+1)%rs),mi+rs+j,((j+1)%rs)],
                 [((j+1)%rs),mi+rs+j,j]]]
        ), convexity=4);
        for (i=[1:hsst-1], j=[360/(i*6)-0.1:360/(i*6):360])
            translate([sin(j)*i*rad/hsst, cos(j)*i*rad/hsst,0]) cylinder(off-ho, hs, hs,$fn=24);
    }
}

module dome_h(rad, off, thick, stp=12, rs=36) {
    ho = off*cos(20)-2;
    mh = rs*(stp-2);
    mi = rs*(stp*2-2);
    polyhedron(
        points = concat(
            [for (c=[rad/stp:rad/stp:rad],
                  a=[0:360/rs:360-360/rs])
                [c*sin(a),c*cos(a),off*cos(c/rad*20)-ho]],
            [for (c=[rad:-rad/stp:rad/stp-0.1],
                  a=[0:360/rs:360-360/rs])
                [c*sin(a),c*cos(a),off*cos(c/rad*20)-ho-thick]]
        ),
        faces = concat(
            [for (i=[0:rs*2:mh],j=[0:rs-1]) each
                [[i+((j+1)%rs),i+j,i+rs+((j+1)%rs)],
                 [i+rs+((j+1)%rs),i+j,i+rs+j]]],
            [for (i=[rs:rs*2:mh],
                  j=[(((i/rs)%4)==1?1:0):2:rs-1]) each
                [[i+((j+1)%rs),i+j,i+rs+((j+1)%rs)],
                 [i+rs+((j+1)%rs),i+j,i+rs+j]]],
            [for (i=[mh+rs:mh+rs],j=[0:rs-1]) each
                [[i+((j+1)%rs),i+j,i+rs+((j+1)%rs)],
                 [i+rs+((j+1)%rs),i+j,i+rs+j]]],
            [for (i=[mh+rs*2:rs*2:mi],j=[0:rs-1]) each
                [[i+((j+1)%rs),i+j,i+rs+((j+1)%rs)],
                 [i+rs+((j+1)%rs),i+j,i+rs+j]]],
            [for (i=[mh+rs*3:rs*2:mi],j=[(((i/rs)%4)==1?1:0):2:rs-1]) each
                [[i+((j+1)%rs),i+j,i+rs+((j+1)%rs)],
                 [i+rs+((j+1)%rs),i+j,i+rs+j]]],
            [for (j=[0:rs-1]) each
                [[mi+rs+((j+1)%rs),mi+rs+j,((j+1)%rs)],
                 [((j+1)%rs),mi+rs+j,j]]],
            
            [for (i=[rs:rs*2:mh],
                  j=[(((i/rs)%4)==1?0:1):2:rs-1]) each
                [[i+((j+1)%rs),i+j,mi+rs-i+((j+1)%rs)],
                 [mi+rs-i+((j+1)%rs),i+j,mi+rs-i+j],
                 [i+rs+j,i+rs+((j+1)%rs),mi-i+((j+1)%rs)],
                 [i+rs+j,mi-i+((j+1)%rs),mi-i+j],
                 [i+j,i+rs+j,mi-i+j],
                 [i+j,mi-i+j,mi+rs-i+j],
                 [i+rs+((j+1)%rs),i+((j+1)%rs),mi-i+((j+1)%rs)],
                 [mi-i+((j+1)%rs),i+((j+1)%rs),mi+rs-i+((j+1)%rs)],
            ]]
        ), convexity=4);
}

module conv_3v() {
    wid = 17.1;
    hei = 22.2;
    thi = 1.4;
    thi2 = 4.4;

    translate([-wid/2,0,0]) cube([wid,hei,thi]);
    translate([(-wid+1)/2,0.5,0]) cube([wid-1,hei-1,thi2]);
}

module amp_tda() {
    wid = 33.3;
    hei = 45.1;
    thi = 1.5;
    
    bw = 23;
    bh = 16;
    bt = 25.3;
    
    vd = 15.8/2;
    vt = 2;
    vx = 3.4;
    vy = 40.4;
    vz = 3;
    
    translate([-wid/2,4,0]) cube([wid,hei,thi]);
    translate([-bw/2,0,thi]) cube([bw,bh,bt]);
    translate([vx,vy+4,vz]) cylinder(vt,vd,vd);
}

module esp_ttgo() {
    wid = 31.1;
    hei = 39.2;
    thi = 1.1;
    
    hei2 = 40.5;
    
    thi3 = 4.5;
    wid3 = 16;
    hei3 = 24;
    
    ctt = 2.5;
    ctw = 8;
    cth = 25.2;
    
    et = 4.5;
    
    translate([-wid/2,0,0]) cube([wid,hei,thi]);
    translate([-wid3/2,6.6,0]) cube([wid3, hei3, thi3]);
    translate([-wid3/2,6.6,0]) cube([wid3, hei-6.6, 2]);

    
    translate([-wid/2+ctt,7.7,-ctw]) cube([ctt,cth,ctw]);
    translate([wid/2-ctt,7.7,-ctw]) cube([ctt,cth,ctw]);
    
    translate([-wid/2+0.1,0.1,0]) cube([wid-0.2,5,et]);
}

module powercon() {
    tol = 0.2;
    midh = 60;
    toph = 54.5;
    topt = 60;
    midt = 0;
    holedia = 10;
    pindia = 3.2;
    fdia = 12.5;
    midsidesl = ((midh-toph)/2)/(topt-midt);
    midsidean = atan(midsidesl);
    nutdia = 14 / sqrt(3);
    
    translate([0,-midh/2-tol+holeoff*midsidesl+0.1,holeoff])
    rotate([-midsidean+90,0,0]) union() {
        cylinder(2, fdia/2, fdia/2, $fn=60);
        translate([0,0,-14.4]) cylinder(16.4, holedia/2-tol, holedia/2-tol, $fn=60);
        translate([0,0,-21]) cylinder(23, pindia/2, pindia/2, $fn=60);
        translate([0,0,-7.5]) cylinder(2, nutdia, nutdia, $fn=6);
        translate([0,0,-5.5]) cylinder(1, 7, 7, $fn=60);
    }
}

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
            s_plate(platexy/2, platedia/2, plate);
        }
        translate([0,0,-plate-ring])
        cylinder(plate+ring+0.1,
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
    }
}

module s_plate(d1, d2, h, stp = 10) {
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
