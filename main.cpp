#include "tiffdp.hpp"
#include <cstdint>
#include <iostream>
#include <cmath>

int main() {
    unsigned width = 1024;
    unsigned height = 1024;

    // Crée une image grayscale 8 bits
    tiffdp<uint8_t> img(width, height, false, "sin_gray8.tiff");

    // Génération sinusoïdale
    for (unsigned y = 0; y < height; y++) {
        for (unsigned x = 0; x < width; x++) {
            // Calcul de l'intensité avec sin(x) + sin(y) normalisé [0,255]
            double value = (std::sin(2 * M_PI * x / width) + std::sin(2 * M_PI * y / height) + 2.0) / 4.0;
            uint8_t pixel = static_cast<uint8_t>(value * 255.0);
            img.setPixel(x, y, pixel);
        }
    }

    if (img.writeTIFF()) {
        std::cout << "sin_gray8.tiff créé avec succès\n";
    } else {
        std::cerr << "Erreur lors de l'écriture du TIFF\n";
    }

    return 0;
}