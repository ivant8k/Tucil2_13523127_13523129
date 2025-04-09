#include "quadtree.h"
#include <cmath>
#include <algorithm>
#include <map>
#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;

// QuadTreeNode constructor
QuadTreeNode::QuadTreeNode(int x, int y, int size) : x(x), y(y), size(size), isLeaf(true) {
    for (int i = 0; i < 4; i++) {
        children[i] = nullptr;
    }
}

// QuadTreeNode destructor
QuadTreeNode::~QuadTreeNode() {
    for (int i = 0; i < 4; i++) {
        if (children[i]) {
            delete children[i];
        }
    }
}

// Calculate average color of a block
Pixel calculateAvgColor(const vector<vector<Pixel>>& data, int x, int y, int size) {
    long sumR = 0, sumG = 0, sumB = 0;
    int count = 0;

    for (int j = y; j < y + size && j < data.size(); j++) {
        for (int i = x; i < x + size && i < data[0].size(); i++) {
            sumR += data[j][i].r;
            sumG += data[j][i].g;
            sumB += data[j][i].b;
            count++;
        }
    }

    Pixel avg;
    if (count > 0) {
        avg.r = static_cast<unsigned char>(sumR / count);
        avg.g = static_cast<unsigned char>(sumG / count);
        avg.b = static_cast<unsigned char>(sumB / count);
    } else {
        avg.r = avg.g = avg.b = 0;
    }

    return avg;
}

// Calculate block statistics for error methods
BlockStats calculateBlockStats(const vector<vector<Pixel>>& data, int x, int y, int size) {
    BlockStats stats;
    long sumR = 0, sumG = 0, sumB = 0;
    int count = 0;
    unsigned char minR = 255, minG = 255, minB = 255;
    unsigned char maxR = 0, maxG = 0, maxB = 0;

    // First pass - calculate means and min/max
    for (int j = y; j < y + size && j < data.size(); j++) {
        for (int i = x; i < x + size && i < data[0].size(); i++) {
            sumR += data[j][i].r;
            sumG += data[j][i].g;
            sumB += data[j][i].b;
            
            minR = min(minR, data[j][i].r);
            minG = min(minG, data[j][i].g);
            minB = min(minB, data[j][i].b);
            
            maxR = max(maxR, data[j][i].r);
            maxG = max(maxG, data[j][i].g);
            maxB = max(maxB, data[j][i].b);
            
            count++;
        }
    }

    if (count == 0) {
        return BlockStats{0};
    }

    stats.meanR = sumR / static_cast<double>(count);
    stats.meanG = sumG / static_cast<double>(count);
    stats.meanB = sumB / static_cast<double>(count);

    stats.maxDiffR = maxR - minR;
    stats.maxDiffG = maxG - minG;
    stats.maxDiffB = maxB - minB;

    // Second pass - calculate variance and MAD
    double sumVarR = 0, sumVarG = 0, sumVarB = 0;
    double sumMadR = 0, sumMadG = 0, sumMadB = 0;
    
    // For entropy calculation
    map<unsigned char, int> histR, histG, histB;

    for (int j = y; j < y + size && j < data.size(); j++) {
        for (int i = x; i < x + size && i < data[0].size(); i++) {
            // Variance calculation
            double diffR = data[j][i].r - stats.meanR;
            double diffG = data[j][i].g - stats.meanG;
            double diffB = data[j][i].b - stats.meanB;
            
            sumVarR += diffR * diffR;
            sumVarG += diffG * diffG;
            sumVarB += diffB * diffB;
            
            // Mean Absolute Deviation calculation
            sumMadR += abs(diffR);
            sumMadG += abs(diffG);
            sumMadB += abs(diffB);
            
            // Entropy calculation - count frequencies
            histR[data[j][i].r]++;
            histG[data[j][i].g]++;
            histB[data[j][i].b]++;
        }
    }

    stats.varianceR = sumVarR / count;
    stats.varianceG = sumVarG / count;
    stats.varianceB = sumVarB / count;
    
    stats.madR = sumMadR / count;
    stats.madG = sumMadG / count;
    stats.madB = sumMadB / count;
    
    // Calculate entropy
    stats.entropyR = 0;
    stats.entropyG = 0;
    stats.entropyB = 0;
    
    for (const auto& pair : histR) {
        double probability = pair.second / static_cast<double>(count);
        stats.entropyR -= probability * log2(probability);
    }
    
    for (const auto& pair : histG) {
        double probability = pair.second / static_cast<double>(count);
        stats.entropyG -= probability * log2(probability);
    }
    
    for (const auto& pair : histB) {
        double probability = pair.second / static_cast<double>(count);
        stats.entropyB -= probability * log2(probability);
    }

    return stats;
}

// Calculate variance error
double calculateVariance(const vector<vector<Pixel>>& data, int x, int y, int size) {
    BlockStats stats = calculateBlockStats(data, x, y, size);
    double avgVariance = (stats.varianceR + stats.varianceG + stats.varianceB) / 3.0;
    return avgVariance;
}

// Calculate Mean Absolute Deviation (MAD) error
double calculateMAD(const vector<vector<Pixel>>& data, int x, int y, int size) {
    BlockStats stats = calculateBlockStats(data, x, y, size);
    double avgMAD = (stats.madR + stats.madG + stats.madB) / 3.0;
    return avgMAD;
}

