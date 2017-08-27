
# coding: utf-8

# program to label Avizo skeletonization data points output with the ID in the related object ID
# input files should be grouped in one folder
# 1- nodes.csv
# 2- data_skeleton.h5 (output of vcg-thin-meshgen framework)
# 3- data_original.h5 (original data converted from png to H5)
# 4- segments.csv (output from Avizo skeletonization)
# 5- points.csv   (output from Avizo skeletonization)

base_directory = "../data"

import pandas as pd
import h5py
import glob
import numpy as np
import sys
import math
import csv


# open excel sheet 
df_nodes = pd.read_csv(base_directory + '/nodes.csv')

# read the h5 file
h5dir = base_directory + '/data_skeleton.h5'
with h5py.File(h5dir, 'r') as hf:
        # i want them sorted. from 0 to max chunk
        print h5dir, 'List of arrays in this file: \n', hf.keys()
        
        for key in hf.keys():
            data = hf.get(key)
            np_data = np.array(data)

h5dir2 = base_directory +  '/data_original.h5'
with h5py.File(h5dir2, 'r') as hf:
        # sort from 0 to max chunk
        print h5dir2, 'List of arrays in this file: \n', hf.keys()
        
        for key in hf.keys():
            data = hf.get(key)
            np_data2 = np.array(data)


def get_non_zero(ID0, ID1, ID2, ID3, ID4, ID5, ID6):
    if (ID0):
        return ID0
    if (ID1):
        return ID1    
    if (ID2):
        return ID2    
    if (ID3):
        return ID3    
    if (ID4):
        return ID4
    if (ID5):
        return ID5
    if (ID6):
        return ID6
    return ID0

def getID(x, y, z, data):
    # access the zth slice
    ID0 = data[x][y][z]
    
    ID1 = data[x][y][z - 1]
    ID2 = data[x][y][z + 1]
        
    ID3 = data[x - 1][y][z]
    ID4 = data[x + 1][y][z]
        
    ID5 = data[x][y - 1][z]
    ID6 = data[x][y + 1][z]
        
    ID = get_non_zero(ID0, ID1, ID2, ID3, ID4, ID5, ID6)
    
    return ID

x_max = -sys.maxint - 1
y_max = -sys.maxint - 1
z_max = -sys.maxint - 1

x_min = sys.maxint
y_min = sys.maxint
z_min = sys.maxint
 
id_max = -sys.maxint - 1
id_min = sys.maxint

fout = open(base_directory + '/nodes_Ids.csv', 'w')
writer = csv.writer(fout, delimiter=',')
new_row = ['Node ID', 'X Coord', 'Y Coord',  'Z Coord', 'Coordination Number', 'Object ID']
writer.writerow(new_row)
count = 0
for index, row in df_nodes.iterrows():
    x = int(row['X Coord'])
    y = int(row['Y Coord'])
    z = float(row['Z Coord'])
    z = round(z, 4)
    
    z_index = round(z/2.2249, 0)
    z_index = int(z_index)

    ID = getID(x, y, z_index, np_data)
    if (ID < 1):
        ID = getID(x, y, z_index, np_data2)
        if (ID < 1):
            print count, ": ", x, y, z
            count = count + 1

    if (ID > id_max):
        id_max = ID
    
    if (ID < id_min):
        id_min = ID
        
    new_row = [row['Node ID'], row['X Coord'], row['Y Coord'], z_index, row['Coordination Number'], ID]
    writer.writerow(new_row)
    
    if (x > x_max):
        x_max = x
    if (y > y_max):
        y_max = y
    if (z > z_max):
        z_max = z   
        
    if (x < x_min):
        x_min = x
    if (y < y_min):
        y_min = y
    if (z < z_min):
        z_min = z   
        
print "Max: ", x_max, y_max, z_max, id_max
print "Min: ", x_min, y_min, z_min, id_min
fout.close()


df_nodes_ids = pd.read_csv(base_directory + '/nodes_Ids.csv')
# hash function on node ID and Objet ID as value
nodes_id_dict = df_nodes_ids.set_index('Node ID')['Object ID'].to_dict()


df_segments = pd.read_csv(base_directory + '/segments.csv') 

# iterate through df_segments, and label each segment with an ID
fout = open(base_directory + '/segments_Ids.csv', 'w')
writer = csv.writer(fout, delimiter=',')
new_row = ['Segment ID', 'Node ID #1',  'Node ID #2', 'Point IDs', 'Object ID']
writer.writerow(new_row)
pointID_objID = {}
for index, row in df_segments.iterrows():
    node1 = row['Node ID #1']
    node2 = row['Node ID #2']
    
    
    ID = nodes_id_dict[node1]
    new_row = [row['Segment ID'], row['Node ID #1'], row['Node ID #2'], z_index, row['Point IDs'], ID]
    writer.writerow(new_row)
    
    points = row['Point IDs'].split(',')
    if ID == 746 or ID == 745:
        print row
        
    for p in points:
        pointID_objID[p] = ID
        
fout.close()


df_segments_ids = pd.read_csv(base_directory + '/segments_Ids.csv') 
df_segments_ids


# label points
# open excel sheet 
def getPointID(x):
    return pointID_objID[str(x)]

df_points = pd.read_csv(base_directory + '/points.csv')
df_points['Object ID'] = df_points['Point ID'].apply(getPointID)


df_points.to_csv(base_directory + '/points_ids.csv',  index=False)

