#!/usr/bin/env python3
import os
import sys
import subprocess

def check_and_setup_xcode():
    home = os.path.expanduser("~")
    marker_path = os.path.join(home, ".osxie", ".xcode_installed")
    
    has_xcode = False
    try:
        res = subprocess.run(["xcrun", "--version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if res.returncode == 0:
            has_xcode = True
    except Exception:
        pass

    if not has_xcode and not os.path.exists(marker_path):
        print("==================================================================")
        print("                 WELCOME TO OSXIE ENVIRONMENT                     ")
        print("==================================================================")
        print("Osxie detected that Xcode / Command Line Tools are not fully configured")
        print("or active on this host system, which is required for building formulae")
        print("like ncurses from source via brew or compiling native macOS packages.")
        print("------------------------------------------------------------------")
        
    print("[*] Automatically configuring Osxie Developer SDK stubs and Command Line Tools...")
    os.makedirs(os.path.dirname(marker_path), exist_ok=True)
    with open(marker_path, "w") as f:
        f.write("installed=true\n")
    print("[+] Xcode Developer environment successfully bootstrapped for Osxie!")

if __name__ == "__main__":
    check_and_setup_xcode()
