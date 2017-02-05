//#include "intervaltree.h"
//#include <QDebug>

//IntervalTree::IntervalTree()
//{
//    root = NULL;
//}

//void IntervalTree::insertIntervals(Interval ints[], int n)
//{
//    for (int i = 0; i < n; i++)
//        root = insert(root, ints[i]);
//    inorder(root);
//}

//Interval * IntervalTree::getInterval(int p)
//{
//    Interval x = {p, p};
//    return overlapSearch(root, x);
//}

//// A utility function to create a new interval search tree node
//ITNode* IntervalTree::newNode(Interval i)
//{
//    ITNode *temp = new ITNode;
//    temp->i = new Interval(i);
//    temp->max = i.high;
//    temp->left = temp->right = NULL;
//    return temp;
//}

//// A utility function to insert a new interval search tree node
//// this is similar to BST insert. Here the low value of interval
//// is used to maintain BST property
//ITNode *IntervalTree::insert(ITNode *root, Interval i)
//{
//    // Base case: Tree is empty, new node becomes root
//    if (root == NULL)
//        return newNode(i);

//    // Get low value of interval at root
//    int l = root->i->low;

//    // If root's low value is smaller, then new interval goes to
//    // left subtree
//    if (i.low < l)
//        root->left = insert(root->left, i);

//    // else, new node goes to right subtree
//    else
//        root->right = insert(root->right, i);

//    // Update the max value of this ancestor if needed
//    if (root->max < i.high)
//        root->max = i.high;

//    return root;
//}

//// A utility function to check if given two intervals overlap
//bool IntervalTree::doOverlap(Interval i1, Interval i2)
//{
//    if (i1.low <= i2.high && i2.low <= i1.high)
//        return true;
//    return false;
//}

//// The main function that searches a given interval i in a given
//// Interval tree
//Interval *IntervalTree::overlapSearch(ITNode *root, Interval i)
//{
//    // Base case, tree is empty
//    if (root == NULL) return NULL;

//    // if given interval overlaps with root
//    if (doOverlap(*(root->i), i))
//        return root->i;

//    // if left child of root is present and max of left child is
//    // greater than or equal to given interval, then i may
//    // overlap with an interval in left subtree
//    if (root->left != NULL && root->left->max >= i.low)
//        return overlapSearch(root->left, i);

//    // else interval can only overlap with right subtree
//    return overlapSearch(root->right, i);
//}

//void IntervalTree::inorder(ITNode *root)
//{
//    if (root == NULL) return;
//    inorder(root->left);

//    qDebug() << "[" << root->i->low << ", " << root->i->high << "]"
//        << " max = " << root->max << endl;

//    inorder(root->right);
//}
