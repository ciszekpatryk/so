#!/usr/bin/env python3

import os

def get_username(uid):
    try:
        import pwd
        return pwd.getpwuid(uid).pw_name
    except Exception:
        return str(uid)

def list_processes():
    proc_path = '/proc'
    processes = []

    for pid in os.listdir(proc_path):
        if pid.isdigit():
            try:
                
        