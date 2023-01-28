$fn=20;
tol=0.001;
num_of_legs=4;

// Main geometry
rotate([-90, 0, 0]) translate([0, 0, -0.3]) {
    difference() {
        body();
        
    }
    rotate([-10, 0, 0]) translate([0, 0, -0.1]) {
        difference() {
            engine_nozzle();
            translate([0, 0, -0.8]) cube(1.0);
            translate([-0.9, -0.1, -0.8]) cube(1.0);
        }
    }
    for (i = [0 : 1 : num_of_legs-3]) {
        angle_offset = [10, -60, 30];
        rotate([0, angle_offset[i], 360 * i / num_of_legs]) translate([0, 0.9, -0.4]) leg();
    }
}

module body() {
    bottom = 1.0;
    top = 0.5;
    
    difference() {
        cylinder(r1=bottom, r2=top, h=1.0);
        translate([0, 0, -tol]) {
            cylinder(r1=bottom*0.6, r2=bottom*0.4, h=0.4);
        }
    }
}

module engine_nozzle() {
    $fn = 10;
    bottom = 0.3;
    middle = 0.26;
    top = 0.15;
    t = 0.04;
    h1 = 0.3;
    h2 = 0.2;
    
    translate([0, 0, h2]) difference() {
        cylinder(r1=middle, r2=top, h=h1);
        translate([0, 0, -tol]) cylinder(r1=middle-t, r2=top-t, h=h1+2*tol);
    }
    
    difference() {
        cylinder(r1=bottom, r2=middle, h=h2);
        translate([0, 0, -tol]) cylinder(r1=bottom-t, r2=middle-t, h=h2+2*tol);
    }
}

module leg() {
    $fn = 6;
    cylinder(r=0.2, h=0.05);
    translate([0, 0, 0.01]) rotate([5, 0, 0]) cylinder(d=0.1, h=0.5);
    translate([0, 0, 0.02]) rotate([20, 0, 0]) cylinder(d=0.1, h=0.5);
}