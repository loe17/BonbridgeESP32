import os, json

def generate_kicad_pro(filepath):
    pro_data = {
        "meta": {
            "filename": os.path.basename(filepath),
            "version": 1
        },
        "schematic": {
            "annotate_start_num": 1,
            "drawing": {
                "dashed_lines_dash_length_ratio": 12.0,
                "dashed_lines_gap_length_ratio": 3.0,
                "default_line_thickness": 6.0,
                "default_text_size": 50.0,
                "field_names": [],
                "intersheets_ref_own_page": False,
                "intersheets_ref_prefix": "",
                "intersheets_ref_short": False,
                "intersheets_ref_show": False,
                "intersheets_ref_suffix": "",
                "pin_symbol_size": 25.0,
                "text_offset_ratio": 0.15
            }
        },
        "boards": [],
        "cvpcb": {"equivalence_files": []},
        "erc": {
            "erc_exclusions": [],
            "meta": {"version": 0},
            "pin_map": []
        },
        "net_settings": {
            "classes": [
                {
                    "bus_width": 12.0,
                    "clearance": 0.2,
                    "diff_pair_gap": 0.25,
                    "diff_pair_via_gap": 0.25,
                    "diff_pair_width": 0.2,
                    "line_style": 0,
                    "microvia_diameter": 0.3,
                    "microvia_drill": 0.1,
                    "name": "Default",
                    "pcb_color": "rgba(0, 0, 0, 0.000)",
                    "schematic_color": "rgba(0, 0, 0, 0.000)",
                    "track_width": 0.25,
                    "via_diameter": 0.6,
                    "via_drill": 0.3
                },
                {
                    "bus_width": 12.0,
                    "clearance": 0.3,
                    "diff_pair_gap": 0.25,
                    "diff_pair_via_gap": 0.25,
                    "diff_pair_width": 0.2,
                    "line_style": 0,
                    "microvia_diameter": 0.3,
                    "microvia_drill": 0.1,
                    "name": "Power",
                    "pcb_color": "rgba(255, 0, 0, 0.000)",
                    "schematic_color": "rgba(255, 0, 0, 0.000)",
                    "track_width": 0.5,
                    "via_diameter": 0.8,
                    "via_drill": 0.4
                },
                {
                    "bus_width": 12.0,
                    "clearance": 0.2,
                    "diff_pair_gap": 0.2,
                    "diff_pair_via_gap": 0.2,
                    "diff_pair_width": 0.25,
                    "line_style": 0,
                    "microvia_diameter": 0.3,
                    "microvia_drill": 0.1,
                    "name": "USB_90Ohm",
                    "pcb_color": "rgba(0, 180, 0, 0.000)",
                    "schematic_color": "rgba(0, 180, 0, 0.000)",
                    "track_width": 0.25,
                    "via_diameter": 0.6,
                    "via_drill": 0.3
                }
            ]
        }
    }
    with open(filepath, "w", encoding="utf-8") as f:
        json.dump(pro_data, f, indent=2)

