#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <type_traits>
#include <tiffio.h>

template<typename T>
class tiffdp {
public:
    // Constructeurs
    tiffdp(unsigned width, unsigned height, bool isRGB = false, const std::string& filename = "");
    tiffdp(unsigned width, unsigned height, unsigned depth, bool isRGB, const std::string& filename);
    tiffdp(const std::string& filename, bool isRGB = false);
    ~tiffdp() = default;

    // IO
    bool readTIFF(const std::string& file = "");
    bool writeTIFF(const std::string& file = "");

    // Pixels 2D
    T getPixel(unsigned x, unsigned y, char channel) const;
    T getPixel(unsigned x, unsigned y) const;
    void setPixel(unsigned x, unsigned y, char channel, T value);
    void setPixel(unsigned x, unsigned y, T value);

    // Pixels 3D (stack)
    T getVoxel(unsigned x, unsigned y, unsigned z, char channel) const;
    T getVoxel(unsigned x, unsigned y, unsigned z) const;
    void setVoxel(unsigned x, unsigned y, unsigned z, char channel, T value);
    void setVoxel(unsigned x, unsigned y, unsigned z, T value);

    // Getters
    const std::string& getFilename() const;
    unsigned getWidth() const;
    unsigned getHeight() const;
    unsigned getDepth() const;
    bool getIsRGB() const;
    std::vector<T>& getDatas();

    // Setters
    void setFilename(const std::string& filename);
    void setWidth(unsigned width);
    void setHeight(unsigned height);
    void setDepth(unsigned depth);
    void setIsRGB(bool isRGB);
    void setDatas(const std::vector<T>& image);

private:
    std::string filename;
    unsigned width = 0;
    unsigned height = 0;
    unsigned depth = 1;
    unsigned nbBits = 0;
    bool isRGB = false;

    std::vector<T> image;

    size_t idx2D(unsigned x, unsigned y, unsigned channels) const;
    size_t idx3D(unsigned x, unsigned y, unsigned z, unsigned channels) const;
};

#include "tiffdp.tpp"

-------------------------------------------------------

template<typename T>
tiffdp<T>::tiffdp(unsigned w, unsigned h, bool rgb, const std::string& file)
    : filename(file), width(w), height(h), depth(1), isRGB(rgb)
{
    unsigned c = isRGB ? 3 : 1;
    image.resize(width * height * c, 0);
}

template<typename T>
tiffdp<T>::tiffdp(unsigned w, unsigned h, unsigned d, bool rgb, const std::string& file)
    : filename(file), width(w), height(h), depth(d), isRGB(rgb)
{
    unsigned c = isRGB ? 3 : 1;
    image.resize(width * height * depth * c, 0);
}

template<typename T>
tiffdp<T>::tiffdp(const std::string& file, bool rgb)
    : filename(file), isRGB(rgb)
{
    readTIFF(file);
}

// ===================== Index =====================

template<typename T>
size_t tiffdp<T>::idx2D(unsigned x, unsigned y, unsigned channels) const {
    return (y * width + x) * channels;
}

template<typename T>
size_t tiffdp<T>::idx3D(unsigned x, unsigned y, unsigned z, unsigned channels) const {
    return ((z * height + y) * width + x) * channels;
}

// ===================== Lecture =====================