// Calculate Max Pixel Difference error
double calculateMaxDiff(const vector<vector<Pixel>>& data, int x, int y, int size) {
    BlockStats stats = calculateBlockStats(data, x, y, size);
    double avgMaxDiff = (stats.maxDiffR + stats.maxDiffG + stats.maxDiffB) / 3.0;
    return avgMaxDiff;
}

// Calculate Entropy error
double calculateEntropy(const vector<vector<Pixel>>& data, int x, int y, int size) {
    BlockStats stats = calculateBlockStats(data, x, y, size);
    double avgEntropy = (stats.entropyR + stats.entropyG + stats.entropyB) / 3.0;
    return avgEntropy;
}

// Calculate error based on selected method
double calculateError(const vector<vector<Pixel>>& data, int x, int y, int size, int method) {
    switch (method) {
        case 1: return calculateVariance(data, x, y, size);
        case 2: return calculateMAD(data, x, y, size);
        case 3: return calculateMaxDiff(data, x, y, size);
        case 4: return calculateEntropy(data, x, y, size);
        default: return calculateVariance(data, x, y, size);
    }
}

// Build QuadTree using divide and conquer approach
QuadTreeNode* buildQuadTree(const vector<vector<Pixel>>& data, int x, int y, int size, double threshold, int minBlockSize, int method, int depth) {
    QuadTreeNode* node = new QuadTreeNode(x, y, size);
    
    // Calculate error for the current block
    double error = calculateError(data, x, y, size, method);
    
    // Calculate average color for this block
    node->avgColor = calculateAvgColor(data, x, y, size);
    
    // Check if we should subdivide this block
    if (error > threshold && size > minBlockSize && size / 2 >= minBlockSize) {
        node->isLeaf = false;
        int halfSize = size / 2;
        
        // Create the four children (NW, NE, SW, SE)
        node->children[0] = buildQuadTree(data, x, y, halfSize, threshold, minBlockSize, method, depth + 1);
        node->children[1] = buildQuadTree(data, x + halfSize, y, halfSize, threshold, minBlockSize, method, depth + 1);
        node->children[2] = buildQuadTree(data, x, y + halfSize, halfSize, threshold, minBlockSize, method, depth + 1);
        node->children[3] = buildQuadTree(data, x + halfSize, y + halfSize, halfSize, threshold, minBlockSize, method, depth + 1);
    }
    
    return node;
}

// Reconstruct the image from the QuadTree
void reconstructImage(const QuadTreeNode* node, vector<vector<Pixel>>& outputImage) {
    if (!node) return;
    
    // If this is a leaf node, fill the corresponding area with the average color
    if (node->isLeaf) {
        for (int j = node->y; j < node->y + node->size && j < outputImage.size(); j++) {
            for (int i = node->x; i < node->x + node->size && i < outputImage[0].size(); i++) {
                outputImage[j][i] = node->avgColor;
            }
        }
    } else {
        // Otherwise, recursively process children
        for (int i = 0; i < 4; i++) {
            reconstructImage(node->children[i], outputImage);
        }
    }
}

// Save the reconstructed image to a file
bool saveQuadTreeImage(const string& filename, const vector<vector<Pixel>>& image) {
    int width = image[0].size();
    int height = image.size();
    
    // Create a buffer for the output image
    unsigned char* buffer = new unsigned char[width * height * 3];
    
    // Copy the pixel data to the buffer
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;
            buffer[idx] = image[y][x].r;
            buffer[idx + 1] = image[y][x].g;
            buffer[idx + 2] = image[y][x].b;
        }
    }
    
    // Determine the file format based on the extension
    string extension = filename.substr(filename.find_last_of(".") + 1);
    transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    bool success = false;
    if (extension == "png") {
        success = stbi_write_png(filename.c_str(), width, height, 3, buffer, width * 3);
    } else if (extension == "jpg" || extension == "jpeg") {
        success = stbi_write_jpg(filename.c_str(), width, height, 3, buffer, 90); // Quality 90
    } else if (extension == "bmp") {
        success = stbi_write_bmp(filename.c_str(), width, height, 3, buffer);
    } else {
        cerr << "Unsupported output format: " << extension << endl;
    }
    
    delete[] buffer;
    return success;
}

// Count the number of nodes in the QuadTree
int countNodes(const QuadTreeNode* node) {
    if (!node) return 0;
    
    int count = 1; // Count this node
    
    if (!node->isLeaf) {
        for (int i = 0; i < 4; i++) {
            count += countNodes(node->children[i]);
        }
    }
    
    return count;
}

// Get the maximum depth of the QuadTree
int getTreeDepth(const QuadTreeNode* node) {
    if (!node) return 0;
    
    if (node->isLeaf) {
        return 1;
    }
    
    int maxChildDepth = 0;
    for (int i = 0; i < 4; i++) {
        maxChildDepth = max(maxChildDepth, getTreeDepth(node->children[i]));
    }
    
    return 1 + maxChildDepth;
}

// Generate a GIF of the compression process (Bonus)
// This is a placeholder - implementing actual GIF creation would require additional libraries
bool generateGif(const string& filename, const QuadTreeNode* root, int width, int height) {
    // This would require an external GIF library to implement fully
    // For a complete implementation, consider using a library like giflib
    
    cout << "GIF generation is not implemented in this version." << endl;
    return false;
}