/* ======================================================================
   MOBILE CHARGER CASE — v2 (IIT Hyderabad, EE ICDT)
   ----------------------------------------------------------------------
   Layout (X = length ~8cm, Y = width ~5cm, Z = height):
     - Transformer chamber: fixed, at the bottom.
     - Sliding TRAY: same footprint as the perfboard (7.3 x 5 x 0.15),
       sits on top of the transformer chamber, slides OUT through the
       front long wall (Y = 0) — pull the tab, the whole tray comes out.
     - Perfboard chamber: open top, sits above the tray.
     - Because the perfboard (7.3) is shorter than the transformer (8),
       the tray leaves a ~0.8cm gap along one side of the box — this is
       reused as the routing gap for the 2 thin secondary wires running
       from the transformer up to the perfboard. No extra hole needed.
     - A separate keyhole+channel on the BACK wall carries the thick
       incoming AC mains wire down from the top of the box to the
       transformer.
     - Lid: friction-fit skirt over the whole perfboard-chamber's
       outside walls, with its own USB / wire-channel cutouts so both
       stay usable with the lid on.

   Units: cm. ASSUMPTIONS ARE FLAGGED — check the echo() output and the
   comments marked "ASSUMPTION".
   ====================================================================== */

// ---------- WHICH PART TO SHOW / EXPORT ----------
PART = "assembly";   // "assembly" | "box" | "tray" | "lid"

// ================= GIVEN MEASUREMENTS =================
transformer_l = 8;     transformer_w = 5;     transformer_h = 4.8;
perfboard_l   = 7.3;   perfboard_w   = 5;     perfboard_h   = 0.4;

usb_w = 1.7;   usb_h = 1.0;   // ASSUMPTION: you wrote "1.7*10.8" — treated the
                              // trailing ".8" as a typo and kept 1.7 x 1 (matches
                              // every earlier message). Tell me the real number
                              // if this is wrong — it's one variable to fix.

clear_above_pcb = 2.5;        // air gap above the perfboard's top surface

tray_t = 0.15;                // 1.5mm, as you specified

// ================= BUFFERS =================
wall          = 0.2;   // 2mm walls — strong enough to carry the transformer
fit_buffer    = 0.2;   // static footprint clearance (2mm, "don't overdo it")
near_gap      = 0.1;   // small sliding clearance kept on the tray's flush side
vert_clear    = 0.1;   // total Z clearance for the tray inside its slot
lid_clearance = 0.15;  // total clearance for the lid's friction-fit skirt
hole_margin   = 0.15;  // extra margin on the LID's own USB/wire cutouts, so
                        // print tolerance never blocks them

// ================= DERIVED =================
// interior sized to the BIGGER footprint (the transformer, 8 x 5)
inner_L = max(transformer_l, perfboard_l) + fit_buffer;   // 8.2
inner_W = max(transformer_w, perfboard_w) + fit_buffer;   // 5.2
outer_L = inner_L + 2*wall;                               // 8.6
outer_W = inner_W + 2*wall;                               // 5.6

trans_h   = transformer_h + fit_buffer;                   // 5.0
trans_top = wall + trans_h;                               // 5.2

slot_h      = tray_t + vert_clear;                        // 0.25
slot_bottom = trans_top - vert_clear/2;                   // 5.15
slot_top    = slot_bottom + slot_h;                       // 5.40
perf_bottom = slot_top;                                    // floor of perfboard chamber

perf_chamber_h = perfboard_h + clear_above_pcb;           // 2.9
outer_H        = perf_bottom + perf_chamber_h;            // 8.30

// ---- tray (slides along Y, out through the Y=0 wall) ----
gap_total_x = inner_L - perfboard_l;                      // 0.9 leftover next to transformer
tray_x0 = wall + near_gap;                                // 0.3
tray_x1 = tray_x0 + perfboard_l;                          // 7.6  (far_gap = 0.8 beyond this)
blade_y = inner_W + wall;                                 // 5.4  reaches flush with outer front face
handle_w = 3.0;   handle_l = 1.5;                         // grip tab, centered in X

// ---- USB opening (short wall at X = outer_L) ----
usb_y0 = 3.0;                       // ASSUMPTION: "3cm from the longer (Y-normal) wall"
usb_z0 = perf_bottom + 0.5;         // ASSUMPTION: "0.5cm" read as height above the tray surface
                                    // -> re-check against your photo before printing (you flagged
                                    //    this hole's position as the one to double-check)

