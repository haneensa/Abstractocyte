
# coding: utf-8

# script to convert PNG stack to H5 for vcg-thin-meshgen framework

# example: 
# 449 slices -> 16 chunks in each chunk, slices are represented as groups, in each -> (3, N) 
# (0, N) -> flat 
# (1, N) -> ID
# (2, N) -> ?
# (449, 999, 999)

import os
import sys
import string
import math
import mahotas
import numpy as np
import h5py
import glob
import sys
sys.path.append('/Users/haneen')
import cv2


# In[3]:

# number of chunks = # original slices/ #compressed slices
def mkdir_safe( dir_to_make ):
    if not os.path.exists( dir_to_make ):
        execute_string = 'mkdir ' + '"' + dir_to_make + '"'
        print execute_string
        print os.system( execute_string )


def load_id_image ( file_path ):
    ids = np.int32( np.array( mahotas.imread( file_path ) ) )
    # ids.shape = (999, 999, 4)
    # ids[0][0] = [  0   0   0 255]
    if len( ids.shape ) == 3:
        ids = ids[ :, :, 2 ] + ids[ :, :, 1 ] * 2**8 + ids[ :, :, 0 ] * 2**16
    else:
        # Read from pipeline format
        ids = ids.transpose() - 1
    return ids

original_input_ids_path       = '../Astrocytes_png/'
output_path                   = '../Astrocytes_h5'
mkdir_safe( output_path )
input_file_format             = 'png'

# count how many input images are loaded
input_search_string  = original_input_ids_path + '/*.' + input_file_format # -> ../input/segmentation/*.png
files                = sorted( glob.glob( input_search_string ) )
print "Found {0} input images in {1}".format( len(files), input_search_string )
nimages_to_process            = len(files) 

if len(files) > 0:
    id_max               = 0;
    id_counts            = np.zeros( 0, dtype=np.int64 );
    tile_index_z         = 0
    K                    = 10 # how many slices per chunk
    
    fout = open(output_path + "/chunkMap.txt", "w")
    
    i, j = 0, 0
    s = 0
    MAX = 0
    D = 3 # dimension
    voxelsPerChunk = 0
    for file in files:  
        print file     
        original_input_ids_name = file
        original_ids = load_id_image( original_input_ids_name )
        ( original_image_num_pixels_x, original_image_num_pixels_y ) = original_ids.shape
        print "original_ids.shape: " + str(original_ids.shape) #(999, 999)
        print np.unique(original_ids)
        N = 0
        filtered_ids, filtered_index = [], []
        for row in original_ids:
            for pixel in row:
                if (pixel):    
                    filtered_ids.append(pixel)
                    filtered_index.append(N)
                
                N = N + 1                    
        print np.unique(filtered_ids)

        # open the hdf5 file every offset, open a file, and write to it the chunk of slices
        if (tile_index_z%K == 0):
            if (voxelsPerChunk > MAX):
                MAX = voxelsPerChunk
            voxelsPerChunk = 0
            if (tile_index_z + K > nimages_to_process):
                i = j
                j = nimages_to_process
                print str(i) + " " + str(j)
                fout.write(str(i) + " " + str(j) + "\n")
                print "tile_index_z: " + str(tile_index_z)
                current_tile_ids_name    = output_path    + '/%d' % ( s ) + '.h5'
                group = 0
                s = s + 1
                print current_tile_ids_name
                hdf5             = h5py.File( current_tile_ids_name, 'w' )
            else:
                print "tile_index_z: " + str(tile_index_z)
                current_tile_ids_name    = output_path    + '/%d' % ( s ) + '.h5'
                group = 0
                s = s + 1
                print current_tile_ids_name
                hdf5             = h5py.File( current_tile_ids_name, 'w' )

                ########## chunkMapping.txt ###############
                # (i, j), i=j, j = i+K, init: i = 0, j = 0 
                i = j
                j = i + K
                print str(i) + " " + str(j)
                fout.write(str(i) + " " + str(j) + "\n")
        
        N = original_image_num_pixels_x * original_image_num_pixels_x

        tile_index_z = tile_index_z + 1
        D = 3
        if (len(filtered_ids) <= 0):
            print "SKIPED!"
            D = 1
            filtered_index.append(0)
            
        
        voxelsPerChunk += len(filtered_ids)
            
        group_name = '%d' % (group) 
        group = group + 1
        dataset = hdf5.get("/").create_dataset(group_name, (D , len(filtered_index)))
        print "Dataset shape: " + str(dataset.shape) + ", Dataset type: " + str(dataset.dtype)

        print "Writing: " + str(tile_index_z)
        if (D == 1):
            dataset[0,:]  = 0
            continue
        
        # flattened ij -> index = x + y * WIDTH
        # original_ids[d] -> d = 1 (y) , d = 0 (x)
        dataset[0, :]  = filtered_index
        # object id
        # filtered_ids, filtered_index
        dataset[1, :] = filtered_ids
        # birth
        dataset[2, :] = 1

         
        if (tile_index_z%(K) == 0):
            hdf5.close()
            
        if tile_index_z > nimages_to_process:
            print str(tile_index_z) + " >= " + str(nimages_to_process)
            break
                
    fout.close() 
hdf5.close()