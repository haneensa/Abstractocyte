
# coding: utf-8

# merge information from .obj data and metadata in hvgx file

import re
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import json
import csv
import sys
import math


numLines = 0
vcount, fcount = 0, 0
vlist, flist, olist = [], [], []
flag = 0
with open("../data/mouse.obj", "r") as fin:
    for line in fin:
        numLines = numLines + 1
        tokens = line.split()
        if (tokens[0] == 'o'):
            name = tokens[1]
            olist.append(name.lower())
            if (flag == 1):
                vlist.append(vcount)
                flist.append(fcount)
                flag = 0
            flag = 1
            vcount = 0
            fcount = 0
        elif (tokens[0] == 'v'):
            vcount = vcount + 1
        elif (tokens[0] == 'f'):
            fcount = fcount + 1    
vlist.append(vcount)
flist.append(fcount)
print len(olist)
print len(vlist)
print len(flist)

df_obj = pd.DataFrame({'name': olist, 'vcount': vlist, 'fcount': flist})

# read hvgx file
sg = {}
SG = []
parent = ''
with open("../data/mouse_metadata.hvgx", "r") as fin:
    for line in fin:
        tokens = line.split(',')
        if (tokens[0] == 'sg'): 
            #12) sg: segmentation metadata
            # vast_id, flags, r,g,b, pattern1, r2, g2, b2, pattern2, anchorx, anchory, anchorz, 
            # parent_id, child_id, previous_sibiling_id, next_sibiling_id, collapsed, bboxx1, 
            # bboxy1, bboxz1, bboxx2, bboxy2, bboxz2, voxels, type, object_id, name
            name = tokens[-1][:-2]
            if name[:3] == 'bou' or name[:3] == 'spi':
                name_tok = name.split('.')
                name = name + "." + str(parent)
            elif name[:3] == 'den':
                parent =  name
            elif name[:3] == 'axo':
                parent = name
                
            if (bool(sg.get(name))):
                print "dup: ", name
                
            # remove \r\n from name
            tokens[-1] = tokens[-1][:-2]  
                
            sg[name] = tokens[1:]
            
            SG.append(tokens[1:])
        
print "sg (segmentation metadata): " + str(len(sg))


df_sg = pd.DataFrame(SG, columns = ['vast_id', 'flags', 'r', 'g', 'b', 'pattern1', 'r2', 'g2', 'b2', 
                                    'pattern2', 'anchorx', 'anchory', 'anchorz', 
                                    'parent_id', 'child_id', 'previous_sibiling_id', 
                                    'next_sibiling_id', 'collapsed', 'bboxx1', 
                                    'bboxy1', 'bboxz1', 'bboxx2', 'bboxy2', 'bboxz2', 
                                    'voxels', 'type', 'object_id', 'name'])

df_sg['name'] = df_sg['name'].apply(lambda x: x[:-2])
df_sg.drop(df_sg.columns[[5, 6, 7, 8, 9, 10, 11, 12, 18, 19, 20, 21, 22, 23, 24]], axis=1, inplace=True)
print df_sg.type.unique()

header = ['hvgxId', 'hvgxName', 'objName']
hvgxToObj = {}

parent = -1
parentName = ''
sbCounter = 0
for obj in olist:
    # clean the name
    # check in sg metada
    key = obj
    code =  obj[:3]
    tokens = re.findall(r"([\w_']*)", obj)
    tokens = tokens[0].split('_')
    if ("surf" in tokens or "curve" in tokens or "surface" in tokens or 'length' in tokens or 'sphere' in tokens or 'submesh' in tokens): 
        continue

    tokens = obj.split('.')    
    key = tokens[0]

    if (code[0] == 'd' and code[1] != 'e'):
        # synapse
        key = obj.split('[')[0]
    elif (code == 'bou'):
        k = 0
        key = tokens[0] +"."+ pAxonName
    elif (code == 'spi'):
        key = tokens[0] +"."+ pDedriteName
    elif (code == 'gly'):
        continue
        
    if (not bool(sg.get(key))):
        print 'DIDNT FIND IN SG', key
        continue
      
    if (code == 'spi'):
        hvgxID = sg[key][26]
        parent = sg[key][13]
        name = sg[key][27][:-2]
             
    elif (code == 'bou'):
        hvgxID = sg[key][26]
        parent = sg[key][13]
        name = sg[key][27][:-2]

    elif (code == 'den'):
        pDendriteID = sg[key][26]
        pDedriteName = sg[key][27]
    elif (code == 'axo'):
        pAxonID = sg[key][26]
        pAxonName = sg[key][27]
    
    hvgxToObj[obj] = sg[key]


df_hvgxToObj = pd.DataFrame(hvgxToObj)


obj_properties = []
paxon = -1
pden = -1
for index, row in df_glyObj.iterrows():
    key = row['oname'].lower()
    if ( not bool(hvgxToObj.get(key))):
        continue
    Id = hvgxToObj[key][0]
    name = hvgxToObj[key][-1]
    type = hvgxToObj[key][-3]
    # 'oname', 'distance', 'glyId', 'clusters', 'count'
    glyId = row['glyId']
    clusters = row['clusters']
    count = row['count']
     
    exitory_inhibit = -1
    if (bool(axons.get(name))):
        exitory_inhibit = axons[name][1]
        paxon = exitory_inhibit
    elif (bool(dn.get(name))):
        exitory_inhibit = dn[name][1]
        pden = exitory_inhibit
    #   get the volume from these: Allpixels, Astropixels, ADpixels, NSpixels, Spixels
    # and bool(Spixels.get(str(Id)))
    if type == 'SYNAPSE' and bool(Spixels.get(str(Id))):
        vol =  Spixels.get(str(Id))
    elif (type == 'SPINE' or type == 'BOUTON') and bool(NSpixels.get(str(Id))):
        vol =  NSpixels.get(str(Id))
        if (type == 'SPINE'):
            exitory_inhibit  =  pden
        else:
            exitory_inhibit  =  paxon
    elif (type == 'AXON' or type == 'DENDRITE') and bool(ADpixels.get(str(Id))):
        vol =  ADpixels.get(str(Id))
    elif ( bool(Allpixels.get(str(Id)))):
        vol =  Allpixels.get(str(Id))
    #print Id, name, type, distance, glyId, clusters, count
    obj_properties.append([int(Id), name, key, type, glyId, clusters, count, vol, int(exitory_inhibit)])

df_obj_properties = pd.DataFrame(obj_properties, columns=['Id', 'hvgxName', 'objName', 'type', 
                                                          'glyId', 'clusters', 'count', 'volume', 'exitory_inhibit'])
df_obj_properties.to_csv('mouse_df_obj_properties.csv')

