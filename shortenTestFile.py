#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Oct 13 16:22:12 2019

@author: necker
"""

longfile = open('./masterthesis/nell/longtest2id.txt', 'r')
newfile = open('./masterthesis/nell/test2id.txt', 'w+')

line = longfile.readline()
line = longfile.readline()
cnt = 0
newfile.write("80000\n")

while line:
    if cnt % 5 == 0:
        newfile.write(line)
    line = longfile.readline()
    cnt += 1
    
    if cnt / 80000 == 5:
        break
    
longfile.close()
newfile.close()