// ---- input-wire channel (back wall, Y = outer_W, inside the 0.8cm gap column) ----
wire_w        = 0.6;
wire_x0       = tray_x1 + (gap_total_x - near_gap - wire_w)/2;  // centered in the gap column
slit_z_top    = slot_bottom - 0.5;      // "the slit, 0.5cm below the sliding part"
keyhole_rect_h= 0.2;
keyhole_bottom= slit_z_top - keyhole_rect_h - wire_w/2;  // rectangle(0.2) + semicircle radius(0.3) = 0.5 total

echo("outer box L,W,H =", outer_L, outer_W, outer_H);
echo("USB opening Y,Z start =", usb_y0, usb_z0, " size", usb_w, "x", usb_h);
echo("wire gap column: X", tray_x1, "to", wall+inner_L, " width", inner_L - perfboard_l - near_gap);

// ======================================================================
//  SHARED CUTTERS
// ======================================================================
module usb_cut(margin=0) {
    translate([outer_L - wall - 0.1, usb_y0 - margin/2, usb_z0 - margin/2])
        cube([wall + 0.2, usb_w + margin, usb_h + margin]);
}

module tray_slot_cut() {
    translate([wall - 0.1, -0.1, slot_bottom])
        cube([inner_L + 0.2, wall + 0.2, slot_h]);
}

// the whole input-wire feature: straight channel from the top down to the
// slit, then a rectangle+semicircle "keyhole" for the last 0.5cm
module wire_channel_cut(margin=0) {
    y0 = inner_W + wall - 0.1;      // through the back (Y = outer_W) wall
    y1 = outer_W + 0.1;
    // upper straight channel
    translate([wire_x0 - margin/2, y0, slit_z_top])
        cube([wire_w + margin, y1 - y0, outer_H - slit_z_top + 0.2]);
    // keyhole rectangle part
    translate([wire_x0 - margin/2, y0, slit_z_top - keyhole_rect_h])
        cube([wire_w + margin, y1 - y0, keyhole_rect_h]);
    // keyhole rounded bottom
    translate([wire_x0 + wire_w/2, (y0+y1)/2, slit_z_top - keyhole_rect_h])
        rotate([90,0,0])
            cylinder(h = y1 - y0, r = wire_w/2 + margin/2, center = true, $fn = 32);
}

// ======================================================================
//  MAIN BOX
// ======================================================================
module main_box() {
    difference() {
        cube([outer_L, outer_W, outer_H]);
        translate([wall, wall, wall])
            cube([inner_L, inner_W, outer_H - wall + 1]);   // hollow, open top
        tray_slot_cut();
        usb_cut();
        wire_channel_cut();
    }
}

// ======================================================================
//  SLIDING TRAY
// ======================================================================
// Local Y convention: Y=0 is the OUTER tip of the handle (outside the case);
// Y increases going INTO the box. Blade spans [handle_l, handle_l+blade_y].
module tray() {
    union() {
        translate([perfboard_l/2 - handle_w/2, 0, 0])
            cube([handle_w, handle_l, tray_t]);                // grip tab, outside the case
        translate([0, handle_l, 0])
            cube([perfboard_l, blade_y, tray_t]);               // blade: matches perfboard footprint
    }
}

// ======================================================================
//  LID
// ======================================================================
module lid() {
    skirt_depth  = outer_H - perf_bottom;
    skirt_wall   = 0.2;
    lid_top_t    = 0.2;
    so_L = outer_L + lid_clearance + 2*skirt_wall;
    so_W = outer_W + lid_clearance + 2*skirt_wall;
    si_L = outer_L + lid_clearance;
    si_W = outer_W + lid_clearance;

    union() {
        translate([-skirt_wall - lid_clearance/2, -skirt_wall - lid_clearance/2, 0])
            cube([so_L, so_W, lid_top_t]);

        translate([0, 0, -skirt_depth])
            difference() {
                translate([-skirt_wall - lid_clearance/2, -skirt_wall - lid_clearance/2, 0])
                    cube([so_L, so_W, skirt_depth + lid_top_t]);
                translate([-lid_clearance/2, -lid_clearance/2, -0.1])
                    cube([si_L, si_W, skirt_depth + 0.1]);
                translate([0,0,skirt_depth]) usb_cut(hole_margin);
                translate([0,0,skirt_depth]) wire_channel_cut(hole_margin);
            }
    }
}

// ======================================================================
//  RENDER
// ======================================================================
if (PART == "assembly") {
    color("SteelBlue")      main_box();
    color("Orange")         translate([tray_x0, -handle_l, slot_bottom]) tray();
    color("LightGray", 0.5) translate([0, 0, outer_H]) lid();
} else if (PART == "box") {
    main_box();
} else if (PART == "tray") {
    tray();
} else if (PART == "lid") {
    translate([0, 0, outer_H - perf_bottom]) lid();   // sits flat, right-side up, for printing
}
