import os, json, subprocess

def create_board():
    pcb_path = r"c:\Users\Lukas\Documents\KITemp\BonbridgeESP32\hardware\BonbridgeESP32_CustomBoard.kicad_pcb"
    pro_path = r"c:\Users\Lukas\Documents\KITemp\BonbridgeESP32\hardware\BonbridgeESP32_CustomBoard.kicad_pro"

    # Configure .kicad_pro with appropriate rules
    pro_data = {
        "meta": {"filename": "BonbridgeESP32_CustomBoard.kicad_pro", "version": 1},
        "board": {
            "design_settings": {
                "rules": {
                    "max_error": 0.005,
                    "min_clearance": 0.15,
                    "min_connection": 0.0,
                    "min_copper_edge_clearance": 0.35,
                    "min_hole_clearance": 0.25,
                    "min_hole_to_hole": 0.25,
                    "min_silk_clearance": 0.0,
                    "min_text_height": 0.5,
                    "min_text_thickness": 0.08,
                    "min_track_width": 0.2
                },
                "rule_severities": {
                    "annular_width": "error",
                    "clearance": "error",
                    "copper_edge_clearance": "error",
                    "shorting_items": "error",
                    "solder_mask_bridge": "error",
                    "lib_footprint_mismatch": "ignore",
                    "lib_footprint_issues": "ignore",
                    "missing_footprint": "ignore",
                    "silk_over_copper": "warning",
                    "text_height": "ignore"
                }
            }
        },
        "net_settings": {
            "classes": [
                {
                    "name": "Default",
                    "clearance": 0.2,
                    "track_width": 0.25,
                    "via_diameter": 0.6,
                    "via_drill": 0.3
                },
                {
                    "name": "Power",
                    "clearance": 0.25,
                    "track_width": 0.5,
                    "via_diameter": 0.8,
                    "via_drill": 0.4
                }
            ]
        }
    }
    with open(pro_path, "w", encoding="utf-8") as f:
        json.dump(pro_data, f, indent=2)

    X0, Y0 = 100.0, 100.0
    W, H = 55.0, 25.0
    R = 2.0

    lines = [
        '(kicad_pcb (version 20221018) (generator pcbnew)',
        '  (general (thickness 1.6))',
        '  (paper "A4")',
        '  (layers',
        '    (0 "F.Cu" signal)',
        '    (31 "B.Cu" signal)',
        '    (32 "B.Adhes" user "B.Adhesive")',
        '    (33 "F.Adhes" user "F.Adhesive")',
        '    (34 "B.Paste" user)',
        '    (35 "F.Paste" user)',
        '    (36 "B.SilkS" user "B.Silkscreen")',
        '    (37 "F.SilkS" user "F.Silkscreen")',
        '    (38 "B.Mask" user)',
        '    (39 "F.Mask" user)',
        '    (40 "Dwgs.User" user "User.Drawings")',
        '    (41 "Cmts.User" user "User.Comments")',
        '    (42 "Eco1.User" user "User.Eco1")',
        '    (43 "Eco2.User" user "User.Eco2")',
        '    (44 "Edge.Cuts" user)',
        '    (45 "Margin" user)',
        '    (46 "B.CrtYd" user "B.Courtyard")',
        '    (47 "F.CrtYd" user "F.Courtyard")',
        '    (48 "B.Fab" user)',
        '    (49 "F.Fab" user)',
        '  )',
        '  (setup',
        '    (pad_to_mask_clearance 0.05)',
        '  )',
        '  (net 0 "")',
        '  (net 1 "GND")',
        '  (net 2 "+24V")',
        '  (net 3 "+5V")',
        '  (net 4 "+3V3")',
        '  (net 5 "USB_DM")',
        '  (net 6 "USB_DP")',
        '  (net 7 "ETH_SCLK")',
        '  (net 8 "ETH_MOSI")',
        '  (net 9 "ETH_MISO")',
        '  (net 10 "ETH_CS")',
        '  (net 11 "ETH_INT")',
        '  (net 12 "ETH_RST")',
        '',
        f'  (gr_line (start {X0+R} {Y0}) (end {X0+W-R} {Y0}) (layer "Edge.Cuts") (width 0.15))',
        f'  (gr_arc (start {X0+W-R} {Y0}) (mid {X0+W-R+R*0.707} {Y0+R-R*0.707}) (end {X0+W} {Y0+R}) (layer "Edge.Cuts") (width 0.15))',
        f'  (gr_line (start {X0+W} {Y0+R}) (end {X0+W} {Y0+H-R}) (layer "Edge.Cuts") (width 0.15))',
        f'  (gr_arc (start {X0+W} {Y0+H-R}) (mid {X0+W-R+R*0.707} {Y0+H-R+R*0.707}) (end {X0+W-R} {Y0+H}) (layer "Edge.Cuts") (width 0.15))',
        f'  (gr_line (start {X0+W-R} {Y0+H}) (end {X0+R} {Y0+H}) (layer "Edge.Cuts") (width 0.15))',
        f'  (gr_arc (start {X0+R} {Y0+H}) (mid {X0+R-R*0.707} {Y0+H-R+R*0.707}) (end {X0} {Y0+H-R}) (layer "Edge.Cuts") (width 0.15))',
        f'  (gr_line (start {X0} {Y0+H-R}) (end {X0} {Y0+R}) (layer "Edge.Cuts") (width 0.15))',
        f'  (gr_arc (start {X0} {Y0+R}) (mid {X0+R-R*0.707} {Y0+R-R*0.707}) (end {X0+R} {Y0}) (layer "Edge.Cuts") (width 0.15))',
        ''
    ]

    # 4 Mounting Holes (M2.5, 2.7mm drill, 3.8mm pad, distance to edge 2.5mm -> copper to edge is 0.6mm!)
    holes = [
        ("H1", X0 + 2.5, Y0 + 2.5),
        ("H2", X0 + W - 2.5, Y0 + 2.5),
        ("H3", X0 + 2.5, Y0 + H - 2.5),
        ("H4", X0 + W - 2.5, Y0 + H - 2.5)
    ]
    for ref, hx, hy in holes:
        lines.extend([
            f'  (footprint "MountingHole:MountingHole_2.7mm_M2.5_Pad_TopBottom" (layer "F.Cu")',
            f'    (at {hx} {hy})',
            f'    (fp_text reference "{ref}" (at 0 0) (layer "F.Fab") (effects (font (size 0.8 0.8) (thickness 0.12))))',
            f'    (fp_text value "M2.5" (at 0 0) (layer "F.Fab") (effects (font (size 0.6 0.6) (thickness 0.1))))',
            f'    (pad "" thru_hole circle (at 0 0) (size 3.8 3.8) (drill 2.7) (layers *.Cu *.Mask) (net 1 "GND"))',
            '  )'
        ])

    def add_smd_0603(ref, val, x, y, rot=0, layer="F.Cu", net1=0, net2=0):
        fab_layer = "F.Fab" if layer == "F.Cu" else "B.Fab"
        paste_layer = "F.Paste" if layer == "F.Cu" else "B.Paste"
        mask_layer = "F.Mask" if layer == "F.Cu" else "B.Mask"
        mirror = " (justify mirror)" if layer == "B.Cu" else ""
        lines.extend([
            f'  (footprint "Resistor_SMD:R_0603_1608Metric" (layer "{layer}")',
            f'    (at {x} {y} {rot})',
            f'    (fp_text reference "{ref}" (at 0 0 {rot}) (layer "{fab_layer}") (effects (font (size 0.5 0.5) (thickness 0.08)){mirror}))',
            f'    (fp_text value "{val}" (at 0 0 {rot}) (layer "{fab_layer}") (effects (font (size 0.4 0.4) (thickness 0.06)){mirror}))',
            f'    (fp_rect (start -0.8 -0.4) (end 0.8 0.4) (layer "{fab_layer}") (stroke (width 0.1) (type solid)) (fill no))',
            f'    (pad "1" smd roundrect (at -0.75 0 {rot}) (size 0.7 0.8) (layers "{layer}" "{paste_layer}" "{mask_layer}") (roundrect_rratio 0.25) (net {net1} ""))',
            f'    (pad "2" smd roundrect (at 0.75 0 {rot}) (size 0.7 0.8) (layers "{layer}" "{paste_layer}" "{mask_layer}") (roundrect_rratio 0.25) (net {net2} ""))',
            '  )'
        ])

    def add_smd_0805(ref, val, x, y, rot=0, layer="F.Cu", net1=0, net2=0):
        fab_layer = "F.Fab" if layer == "F.Cu" else "B.Fab"
        paste_layer = "F.Paste" if layer == "F.Cu" else "B.Paste"
        mask_layer = "F.Mask" if layer == "F.Cu" else "B.Mask"
        mirror = " (justify mirror)" if layer == "B.Cu" else ""
        lines.extend([
            f'  (footprint "Capacitor_SMD:C_0805_2012Metric" (layer "{layer}")',
            f'    (at {x} {y} {rot})',
            f'    (fp_text reference "{ref}" (at 0 0 {rot}) (layer "{fab_layer}") (effects (font (size 0.5 0.5) (thickness 0.08)){mirror}))',
            f'    (fp_text value "{val}" (at 0 0 {rot}) (layer "{fab_layer}") (effects (font (size 0.4 0.4) (thickness 0.06)){mirror}))',
            f'    (fp_rect (start -1.0 -0.6) (end 1.0 0.6) (layer "{fab_layer}") (stroke (width 0.1) (type solid)) (fill no))',
            f'    (pad "1" smd roundrect (at -0.85 0 {rot}) (size 0.8 1.0) (layers "{layer}" "{paste_layer}" "{mask_layer}") (roundrect_rratio 0.25) (net {net1} ""))',
            f'    (pad "2" smd roundrect (at 0.85 0 {rot}) (size 0.8 1.0) (layers "{layer}" "{paste_layer}" "{mask_layer}") (roundrect_rratio 0.25) (net {net2} ""))',
            '  )'
        ])

    def add_smd_1206(ref, val, x, y, rot=0, layer="F.Cu", net1=0, net2=0):
        fab_layer = "F.Fab" if layer == "F.Cu" else "B.Fab"
        paste_layer = "F.Paste" if layer == "F.Cu" else "B.Paste"
        mask_layer = "F.Mask" if layer == "F.Cu" else "B.Mask"
        lines.extend([
            f'  (footprint "Capacitor_SMD:C_1206_3216Metric" (layer "{layer}")',
            f'    (at {x} {y} {rot})',
            f'    (fp_text reference "{ref}" (at 0 0 {rot}) (layer "{fab_layer}") (effects (font (size 0.6 0.6) (thickness 0.1))))',
            f'    (fp_text value "{val}" (at 0 0 {rot}) (layer "{fab_layer}") (effects (font (size 0.5 0.5) (thickness 0.08))))',
            f'    (fp_rect (start -1.6 -0.8) (end 1.6 0.8) (layer "{fab_layer}") (stroke (width 0.1) (type solid)) (fill no))',
            f'    (pad "1" smd roundrect (at -1.3 0 {rot}) (size 0.9 1.4) (layers "{layer}" "{paste_layer}" "{mask_layer}") (roundrect_rratio 0.25) (net {net1} ""))',
            f'    (pad "2" smd roundrect (at 1.3 0 {rot}) (size 0.9 1.4) (layers "{layer}" "{paste_layer}" "{mask_layer}") (roundrect_rratio 0.25) (net {net2} ""))',
            '  )'
        ])

    def add_smd_button(ref, val, x, y):
        lines.extend([
            f'  (footprint "Button_Switch_SMD:SW_Push_SPST_NO_Alps_SKRK" (layer "F.Cu")',
            f'    (at {x} {y})',
            f'    (fp_text reference "{ref}" (at 0 0) (layer "F.Fab") (effects (font (size 0.6 0.6) (thickness 0.1))))',
            f'    (fp_text value "{val}" (at 0 0) (layer "F.Fab") (effects (font (size 0.5 0.5) (thickness 0.08))))',
            f'    (fp_rect (start -1.9 -1.4) (end 1.9 1.4) (layer "F.Fab") (stroke (width 0.12) (type solid)) (fill no))',
            f'    (pad "1" smd rect (at -1.75 0) (size 0.8 1.1) (layers "F.Cu" "F.Paste" "F.Mask") (net 1 "GND"))',
            f'    (pad "2" smd rect (at 1.75 0) (size 0.8 1.1) (layers "F.Cu" "F.Paste" "F.Mask"))',
            '  )'
        ])

    # 1. RJ45 MagJack on BOTTOM LONG SIDE (3.0mm Overhang past Y = 125.0 -> front at 128.0)
    # Center: X = 113.5, Y = 117.5. Width: 16mm (X in [105.5, 121.5]).
    lines.extend([
        f'  (footprint "Connector_RJ:RJ45_Amphenol_RJMG1BD3B8K1ANR" (layer "F.Cu")',
        f'    (at {X0 + 13.5} {Y0 + H - 7.5})',
        f'    (fp_text reference "J1" (at 0 -8.5) (layer "F.SilkS") (effects (font (size 0.8 0.8) (thickness 0.12))))',
        f'    (fp_text value "RJ45 (+3mm)" (at 0 0) (layer "F.Fab") (effects (font (size 0.7 0.7) (thickness 0.1))))',
        f'    (fp_rect (start -8.0 -11.0) (end 8.0 10.5) (layer "F.Fab") (stroke (width 0.15) (type solid)) (fill no))',
        f'    (fp_line (start -8.0 7.5) (end 8.0 7.5) (layer "F.Fab") (stroke (width 0.15) (type solid)))', # Board edge marker on Fab
        f'    (pad "1" thru_hole circle (at -3.81 -6.0) (size 1.4 1.4) (drill 0.9) (layers *.Cu *.Mask))',
        f'    (pad "2" thru_hole circle (at -2.54 -3.5) (size 1.4 1.4) (drill 0.9) (layers *.Cu *.Mask))',
        f'    (pad "3" thru_hole circle (at -1.27 -6.0) (size 1.4 1.4) (drill 0.9) (layers *.Cu *.Mask))',
        f'    (pad "4" thru_hole circle (at 0.0 -3.5) (size 1.4 1.4) (drill 0.9) (layers *.Cu *.Mask))',
        f'    (pad "5" thru_hole circle (at 1.27 -6.0) (size 1.4 1.4) (drill 0.9) (layers *.Cu *.Mask))',
        f'    (pad "6" thru_hole circle (at 2.54 -3.5) (size 1.4 1.4) (drill 0.9) (layers *.Cu *.Mask))',
        f'    (pad "7" thru_hole circle (at 3.81 -6.0) (size 1.4 1.4) (drill 0.9) (layers *.Cu *.Mask))',
        f'    (pad "8" thru_hole circle (at 5.08 -3.5) (size 1.4 1.4) (drill 0.9) (layers *.Cu *.Mask))',
        f'    (pad "SH1" thru_hole circle (at -7.5 1.0) (size 2.0 2.0) (drill 1.4) (layers *.Cu *.Mask) (net 1 "GND"))',
        f'    (pad "SH2" thru_hole circle (at 7.5 1.0) (size 2.0 2.0) (drill 1.4) (layers *.Cu *.Mask) (net 1 "GND"))',
        '  )'
    ])

    # 2. JST-XH 2-Pin (24V In) at TOP-LEFT (Pin 1: X = 107.0, Pin 2: X = 109.5, Y = 103.5)
    lines.extend([
        f'  (footprint "Connector_JST:JST_XH_B2B-XH-A_1x02_P2.50mm_Vertical" (layer "F.Cu")',
        f'    (at {X0 + 8.25} {Y0 + 3.5})',
        f'    (fp_text reference "J3" (at 0 -2.4) (layer "F.SilkS") (effects (font (size 0.7 0.7) (thickness 0.12))))',
        f'    (fp_text value "24V_IN" (at 0 2.4) (layer "F.Fab") (effects (font (size 0.5 0.5) (thickness 0.1))))',
        f'    (fp_rect (start -3.7 -2.8) (end 3.7 2.8) (layer "F.Fab") (stroke (width 0.12) (type solid)) (fill no))',
        f'    (pad "1" thru_hole rect (at -1.25 0) (size 1.5 1.5) (drill 1.0) (layers *.Cu *.Mask) (net 2 "+24V"))',
        f'    (pad "2" thru_hole circle (at 1.25 0) (size 1.5 1.5) (drill 1.0) (layers *.Cu *.Mask) (net 1 "GND"))',
        '  )'
    ])

    # 3. JST-XH 4-Pin (Printer USB) at TOP-LEFT (Center X = 117.0, Y = 103.5)
    lines.extend([
        f'  (footprint "Connector_JST:JST_XH_B4B-XH-A_1x04_P2.50mm_Vertical" (layer "F.Cu")',
        f'    (at {X0 + 17.0} {Y0 + 3.5})',
        f'    (fp_text reference "J4" (at 0 -2.4) (layer "F.SilkS") (effects (font (size 0.7 0.7) (thickness 0.12))))',
        f'    (fp_text value "PRINTER_USB" (at 0 2.4) (layer "F.Fab") (effects (font (size 0.5 0.5) (thickness 0.1))))',
        f'    (fp_rect (start -6.2 -2.8) (end 6.2 2.8) (layer "F.Fab") (stroke (width 0.12) (type solid)) (fill no))',
        f'    (pad "1" thru_hole rect (at -3.75 0) (size 1.5 1.5) (drill 1.0) (layers *.Cu *.Mask) (net 3 "+5V"))',
        f'    (pad "2" thru_hole circle (at -1.25 0) (size 1.5 1.5) (drill 1.0) (layers *.Cu *.Mask) (net 5 "USB_DM"))',
        f'    (pad "3" thru_hole circle (at 1.25 0) (size 1.5 1.5) (drill 1.0) (layers *.Cu *.Mask) (net 6 "USB_DP"))',
        f'    (pad "4" thru_hole circle (at 3.75 0) (size 1.5 1.5) (drill 1.0) (layers *.Cu *.Mask) (net 1 "GND"))',
        '  )'
    ])

    # 4. USB-C Receptacle (Standard HRO TYPE-C-31-M-12) at TOP (Center: X = 127.5, Y = 101.5)
    lines.extend([
        f'  (footprint "Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12" (layer "F.Cu")',
        f'    (at {X0 + 27.5} {Y0 + 1.5} 180)',
        f'    (fp_text reference "J2" (at 0 5.0) (layer "F.SilkS") (effects (font (size 0.7 0.7) (thickness 0.12))))',
        f'    (fp_text value "USB-C" (at 0 -4.5) (layer "F.Fab") (effects (font (size 0.5 0.5) (thickness 0.1))))',
        f'    (fp_rect (start -4.5 -3.5) (end 4.5 3.5) (layer "F.Fab") (stroke (width 0.15) (type solid)) (fill no))',
        f'    (pad "A1" smd roundrect (at -3.25 -4.0) (size 0.45 1.1) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.25) (net 1 "GND"))',
        f'    (pad "A4" smd roundrect (at -2.45 -4.0) (size 0.45 1.1) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.25) (net 3 "+5V"))',
        f'    (pad "A5" smd roundrect (at -1.25 -4.0) (size 0.3 1.1) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.25))',
        f'    (pad "A6" smd roundrect (at -0.25 -4.0) (size 0.3 1.1) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.25) (net 6 "USB_DP"))',
        f'    (pad "A7" smd roundrect (at 0.25 -4.0) (size 0.3 1.1) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.25) (net 5 "USB_DM"))',
        f'    (pad "B5" smd roundrect (at 1.25 -4.0) (size 0.3 1.1) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.25))',
        f'    (pad "B9" smd roundrect (at 2.45 -4.0) (size 0.45 1.1) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.25) (net 3 "+5V"))',
        f'    (pad "B12" smd roundrect (at 3.25 -4.0) (size 0.45 1.1) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.25) (net 1 "GND"))',
        f'    (pad "SH1" thru_hole circle (at -4.3 0) (size 1.3 1.3) (drill 0.8) (layers *.Cu *.Mask) (net 1 "GND"))',
        f'    (pad "SH2" thru_hole circle (at 4.3 0) (size 1.3 1.3) (drill 0.8) (layers *.Cu *.Mask) (net 1 "GND"))',
        '  )'
    ])

    # 5. 3.3V LDO (U4 AP2112K SOT-23-5) at X = 124.0, Y = 108.5
    lines.extend([
        f'  (footprint "Package_TO_SOT_SMD:SOT-23-5" (layer "F.Cu")',
        f'    (at {X0 + 24.0} {Y0 + 8.5})',
        f'    (fp_text reference "U4" (at 0 0) (layer "F.Fab") (effects (font (size 0.6 0.6) (thickness 0.1))))',
        f'    (fp_text value "3.3V" (at 0 0) (layer "F.Fab") (effects (font (size 0.4 0.4) (thickness 0.08))))',
        f'    (fp_rect (start -1.5 -0.8) (end 1.5 0.8) (layer "F.Fab") (stroke (width 0.1) (type solid)) (fill no))',
        f'    (pad "1" smd rect (at -0.95 -1.1) (size 0.5 0.8) (layers "F.Cu" "F.Paste" "F.Mask") (net 3 "+5V"))',
        f'    (pad "2" smd rect (at 0 -1.1) (size 0.5 0.8) (layers "F.Cu" "F.Paste" "F.Mask") (net 1 "GND"))',
        f'    (pad "3" smd rect (at 0.95 -1.1) (size 0.5 0.8) (layers "F.Cu" "F.Paste" "F.Mask") (net 3 "+5V"))',
        f'    (pad "5" smd rect (at -0.95 1.1) (size 0.5 0.8) (layers "F.Cu" "F.Paste" "F.Mask") (net 4 "+3V3"))',
        '  )'
    ])

    # 6. Step-Down Buck (U3 TPS54302 SOT-23-6) at X = 124.0, Y = 121.5
    lines.extend([
        f'  (footprint "Package_TO_SOT_SMD:SOT-23-6" (layer "F.Cu")',
        f'    (at {X0 + 24.0} {Y0 + 21.5})',
        f'    (fp_text reference "U3" (at 0 0) (layer "F.Fab") (effects (font (size 0.6 0.6) (thickness 0.1))))',
        f'    (fp_text value "Buck" (at 0 0) (layer "F.Fab") (effects (font (size 0.4 0.4) (thickness 0.08))))',
        f'    (fp_rect (start -1.5 -0.8) (end 1.5 0.8) (layer "F.Fab") (stroke (width 0.1) (type solid)) (fill no))',
        f'    (pad "1" smd rect (at -0.95 -1.1) (size 0.5 0.8) (layers "F.Cu" "F.Paste" "F.Mask"))',
        f'    (pad "2" smd rect (at 0 -1.1) (size 0.5 0.8) (layers "F.Cu" "F.Paste" "F.Mask") (net 1 "GND"))',
        f'    (pad "3" smd rect (at 0.95 -1.1) (size 0.5 0.8) (layers "F.Cu" "F.Paste" "F.Mask"))',
        f'    (pad "4" smd rect (at 0.95 1.1) (size 0.5 0.8) (layers "F.Cu" "F.Paste" "F.Mask"))',
        f'    (pad "5" smd rect (at 0 1.1) (size 0.5 0.8) (layers "F.Cu" "F.Paste" "F.Mask") (net 2 "+24V"))',
        f'    (pad "6" smd rect (at -0.95 1.1) (size 0.5 0.8) (layers "F.Cu" "F.Paste" "F.Mask"))',
        '  )'
    ])

    # 6b. Buck Inductor L1 (4.0 x 4.0 mm SRN4018) at X = 128.5, Y = 121.5
    lines.extend([
        f'  (footprint "Inductor_SMD:L_Bourns-SRN4018" (layer "F.Cu")',
        f'    (at {X0 + 28.5} {Y0 + 21.5})',
        f'    (fp_text reference "L1" (at 0 -2.4) (layer "F.SilkS") (effects (font (size 0.6 0.6) (thickness 0.1))))',
        f'    (fp_text value "4.7uH" (at 0 2.4) (layer "F.Fab") (effects (font (size 0.4 0.4) (thickness 0.08))))',
        f'    (fp_rect (start -2.0 -2.0) (end 2.0 2.0) (layer "F.Fab") (stroke (width 0.12) (type solid)) (fill no))',
        f'    (pad "1" smd rect (at -1.6 0) (size 1.0 3.2) (layers "F.Cu" "F.Paste" "F.Mask"))',
        f'    (pad "2" smd rect (at 1.6 0) (size 1.0 3.2) (layers "F.Cu" "F.Paste" "F.Mask") (net 3 "+5V"))',
        '  )'
    ])

    # 7. W5500 SPI Ethernet Controller (QFN-48 7x7 mm) at Center: X = 126.5, Y = 114.5
    lines.extend([
        f'  (footprint "Package_DFN_QFN:QFN-48-1EP_7x7mm_P0.5mm_EP5.15x5.15mm" (layer "F.Cu")',
        f'    (at {X0 + 26.5} {Y0 + 14.5})',
        f'    (fp_text reference "U2" (at 0 -4.2) (layer "F.SilkS") (effects (font (size 0.7 0.7) (thickness 0.12))))',
        f'    (fp_text value "W5500" (at 0 4.2) (layer "F.Fab") (effects (font (size 0.5 0.5) (thickness 0.1))))',
        f'    (fp_rect (start -3.5 -3.5) (end 3.5 3.5) (layer "F.Fab") (stroke (width 0.15) (type solid)) (fill no))',
        f'    (pad "EP" smd rect (at 0 0) (size 3.6 3.6) (layers "F.Cu" "F.Paste" "F.Mask") (net 1 "GND"))',
        '  )'
    ])

    # 8. ESP32-S3-WROOM-1U (with IPEX/U.FL connector) at X = 141.0, Y = 112.5 (Rotation 90)
    # Length: 19.2mm (X from 131.4 to 150.6). Width: 18.0mm (Y from 103.5 to 121.5).
    # Pads: in local coords size (1.4 0.8). Rotated 90 -> in board space size is (0.8 1.4), so width along Y is 0.8mm!
    # Pitch is 1.27mm -> clearance between adjacent pads is 1.27 - 0.8 = 0.47mm! (No short!)
    lines.extend([
        f'  (footprint "RF_Module:ESP32-S3-WROOM-1U" (layer "F.Cu")',
        f'    (at {X0 + 41.0} {Y0 + 12.5} 90)',
        f'    (fp_text reference "U1" (at 0 -10.0 90) (layer "F.SilkS") (effects (font (size 0.8 0.8) (thickness 0.12))))',
        f'    (fp_text value "ESP32-S3-WROOM-1U" (at 0 10.0 90) (layer "F.Fab") (effects (font (size 0.6 0.6) (thickness 0.1))))',
        f'    (fp_rect (start -9.6 -9.0) (end 9.6 9.0) (layer "F.Fab") (stroke (width 0.15) (type solid)) (fill no))',
        f'    (fp_circle (center -6.5 6.0) (end -6.5 7.5) (layer "F.SilkS") (stroke (width 0.15) (type solid)) (fill no))',
        f'    (fp_text user "IPEX" (at -4.0 6.0 90) (layer "F.SilkS") (effects (font (size 0.5 0.5) (thickness 0.1))))',
        f'    (pad "EP" smd rect (at 0 0) (size 4.5 4.5) (layers "F.Cu" "F.Paste" "F.Mask") (net 1 "GND"))',
        # Left side pins (local X = -9.0, local Y from -8.25 to +8.25)
        f'    (pad "1" smd rect (at -7.62 -8.75) (size 1.4 0.8) (layers "F.Cu" "F.Paste" "F.Mask") (net 1 "GND"))',
        f'    (pad "2" smd rect (at -6.35 -8.75) (size 1.4 0.8) (layers "F.Cu" "F.Paste" "F.Mask") (net 4 "+3V3"))',
        f'    (pad "13" smd rect (at 6.35 -8.75) (size 1.4 0.8) (layers "F.Cu" "F.Paste" "F.Mask") (net 5 "USB_DM"))',
        f'    (pad "14" smd rect (at 7.62 -8.75) (size 1.4 0.8) (layers "F.Cu" "F.Paste" "F.Mask") (net 6 "USB_DP"))',
        # Right side pins (local X = +9.0)
        f'    (pad "21" smd rect (at 7.62 8.75) (size 1.4 0.8) (layers "F.Cu" "F.Paste" "F.Mask") (net 7 "ETH_SCLK"))',
        f'    (pad "22" smd rect (at 6.35 8.75) (size 1.4 0.8) (layers "F.Cu" "F.Paste" "F.Mask") (net 8 "ETH_MOSI"))',
        f'    (pad "23" smd rect (at 5.08 8.75) (size 1.4 0.8) (layers "F.Cu" "F.Paste" "F.Mask") (net 9 "ETH_MISO"))',
        f'    (pad "24" smd rect (at 3.81 8.75) (size 1.4 0.8) (layers "F.Cu" "F.Paste" "F.Mask") (net 10 "ETH_CS"))',
        f'    (pad "25" smd rect (at 2.54 8.75) (size 1.4 0.8) (layers "F.Cu" "F.Paste" "F.Mask") (net 11 "ETH_INT"))',
        f'    (pad "26" smd rect (at 1.27 8.75) (size 1.4 0.8) (layers "F.Cu" "F.Paste" "F.Mask") (net 12 "ETH_RST"))',
        '  )'
    ])

    # 9. WS2812B RGB Status LED at Center Right: X = 152.8, Y = 112.5
    lines.extend([
        f'  (footprint "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" (layer "F.Cu")',
        f'    (at {X0 + W - 2.2} {Y0 + 12.5})',
        f'    (fp_text reference "D1" (at 0 0) (layer "F.Fab") (effects (font (size 0.6 0.6) (thickness 0.1))))',
        f'    (fp_text value "RGB" (at 0 0) (layer "F.Fab") (effects (font (size 0.4 0.4) (thickness 0.08))))',
        f'    (fp_rect (start -1.7 -1.7) (end 1.7 1.7) (layer "F.Fab") (stroke (width 0.1) (type solid)) (fill no))',
        f'    (pad "1" smd rect (at -1.2 -1.2) (size 0.8 0.7) (layers "F.Cu" "F.Paste" "F.Mask") (net 4 "+3V3"))',
        f'    (pad "2" smd rect (at 1.2 -1.2) (size 0.8 0.7) (layers "F.Cu" "F.Paste" "F.Mask"))',
        f'    (pad "3" smd rect (at 1.2 1.2) (size 0.8 0.7) (layers "F.Cu" "F.Paste" "F.Mask") (net 1 "GND"))',
        f'    (pad "4" smd rect (at -1.2 1.2) (size 0.8 0.7) (layers "F.Cu" "F.Paste" "F.Mask"))',
        '  )'
    ])

    # 10. Buttons: Reset (SW1) and Boot (SW2)
    add_smd_button("SW1", "RESET", X0 + 34.0, Y0 + 2.3)
    add_smd_button("SW2", "BOOT",  X0 + 34.0, Y0 + H - 2.3)

    # 11. Discrete Passives:
    # 24V Input filtering
    add_smd_1206("C1", "10uF/50V", X0 + 3.0, Y0 + 8.5, rot=90, net1=2, net2=1)
    add_smd_0603("C2", "100nF",    X0 + 3.0, Y0 + 13.5, rot=90, net1=2, net2=1)

    # RJ45 Termination on B.Cu
    add_smd_0603("R5", "49.9R",    X0 + 9.5, Y0 + 21.0, rot=0, layer="B.Cu")
    add_smd_0603("R6", "49.9R",    X0 + 12.0, Y0 + 21.0, rot=0, layer="B.Cu")
    add_smd_0603("R7", "49.9R",    X0 + 15.0, Y0 + 21.0, rot=0, layer="B.Cu")
    add_smd_0603("R8", "49.9R",    X0 + 17.5, Y0 + 21.0, rot=0, layer="B.Cu")
    add_smd_0805("C14", "10nF/2kV", X0 + 13.5, Y0 + 18.0, rot=0, layer="B.Cu", net1=0, net2=1)

    # LDO Passives
    add_smd_0603("C6", "4.7uF_IN",   X0 + 21.5, Y0 + 8.5, rot=90, net1=3, net2=1)
    add_smd_0603("C7", "4.7uF_OUT",  X0 + 26.5, Y0 + 8.5, rot=90, net1=4, net2=1)

    # USB-C Passives on B.Cu
    add_smd_0603("R11", "5.1k_CC1",  X0 + 25.5, Y0 + 5.5, rot=0, layer="B.Cu", net1=0, net2=1)
    add_smd_0603("R12", "5.1k_CC2",  X0 + 29.5, Y0 + 5.5, rot=0, layer="B.Cu", net1=0, net2=1)

    # Buck Converter Passives (Boot on Top, FB and 5V Smoothing on B.Cu under L1/U3)
    add_smd_0603("C3", "100nF_BOOT", X0 + 21.5, Y0 + 21.5, rot=90, net1=0, net2=0)
    add_smd_0603("R1", "100k_FB",    X0 + 23.5, Y0 + 23.5, rot=0,  layer="B.Cu", net1=3, net2=0)
    add_smd_0603("R2", "13.3k_FB",   X0 + 26.0, Y0 + 23.5, rot=0,  layer="B.Cu", net1=0, net2=1)
    add_smd_0805("C4", "22uF_5V",    X0 + 28.5, Y0 + 22.0, rot=0,  layer="B.Cu", net1=3, net2=1)
    add_smd_0805("C5", "22uF_5V",    X0 + 28.5, Y0 + 19.5, rot=0,  layer="B.Cu", net1=3, net2=1)

    # W5500 Passives on B.Cu
    add_smd_0603("R3",  "12.4k_BIAS", X0 + 24.5, Y0 + 14.5, rot=90, layer="B.Cu", net1=0, net2=1)
    add_smd_0603("R4",  "10k_RST",    X0 + 28.5, Y0 + 14.5, rot=90, layer="B.Cu", net1=4, net2=12)
    add_smd_0603("C10", "100nF",      X0 + 26.5, Y0 + 11.5, rot=0,  layer="B.Cu", net1=4, net2=1)
    add_smd_0603("C11", "100nF",      X0 + 26.5, Y0 + 17.5, rot=0,  layer="B.Cu", net1=4, net2=1)

    # ESP32-S3 Passives
    add_smd_0805("C15", "10uF",   X0 + 38.0, Y0 + 2.3, rot=0, net1=4, net2=1)
    add_smd_0603("C16", "100nF",  X0 + 42.0, Y0 + 2.3, rot=0, net1=4, net2=1)
    add_smd_0603("R13", "10k_EN", X0 + 34.0, Y0 + 5.5, rot=90, layer="B.Cu", net1=4, net2=0)
    add_smd_0603("C17", "1uF_EN", X0 + 34.0, Y0 + 8.5, rot=90, layer="B.Cu", net1=0, net2=1)
    add_smd_0603("C18", "100nF",  X0 + W - 2.2, Y0 + 8.5, rot=90, net1=4, net2=1)

    # Silkscreen Branding
    lines.extend([
        f'  (gr_text "BonbridgeESP32 v1.2" (at {X0 + 44.0} {Y0 + 23.0}) (layer "F.SilkS") (effects (font (size 0.8 0.8) (thickness 0.12))))',
        ')'
    ])

    with open(pcb_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))
    print("SUCCESS: Generated board at", pcb_path)

if __name__ == "__main__":
    create_board()
