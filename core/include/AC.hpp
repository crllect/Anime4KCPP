#pragma once

#include<iostream>
#include<memory>
#include<cmath>
#include<cstddef>
#include<cstdint>

#include<opencv2/opencv.hpp>

#include<ac_export.h>

#include"ACException.hpp"
#include"ACProcessor.hpp"

namespace Anime4KCPP
{
    struct AC_EXPORT Parameters;
    class AC_EXPORT AC;

    static constexpr int B = 0, G = 1, R = 2, A = 3;
    static constexpr int Y = 0, U = 1, V = 2;

    namespace Filter
    {
        static constexpr std::uint8_t
            Median_Blur = 1 << 0,
            Mean_Blur = 1 << 1,
            CAS_Sharpening = 1 << 2,
            Gaussian_Blur_Weak = 1 << 3,
            Gaussian_Blur = 1 << 4,
            Bilateral_Filter = 1 << 5,
            Bilateral_Filter_Fast = 1 << 6;
    }

    namespace Utils
    {
        int fastCeilLog2(double v) noexcept;
    }
}

// compute log2(v) then do ceil(v)
inline int Anime4KCPP::Utils::fastCeilLog2(double v) noexcept
{
    long long data = *reinterpret_cast<long long*>(&v);
    return static_cast<int>((((data >> 52) & 0x7ff) - 1023) + ((data << 12) != 0));
}

struct Anime4KCPP::Parameters
{
    int passes;
    int pushColorCount;
    double strengthColor;
    double strengthGradient;
    double zoomFactor;
    bool fastMode;
    bool preprocessing;
    bool postprocessing;
    std::uint8_t preFilters;
    std::uint8_t postFilters;
    bool HDN;
    bool alpha;
    int HDNLevel;

    // return true if zoomFactor is not a power of 2 or zoomFactor < 2.0
    inline bool isNonIntegerScale() noexcept
    {
        return ((*reinterpret_cast<long long int*>(&zoomFactor)) << 12) || (zoomFactor < 2.0);
    }

    void reset() noexcept;

    explicit Parameters(
        int passes = 2,
        int pushColorCount = 2,
        double strengthColor = 0.3,
        double strengthGradient = 1.0,
        double zoomFactor = 2.0,
        bool fastMode = false,
        bool preprocessing = false,
        bool postprocessing = false,
        std::uint8_t preFilters = 4,
        std::uint8_t postFilters = 40,
        bool HDN = false,
        int HDNLevel = 1,
        bool alpha = false
    ) noexcept;
};

//Base class for IO operation
class Anime4KCPP::AC
{
public:
    explicit AC(const Parameters& parameters);
    virtual ~AC();

    virtual void setParameters(const Parameters& parameters);
    Parameters getParameters();

#ifdef ENABLE_IMAGE_IO
    void loadImage(const std::string& srcFile);
#endif // ENABLE_IMAGE_IO
    void loadImage(const cv::Mat& srcImage);
    void loadImage(const std::vector<std::uint8_t>& buf);
    void loadImage(const std::uint8_t* buf, std::size_t size);
    void loadImage(int rows, int cols, std::size_t stride, std::uint8_t* data, bool inputAsYUV444 = false, bool inputAsRGB32 = false, bool inputAsGrayscale = false);
    void loadImage(int rows, int cols, std::size_t stride, std::uint16_t* data, bool inputAsYUV444 = false, bool inputAsRGB32 = false, bool inputAsGrayscale = false);
    void loadImage(int rows, int cols, std::size_t stride, float* data, bool inputAsYUV444 = false, bool inputAsRGB32 = false, bool inputAsGrayscale = false);
    void loadImage(int rows, int cols, std::size_t stride, std::uint8_t* r, std::uint8_t* g, std::uint8_t* b, bool inputAsYUV444 = false);
    void loadImage(int rows, int cols, std::size_t stride, std::uint16_t* r, std::uint16_t* g, std::uint16_t* b, bool inputAsYUV444 = false);
    void loadImage(int rows, int cols, std::size_t stride, float* r, float* g, float* b, bool inputAsYUV444 = false);
    void loadImage(
        int rowsY, int colsY, std::size_t strideY, std::uint8_t* y,
        int rowsU, int colsU, std::size_t strideU, std::uint8_t* u,
        int rowsV, int colsV, std::size_t strideV, std::uint8_t* v);
    void loadImage(
        int rowsY, int colsY, std::size_t strideY, std::uint16_t* y,
        int rowsU, int colsU, std::size_t strideU, std::uint16_t* u,
        int rowsV, int colsV, std::size_t strideV, std::uint16_t* v);
    void loadImage(
        int rowsY, int colsY, std::size_t strideY, float* y,
        int rowsU, int colsU, std::size_t strideU, float* u,
        int rowsV, int colsV, std::size_t strideV, float* v);
    void loadImage(const cv::Mat& y, const cv::Mat& u, const cv::Mat& v);
#ifdef ENABLE_IMAGE_IO
    void saveImage(const std::string& dstFile);
#endif // ENABLE_IMAGE_IO
    void saveImage(const std::string suffix, std::vector<std::uint8_t>& buf);
    void saveImage(cv::Mat& dstImage);
    void saveImage(cv::Mat& r, cv::Mat& g, cv::Mat& b);
    void saveImage(std::uint8_t* data, std::size_t dstStride = 0);
    void saveImage(
        std::uint8_t* r, std::size_t dstStrideR, 
        std::uint8_t* g, std::size_t dstStrideG, 
        std::uint8_t* b, std::size_t dstStrideB);
    void saveImageBufferSize(std::size_t& dataSize, std::size_t dstStride = 0);
    void saveImageBufferSize(
        std::size_t& rSize, std::size_t dstStrideR, 
        std::size_t& gSize, std::size_t dstStrideG, 
        std::size_t& bSize, std::size_t dstStrideB);
    void saveImageShape(int& cols, int& rows, int& channels);
    //R2B = true will exchange R channel and B channel
    void showImage(bool R2B = false);

    void process();

    virtual std::string getInfo();
    virtual std::string getFiltersInfo();
    virtual Processor::Type getProcessorType() noexcept = 0;
    virtual std::string getProcessorInfo() = 0;
protected:
    virtual void processYUVImage() = 0;
    virtual void processRGBImage() = 0;
    virtual void processGrayscale() = 0;
private:
    bool inputRGB32 = false;
    bool checkAlphaChannel = false;
    bool inputYUV = false;
    bool inputGrayscale = false;

    cv::Mat alphaChannel;
protected:
    int height, width;

    cv::Mat orgImg, dstImg;
    cv::Mat orgU, orgV;
    cv::Mat dstU, dstV;

    Parameters param;
};
