#include <iostream>
#include <fstream>
#include "quadtree.h"

// Contoh fungsi konversi gambar BMP ke matriks warna dan sebaliknya akan ditambahkan nanti.

int main() {
    // Contoh input dummy 8x8 semua putih
    std::vector<std::vector<Color>> image(8, std::vector<Color>(8, WHITE));

    // Kompresi
    QuadTree qt;
    qt.compress(image);
    std::vector<char> serialized = qt.serialize();

    std::cout << "Hasil serialisasi quadtree:\n";
    for (char c : serialized) std::cout << c << " ";
    std::cout << "\n";

    // Dekompresi
    std::vector<std::vector<Color>> result;
    qt.decompress(result, 8);

    return 0;
}
