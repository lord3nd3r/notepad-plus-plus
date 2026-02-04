#!/usr/bin/env python3
"""
Convert Windows .ico files to PNG format for GTK toolbar use
"""

import os
import subprocess
from pathlib import Path

# Icon directories
ICON_BASE = Path("PowerEditor/src/icons")
OUTPUT_DIR = Path("linux-gtk-prototype/icons")

# Essential toolbar icons to convert
TOOLBAR_ICONS = [
    "new_off.ico",
    "open_off.ico",
    "save_off.ico",
    "saveall_off.ico",
    "close_off.ico",
    "closeall_off.ico",
    "cut_off.ico",
    "copy_off.ico",
    "paste_off.ico",
    "undo_off.ico",
    "redo_off.ico",
    "find_off.ico",
    "findrep_off.ico",
    "zoomIn_off.ico",
    "zoomOut_off.ico",
    "wrap_off.ico",
    "allChars_off.ico",
    "indentGuide_off.ico",
    "docMap_off.ico",
    "funcList_off.ico",
    "fileBrowser_off.ico",
    "monitoring_off.ico",
    "startrecord_off.ico",
    "stoprecord_off.ico",
    "playrecord_off.ico",
]

def convert_icon(ico_path, png_path, size=16):
    """Convert .ico to .png using ImageMagick with brightness adjustment"""
    try:
        # Extract the first (smallest) icon from .ico file
        # Apply brightness/contrast adjustments to make icons more visible
        subprocess.run([
            "convert",
            str(ico_path) + "[0]",  # [0] gets the first icon
            "-resize", f"{size}x{size}",
            "-brightness-contrast", "15x10",  # Increase brightness and contrast
            str(png_path)
        ], check=True, capture_output=True)
        print(f"✓ Converted: {ico_path.name} -> {png_path.name}")
        return True
    except subprocess.CalledProcessError as e:
        print(f"✗ Failed: {ico_path.name} - {e.stderr.decode()}")
        return False

def main():
    # Create output directory
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    
    # Use dark theme icons (lighter colors for better visibility)
    themes = ["dark/toolbar/regular", "light/toolbar/regular", "standard/toolbar"]
    
    converted = 0
    failed = 0
    
    for icon_name in TOOLBAR_ICONS:
        found = False
        for theme in themes:
            ico_path = ICON_BASE / theme / icon_name
            if ico_path.exists():
                png_name = icon_name.replace("_off.ico", ".png")
                png_path = OUTPUT_DIR / png_name
                
                if convert_icon(ico_path, png_path):
                    converted += 1
                else:
                    failed += 1
                found = True
                break
        
        if not found:
            print(f"✗ Not found: {icon_name}")
            failed += 1
    
    print(f"\n{'='*50}")
    print(f"Conversion complete!")
    print(f"  Converted: {converted}")
    print(f"  Failed: {failed}")
    print(f"  Output: {OUTPUT_DIR}")
    print(f"{'='*50}")

if __name__ == "__main__":
    main()
