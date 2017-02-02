#ifndef INTERVALTREE_H
#define INTERVALTREE_H

// Structure to represent an interval
struct Interval
{
    int low, high;
};

// Structure to represent a node in interval search tree
struct ITNode
{
    Interval *i; // 'i' could also be a normal variable
    int max;
    ITNode *left, *right;
};

class IntervalTree
{
public:
    IntervalTree();



public:
    void insertIntervals(Interval ints[], int n);
    void getInterval(int p);

protected:
    ITNode *newNode(Interval i);
    ITNode *insert(ITNode *root, Interval i);
    bool doOverlap(Interval i1, Interval i2);
    Interval *overlapSearch(ITNode *root, Interval i);
    void inorder(ITNode *root);

protected:
    ITNode *root;

};

#endif // INTERVALTREE_H
