
# coding: utf-8

# In[17]:

import re

# open new obj file 
f = open("mouse03_clean_faces.obj", 'w')
flag = 0
count = 0
vcount = 0
prev_count = 0
with open("mouse03.obj", "r") as fin:
    for line in fin:
        tokens = line.split()
        if (tokens[0] == 'o'):
            name = tokens[1]
            name = name.lower()
            tokens = re.findall(r"([\w_']*)", name)
            tokens = tokens[0].split('_')
            if ("glycogen" in tokens or "surf" in tokens or "curve" in tokens or 
                "surface" in tokens or 'length' in tokens or 'sphere' in tokens or 'submesh' in tokens): 
                flag = 0
                count = count + 1
            else:
                flag = 1
                f.write(line)  
            print "prev_count: ", prev_count, " vcount: ", vcount
            prev_count = prev_count + vcount
            vcount = 0
        elif (tokens[0] == 'v'):
            if (flag == 1):
                f.write(line)
            
            vcount = vcount + 1
        elif (tokens[0] == 'f'):
            if (flag == 1):
                face_number = [0, 0, 0]
                face_number[0] = int(tokens[1]) - prev_count
                face_number[1] = int(tokens[2]) - prev_count
                face_number[2] = int(tokens[3]) - prev_count
                newLine = tokens[0] + " " + str(face_number[0]) + " " + str(face_number[1])  + " " + str(face_number[2]) + "\n"
                f.write(newLine)

print "Skipped: ", count, " objects."
f.close()