def generate_kicad_pcb(filepath):
    # Board dimensions: 55mm (width) x 25mm (height)
    X0, Y0 = 100.0, 100.0
    W, H = 55.0, 25.0
    R = 2.0 # 2mm corner radius

    # 4 mounting holes for M2.5 (2.7mm drill, 4.8mm pad), offset 3.0mm from corners
    holes = [
        (X0 + 3.0, Y0 + 3.0),
        (X0 + W - 3.0, Y0 + 3.0),
        (X0 + 3.0, Y0 + H - 3.0),
        (X0 + W - 3.0, Y0 + H - 3.0)
    ]

    lines = [
        '(kicad_pcb (version 20221018) (generator pcbnew)',
        '  (general',
        '    (thickness 1.6)',
        '  )',
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
        '    (grid 0.5 0.5)',
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
        '  ;; ----------------- BOARD OUTLINE (Edge.Cuts: 55 x 25 mm mit R=2mm) -----------------',
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

    # Add 4 mounting holes
    for i, (hx, hy) in enumerate(holes, 1):
        lines.extend([
            f'  (footprint "MountingHole:MountingHole_2.7mm_M2.5_Pad_TopBottom" (layer "F.Cu")',
            f'    (at {hx} {hy})',
            f'    (fp_text reference "H{i}" (at 0 -2.5) (layer "F.SilkS") (effects (font (size 0.8 0.8) (thickness 0.15))))',
            f'    (fp_text value "M2.5" (at 0 2.5) (layer "F.Fab") (effects (font (size 0.8 0.8) (thickness 0.15))))',
            f'    (pad "" thru_hole circle (at 0 0) (size 4.8 4.8) (drill 2.7) (layers *.Cu *.Mask) (net 1 "GND"))',
            '  )'
        ])

    # Footprint 1: RJ45 MagJack on BOTTOM LONG SIDE (opening downwards at Y = Y0 + H)
    # Centered at X = X0 + 15.0, Y = Y0 + H - 8.0 (opening facing down)
    lines.extend([
        f'  (footprint "Connector_RJ:RJ45_Amphenol_RJMG1BD3B8K1ANR" (layer "F.Cu")',
        f'    (at {X0 + 15.0} {Y0 + H - 8.0} 90)',
        f'    (fp_text reference "J1" (at -10.0 0) (layer "F.SilkS") (effects (font (size 1 1) (thickness 0.15))))',
        f'    (fp_text value "RJ45_LAN_10/100M" (at 10.0 0) (layer "F.SilkS") (effects (font (size 0.8 0.8) (thickness 0.15))))',
        f'    (fp_line (start -8.0 -8.0) (end 8.0 -8.0) (layer "F.SilkS") (width 0.15))',
        f'    (fp_line (start 8.0 -8.0) (end 8.0 8.0) (layer "F.SilkS") (width 0.15))',
        f'    (fp_line (start 8.0 8.0) (end -8.0 8.0) (layer "F.SilkS") (width 0.15))',
        f'    (fp_line (start -8.0 8.0) (end -8.0 -8.0) (layer "F.SilkS") (width 0.15))',
        f'    (pad "1" thru_hole circle (at -2.54 3.175) (size 1.6 1.6) (drill 0.9) (layers *.Cu *.Mask))',
        f'    (pad "2" thru_hole circle (at -2.54 1.905) (size 1.6 1.6) (drill 0.9) (layers *.Cu *.Mask))',
        f'    (pad "3" thru_hole circle (at -2.54 0.635) (size 1.6 1.6) (drill 0.9) (layers *.Cu *.Mask))',
        f'    (pad "4" thru_hole circle (at -2.54 -0.635) (size 1.6 1.6) (drill 0.9) (layers *.Cu *.Mask))',
        f'    (pad "5" thru_hole circle (at -2.54 -1.905) (size 1.6 1.6) (drill 0.9) (layers *.Cu *.Mask))',
        f'    (pad "6" thru_hole circle (at -2.54 -3.175) (size 1.6 1.6) (drill 0.9) (layers *.Cu *.Mask))',
        f'    (pad "SH1" thru_hole circle (at 0 -7.5) (size 2.5 2.5) (drill 1.6) (layers *.Cu *.Mask) (net 1 "GND"))',
        f'    (pad "SH2" thru_hole circle (at 0 7.5) (size 2.5 2.5) (drill 1.6) (layers *.Cu *.Mask) (net 1 "GND"))',
        '  )'
    ])

    # Footprint 2: JST-XH 2-Pin (24V In) at TOP-LEFT (Y = Y0 + 3.8, X = X0 + 10.0)
    lines.extend([
        f'  (footprint "Connector_JST:JST_XH_B2B-XH-A_1x02_P2.50mm_Vertical" (layer "F.Cu")',
        f'    (at {X0 + 10.0} {Y0 + 3.8})',
        f'    (fp_text reference "J3" (at 0 -2.5) (layer "F.SilkS") (effects (font (size 0.8 0.8) (thickness 0.15))))',
        f'    (fp_text value "24V_IN" (at 0 2.5) (layer "F.SilkS") (effects (font (size 0.8 0.8) (thickness 0.15))))',
        f'    (fp_text user "+24V" (at -1.25 4.0) (layer "F.SilkS") (effects (font (size 0.6 0.6) (thickness 0.1))))',
        f'    (fp_text user "GND" (at 1.25 4.0) (layer "F.SilkS") (effects (font (size 0.6 0.6) (thickness 0.1))))',
        f'    (pad "1" thru_hole rect (at -1.25 0) (size 1.7 1.7) (drill 1.0) (layers *.Cu *.Mask) (net 2 "+24V"))',
        f'    (pad "2" thru_hole circle (at 1.25 0) (size 1.7 1.7) (drill 1.0) (layers *.Cu *.Mask) (net 1 "GND"))',
        '  )'
    ])

    # Footprint 3: JST-XH 4-Pin (Printer USB: 5V, D-, D+, GND) NEXT TO JST-24V AT TOP-LEFT (X = X0 + 21.0, Y = Y0 + 3.8)
    lines.extend([
        f'  (footprint "Connector_JST:JST_XH_B4B-XH-A_1x04_P2.50mm_Vertical" (layer "F.Cu")',
        f'    (at {X0 + 21.0} {Y0 + 3.8})',
        f'    (fp_text reference "J4" (at 0 -2.5) (layer "F.SilkS") (effects (font (size 0.8 0.8) (thickness 0.15))))',
        f'    (fp_text value "PRINTER_USB" (at 0 2.5) (layer "F.SilkS") (effects (font (size 0.8 0.8) (thickness 0.15))))',
        f'    (fp_text user "5V" (at -3.75 4.0) (layer "F.SilkS") (effects (font (size 0.6 0.6) (thickness 0.1))))',
        f'    (fp_text user "D-" (at -1.25 4.0) (layer "F.SilkS") (effects (font (size 0.6 0.6) (thickness 0.1))))',
        f'    (fp_text user "D+" (at 1.25 4.0) (layer "F.SilkS") (effects (font (size 0.6 0.6) (thickness 0.1))))',
        f'    (fp_text user "GND" (at 3.75 4.0) (layer "F.SilkS") (effects (font (size 0.6 0.6) (thickness 0.1))))',
        f'    (pad "1" thru_hole rect (at -3.75 0) (size 1.7 1.7) (drill 1.0) (layers *.Cu *.Mask) (net 3 "+5V"))',
        f'    (pad "2" thru_hole circle (at -1.25 0) (size 1.7 1.7) (drill 1.0) (layers *.Cu *.Mask) (net 5 "USB_DM"))',
        f'    (pad "3" thru_hole circle (at 1.25 0) (size 1.7 1.7) (drill 1.0) (layers *.Cu *.Mask) (net 6 "USB_DP"))',
        f'    (pad "4" thru_hole circle (at 3.75 0) (size 1.7 1.7) (drill 1.0) (layers *.Cu *.Mask) (net 1 "GND"))',
        '  )'
    ])

    # Footprint 4: USB-C Receptacle OPPOSITE RJ45 on TOP LONG SIDE (X = X0 + 44.0, Y = Y0 + 2.0, opening UPWARDS)
    lines.extend([
        f'  (footprint "Connector_USB:USB_C_Receptacle_GCT_USB4085" (layer "F.Cu")',
        f'    (at {X0 + 44.0} {Y0 + 2.5} 180)',
        f'    (fp_text reference "J2" (at 0 5.0) (layer "F.SilkS") (effects (font (size 0.9 0.9) (thickness 0.15))))',
        f'    (fp_text value "USB-C_PROG" (at 0 -4.5) (layer "F.SilkS") (effects (font (size 0.8 0.8) (thickness 0.15))))',
        f'    (fp_line (start -4.5 -3.5) (end 2.5 -3.5) (layer "F.SilkS") (width 0.15))',
        f'    (fp_line (start 2.5 -3.5) (end 2.5 3.5) (layer "F.SilkS") (width 0.15))',
        f'    (fp_line (start 2.5 3.5) (end -4.5 3.5) (layer "F.SilkS") (width 0.15))',
        f'    (fp_line (start -4.5 3.5) (end -4.5 -3.5) (layer "F.SilkS") (width 0.15))',
        f'    (pad "A1/B12" smd rect (at 1.0 2.75) (size 0.6 1.2) (layers "F.Cu" "F.Paste" "F.Mask") (net 1 "GND"))',
        f'    (pad "A4/B9" smd rect (at 1.0 1.75) (size 0.6 1.2) (layers "F.Cu" "F.Paste" "F.Mask") (net 3 "+5V"))',
        f'    (pad "A6" smd rect (at 1.0 0.25) (size 0.6 1.2) (layers "F.Cu" "F.Paste" "F.Mask") (net 6 "USB_DP"))',
        f'    (pad "A7" smd rect (at 1.0 -0.25) (size 0.6 1.2) (layers "F.Cu" "F.Paste" "F.Mask") (net 5 "USB_DM"))',
        f'    (pad "SH1" thru_hole oval (at 0.0 4.3) (size 1.2 2.0) (drill 0.6 1.4) (layers *.Cu *.Mask) (net 1 "GND"))',
        f'    (pad "SH2" thru_hole oval (at 0.0 -4.3) (size 1.2 2.0) (drill 0.6 1.4) (layers *.Cu *.Mask) (net 1 "GND"))',
        '  )'
    ])

    # Footprint 5: W5500 SPI Ethernet Controller (QFN-48, 7x7 mm) (Center: X = X0 + 29.0, Y = Y0 + 14.5)
    lines.extend([
        f'  (footprint "Package_DFN_QFN:QFN-48-1EP_7x7mm_P0.5mm_EP5.15x5.15mm" (layer "F.Cu")',
        f'    (at {X0 + 28.5} {Y0 + 14.0})',
        f'    (fp_text reference "U2" (at 0 -4.5) (layer "F.SilkS") (effects (font (size 0.9 0.9) (thickness 0.15))))',
        f'    (fp_text value "W5500" (at 0 4.5) (layer "F.SilkS") (effects (font (size 0.8 0.8) (thickness 0.15))))',
        f'    (fp_line (start -3.5 -3.5) (end 3.5 -3.5) (layer "F.SilkS") (width 0.15))',
        f'    (fp_line (start 3.5 -3.5) (end 3.5 3.5) (layer "F.SilkS") (width 0.15))',
        f'    (fp_line (start 3.5 3.5) (end -3.5 3.5) (layer "F.SilkS") (width 0.15))',
        f'    (fp_line (start -3.5 3.5) (end -3.5 -3.5) (layer "F.SilkS") (width 0.15))',
        f'    (pad "EP" smd rect (at 0 0) (size 4.0 4.0) (layers "F.Cu" "F.Paste" "F.Mask") (net 1 "GND"))',
        '  )'
    ])

    # Footprint 6: Step-Down Buck Converter (24V -> 5.0V, TPS54302 SOT-23-6) (Bottom center: X = X0 + 28.5, Y = Y0 + 21.5)
    lines.extend([
        f'  (footprint "Package_TO_SOT_SMD:SOT-23-6" (layer "F.Cu")',
        f'    (at {X0 + 28.5} {Y0 + 21.0})',
        f'    (fp_text reference "U3" (at 0 -2.5) (layer "F.SilkS") (effects (font (size 0.8 0.8) (thickness 0.15))))',
        f'    (fp_text value "Buck_24V_to_5V_3A" (at 0 2.5) (layer "F.Fab") (effects (font (size 0.6 0.6) (thickness 0.1))))',
        f'    (pad "1" smd rect (at -0.95 -1.35) (size 0.6 1.0) (layers "F.Cu" "F.Paste" "F.Mask"))',
        f'    (pad "2" smd rect (at 0 -1.35) (size 0.6 1.0) (layers "F.Cu" "F.Paste" "F.Mask") (net 1 "GND"))',
        f'    (pad "3" smd rect (at 0.95 -1.35) (size 0.6 1.0) (layers "F.Cu" "F.Paste" "F.Mask"))',
        f'    (pad "4" smd rect (at 0.95 1.35) (size 0.6 1.0) (layers "F.Cu" "F.Paste" "F.Mask"))',
        f'    (pad "5" smd rect (at 0 1.35) (size 0.6 1.0) (layers "F.Cu" "F.Paste" "F.Mask") (net 2 "+24V"))',
        f'    (pad "6" smd rect (at -0.95 1.35) (size 0.6 1.0) (layers "F.Cu" "F.Paste" "F.Mask"))',
        '  )'
    ])

    # Footprint 7: ESP32-S3-WROOM-1U (with IPEX/U.FL antenna) (Right half: X = X0 + 43.0, Y = Y0 + 13.5)
    lines.extend([
        f'  (footprint "RF_Module:ESP32-S3-WROOM-1U" (layer "F.Cu")',
        f'    (at {X0 + 43.0} {Y0 + 14.0})',
        f'    (fp_text reference "U1" (at 0 -10.5) (layer "F.SilkS") (effects (font (size 0.9 0.9) (thickness 0.15))))',
        f'    (fp_text value "ESP32-S3-WROOM-1U" (at 0 10.5) (layer "F.SilkS") (effects (font (size 0.8 0.8) (thickness 0.15))))',
        f'    (fp_line (start -9.0 -9.6) (end 9.0 -9.6) (layer "F.SilkS") (width 0.15))',
        f'    (fp_line (start 9.0 -9.6) (end 9.0 9.6) (layer "F.SilkS") (width 0.15))',
        f'    (fp_line (start 9.0 9.6) (end -9.0 9.6) (layer "F.SilkS") (width 0.15))',
        f'    (fp_line (start -9.0 9.6) (end -9.0 -9.6) (layer "F.SilkS") (width 0.15))',
        f'    (fp_circle (center 6.0 6.5) (end 7.5 6.5) (layer "F.SilkS") (width 0.15))', # U.FL marker
        f'    (fp_text user "IPEX Ant." (at 6.0 4.0) (layer "F.SilkS") (effects (font (size 0.6 0.6) (thickness 0.1))))',
        f'    (pad "EP" smd rect (at 0 0) (size 5.0 5.0) (layers "F.Cu" "F.Paste" "F.Mask") (net 1 "GND"))',
        f'    (pad "1" smd rect (at -9.0 -7.5) (size 1.5 0.9) (layers "F.Cu" "F.Paste" "F.Mask") (net 1 "GND"))',
        f'    (pad "2" smd rect (at -9.0 -6.0) (size 1.5 0.9) (layers "F.Cu" "F.Paste" "F.Mask") (net 4 "+3V3"))',
        f'    (pad "13" smd rect (at -9.0 4.5) (size 1.5 0.9) (layers "F.Cu" "F.Paste" "F.Mask") (net 5 "USB_DM"))',
        f'    (pad "14" smd rect (at -9.0 6.0) (size 1.5 0.9) (layers "F.Cu" "F.Paste" "F.Mask") (net 6 "USB_DP"))',
        f'    (pad "21" smd rect (at 9.0 6.0) (size 1.5 0.9) (layers "F.Cu" "F.Paste" "F.Mask") (net 7 "ETH_SCLK"))',
        f'    (pad "22" smd rect (at 9.0 4.5) (size 1.5 0.9) (layers "F.Cu" "F.Paste" "F.Mask") (net 8 "ETH_MOSI"))',
        f'    (pad "23" smd rect (at 9.0 3.0) (size 1.5 0.9) (layers "F.Cu" "F.Paste" "F.Mask") (net 9 "ETH_MISO"))',
        f'    (pad "24" smd rect (at 9.0 1.5) (size 1.5 0.9) (layers "F.Cu" "F.Paste" "F.Mask") (net 10 "ETH_CS"))',
        f'    (pad "25" smd rect (at 9.0 0.0) (size 1.5 0.9) (layers "F.Cu" "F.Paste" "F.Mask") (net 11 "ETH_INT"))',
        f'    (pad "26" smd rect (at 9.0 -1.5) (size 1.5 0.9) (layers "F.Cu" "F.Paste" "F.Mask") (net 12 "ETH_RST"))',
        '  )'
    ])

    # Footprint 8: 3.3V LDO Regulator (5V -> 3.3V, SOT-23-5) (X = X0 + 36.0, Y = Y0 + 4.5)
    lines.extend([
        f'  (footprint "Package_TO_SOT_SMD:SOT-23-5" (layer "F.Cu")',
        f'    (at {X0 + 34.0} {Y0 + 4.5})',
        f'    (fp_text reference "U4" (at 0 -2.2) (layer "F.SilkS") (effects (font (size 0.7 0.7) (thickness 0.15))))',
        f'    (fp_text value "LDO_3.3V_600mA" (at 0 2.2) (layer "F.Fab") (effects (font (size 0.6 0.6) (thickness 0.1))))',
        f'    (pad "1" smd rect (at -0.95 -1.3) (size 0.6 1.0) (layers "F.Cu" "F.Paste" "F.Mask") (net 3 "+5V"))',
        f'    (pad "2" smd rect (at 0 -1.3) (size 0.6 1.0) (layers "F.Cu" "F.Paste" "F.Mask") (net 1 "GND"))',
        f'    (pad "3" smd rect (at 0.95 -1.3) (size 0.6 1.0) (layers "F.Cu" "F.Paste" "F.Mask") (net 3 "+5V"))',
        f'    (pad "5" smd rect (at -0.95 1.3) (size 0.6 1.0) (layers "F.Cu" "F.Paste" "F.Mask") (net 4 "+3V3"))',
        '  )'
    ])

    # Footprint 9: WS2812B RGB Status LED (GPIO 48)
    lines.extend([
        f'  (footprint "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" (layer "F.Cu")',
        f'    (at {X0 + 51.5} {Y0 + 13.0})',
        f'    (fp_text reference "D1" (at 0 -3.5) (layer "F.SilkS") (effects (font (size 0.7 0.7) (thickness 0.15))))',
        f'    (fp_text value "WS2812B (RGB)" (at 0 3.5) (layer "F.Fab") (effects (font (size 0.6 0.6) (thickness 0.1))))',
        f'    (pad "1" smd rect (at -1.6 -1.6) (size 1.5 1.0) (layers "F.Cu" "F.Paste" "F.Mask") (net 4 "+3V3"))',
        f'    (pad "2" smd rect (at 1.6 -1.6) (size 1.5 1.0) (layers "F.Cu" "F.Paste" "F.Mask"))',
        f'    (pad "3" smd rect (at 1.6 1.6) (size 1.5 1.0) (layers "F.Cu" "F.Paste" "F.Mask") (net 1 "GND"))',
        f'    (pad "4" smd rect (at -1.6 1.6) (size 1.5 1.0) (layers "F.Cu" "F.Paste" "F.Mask"))',
        '  )'
    ])

    # Silkscreen Branding & Technical Info
    lines.extend([
        f'  (gr_text "BonbridgeESP32 v1.2" (at {X0 + 38.0} {Y0 + 22.5}) (layer "F.SilkS") (effects (font (size 1.0 1.0) (thickness 0.18))))',
        f'  (gr_text "55x25mm" (at {X0 + 49.0} {Y0 + 22.5}) (layer "F.SilkS") (effects (font (size 0.7 0.7) (thickness 0.12))))',
        f'  (gr_text "RJ45 LAN (MagJack)" (at {X0 + 15.0} {Y0 + 13.5}) (layer "F.SilkS") (effects (font (size 0.8 0.8) (thickness 0.15))))',
        f'  (gr_text "USB-C PROG (OBEN)" (at {X0 + 44.0} {Y0 + 7.5}) (layer "F.SilkS") (effects (font (size 0.7 0.7) (thickness 0.12))))',
        ')'
    ])

    with open(filepath, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))

if __name__ == "__main__":
    out_dir = r"c:\Users\Lukas\Documents\KITemp\BonbridgeESP32\hardware"
    os.makedirs(out_dir, exist_ok=True)
    pro_file = os.path.join(out_dir, "BonbridgeESP32_CustomBoard.kicad_pro")
    pcb_file = os.path.join(out_dir, "BonbridgeESP32_CustomBoard.kicad_pcb")
    generate_kicad_pro(pro_file)
    generate_kicad_pcb(pcb_file)
    print("SUCCESS: Updated KiCad project and PCB files in", out_dir)
