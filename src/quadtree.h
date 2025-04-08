#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <string>

enum Color { BLACK, WHITE, MIXED };

struct QuadTreeNode {
    Color color;
    QuadTreeNode* children[4]; // Urutan: top-left, top-right, bottom-left, bottom-right

    QuadTreeNode(Color c);
    ~QuadTreeNode();
};

class QuadTree {
private:
    QuadTreeNode* root;

    QuadTreeNode* build(const std::vector<std::vector<Color>>& image, int x, int y, int size);
    void destroy(QuadTreeNode* node);
    void serialize(QuadTreeNode* node, std::vector<char>& result);
    void deserializeHelper(const std::vector<char>& data, int& index, QuadTreeNode*& node);
    void reconstruct(QuadTreeNode* node, std::vector<std::vector<Color>>& image, int x, int y, int size);

public:
    QuadTree();
    ~QuadTree();

    void compress(const std::vector<std::vector<Color>>& image);
    void decompress(std::vector<std::vector<Color>>& image, int size);
    std::vector<char> serialize();
    void deserialize(const std::vector<char>& data);
};

#endif
