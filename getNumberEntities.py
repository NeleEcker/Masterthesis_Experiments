#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Oct 17 20:42:49 2019

@author: necker
"""
entities = []
file = open('./masterthesis/dbpedia/train2id.txt', 'r')
line = file.readline()
line = file.readline()
while line:
    elements = line.split()
    #print (elements)
    if len(elements) == 3:
        if int(elements[2]) < 100:
            entities.append(elements[0])
            entities.append(elements[1])
        
    line = file.readline()

print('all lines watched')    
file.close()
print('transforming entities')
entities = list(dict.fromkeys(entities))
print (len(entities))     
    