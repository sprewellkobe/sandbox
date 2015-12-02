#!/bin/bash
MYPATH=`pwd`
LD_PRELOAD=${MYPATH}/libkobehook.so ./hooktest
