#include "quadtree.h"
#include <iostream>

QuadTreeNode::QuadTreeNode(Color c) : color(c) {
    for (int i = 0; i < 4; i++) children[i] = nullptr;
}

QuadTreeNode::~QuadTreeNode() {
    for (int i = 0; i < 4; i++) delete children[i];
}

QuadTree::QuadTree() : root(nullptr) {}

QuadTree::~QuadTree() {
    destroy(root);
}

void QuadTree::destroy(QuadTreeNode* node) {
    if (node) {
        for (int i = 0; i < 4; i++) destroy(node->children[i]);
        delete node;
    }
}

QuadTreeNode* QuadTree::build(const std::vector<std::vector<Color>>& image, int x, int y, int size) {
    Color first = image[y][x];
    bool same = true;
    for (int i = y; i < y + size && same; ++i) {
        for (int j = x; j < x + size && same; ++j) {
            if (image[i][j] != first) same = false;
        }
    }

    if (same) return new QuadTreeNode(first);

    QuadTreeNode* node = new QuadTreeNode(MIXED);
    int half = size / 2;
    node->children[0] = build(image, x, y, half);
    node->children[1] = build(image, x + half, y, half);
    node->children[2] = build(image, x, y + half, half);
    node->children[3] = build(image, x + half, y + half, half);
    return node;
}

void QuadTree::compress(const std::vector<std::vector<Color>>& image) {
    destroy(root);
    root = build(image, 0, 0, image.size());
}

std::vector<char> QuadTree::serialize() {
    std::vector<char> result;
    serialize(root, result);
    return result;
}

void QuadTree::serialize(QuadTreeNode* node, std::vector<char>& result) {
    if (!node) return;
    if (node->color == WHITE) result.push_back('W');
    else if (node->color == BLACK) result.push_back('B');
    else {
        result.push_back('N');
        for (int i = 0; i < 4; ++i)
            serialize(node->children[i], result);
    }
}

void QuadTree::deserialize(const std::vector<char>& data) {
    destroy(root);
    int idx = 0;
    deserializeHelper(data, idx, root);
}

void QuadTree::deserializeHelper(const std::vector<char>& data, int& index, QuadTreeNode*& node) {
    if (index >= data.size()) return;

    char c = data[index++];
    if (c == 'W') node = new QuadTreeNode(WHITE);
    else if (c == 'B') node = new QuadTreeNode(BLACK);
    else {
        node = new QuadTreeNode(MIXED);
        for (int i = 0; i < 4; ++i)
            deserializeHelper(data, index, node->children[i]);
    }
}

void QuadTree::decompress(std::vector<std::vector<Color>>& image, int size) {
    image.assign(size, std::vector<Color>(size, WHITE));
    reconstruct(root, image, 0, 0, size);
}

void QuadTree::reconstruct(QuadTreeNode* node, std::vector<std::vector<Color>>& image, int x, int y, int size) {
    if (!node) return;
    if (node->color == BLACK || node->color == WHITE) {
        for (int i = y; i < y + size; ++i)
            for (int j = x; j < x + size; ++j)
                image[i][j] = node->color;
    } else {
        int half = size / 2;
        reconstruct(node->children[0], image, x, y, half);
        reconstruct(node->children[1], image, x + half, y, half);
        reconstruct(node->children[2], image, x, y + half, half);
        reconstruct(node->children[3], image, x + half, y + half, half);
    }
}
