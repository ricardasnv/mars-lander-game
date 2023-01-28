// Main geometry
leg();

module leg() {
    $fn = 6;
    cylinder(r=0.2, h=0.05);
    translate([0, 0, 0.01]) rotate([5, 0, 0]) cylinder(d=0.1, h=0.5);
    translate([0, 0, 0.02]) rotate([20, 0, 0]) cylinder(d=0.1, h=0.5);
}