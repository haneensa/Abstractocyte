// need to download the PCL library
#include <pcl/point_cloud.h>
#include <pcl/octree/octree.h>

#include <iostream>
#include <vector>
#include <ctime>
#include <fstream>
#include <string>
#include <sstream>

pcl::PointXYZ get_nearest_skeleton(float x, float y, float z, 
        pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> octree, 
        pcl::PointCloud<pcl::PointXYZ>::Ptr cloud)
{
	pcl::PointXYZ searchPoint;
    searchPoint.x = x;
    searchPoint.y = y;
    searchPoint.z = z;
    // Neighbors withing Radius search
    std::vector<int> pointIdxKNNSearch;
    std::vector<float> pointKNNSquaredDistance;
    int K = 1;
    if (octree.nearestKSearch (searchPoint, K, pointIdxKNNSearch, pointKNNSquaredDistance) > 0) {
        return cloud->points[ pointIdxKNNSearch[0] ];
    }

}

int main (int argc, char **argv)
{
    if (argc < 3) {
        return -1;
    } else {
        std::cout << "skeleton: " << argv[1] << std::endl;
        std::cout << "mesh: " << argv[2] << std::endl;
    }
    
	srand ((unsigned int) time (NULL));
	// skeleton points
	std::vector <std::string> record;
	std::vector <std::vector <std::string> > skeleton_vertices;
	std::string item;
	std::ifstream infile(argv[1]);
	for (std::string line; std::getline(infile, line);) {
		std::istringstream ss(line);
		while(std::getline(ss, item, ',')) {
			record.push_back(item);
		}

		skeleton_vertices.push_back(record);
		record.clear();

	}
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);

	// Generate pointcloud
	// I have to get how many glycogen nodes 
	cloud->width = skeleton_vertices.size();
	cloud->height = 1;
	cloud->points.resize (cloud->width * cloud->height);
	
	// fill it with the glycogen points
	for (size_t i = 0; i < skeleton_vertices.size (); ++i)
	{
		// Point ID,thickness,X Coord,Y Coord,Z Coord
		float x = atof(skeleton_vertices[i][2].c_str());
		float y = atof(skeleton_vertices[i][3].c_str());
		float z = atof(skeleton_vertices[i][4].c_str());
		cloud->points[i].x = x;
		cloud->points[i].y = y;
		cloud->points[i].z = z;
	}
	float resolution = 0.1f; // length of the smallest voxels at lowest octree level
	pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> octree (resolution);

	octree.setInputCloud (cloud);
	octree.addPointsFromInputCloud ();

	// node in the skeleton
	// open the skeleton nodes here
	record.clear();

	int flag = 0;
	std::vector < std::vector <std::string> > mesh_vertices;
	std::ifstream infile2(argv[2]);
	for (std::string line; std::getline(infile2, line);) {
		std::istringstream ss(line);
		while(std::getline(ss, item, ' ')) {
            if (item[0] == 'v') {
                flag = 1;
                record.clear();
                continue;
            } 
            if (flag != 1) 
                continue;

            if (flag == 1) {
			    record.push_back(item);
            }
		}

        if (flag == 1) {
		    flag = 0;
		    float x = atof(record[0].c_str());
		    float y = atof(record[1].c_str());
		    float z = atof(record[2].c_str());
            pcl::PointXYZ skeleton_point = get_nearest_skeleton(x, y, z, octree, cloud);
            std::cout   << "v " << x << " " << y << " " << z 
                        << " " << skeleton_point.x
                        << " " << skeleton_point.y
                        << " " << skeleton_point.z << std::endl;
		    record.clear();
        } else {
            std::cout << line << std::endl;
        }
	}


    return 0;
}
