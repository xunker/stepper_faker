/*
  Mount for uln2003A driver boards, common with 28BYJ-48 stepper motors.
*/

driver_board_hole_d = 2.8;
driver_board_standoff_d = driver_board_hole_d*2;
driver_board_standoff_h = 2.30; // how high the board should be off the bottom
driver_board_w = 31.0; // how much larger than board dimensions to make the base; //31.68 actually
driver_board_l = 36; // 35.2, actually
driver_board_thickness = 1.6; // 1.51 actually

// Thickness of bottom. Makes sense to be a multiple of your layer height.
floor_thickness = 1.2;

// Spacing between holes on X/Y planes
driver_board_hole_x_spacing = 25.5;
driver_board_hole_y_spacing = 30.5;

boards = 3; // number of boards to fit
board_spacing = 2; // spacing between boards, if multiple


module standoffs() {
  module standoff() {
    cylinder(d=driver_board_standoff_d, h=driver_board_standoff_h, $fn=8);
    translate([0,0,driver_board_standoff_h]) cylinder(d=driver_board_hole_d, h=driver_board_thickness, $fn=16);
  }

  translate([0,0,floor_thickness]) {
    translate([0, 0, 0]) standoff();
    translate([driver_board_hole_x_spacing, 0, 0]) standoff();
    translate([0, driver_board_hole_y_spacing, 0]) standoff();
    translate([driver_board_hole_x_spacing, driver_board_hole_y_spacing, 0]) standoff();
  }
}

module floor() {

  module floor_corner() {
    cylinder(d=driver_board_standoff_d, h=floor_thickness, $fn=8);
  }

  hull() {
    translate([0, 0, 0]) floor_corner();
    translate([driver_board_hole_x_spacing, 0, 0]) floor_corner();
    translate([0, driver_board_hole_y_spacing, 0]) floor_corner();
    translate([driver_board_hole_x_spacing, driver_board_hole_y_spacing, 0]) floor_corner();
  }
}

hull() {
  for (f = [0:boards-1]) {
    translate([(driver_board_w*f)+(board_spacing*f), 0, 0]) floor();
  }
}

for (f = [0:boards-1]) {
  translate([(driver_board_w*f)+(board_spacing*f), 0, 0]) standoffs();
}
