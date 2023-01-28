$fn=20;
tol=0.001;
num_of_legs=4;

// Main geometry
rotate([-90, 0, 0]) translate([0, -0.1, -0.1]) {
    intersection() {
        engine_nozzle();
        union() {
            translate([0, 0, -0.8]) cube(1.0);
            translate([-0.9, -0.1, -0.8]) cube(1.0);
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
