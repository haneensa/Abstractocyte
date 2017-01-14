
# coding: utf-8

# In[17]:

import re

avg_cooridinate = {}

# todo: I need to subtract only if we are removing the object
# open new obj file 
vcount = 0
x = 0.0
y = 0.0
z = 0.0
flag = 0
name = ''
with open("../data/mouse03_neurites.obj", "r") as fin:
    for line in fin:
        tokens = line.split()
        if (tokens[0] == 'o'):
            if (flag == 1):
                avg_cooridinate[name] = [x/vcount, y/vcount, z/vcount]
            
            name = tokens[1]
            name = name.lower()
            flag = 1
            x = 0.0
            y = 0.0
            z = 0.0
            vcount = 0
        elif (tokens[0] == 'v'):
            x = x + float(tokens[1])
            y = y + float(tokens[2])
            z = y + float(tokens[3])
            
            vcount = vcount + 1

avg_cooridinate[name] = [x/vcount, y/vcount, z/vcount]

f = open("../data/mouse03_neurites_avg.obj", 'w')

coord = []
with open("../data/mouse03_neurites.obj", "r") as fin:
    for line in fin:
        tokens = line.split()
        if (tokens[0] == 'o'):
            name = tokens[1]
            name = name.lower()
            coord = avg_cooridinate[name]
            f.write(line)
        elif (tokens[0] == 'v'):
            newline = 'v '  + str(tokens[1]) + " " + str(tokens[2]) + " " + str(tokens[3]) + " " + str(coord[0]) + " "  + str(coord[1]) + " "  + str(coord[2]) + "\n"
            f.write(newline)
        else:
            f.write(line)

f.close()
