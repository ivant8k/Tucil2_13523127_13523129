#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include "quadtree.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

int main(int argc, char** argv) {
    // Start timing
    auto start = chrono::high_resolution_clock::now();

    // Default values
    string inputFilePath = "";
    int errorMethod = 1;
    double threshold = 10.0;
    int minBlockSize = 2;
    double targetCompression = 0.0;
    string outputFilePath = "";
    string gifOutputPath = "";

    // Get user input
    cout << "Enter absolute path to input image: ";
    cin >> inputFilePath;

    cout << "Select error calculation method:" << endl;
    cout << "1. Variance" << endl;
    cout << "2. Mean Absolute Deviation (MAD)" << endl;
    cout << "3. Max Pixel Difference" << endl;
    cout << "4. Entropy" << endl;
    cout << "Enter method number (1-4): ";
    cin >> errorMethod;

    cout << "Enter threshold value: ";
    cin >> threshold;

    cout << "Enter minimum block size: ";
    cin >> minBlockSize;

    cout << "Enter target compression percentage (0.0-1.0, 0 to disable): ";
    cin >> targetCompression;

    cout << "Enter absolute path for output image: ";
    cin >> outputFilePath;
    
    // Load image
    int imageWidth, imageHeight, imageChannels;
    unsigned char* imageBuffer = stbi_load(inputFilePath.c_str(), &imageWidth, &imageHeight, &imageChannels, 3);
    if (!imageBuffer) {
        cerr << "Error: Could not load image " << inputFilePath << endl;
        return 1;
    }

    // Convert to our internal format
    vector<vector<Pixel>> imageData(imageHeight, vector<Pixel>(imageWidth));
    for (int y = 0; y < imageHeight; y++) {
        for (int x = 0; x < imageWidth; x++) {
            int idx = (y * imageWidth + x) * 3;
            imageData[y][x].r = imageBuffer[idx];
            imageData[y][x].g = imageBuffer[idx + 1];
            imageData[y][x].b = imageBuffer[idx + 2];
        }
    }
    stbi_image_free(imageBuffer);

    // Calculate original image size in bytes (assuming 24-bit color)
    size_t originalSize = imageWidth * imageHeight * 3;

    // Ensure image dimensions are powers of 2 for quadtree
    int maxDim = max(imageWidth, imageHeight);
    int size = 1;
    while (size < maxDim) {
        size *= 2;
    }

    // Adaptive threshold for target compression (Bonus)
    if (targetCompression > 0) {
        double minThreshold = 0.01;
        double maxThreshold = 1000.0;
        double currentCompression = 0.0;
        int iterations = 0;
        const int MAX_ITERATIONS = 20;
        const double STEP_FACTOR = 0.5; // Faktor langkah untuk penyesuaian proporsional
        
        while (iterations < MAX_ITERATIONS) {
            QuadTreeNode* root = buildQuadTree(imageData, 0, 0, size, threshold, minBlockSize, errorMethod);
            int nodes = countNodes(root);
            size_t compressedSize = nodes * sizeof(QuadTreeNode);
            currentCompression = 1.0 - (double)compressedSize / originalSize;
            
            cout << "Iteration " << iterations + 1 << ": Threshold = " << threshold 
                 << ", Nodes = " << nodes << ", Compression = " << (currentCompression * 100) << "%" << endl;
            
            if (abs(currentCompression - targetCompression) < 0.01) {
                cout << "Target compression reached with threshold: " << threshold << endl;
                delete root;
                break;
            }
            
            // Hitung error relatif
            double error =  targetCompression - currentCompression ;
            
            // Sesuaikan threshold secara proporsional
            threshold = threshold * (1.0 + STEP_FACTOR * error);
            
            // Pastikan threshold tetap dalam rentang
            if (threshold < minThreshold) {
                threshold = minThreshold;
            } else if (threshold > maxThreshold) {
                threshold = maxThreshold;
            }
            
            delete root;
            iterations++;
        }
    }

    // Build the QuadTree
    QuadTreeNode* root = buildQuadTree(imageData, 0, 0, size, threshold, minBlockSize, errorMethod);

    // Calculate tree statistics
    int totalNodes = countNodes(root);
    int maxTreeDepth = getTreeDepth(root);

    // Reconstruct the image
    vector<vector<Pixel>> outputImage(imageHeight, vector<Pixel>(imageWidth));
    reconstructImage(root, outputImage);

    // Save the output image
    if (!saveQuadTreeImage(outputFilePath, outputImage)) {
        cerr << "Error: Could not save output image" << endl;
    }

    // Calculate compressed size (approximate, based on QuadTree nodes)
    size_t compressedSize = totalNodes * sizeof(QuadTreeNode); // Simple approximation
    double compressionPercentage = (1.0 - (double)compressedSize / originalSize) * 100.0;

    // End timing
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    // Output results
    cout << fixed << setprecision(2);
    cout << "Execution time: " << duration.count() << " seconds" << endl;
    cout << "Original image size: " << originalSize << " bytes" << endl;
    cout << "Compressed size (approximate): " << compressedSize << " bytes" << endl;
    cout << "Compression percentage: " << compressionPercentage << "%" << endl;
    cout << "Tree depth: " << maxTreeDepth << endl;
    cout << "Number of nodes: " << totalNodes << endl;
    cout << "Output image saved to: " << outputFilePath << endl;

    // Clean up
    delete root;

    return 0;
}