template<typename T>
bool tiffdp<T>::readTIFF(const std::string& file) {
    const std::string& fn = file.empty() ? filename : file;
    TIFF* tif = TIFFOpen(fn.c_str(), "r");
    if (!tif) return false;

    uint32_t w, h;
    uint16_t spp, bps, sf, photo;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);
    TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sf);
    TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photo);

    width = w;
    height = h;
    isRGB = (spp == 3 && photo == PHOTOMETRIC_RGB);
    depth = 1;

    image.resize(width * height * spp);

    tsize_t scanSize = TIFFScanlineSize(tif);
    std::vector<uint8_t> buffer(scanSize);

    for (uint32_t y = 0; y < height; y++) {
        TIFFReadScanline(tif, buffer.data(), y);

        for (uint32_t x = 0; x < width; x++) {
            size_t base = idx2D(x, y, spp);

            if constexpr (std::is_same_v<T, uint8_t>) {
                if (!isRGB) image[base] = buffer[x];
                else for (int c = 0; c < 3; c++) image[base + c] = buffer[x * 3 + c];
            }
            else if constexpr (std::is_same_v<T, uint16_t>) {
                auto* buf = reinterpret_cast<uint16_t*>(buffer.data());
                if (!isRGB) image[base] = buf[x];
                else for (int c = 0; c < 3; c++) image[base + c] = buf[x * 3 + c];
            }
            else if constexpr (std::is_same_v<T, float>) {
                auto* buf = reinterpret_cast<float*>(buffer.data());
                if (!isRGB) image[base] = buf[x];
                else for (int c = 0; c < 3; c++) image[base + c] = buf[x * 3 + c];
            }
        }
    }

    TIFFClose(tif);
    return true;
}

// ===================== Écriture STACK =====================

template<typename T>
bool tiffdp<T>::writeTIFF(const std::string& file) {
    const std::string& fn = file.empty() ? filename : file;
    TIFF* tif = TIFFOpen(fn.c_str(), "w");
    if (!tif) return false;

    uint16_t channels = isRGB ? 3 : 1;
    uint16_t bps = sizeof(T) * 8;
    uint16_t sf = std::is_same_v<T, float> ? SAMPLEFORMAT_IEEEFP : SAMPLEFORMAT_UINT;

    std::vector<uint8_t> buffer(width * channels * sizeof(T));

    for (unsigned z = 0; z < depth; z++) {

        TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
        TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, channels);
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps);
        TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, sf);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, isRGB ? PHOTOMETRIC_RGB : PHOTOMETRIC_MINISBLACK);
        TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

        for (unsigned y = 0; y < height; y++) {
            for (unsigned x = 0; x < width; x++) {

                size_t base = idx3D(x, y, z, channels);

                if constexpr (std::is_same_v<T, uint8_t>) {
                    if (!isRGB) buffer[x] = image[base];
                    else for (int c = 0; c < 3; c++) buffer[x * 3 + c] = image[base + c];
                }
                else if constexpr (std::is_same_v<T, uint16_t>) {
                    auto* buf = reinterpret_cast<uint16_t*>(buffer.data());
                    if (!isRGB) buf[x] = image[base];
                    else for (int c = 0; c < 3; c++) buf[x * 3 + c] = image[base + c];
                }
                else if constexpr (std::is_same_v<T, float>) {
                    auto* buf = reinterpret_cast<float*>(buffer.data());
                    if (!isRGB) buf[x] = image[base];
                    else for (int c = 0; c < 3; c++) buf[x * 3 + c] = image[base + c];
                }
            }

            TIFFWriteScanline(tif, buffer.data(), y);
        }

        TIFFWriteDirectory(tif); // 👈 STACK PAGE
    }

    TIFFClose(tif);
    return true;
}

// ===================== Pixels 2D =====================

template<typename T>
T tiffdp<T>::getPixel(unsigned x, unsigned y, char c) const {
    unsigned ch = isRGB ? 3 : 1;
    size_t b = idx2D(x, y, ch);
    if (c == 'R') return image[b];
    if (c == 'G') return image[b + 1];
    if (c == 'B') return image[b + 2];
    return image[b];
}

template<typename T>
T tiffdp<T>::getPixel(unsigned x, unsigned y) const {
    unsigned ch = isRGB ? 3 : 1;
    size_t b = idx2D(x, y, ch);
    if (ch == 1) return image[b];
    return (image[b] + image[b + 1] + image[b + 2]) / 3;
}

// ===================== Pixels 3D =====================

template<typename T>
T tiffdp<T>::getVoxel(unsigned x, unsigned y, unsigned z, char c) const {
    unsigned ch = isRGB ? 3 : 1;
    size_t b = idx3D(x, y, z, ch);
    if (c == 'R') return image[b];
    if (c == 'G') return image[b + 1];
    if (c == 'B') return image[b + 2];
    return image[b];
}

