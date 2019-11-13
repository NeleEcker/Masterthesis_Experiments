#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Oct 13 16:22:12 2019

@author: necker
"""

longfile = open('./masterthesis/DBPedia500K/test2idLong.txt', 'r')
newfile = open('./masterthesis/DBPedia500K/test2id.txt', 'w+')

line = longfile.readline()
line = longfile.readline()
cnt = 0
newfile.write("10\n")

while line:
    newfile.write(line)
    line = longfile.readline()
    cnt += 1
    
    if cnt >= 10:
        break
    
longfile.close()
newfile.close()