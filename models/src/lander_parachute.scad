$fn = 20;
num_of_cords = 6;

// Main geometry
rotate([-90, 0, 0]) union() {
    canopy();
    for (i = [0 : 1 : num_of_cords-1])
        rotate([35.0, 0.0, 360 * i / num_of_cords]) cord();
}

module canopy() {
    difference() {
        sphere(4.0);
        sphere(3.96);
        translate([0.0, 0.0, -0.8]) cube(8.0, center=true);
        cylinder(d=0.8, h=4.0);
    }
}

module cord() {
    $fn = 6;
    cylinder(d=0.02, h=1.965*2);
}
