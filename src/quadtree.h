#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <memory>
#include <string>

struct Pixel {
    unsigned char r, g, b;
};

struct QuadNode {
    int x, y, width, height;
    Pixel color;
    std::unique_ptr<QuadNode> children[4]; // NW, NE, SW, SE
    bool isLeaf;

    QuadNode(int x, int y, int w, int h);
};

class Quadtree {
private:
    std::unique_ptr<QuadNode> root;
    std::vector<Pixel> image;
    int imgWidth, imgHeight;
    int minBlockSize;
    double threshold;
    int errorMethod;

    double calculateError(const std::vector<Pixel>& block);
    double calculateVariance(const std::vector<Pixel>& block);
    double calculateMAD(const std::vector<Pixel>& block);
    double calculateMaxDiff(const std::vector<Pixel>& block);
    double calculateEntropy(const std::vector<Pixel>& block);
    Pixel calculateAverageColor(const std::vector<Pixel>& block);
    void buildQuadtree(std::unique_ptr<QuadNode>& node);
    void reconstructImage(std::unique_ptr<QuadNode>& node, std::vector<Pixel>& output);

public:
    Quadtree(const std::string& filename, int method, double thresh, int minSize);
    void compress();
    void saveCompressedImage(const std::string& filename);
};

#endif