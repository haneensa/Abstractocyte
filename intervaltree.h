//#ifndef INTERVALTREE_H
//#define INTERVALTREE_H

//#include <QVector4D>

//struct properties {
//    QVector4D pos_alpha;
//    QVector4D trans_alpha;
//    QVector4D color_alpha;
//    QVector4D point_size;
//    QVector4D extra_info;
//    QVector4D render_type; // mesh triangles, mesh points, points skeleton, graph (points, edges)
//};

//// Structure to represent an interval
//struct Interval
//{
//    int low, high;
//    struct properties int_properties;
//};

//// Structure to represent a node in interval search tree
//struct ITNode
//{
//    Interval *i; // 'i' could also be a normal variable
//    int max;
//    ITNode *left, *right;
//};

//class IntervalTree
//{
//public:
//    IntervalTree();



//public:
//    void insertIntervals(Interval ints[], int n);
//    Interval * getInterval(int p);

//protected:
//    ITNode *newNode(Interval i);
//    ITNode *insert(ITNode *root, Interval i);
//    bool doOverlap(Interval i1, Interval i2);
//    Interval *overlapSearch(ITNode *root, Interval i);
//    void inorder(ITNode *root);

//protected:
//    ITNode *root;

//};

//#endif // INTERVALTREE_H