template<typename T>
T tiffdp<T>::getVoxel(unsigned x, unsigned y, unsigned z) const {
    unsigned ch = isRGB ? 3 : 1;
    size_t b = idx3D(x, y, z, ch);
    if (ch == 1) return image[b];
    return (image[b] + image[b + 1] + image[b + 2]) / 3;
}

template<typename T>
void tiffdp<T>::setVoxel(unsigned x, unsigned y, unsigned z, char c, T v) {
    unsigned ch = isRGB ? 3 : 1;
    size_t b = idx3D(x, y, z, ch);
    if (c == 'R') image[b] = v;
    else if (c == 'G') image[b + 1] = v;
    else if (c == 'B') image[b + 2] = v;
}

template<typename T>
void tiffdp<T>::setVoxel(unsigned x, unsigned y, unsigned z, T v) {
    unsigned ch = isRGB ? 3 : 1;
    size_t b = idx3D(x, y, z, ch);
    for (unsigned c = 0; c < ch; c++) image[b + c] = v;
}

// ===================== Getters =====================

template<typename T> const std::string& tiffdp<T>::getFilename() const { return filename; }
template<typename T> unsigned tiffdp<T>::getWidth() const { return width; }
template<typename T> unsigned tiffdp<T>::getHeight() const { return height; }
template<typename T> unsigned tiffdp<T>::getDepth() const { return depth; }
template<typename T> bool tiffdp<T>::getIsRGB() const { return isRGB; }
template<typename T> std::vector<T>& tiffdp<T>::getDatas() { return image; }

// ===================== Setters =====================

template<typename T> void tiffdp<T>::setFilename(const std::string& f) { filename = f; }
template<typename T> void tiffdp<T>::setWidth(unsigned w) { width = w; }
template<typename T> void tiffdp<T>::setHeight(unsigned h) { height = h; }
template<typename T> void tiffdp<T>::setDepth(unsigned d) { depth = d; }
template<typename T> void tiffdp<T>::setIsRGB(bool r) { isRGB = r; }
template<typename T> void tiffdp<T>::setDatas(const std::vector<T>& d) { image = d; }

---------------------------------------------------------------------------

int main() {
    unsigned w = 64;
    unsigned h = 64;
    unsigned d = 64; // 👈 profondeur = cube pour sphère propre

    tiffdp<float> volume(w, h, d, false, "sphere_stack.tiff");

    auto& data = volume.getDatas();

    float cx = w * 0.5f;
    float cy = h * 0.5f;
    float cz = d * 0.5f;

    float radius = std::min(std::min(w, h), d) * 0.3f;
    float r2 = radius * radius;

    for (unsigned z = 0; z < d; z++) {
        for (unsigned y = 0; y < h; y++) {
            for (unsigned x = 0; x < w; x++) {

                float dx = x - cx;
                float dy = y - cy;
                float dz = z - cz;

                float dist2 = dx*dx + dy*dy + dz*dz;

                size_t idx = z * w * h + y * w + x;

                // Sphère binaire
                data[idx] = (dist2 <= r2) ? 1.0f : 0.0f;
            }
        }
    }

    volume.writeTIFF();

    return 0;
}

----------------------------------------------------------------------------------

#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>

// =========================================================
// CONSTANTES
// =========================================================
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// =========================================================
// FACTORIELLE
// =========================================================
static float factorial(int n)
{
    float r = 1.0f;
    for(int i=2;i<=n;i++) r *= i;
    return r;
}

// =========================================================
// NORMALISATION SH
// =========================================================
static float K(int l, int m)
{
    float num = (2.0f*l + 1.0f) * factorial(l - std::abs(m));
    float den = 4.0f * M_PI * factorial(l + std::abs(m));
    return sqrtf(num / den);
}

