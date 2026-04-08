#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <tiffio.h>

template<typename T>
class tiffdp {
public:
    // Constructeurs
    tiffdp(unsigned width, unsigned height, bool isRGB = false, const std::string& filename = "");
    tiffdp(const std::string& filename, bool isRGB = false);
    ~tiffdp() = default;

    // Fonctions utiles
    bool readTIFF(const std::string& file = "");
    bool writeTIFF(const std::string& file = "");

    // Gestion des pixels
    T getPixel(unsigned x, unsigned y, char channel) const;
    T getPixel(unsigned x, unsigned y) const;
    void setPixel(unsigned x, unsigned y, char channel, T value);
    void setPixel(unsigned x, unsigned y, T value);

    // Getters
    const std::string& getFilename() const;
    unsigned getWidth() const;
    unsigned getHeight() const;
    bool getIsRGB() const;
    std::vector<T>& getDatas();

    // Setters
    void setFilename(const std::string& filename);
    void setWidth(unsigned width);
    void setHeight(unsigned height);
    void setIsRGB(bool isRGB);
    void setDatas(const std::vector<T>& image);

private:
    std::string filename;
    unsigned width;
    unsigned height;
    unsigned nbBits;
    bool isRGB; // true = RGB float, false = grayscale
    std::vector<T> image;
};

#include <tiffdp.tpp>