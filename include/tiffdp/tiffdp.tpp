#include <cmath>

// Constructeurs
template<typename T>
tiffdp<T>::tiffdp(unsigned width, unsigned height, bool isRGB, const std::string& filename)
    : filename(filename), width(width), height(height), isRGB(isRGB) {
    unsigned channels = isRGB ? 3 : 1;
    image.resize(width * height * channels, 0);
}

template<typename T>
tiffdp<T>::tiffdp(const std::string& filename, bool isRGB)
    : filename(filename), width(0), height(0), isRGB(isRGB)
{
    readTIFF(filename);
}

// Lecture TIFF
template<typename T>
bool tiffdp<T>::readTIFF(const std::string& file) {
    const std::string& fn = file.empty() ? filename : file;
    TIFF* tif = TIFFOpen(fn.c_str(), "r");
    if (!tif) return false;

    uint32_t w, h;
    uint16_t samplesPerPixel, bitsPerSample, sampleFormat, photometric;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
    TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleFormat);
    TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);

    width = w;
    height = h;
    nbBits = bitsPerSample;
    isRGB = (samplesPerPixel == 3 && photometric == PHOTOMETRIC_RGB);

    // Taille du buffer
    image.resize(width * height * samplesPerPixel);

    tsize_t scanlineSize = TIFFScanlineSize(tif);
    std::vector<uint8_t> buffer(scanlineSize);

    for (uint32_t y = 0; y < height; y++) {
        if (TIFFReadScanline(tif, buffer.data(), y) < 0) { TIFFClose(tif); return false; }

        for (uint32_t x = 0; x < width; x++) {
            if constexpr (std::is_same_v<T,uint8_t>) {
                if (!isRGB) image[y*width + x] = buffer[x];
                else for (int c = 0; c < 3; c++) image[(y*width + x)*3 + c] = buffer[x*3 + c];
            } else if constexpr (std::is_same_v<T,uint16_t>) {
                if (!isRGB) image[y*width + x] = reinterpret_cast<uint16_t*>(buffer.data())[x];
                else for (int c = 0; c < 3; c++) image[(y*width + x)*3 + c] = reinterpret_cast<uint16_t*>(buffer.data())[x*3 + c];
            } else if constexpr (std::is_same_v<T,float>) {
                if (!isRGB) image[y*width + x] = reinterpret_cast<float*>(buffer.data())[x];
                else for (int c = 0; c < 3; c++) image[(y*width + x)*3 + c] = reinterpret_cast<float*>(buffer.data())[x*3 + c];
            }
        }
    }

    TIFFClose(tif);
    return true;
}

// Écriture TIFF
template<typename T>
bool tiffdp<T>::writeTIFF(const std::string& file) {
    const std::string& fn = file.empty() ? filename : file;
    TIFF* tif = TIFFOpen(fn.c_str(), "w");
    if (!tif) return false;

    // Détecte nombre de canaux
    uint16_t channels = isRGB ? 3 : 1;
    uint16_t bitsPerSample = sizeof(T) * 8;
    uint16_t sampleFormat = std::is_same_v<T,float> ? SAMPLEFORMAT_IEEEFP : SAMPLEFORMAT_UINT;

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, channels);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
    TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, sampleFormat);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, isRGB ? PHOTOMETRIC_RGB : PHOTOMETRIC_MINISBLACK);

    tsize_t scanlineSize = TIFFScanlineSize(tif);
    std::vector<uint8_t> buffer(scanlineSize);

    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            unsigned base = (y * width + x) * channels;

            if constexpr (std::is_same_v<T,uint8_t>) {
                if (!isRGB) buffer[x] = image[base];
                else for (int c = 0; c < 3; c++) buffer[x*3 + c] = image[base + c];
            } else if constexpr (std::is_same_v<T,uint16_t>) {
                uint16_t* buf16 = reinterpret_cast<uint16_t*>(buffer.data());
                if (!isRGB) buf16[x] = image[base];
                else for (int c = 0; c < 3; c++) buf16[x*3 + c] = image[base + c];
            } else if constexpr (std::is_same_v<T,float>) {
                float* bufF = reinterpret_cast<float*>(buffer.data());
                if (!isRGB) bufF[x] = image[base];
                else for (int c = 0; c < 3; c++) bufF[x*3 + c] = image[base + c];
            }
        }

        if (TIFFWriteScanline(tif, buffer.data(), y) < 0) {
            TIFFClose(tif);
            return false;
        }
    }

    TIFFClose(tif);
    return true;
}

// Pixels (identiques à la version précédente)
template<typename T>
T tiffdp<T>::getPixel(unsigned x, unsigned y, char channel) const {
    unsigned channels = isRGB ? 3 : 1;
    size_t base = y * width * channels + x * channels;
    switch(channel) {
        case 'R': return isRGB ? image[base+0] : 0;
        case 'G': return isRGB ? image[base+1] : 0;
        case 'B': return isRGB ? image[base+2] : 0;
        default: return image[base];
    }
}

template<typename T>
T tiffdp<T>::getPixel(unsigned x, unsigned y) const {
    unsigned channels = isRGB ? 3 : 1;
    size_t base = y * width * channels + x * channels;
    if (channels == 1) return image[base];
    return (image[base] + image[base+1] + image[base+2])/3;
}

// Méthode pour une image rgb (3 canaux)
template<typename T>
void tiffdp<T>::setPixel(unsigned x, unsigned y, char channel, T value) {
    unsigned channels = isRGB ? 3 : 1;
    size_t base = y * width * channels + x * channels;
    switch(channel) {
        case 'R': if (isRGB) image[base+0]=value; break;
        case 'G': if (isRGB) image[base+1]=value; break;
        case 'B': if (isRGB) image[base+2]=value; break;
        default: image[base]=value; break;
    }
}

// Méthode pour une image grayscale (1 canal)
template<typename T>
void tiffdp<T>::setPixel(unsigned x, unsigned y, T value) {
    unsigned channels = isRGB ? 3 : 1;
    size_t base = y * width * channels + x * channels;
    for (unsigned c=0;c<channels;c++) image[base+c] = value;
}

// Getters / Setters
template<typename T>
const std::string& tiffdp<T>::getFilename() const { return filename; }

template<typename T>
unsigned tiffdp<T>::getWidth() const { return width; }

template<typename T>
unsigned tiffdp<T>::getHeight() const { return height; }

template<typename T>
bool tiffdp<T>::getIsRGB() const { return isRGB; }

template<typename T>
std::vector<T>& tiffdp<T>::getDatas() { return image; }

template<typename T>
void tiffdp<T>::setFilename(const std::string& f) { filename = f; }

template<typename T>
void tiffdp<T>::setWidth(unsigned w) { width=w; image.resize(width*height*(isRGB?3:1)); }

template<typename T>
void tiffdp<T>::setHeight(unsigned h) { height=h; image.resize(width*height*(isRGB?3:1)); }

template<typename T>
void tiffdp<T>::setIsRGB(bool rgb) { isRGB = rgb; image.resize(width*height*(isRGB?3:1)); }

template<typename T>
void tiffdp<T>::setDatas(const std::vector<T>& img) { image = img; }