// =========================================================
// POLYNÔME ASSOCIÉ (stable)
// =========================================================
static float P(int l, int m, float x)
{
    if(l == m)
    {
        float p = 1.0f;
        for(int i=1;i<=m;i++)
            p *= -(2*i - 1) * sqrtf(1.0f - x*x);
        return p;
    }

    if(l == m+1)
        return x * (2*m + 1) * P(m, m, x);

    return ((2*l - 1)*x*P(l-1,m,x) - (l+m-1)*P(l-2,m,x)) / (l - m);
}

// =========================================================
// REAL SPHERICAL HARMONICS
// =========================================================
static float SH(int l, int m, float th, float ph)
{
    float x = cosf(th);
    float abs_m = std::abs(m);

    if(m == 0)
    {
        if(l == 0) return 0.282095f;
        if(l == 1) return 0.488603f * x;
        if(l == 2) return 0.946174f * (x*x - 0.333333f);
        if(l == 3) return 0.846284f * (x*x*x - 0.6f*x);
        return K(l,0) * P(l,0,x);
    }

    float base = P(l, abs_m, x);

    if(m > 0)
        return K(l,m) * sqrtf(2.0f) * base * cosf(m * ph);

    return K(l,abs_m) * sqrtf(2.0f) * base * sinf(abs_m * ph);
}

// =========================================================
// SURFACE SH (SDF PARAMETRIQUE)
// =========================================================
static float evalSH(const std::vector<float>& c, int L,
                    float th, float ph)
{
    float r = 0.30f; // rayon base (IMPORTANT stabilité)

    int i = 0;

    for(int l=0;l<=L;l++)
    for(int m=-l;m<=l;m++)
        r += 0.04f * c[i++] * SH(l,m,th,ph);

    // empêche explosion de sphère
    return std::clamp(r, 0.12f, 0.55f);
}

// =========================================================
// INDEX 3D
// =========================================================
inline int idx(int x,int y,int z,int W,int H)
{
    return x + W*(y + H*z);
}

// =========================================================
// SDF
// =========================================================
static void buildSDF(std::vector<float>& sdf,
                     int W,int H,int D,
                     const std::vector<float>& c,
                     int L)
{
    float cx=W*0.5f, cy=H*0.5f, cz=D*0.5f;
    float scale = 1.0f / std::max({W,H,D});

    for(int z=0;z<D;z++)
    for(int y=0;y<H;y++)
    for(int x=0;x<W;x++)
    {
        float X=(x-cx)*scale;
        float Y=(y-cy)*scale;
        float Z=(z-cz)*scale;

        float r = sqrtf(X*X + Y*Y + Z*Z);

        float th = acosf(Z/(r+1e-8f));
        float ph = atan2f(Y,X);

        float rs = evalSH(c,L,th,ph);

        sdf[idx(x,y,z,W,H)] = r - rs;
    }
}

// =========================================================
// DENSITY
// =========================================================
static float mu(float s)
{
    return std::max(0.0f, 1.0f - fabsf(s));
}

// =========================================================
// PROJECTION 2D (RENDER VOLUME)
// =========================================================
static void project(const std::vector<float>& sdf,
                    int W,int H,int D,
                    std::vector<float>& img,
                    int w,int h)
{
    const int STEPS = 64;
    const float STEP = 0.03f;

    for(int y=0;y<h;y++)
    for(int x=0;x<w;x++)
    {
        float dx = x/(float)w - 0.5f;
        float dy = y/(float)h - 0.5f;
        float dz = 1.0f;

        float len = sqrtf(dx*dx+dy*dy+dz*dz);
        dx/=len; dy/=len; dz/=len;

        float px=0, py=0, pz=0;
        float att=0.0f;

        for(int i=0;i<STEPS;i++)
        {
            px += dx*STEP;
            py += dy*STEP;
            pz += dz*STEP;

            int ix = (int)(px + W*0.5f);
            int iy = (int)(py + H*0.5f);
            int iz = (int)(pz + D*0.5f);

            if(ix<0||iy<0||iz<0||ix>=W||iy>=H||iz>=D)
                continue;

            float s = sdf[idx(ix,iy,iz,W,H)];
            att += mu(s);

            if(s < 0.01f) break;
        }

        img[y*w+x] = expf(-0.06f * att);
    }
}

