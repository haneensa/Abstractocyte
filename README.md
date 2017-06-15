# Abstractocyte

This project provides the implementation of Abstractocyte framework, which is a visual tool for exploring nanoscale astroglial cells.

## Getting Started

The project is implemented in C++ with Qt for the GUI elements, and OpenGL 4.3 with GLSL version 4.3 for support of the Shader Storae Buffer Object (SSBO).

### Input and Data Preprocessing

This framework depends on three input data:
1. [XML Data](https://github.com/haneensa/Abstractocyte/wiki/XML-Data) storing the set of data needed for mesh and skeleton rendering with the mapping between them that is required for the interpolation of the abstraction
levels. 
2. [Metadata as HVGX file](https://github.com/haneensa/Abstractocyte/wiki/Metadata-(HVGX)) which has
contextual information regarding anatomical objects the data contains
3. [Binary Segm. Stack](https://github.com/haneensa/Abstractocyte/wiki/Binary-Segmentation-Stack) for proximity analysis.
