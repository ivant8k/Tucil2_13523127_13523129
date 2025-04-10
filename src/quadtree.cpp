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
double calculateVariance(vector<vector<Pixel>>& data, int x, int y, int size, Pixel avgColor) {
    double varR = 0.0, varG = 0.0, varB = 0.0;
    int count = 0;
    
    // Hitung varians untuk setiap channel warna
    for (int j = y; j < y + size && j < data.size(); j++) {
        for (int i = x; i < x + size && i < data[j].size(); i++) {
            double dr = data[j][i].r - avgColor.r;
            double dg = data[j][i].g - avgColor.g;
            double db = data[j][i].b - avgColor.b;
            
            varR += dr * dr;
            varG += dg * dg;
            varB += db * db;
            count++;
        }
    }
    
    // Bagi dengan jumlah pixel untuk mendapatkan varians
    if (count > 0) {
        varR /= count;
        varG /= count;
        varB /= count;
    }
    
    // Rata-rata varians dari 3 channel
    return (varR + varG + varB) / 3.0;
}

// Calculate Mean Absolute Deviation (MAD) error
double calculateMAD(vector<vector<Pixel>>& data, int x, int y, int size, Pixel avgColor) {
    double madR = 0.0, madG = 0.0, madB = 0.0;
    int count = 0;
    
    // Hitung MAD untuk setiap channel warna
    for (int j = y; j < y + size && j < data.size(); j++) {
        for (int i = x; i < x + size && i < data[j].size(); i++) {
            madR += abs(data[j][i].r - avgColor.r);
            madG += abs(data[j][i].g - avgColor.g);
            madB += abs(data[j][i].b - avgColor.b);
            count++;
        }
    }
    
    // Bagi dengan jumlah pixel untuk mendapatkan MAD
    if (count > 0) {
        madR /= count;
        madG /= count;
        madB /= count;
    }
    
    // Rata-rata MAD dari 3 channel
    return (madR + madG + madB) / 3.0;
}

double calculateMaxDifference(vector<vector<Pixel>>& data, int x, int y, int size) {
    unsigned char minR = 255, minG = 255, minB = 255;
    unsigned char maxR = 0, maxG = 0, maxB = 0;
    
    // Cari nilai min dan max untuk setiap channel
    for (int j = y; j < y + size && j < data.size(); j++) {
        for (int i = x; i < x + size && i < data[j].size(); i++) {
            // Update min values
            minR = min(minR, data[j][i].r);
            minG = min(minG, data[j][i].g);
            minB = min(minB, data[j][i].b);
            
            // Update max values
            maxR = max(maxR, data[j][i].r);
            maxG = max(maxG, data[j][i].g);
            maxB = max(maxB, data[j][i].b);
        }
    }
    
    // Hitung selisih max-min untuk tiap channel
    double diffR = maxR - minR;
    double diffG = maxG - minG;
    double diffB = maxB - minB;
    
    // Rata-rata selisih dari 3 channel
    return (diffR + diffG + diffB) / 3.0;
}

double calculateEntropy(vector<vector<Pixel>>& data, int x, int y, int size) {
    // Hitung histogram untuk setiap channel
    const int BINS = 256;
    vector<int> histR(BINS, 0), histG(BINS, 0), histB(BINS, 0);
    int totalPixels = 0;
    
    for (int j = y; j < y + size && j < data.size(); j++) {
        for (int i = x; i < x + size && i < data[j].size(); i++) {
            histR[data[j][i].r]++;
            histG[data[j][i].g]++;
            histB[data[j][i].b]++;
            totalPixels++;
        }
    }
    
    // Hitung entropy untuk masing-masing channel
    double entropyR = 0.0, entropyG = 0.0, entropyB = 0.0;
    for (int i = 0; i < BINS; i++) {
        if (histR[i] > 0) {
            double pR = (double)histR[i] / totalPixels;
            entropyR -= pR * log2(pR);
        }
        if (histG[i] > 0) {
            double pG = (double)histG[i] / totalPixels;
            entropyG -= pG * log2(pG);
        }
        if (histB[i] > 0) {
            double pB = (double)histB[i] / totalPixels;
            entropyB -= pB * log2(pB);
        }
    }
    
    // Rata-rata entropy dari ketiga channel
    return (entropyR + entropyG + entropyB) / 3.0;
}

// Implementasi calculateError di quadtree.cpp
double calculateError(vector<vector<Pixel>>& data, int x, int y, int size, Pixel avgColor, int method) {
    switch (method) {
        case 1: // Variance
            return calculateVariance(data, x, y, size, avgColor);
        case 2: // Mean Absolute Deviation (MAD)
            return calculateMAD(data, x, y, size, avgColor);
        case 3: // Max Pixel Difference
            return calculateMaxDifference(data, x, y, size);
        case 4: // Entropy
            return calculateEntropy(data, x, y, size);
        default:
            return calculateVariance(data, x, y, size, avgColor); // Default to variance
    }
}

// Build QuadTree using divide and conquer approach
QuadTreeNode* buildQuadTree(const vector<vector<Pixel>>& data, int x, int y, int size, double threshold, int minBlockSize, int method, int depth) {
    QuadTreeNode* node = new QuadTreeNode(x, y, size);

    // Hitung warna rata-rata terlebih dahulu
    Pixel avgColor = calculateAvgColor(data, x, y, size);
    node->avgColor = avgColor;

    // Hitung error dengan parameter yang benar
    double error = calculateError(const_cast<vector<vector<Pixel>>&>(data), x, y, size, avgColor, method);

    if (error > threshold && size > minBlockSize && size / 2 >= minBlockSize) {
    node->isLeaf = false;
    int halfSize = size / 2;

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