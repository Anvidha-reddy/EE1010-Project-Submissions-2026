// =====================================================================
//  PT100 THERMOMETER ENCLOSURE - single-file OpenSCAD design
//  All lengths in mm.
//  Origin (0,0,0) = outer bottom corner on the shelf side.
//  +x = 84 mm length (USB end at +x), +y = width, +z = up.
//  y-values given for shelf / perfboard / pot / PT100 hole are measured
//  from the INNER y=0 wall (shelf side).
// =====================================================================

/* ---------------- SELECTOR ---------------- */
part = "assembly";      // "assembly" | "base" | "lid" | "print_layout"
                        // ("lid" shows the lid flipped, ready to print)

/* ---------------- GENERAL ---------------- */
$fn = 64;
tol = 0.3;              // print clearance
eps = 0.01;             // tiny overlap for clean boolean ops
asm_lid_lift = 25;      // how far the lid is lifted in "assembly"
show_ghosts = true;     // ghost parts in "assembly"
layout_gap = 15;        // gap between base and lid in "print_layout"

/* ---------------- ARDUINO (in acrylic case) ---------------- */
ard_l = 80;             // along x
ard_w = 66;             // along y
ard_h = 17;
ard_gap = 2;            // clearance to every wall

/* ---------------- PERFBOARD ---------------- */
pcb_l = 70;             // along x
pcb_w = 50;             // along y
pcb_t = 1.6;
pcb_x_off = 7;          // from inner end wall (x=0 side)
pcb_y_off = 0;          // 0 = flush with inner wall on shelf side

/* ---------------- LCD 16x2 ---------------- */
lcd_l = 80;             // along x (overhangs perfboard 5 mm each end)
lcd_w = 36;             // along y
lcd_h = 13;             // CONFIRM: LCD top (bezel) above perfboard top
lcd_body_t = 5;         // CONFIRM: LCD thickness (ghost preview only)

/* ---------------- POTENTIOMETER ---------------- */
pot_shaft_d = 5;        // hole/shaft diameter
pot_y_on_pcb = 47.5;    // along the 50 mm side (45..50 range, centre 47.5)
pot_x_on_pcb = 60;      // CONFIRM: from perfboard end (x low end)
pot_shaft_h = 15;       // CONFIRM: shaft top above perfboard top
pot_body_xy = 10;       // CONFIRM: ghost body footprint (preview only)
pot_body_h = 7;         // CONFIRM: ghost body height (preview only)

/* ---------------- BOX ---------------- */
wall = 2.5;
floor_t = 2.5;
lid_t = 2.5;
out_z = 80;             // total outside height

/* ---------------- SHELF ---------------- */
shelf_t = 3;
shelf_top = 50;         // top surface height (5 cm)
shelf_depth = 50;       // from inner y=0 wall; strip beyond stays open

/* ---------------- PERFBOARD LUGS (4x, L-shaped) ---------------- */
lug_t = 1.2;            // lug thickness
lug_h = 2.5;            // lug height above shelf top
lug_clear = tol;        // clearance to perfboard edge
lug_len = 8;            // CONFIRM: length of each L leg

/* ---------------- LID FIXING: FRICTION-FIT LIP (no screws) ---------------- */
lip_h = 4;              // CONFIRM: how deep the lip goes into the box
lip_t = 1.5;            // lip wall thickness
lip_clear = 0.15;       // CONFIRM: gap per side. Tight? raise to 0.2-0.25. Loose? lower to 0.1

/* ---------------- LCD WINDOW + COLLAR ---------------- */
win_l = 70;             // window along x
win_w = 22;             // window along y
col_l = 74;             // collar outer along x
col_w = 26;             // collar outer along y

/* ---------------- POT HOLE / WELL IN LID ---------------- */
pot_hole_lid_d = 5.4;   // through-hole in lid / well floor
well_d = 22;            // recess width
well_wall = 2;          // CONFIRM: wall of the cup that hangs below the lid
well_floor_t = 2;       // CONFIRM: floor thickness of the well
well_floor_above_shaft = 1;  // CONFIRM: well floor TOP is this far above shaft top
                             // (use a negative value if the shaft should poke through)

