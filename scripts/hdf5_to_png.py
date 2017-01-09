import os
import sys
import string
import math
import mahotas
import numpy as np
import h5py
import glob
import lxml
import lxml.etree
import sys
sys.path.append('/Users/haneen')
import cv2
from multiprocessing import Pool
from multiprocessing.dummy import Pool as ThreadPool
import math

'''
input: 
1- dim1, dim2, dim3
'''

DIMS = [999, 999, 449]
hdf5 = h5py.File( '../data/slices_unflattened_skeleton_m3.h5', 'w' )
dataset = hdf5.get("/").create_dataset('channel10', (DIMS[0], DIMS[1], DIMS[2]))
indices_i = []

def getMap(chunkMap):
    prev = 0
    f = open( chunkMap, 'r' )
    chunkMapArray = []
    chunkMapArray.append(0)
    for line in f:
    	token = line.split()
    	token = [int(i) for i in token]
    	num = token[1] - token[0] - 1
    	chunkMapArray.append(num + prev + 1)
    	prev = prev + num + 1
    
    return chunkMapArray



def run(h5dir, chunkMapArray):
    # unflatten the data. collect the chunks into one big chunk,
    # with z = 449 slices, x=999 (width), y=999 (hight)
    h5files = sorted( glob.glob( h5dir ) )
    slices_xy = [0] * DIMS[2]
    for file in h5files:
    	print file
        with h5py.File(file, 'r') as hf:
            # i want them sorted. from 0 to max chunk
            # print file, 'List of arrays in this file: \n', hf.keys()
            for key in hf.keys():
                i = file.split('/')[-1][:-3]
                data = hf.get(key)
                np_data = np.array(data)
                offset = chunkMapArray[int(i)]
                index = offset + int(key)
                print i, key, index
                slices_xy[index] = np_data

    return slices_xy

def subFiller(slices_XY):
    #offset = inputs[0]
    #print "offset: ", offset
    #slices_XY = inputs[1]
    i = 0
    # make this function
    for slice in slices_XY:
        if (len(slice) == 1):
            continue
       
        xy = slice[0]    
        Ids = slice[1]
        
        # flat = x * width + y
        # unflattened x = floor(flattened / width)?
        # unflatteded y = flattened - x * width
        # 0 1 2 3 4 -> 0, 1, 2, 3, 4 -> 
        # 0 1 2 3 4 -> 5, 6, 7, 8, 9
        # 0 1 2 3 4 -> 10, ...
        # 0 1 2 3 4
        XY = np.zeros((DIMS[0], DIMS[1]), dtype='int32')
        for index in range(len(xy)):
            flat_xy = xy[index]
            x = math.floor(flat_xy/DIMS[0])
            y = flat_xy - x * DIMS[0]
            XY[y][x] = Ids[index]
            
        
        flag = 0
        # write this slice to hdf5 file
        indices_i.append(i)
        dataset[:, :, i] = XY
        i = i + 1 

    print indices_i
    return True
def chunks(l, n):
    """ Yield successive n-sized chunks from l."""
    for i in range(0, len(l), n):
        yield l[i: i + n]

if __name__ == '__main__':
	print "Start Processing .."
	
	print 'Number of arguments:', len(sys.argv), 'arguments.'
	print 'Argument List:', str(sys.argv)
	
	base_dir = sys.argv[1]

	chunkMap = base_dir + '/chunkMap.txt'
	chunkMapArray = getMap(chunkMap)

	h5dir = base_dir + '/*.h5'
	slices_xy = run(h5dir, chunkMapArray)
	
	print 'len(slices_xy): ', len(slices_xy)
	print 'each entry has ', len(slices_xy[0]), ' entries.'
	print 'Each entry has variying number of flattened xys', len(slices_xy[10][1])
	
	# iterate over the slices in order and reconstruct one hdf5 file
	i = 0
	
	#pool = ThreadPool()
	#n = 50
	#chunked = list(chunks(slices_xy, n))
	#offsets = range(len(chunked))
	#offsets = [n*k for k in offsets]
	#print offsets
	#zipped = zip(offsets, chunked)
	#thread_results = pool.map(subFiller, zipped)
    #zipped = zip(0, slices_xy)
	subFiller(slices_xy)
	hdf5.close()
