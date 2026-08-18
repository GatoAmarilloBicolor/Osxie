#!/usr/bin/env python3
import os
import sys

def main():
    print("xcode-select: note: installing command line developer tools...")
    prefix = os.environ.get("DPREFIX", os.path.expanduser("~/.osxie"))
    clt_dir = "/Library/Developer/CommandLineTools"
    
    try:
        os.makedirs(os.path.join(clt_dir, "usr", "lib"), exist_ok=True)
        os.makedirs(os.path.join(clt_dir, "usr", "include"), exist_ok=True)
    except PermissionError:
        clt_dir = os.path.join(prefix, "Volumes", "SystemRoot", "Library", "Developer", "CommandLineTools")
        os.makedirs(os.path.join(clt_dir, "usr", "lib"), exist_ok=True)
        os.makedirs(os.path.join(clt_dir, "usr", "include"), exist_ok=True)
        
    libxcrun = os.path.join(clt_dir, "usr", "lib", "libxcrun.dylib")
    with open(libxcrun, "w") as f:
        f.write("real-clt-stub\n")
        
    print("xcode-select: command line tools have been successfully installed.")

if __name__ == "__main__":
    main()
