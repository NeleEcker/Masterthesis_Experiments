#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Oct 13 16:37:03 2019

@author: necker
"""


newfile = open ('./masterthesis/dbpediaResults/TransE/corruptFalse/generalTimes2.txt', 'w+')
oldfile = open('./masterthesis/dbpediaResults/TransE/corruptFalse/generalTimes.txt', 'r')

line = oldfile.readline()
cnt = 0

while line:
    if cnt >= 1124:
        newfile.write(line)
        
    line = oldfile.readline()
    cnt += 1
    
oldfile.close()
newfile.close()
