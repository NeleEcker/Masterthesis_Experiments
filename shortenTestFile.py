#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Oct 13 16:22:12 2019

@author: necker
"""

longfile = open('./masterthesis/dbpedia/test2idLong.txt', 'r')
newfile = open('./masterthesis/dbpedia/test2id.txt', 'w+')

line = longfile.readline()
cnt = 1

while line:
    newfile.write(line)
    line = longfile.readline()
    cnt += 1
    
    if cnt >= 20000:
        break
    
longfile.close()
newfile.close()