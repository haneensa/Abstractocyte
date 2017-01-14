// Example program for the centroid() function for 2D points, 3D points and 3D triangles.
#include <CGAL/Simple_cartesian.h>
#include <CGAL/centroid.h>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

typedef double                      FT;
typedef CGAL::Simple_cartesian<FT>  K;
typedef K::Point_2                  Point_2;
typedef K::Point_3                  Point_3;
typedef K::Triangle_3               Triangle_3;
int main()
{
    const char *filename = "../../data/mouse03_neurites.obj";
    FILE *fp = fopen(filename, "r");
    if ( !fp  ) return 0;
    char line[1024];
    
    std::vector<Point_3> objects;
    std::list<Triangle_3> triangles_3;

    double x, y, z;
    int f1, f2, f3;
    std::string name;
    int flag = 0;
    int i = 0;
    // make this a function: fp, &objects
    while ( fgets( line, 1024, fp ) ) {
        // read obj vertices
        if (line[0] == 'o') {
            std::istringstream ss(line);
            std::string item;
            getline(ss, item, ',');
            if (flag == 1) {
                Point_3 c3 = CGAL::centroid(triangles_3.begin(), triangles_3.end(),CGAL::Dimension_tag<2>());
                std::cout << name;
                std::cout << c3  << std::endl;
            }
            
            name = line;

            objects.clear();
            triangles_3.clear();
            flag = 1;
        } else if (line[0] == 'v') {
            sscanf( line, "%*s%lf%lf%lf", &x, &y, &z );
            Point_3 p(x, y, z);
            objects.push_back(p);
        } else if (line[0] == 'f') {
            sscanf( line, "%*s%d%d%d", &f1, &f2, &f3 );
            Point_3 p1 = objects[f1];
            Point_3 p2 = objects[f2];
            Point_3 p3 = objects[f3];
            triangles_3.push_back(Triangle_3(p1,p2,p3));
        }
    }
    
    Point_3 c3 = CGAL::centroid(triangles_3.begin(), triangles_3.end(),CGAL::Dimension_tag<2>());
    std::cout << name;
    std::cout << c3 << std::endl;

    return 0;
}