/* ---------------- CUTOUTS ---------------- */
// USB (+x end wall)
usb_w = 12;
usb_tol = 0.5;          // added to width
usb_h = 12;
usb_from_ard_left = 15; // left edge, seen from outside (low-y side)
usb_z = 6.5;            // CONFIRM: bottom of cutout

// Power jack (+x end wall) - OFF by default
jack_enable = false;
jack_w = 9;             // CONFIRM
jack_tol = 0.5;
jack_h = 11;            // CONFIRM
jack_from_ard_left = 40;// CONFIRM: left edge from Arduino left edge
jack_z = 6.5;           // CONFIRM: bottom of cutout

// PT100 cable hole (x = 0 end wall)
pt100_d = 7;           // 0.7 cm
pt100_y = 25;           // CONFIRM: from inner y=0 wall
pt100_z = 56;           // CONFIRM

// =====================================================================
//  DERIVED VALUES (do not edit)
// =====================================================================
in_x = ard_l + 2*ard_gap;              // 84
in_y = ard_w + 2*ard_gap;              // 70
in_z = out_z - floor_t - lid_t;        // 75
out_x = in_x + 2*wall;                 // 89
out_y = in_y + 2*wall;                 // 75
lid_z0 = out_z - lid_t;                // lid underside = top of walls

ard_x0 = wall + ard_gap;
ard_y0 = wall + ard_gap;

pcb_x0 = wall + pcb_x_off;
pcb_y0 = wall + pcb_y_off;
pcb_top = shelf_top + pcb_t;

lcd_x0 = pcb_x0 + (pcb_l - lcd_l)/2;
lcd_y0 = pcb_y0;
lcd_top_z = pcb_top + lcd_h;
lcd_cx = lcd_x0 + lcd_l/2;
lcd_cy = lcd_y0 + lcd_w/2;

pot_x = pcb_x0 + pot_x_on_pcb;
pot_y = pcb_y0 + pot_y_on_pcb;
pot_top_z = pcb_top + pot_shaft_h;

use_well = pot_top_z < lid_z0;
well_floor_top = pot_top_z + well_floor_above_shaft;
well_floor_bot = well_floor_top - well_floor_t;

// =====================================================================
//  BASE
// =====================================================================
module box_shell() {
    difference() {
        cube([out_x, out_y, lid_z0]);
        translate([wall, wall, floor_t]) cube([in_x, in_y, lid_z0]);
    }
}

module shelf() {
    // spans full inner length, from inner y=0 wall to shelf_depth; strip beyond is open
    translate([wall - 0.5, wall - 0.5, shelf_top - shelf_t])
        cube([in_x + 1, shelf_depth + 0.5, shelf_t]);
}

// one L-shaped lug, perfboard corner at local (0,0), board toward +x,+y
module lug_local(far) {
    translate([-lug_clear - lug_t, -lug_clear - lug_t, 0])
        cube([lug_t + lug_clear + lug_len, lug_t, lug_h]);      // leg along x
    translate([-lug_clear - lug_t, -lug_clear - lug_t, 0])
        cube([lug_t, lug_t + lug_clear + lug_len, lug_h]);      // leg along y
    if (far)  // foot: continues the shelf edge so the lug is never floating
        translate([-lug_clear - lug_t, -lug_clear - lug_t, -shelf_t])
            cube([lug_t + lug_clear + lug_len, lug_t + lug_clear + eps, shelf_t + eps]);
}

module lugs() {
    for (ix=[0,1], iy=[0,1])
        translate([pcb_x0 + ix*pcb_l, pcb_y0 + iy*pcb_w, shelf_top])
            mirror([ix,0,0]) mirror([0,iy,0]) lug_local(iy==1);
}

module usb_cut() {
    translate([out_x - wall - 1, ard_y0 + usb_from_ard_left, usb_z])
        cube([wall + 2, usb_w + usb_tol, usb_h]);
}

module jack_cut() {
    translate([out_x - wall - 1, ard_y0 + jack_from_ard_left, jack_z])
        cube([wall + 2, jack_w + jack_tol, jack_h]);
}

