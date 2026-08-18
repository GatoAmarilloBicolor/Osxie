#!/usr/bin/env python3
import os
import plistlib
import glob
import subprocess
import shutil

def setup_osxie_integration():
    home = os.path.expanduser("~")
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    
    possible_app_dirs = [
        os.path.join(repo_root, "install", "Volumes", "SystemRoot", "Applications"),
        os.path.join(repo_root, "build_new", "Volumes", "SystemRoot", "Applications"),
        os.path.join(repo_root, "build_all", "Volumes", "SystemRoot", "Applications"),
        os.path.join(home, ".osxie", "Volumes", "SystemRoot", "Applications")
    ]
    
    apps_dir = None
    for d in possible_app_dirs:
        if os.path.exists(d):
            apps_dir = d
            break
            
    if not apps_dir:
        apps_dir = os.path.join(home, ".osxie", "Volumes", "SystemRoot", "Applications")
        os.makedirs(apps_dir, exist_ok=True)

    # 1. Setup GTK Bookmark & KDE Places integration
    gtk_bookmarks_path = os.path.join(home, ".config", "gtk-3.0", "bookmarks")
    os.makedirs(os.path.dirname(gtk_bookmarks_path), exist_ok=True)
    
    bookmark_entry = f"file://{apps_dir} Applications (Osxie)\n"
    bookmarks = []
    if os.path.exists(gtk_bookmarks_path):
        with open(gtk_bookmarks_path, "r") as f:
            bookmarks = [line for line in f.readlines() if "Applications (Osxie)" not in line and apps_dir not in line]
            
    bookmarks.append(bookmark_entry)
    with open(gtk_bookmarks_path, "w") as f:
        f.writelines(bookmarks)
    print(f"Updated GTK bookmark for Osxie Applications: {apps_dir}")

    # KDE Dolphin / Places integration (.local/share/user-places.xbel)
    kde_xbel_path = os.path.join(home, ".local", "share", "user-places.xbel")
    if os.path.exists(kde_xbel_path):
        try:
            with open(kde_xbel_path, "r") as kf:
                xbel_content = kf.read()
            if apps_dir not in xbel_content:
                # Insert bookmark before the last </xbel> tag
                new_place = f'    <bookmark href="file://{apps_dir}">\n        <title>Applications (Osxie)</title>\n        <info>\n            <metadata owner="http://freedesktop.org">\n                <bookmark:icon name="applications-system"/>\n            </metadata>\n        </info>\n    </bookmark>\n'
                if "</xbel>" in xbel_content:
                    xbel_content = xbel_content.replace("</xbel>", new_place + "</xbel>")
                    with open(kde_xbel_path, "w") as kf:
                        kf.write(xbel_content)
                    print(f"Added KDE Dolphin bookmark for Osxie Applications")
        except Exception as e:
            print(f"Skipping KDE XBEL update: {e}")

    # Create .directory file in Applications folder for custom folder icon support (Dolphin/Nemo/etc.)
    directory_ini = os.path.join(apps_dir, ".directory")
    with open(directory_ini, "w") as f:
        f.write("[Desktop Entry]\nType=Directory\nIcon=applications-system\nName=Applications (Osxie)\n")

    # Haiku OS Tracker Query / Deskbar integration support & X11/BeOS integration
    haiku_queries_dir = os.path.join(home, "config", "settings", "Tracker", "DefaultQuery")
    if os.path.exists(os.path.join(home, "config")) or os.path.exists("/boot/home/config"):
        try:
            os.makedirs(haiku_queries_dir, exist_ok=True)
            haiku_apps_link = os.path.join(home, "config", "non-packaged", "apps", "OsxieApplications")
            os.makedirs(os.path.dirname(haiku_apps_link), exist_ok=True)
            if not os.path.lexists(haiku_apps_link):
                os.symlink(apps_dir, haiku_apps_link)
                print(f"Created Haiku Deskbar symlink for Osxie Applications at {haiku_apps_link}")
        except Exception as e:
            print(f"Skipping Haiku integration: {e}")
    desktop_dir = os.path.join(home, ".local", "share", "applications", "osxie")
    icons_dir = os.path.join(home, ".local", "share", "icons", "hicolor", "256x256", "apps")
    os.makedirs(desktop_dir, exist_ok=True)
    os.makedirs(icons_dir, exist_ok=True)
    
    app_bundles = glob.glob(os.path.join(apps_dir, "*.app"))
    if not app_bundles:
        sample_app_dir = os.path.join(apps_dir, "TextEdit.app")
        sample_contents = os.path.join(sample_app_dir, "Contents", "MacOS")
        os.makedirs(sample_contents, exist_ok=True)
        
        sample_plist = os.path.join(sample_app_dir, "Contents", "Info.plist")
        with open(sample_plist, "w") as pf:
            pf.write('<?xml version="1.0" encoding="UTF-8"?>\n<plist version="1.0"><dict><key>CFBundleExecutable</key><string>TextEdit</string><key>CFBundleIdentifier</key><string>com.apple.TextEdit</string></dict></plist>')
            
        app_bundles = [sample_app_dir]

    for app_path in app_bundles:
        app_name = os.path.basename(app_path)[:-4]
        plist_path = os.path.join(app_path, "Contents", "Info.plist")
        
        exec_name = app_name
        icon_file_name = None
        
        if os.path.exists(plist_path):
            try:
                with open(plist_path, "rb") as pf:
                    plist = plistlib.load(pf)
                    exec_name = plist.get("CFBundleExecutable", app_name)
                    icon_file_name = plist.get("CFBundleIconFile")
                    if not icon_file_name and "CFBundleIconFiles" in plist:
                        icons_list = plist.get("CFBundleIconFiles")
                        if isinstance(icons_list, list) and len(icons_list) > 0:
                            icon_file_name = icons_list[0]
            except Exception:
                pass
                
        binary_path = os.path.join(app_path, "Contents", "MacOS", exec_name)
        
        # Resolve icon
        icon_target_name = f"osxie-{app_name.lower()}"
        icon_found = False
        
        if icon_file_name:
            if not icon_file_name.endswith(".icns"):
                icon_file_name += ".icns"
            icns_path = os.path.join(app_path, "Contents", "Resources", icon_file_name)
            if os.path.exists(icns_path):
                # Try converting using imageio/pillow or icotool/pngquant if available
                png_output = os.path.join(icons_dir, f"{icon_target_name}.png")
                # Attempt using python if Pillow/icnsutil is present, or fallback to copy/convert
                try:
                    from PIL import Image
                    # Some simple fallback or icns parser
                    ico_img = Image.open(icns_path)
                    ico_img.save(png_output, "PNG")
                    icon_found = True
                except Exception:
                    # Try system command pngquant / ffmpeg / convert
                    if shutil.which("convert"):
                        subprocess.run(["convert", icns_path, png_output], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                        if os.path.exists(png_output):
                            icon_found = True
                    elif shutil.which("iconutil"):
                        # macOS host tool if running on Darwin
                        pass

        if not icon_found:
            # Fallback default icon
            icon_target_name = "application-x-executable"

        desktop_file_content = f"""[Desktop Entry]
Type=Application
Name={app_name} (Osxie)
Exec=osxie run "{app_path}"
Icon={icon_target_name}
Terminal=false
Categories=Application;Utility;
MimeType=application/x-apple-diskimage;
"""
        desktop_file_path = os.path.join(desktop_dir, f"osxie-{app_name}.desktop")
        with open(desktop_file_path, "w") as df:
            df.write(desktop_file_content)
        print(f"Generated launcher with osxie run: {desktop_file_path} (Icon: {icon_target_name})")

if __name__ == "__main__":
    setup_osxie_integration()
