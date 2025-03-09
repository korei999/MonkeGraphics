#!/bin/bash

scc --exclude-dir src/platform/wayland/wayland-protocols/ -n glcorearb.h,khrplatform.h,wglext.h src/
