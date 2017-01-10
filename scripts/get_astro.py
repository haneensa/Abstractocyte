
# coding: utf-8

# In[17]:

import re


# todo: I need to subtract only if we are removing the object
# open new obj file 
f = open("../data/mouse03_astro.obj", 'w')
flag = 0
vcount = 0
with open("../data/mouse03.obj", "r") as fin:
    for line in fin:
        tokens = line.split()
        if (tokens[0] == 'o'):
            name = tokens[1]
            name = name.lower()
            if (name[:3] == "ast"):
            	print name
                flag = 1
                f.write(line)  
            else:
            	flag = 0

            # decide the object type
            # add new description ex: "t astrocyte" -> type
            # add new description ex: "d 754" -> ID
        elif (tokens[0] == 'v'):
            if (flag == 1):
                f.write(line)
            else:
            	vcount = vcount + 1

        elif (tokens[0] == 'f'):
            if (flag == 1):
                face_number = [0, 0, 0]
                face_number[0] = int(tokens[1]) - vcount
                face_number[1] = int(tokens[2]) - vcount
                face_number[2] = int(tokens[3]) - vcount
                newLine = tokens[0] + " " + str(face_number[0]) + " " + str(face_number[1])  + " " + str(face_number[2]) + "\n"
                f.write(newLine)

f.close()