// =========================================================
// TARGET (IMAGE SYNTHÉTIQUE)
// =========================================================
static void makeTarget(std::vector<float>& img,int w,int h)
{
    for(int y=0;y<h;y++)
    for(int x=0;x<w;x++)
    {
        float u = x/(float)w - 0.5f;
        float v = y/(float)h - 0.5f;

        float r = sqrtf(u*u + v*v);

        img[y*w+x] = expf(-14.0f * r*r);
    }
}

// =========================================================
// LOSS LOG
// =========================================================
static float loss(const std::vector<float>& a,
                  const std::vector<float>& b)
{
    float L=0.0f;
    const float eps=1e-6f;

    for(size_t i=0;i<a.size();i++)
    {
        float da = logf(a[i]+eps);
        float db = logf(b[i]+eps);
        float d = da - db;
        L += d*d;
    }

    return L / a.size();
}

// =========================================================
// MAIN (RECONSTRUCTION COMPLETE)
// =========================================================
int main()
{
    int W=48,H=48,D=48;
    int w=64,h=64;
    int L=3;

    // -----------------------------
    // COEFFICIENTS SH
    // -----------------------------
    std::vector<float> c((L+1)*(L+1),0.05f);

    std::vector<float> sdf(W*H*D);
    std::vector<float> img(w*h);
    std::vector<float> target(w*h);

    makeTarget(target,w,h);

    // save target
    FILE* f = fopen("target.raw","wb");
    fwrite(target.data(),sizeof(float),target.size(),f);
    fclose(f);

    // -----------------------------
    // INIT
    // -----------------------------
    buildSDF(sdf,W,H,D,c,L);
    project(sdf,W,H,D,img,w,h);

    FILE* f2 = fopen("init.raw","wb");
    fwrite(img.data(),sizeof(float),img.size(),f2);
    fclose(f2);

    // -----------------------------
    // OPTIMISATION (Adam simple)
    // -----------------------------
    std::vector<float> m(c.size(),0), v(c.size(),0);

    float lr=0.03f;
    float beta1=0.9f;
    float beta2=0.999f;
    float eps=1e-8f;

    std::vector<float> best;
    float bestL=1e30f;

    for(int it=0; it<120; it++)
    {
        // gradient finite diff simple (robuste baseline)
        float epsg = 1e-3f;

        std::vector<float> grad(c.size(),0);

        buildSDF(sdf,W,H,D,c,L);
        project(sdf,W,H,D,img,w,h);

        float base = loss(img,target);

        for(size_t i=0;i<c.size();i++)
        {
            float old=c[i];
            c[i]+=epsg;

            buildSDF(sdf,W,H,D,c,L);
            project(sdf,W,H,D,img,w,h);

            float l = loss(img,target);

            grad[i]=(l-base)/epsg;

            c[i]=old;
        }

        // Adam update
        for(size_t i=0;i<c.size();i++)
        {
            m[i]=beta1*m[i]+(1-beta1)*grad[i];
            v[i]=beta2*v[i]+(1-beta2)*grad[i]*grad[i];

            float mh=m[i]/(1-powf(beta1,it+1));
            float vh=v[i]/(1-powf(beta2,it+1));

            c[i]-=lr*mh/(sqrtf(vh)+eps);
            c[i]=std::clamp(c[i],-1.0f,1.0f);
        }

        buildSDF(sdf,W,H,D,c,L);
        project(sdf,W,H,D,img,w,h);

        float Lval = loss(img,target);

        std::cout<<"it "<<it<<" loss "<<Lval<<"\n";

        if(Lval<bestL){
            bestL=Lval;
            best=img;
        }
    }

    // -----------------------------
    // SAVE RESULT
    // -----------------------------
    FILE* f3 = fopen("recon.raw","wb");
    fwrite(best.data(),sizeof(float),best.size(),f3);
    fclose(f3);

    std::cout<<"DONE best loss="<<bestL<<"\n";
}

