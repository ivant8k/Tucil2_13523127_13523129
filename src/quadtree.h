#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <string>
#include <memory>

using namespace std;

// Structure to store RGB values of a pixel
struct Pixel {
    unsigned char r, g, b;
};

// Structure to store statistics of a block
struct BlockStats {
    double meanR, meanG, meanB;
    double varianceR, varianceG, varianceB;
    double madR, madG, madB;
    double maxDiffR, maxDiffG, maxDiffB;
    double entropyR, entropyG, entropyB;
};

// QuadTree node structure
class QuadTreeNode {
public:
    int x, y, size;
    bool isLeaf;
    Pixel avgColor;
    QuadTreeNode* children[4]; // NW, NE, SW, SE

    QuadTreeNode(int x, int y, int size);
    ~QuadTreeNode();
};

// Function declarations
double calculateVariance(const vector<vector<Pixel>>& data, int x, int y, int size);
double calculateMAD(const vector<vector<Pixel>>& data, int x, int y, int size);
double calculateMaxDiff(const vector<vector<Pixel>>& data, int x, int y, int size);
double calculateEntropy(const vector<vector<Pixel>>& data, int x, int y, int size);
BlockStats calculateBlockStats(const vector<vector<Pixel>>& data, int x, int y, int size);
Pixel calculateAvgColor(const vector<vector<Pixel>>& data, int x, int y, int size);
double calculateError(const vector<vector<Pixel>>& data, int x, int y, int size, int method);
QuadTreeNode* buildQuadTree(const vector<vector<Pixel>>& data, int x, int y, int size, double threshold, int minBlockSize, int method, int depth = 0);
void reconstructImage(const QuadTreeNode* node, vector<vector<Pixel>>& outputImage);
bool saveQuadTreeImage(const string& filename, const vector<vector<Pixel>>& image);
int countNodes(const QuadTreeNode* node);
int getTreeDepth(const QuadTreeNode* node);
//bool generateGif(const string& filename, const QuadTreeNode* root, int width, int height);

#endif // QUADTREE_H