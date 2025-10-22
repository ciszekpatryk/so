#!/usr/bin/env python3
import os
import pwd

def list_proc():
    print("USER        PID     COMMAND")
    for pid in os.listdir("/proc"):
        if not pid.isdigit():
            continue
        try:
            uid = os.stat(f"/proc/{pid}").st_uid
            user = pwd.getpwuid(uid).pw_name
            with open(f"/proc/{pid}/comm") as f:
                cmd = f.readline().strip()
            print(f"{user:<10} {pid:<7} {cmd}")
        except (FileNotFoundError, PermissionError):
            continue

if __name__ == "__main__":
    list_proc()

        
