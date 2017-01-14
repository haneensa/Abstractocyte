
# coding: utf-8

# In[17]:

import re

avg_cooridinate = {}

# todo: I need to subtract only if we are removing the object
# open new obj file 
vcount = 0
x, y, z = '', '', ''
flag = 0
name = ''
with open("neurites_avg.txt", "r") as fin:
    for line in fin:
        tokens = line.split()
        if (tokens[0] == 'o'):
            if (flag == 1):
                avg_cooridinate[name] = [x, y, z]
            
            name = tokens[1]
            name = name.lower()
            flag = 1
        else:
            x = tokens[0]
            y = tokens[1]
            z = tokens[2]

avg_cooridinate[name] = [x, y, z]

f = open("../../data/mouse03_neurites_avg.obj", 'w')

coord = []
with open("../../data/mouse03_neurites.obj", "r") as fin:
    for line in fin:
        tokens = line.split()
        if (tokens[0] == 'o'):
            name = tokens[1]
            name = name.lower()
            print name
            coord = avg_cooridinate[name]
            f.write(line)
        elif (tokens[0] == 'v'):
            newline = 'v '  + tokens[1] + " " + tokens[2] + " " + tokens[3] + " " + coord[0] + " "  + coord[1] + " "  + coord[2] + "\n"
            f.write(newline)
        else:
            f.write(line)

f.close()