module pt100_cut() {
    translate([-1, wall + pt100_y, pt100_z])
        rotate([0,90,0]) cylinder(d=pt100_d, h=wall + 2);
}

module base() {
    difference() {
        union() {
            box_shell();
            shelf();
            lugs();
        }
        usb_cut();
        if (jack_enable) jack_cut();
        pt100_cut();
    }
}

// =====================================================================
//  LID (modelled in its assembled position, underside at z = lid_z0)
// =====================================================================
module collar() {
    translate([lcd_cx - col_l/2, lcd_cy - col_w/2, lcd_top_z])
        cube([col_l, col_w, lid_z0 - lcd_top_z + 0.1]);
}

module well_tube() {
    translate([pot_x, pot_y, well_floor_bot])
        cylinder(d = well_d + 2*well_wall, h = lid_z0 - well_floor_bot + 0.1);
}

// friction lip: a rectangular ring that pushes into the box opening
module lid_lip() {
    translate([wall + lip_clear, wall + lip_clear, lid_z0 - lip_h])
        difference() {
            cube([in_x - 2*lip_clear, in_y - 2*lip_clear, lip_h + 0.1]);
            translate([lip_t, lip_t, -1])
                cube([in_x - 2*lip_clear - 2*lip_t,
                      in_y - 2*lip_clear - 2*lip_t, lip_h + 2]);
        }
}

module lid() {
    difference() {
        union() {
            translate([0,0,lid_z0]) cube([out_x, out_y, lid_t]);
            collar();
            lid_lip();
            if (use_well) well_tube();
        }
        // LCD window (also opens the collar)
        translate([lcd_cx - win_l/2, lcd_cy - win_w/2, lcd_top_z - 1])
            cube([win_l, win_w, out_z - lcd_top_z + 2]);
        // pot hole
        if (use_well) {
            translate([pot_x, pot_y, well_floor_top])
                cylinder(d=well_d, h=out_z - well_floor_top + 1);
            translate([pot_x, pot_y, well_floor_bot - 1])
                cylinder(d=pot_hole_lid_d, h=well_floor_t + 2);
        } else {
            translate([pot_x, pot_y, lid_z0 - 1])
                cylinder(d=pot_hole_lid_d, h=lid_t + 2);
        }
    }
}

// lid flipped (outer face on the bed), corner at origin
module lid_flipped() {
    translate([0, out_y, lid_t])
        rotate([180,0,0])
            translate([0,0,-lid_z0]) lid();
}

// =====================================================================
//  GHOST PARTS (fit check only)
// =====================================================================
module ghosts() {
    // Arduino in acrylic case
    color("lightskyblue", 0.30)
        translate([ard_x0, ard_y0, floor_t]) cube([ard_l, ard_w, ard_h]);
    // perfboard
    color("seagreen", 0.50)
        translate([pcb_x0, pcb_y0, shelf_top]) cube([pcb_l, pcb_w, pcb_t]);
    // LCD
    color("orange", 0.45)
        translate([lcd_x0, lcd_y0, lcd_top_z - lcd_body_t])
            cube([lcd_l, lcd_w, lcd_body_t]);
    // potentiometer
    color("crimson", 0.60) {
        translate([pot_x - pot_body_xy/2, pot_y - pot_body_xy/2, pcb_top])
            cube([pot_body_xy, pot_body_xy, pot_body_h]);
        translate([pot_x, pot_y, pcb_top])
            cylinder(d=pot_shaft_d, h=pot_shaft_h);
    }
}

// =====================================================================
//  OUTPUT
// =====================================================================
module assembly() {
    color("silver", 0.85) base();
    color("gainsboro", 0.85) translate([0,0,asm_lid_lift]) lid();
    if (show_ghosts) ghosts();
}

module print_layout() {
    base();
    translate([out_x + layout_gap, 0, 0]) lid_flipped();
}

if (part == "assembly")          assembly();
else if (part == "base")         base();
else if (part == "lid")          lid_flipped();
else if (part == "print_layout") print_layout();
else echo("Unknown part! Use assembly | base | lid | print_layout");
