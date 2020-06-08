//-----------------------------------------------------------------------------
#include "pgexOpenCV.h"
#include "opencv2/imgcodecs.hpp"
//#include "opencv2/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
//-----------------------------------------------------------------------------
using namespace olc;
//-----------------------------------------------------------------------------

CVSprite::CVSprite(const std::string& sImageFile, olc::ResourcePack* pack)
{
    //-- Do not support resource packages yet
    if (pack)
    {
        LoadFromFile(sImageFile, pack);
    }
    else
    {
        loadImage(sImageFile);
        updateSprite();
    }
}
//-----------------------------------------------------------------------------

CVSprite::CVSprite(const std::string& sImageFile, int32_t w, int32_t h)
{
    loadImage(sImageFile);
    resize(w, h);
    updateSprite();
}
//-----------------------------------------------------------------------------

CVSprite::CVSprite(const std::string& sImageFile, float scale)
{
    loadImage(sImageFile);
    scaleImage(scale);
    updateSprite();
}
//-----------------------------------------------------------------------------

CVSprite::CVSprite(cv::Mat& image) : ISprite(), m_image(image)
{
    updateSprite();
}
//-----------------------------------------------------------------------------

CVSprite::~CVSprite()
{
    if (isCvImage())
        pColData = nullptr;
}
//-----------------------------------------------------------------------------

void CVSprite::updateSprite()
{
    width  = m_image.cols;
    height = m_image.rows;

    if (m_image.isContinuous())
    {
        uchar* p = m_image.ptr<uchar>(0);
        pColData = (olc::Pixel*)p;
    }
}
//-----------------------------------------------------------------------------

cv::Mat CVSprite::cvToOlc(const cv::Mat& img)
{
    cv::Mat aux(img.size(), CV_MAKE_TYPE(img.depth(), 4));
    cv::mixChannels({ { img } }, { { aux } }, { 0,2, 1,1, 2,0, 3,3 });
    return aux;
}
//-----------------------------------------------------------------------------

olc::rcode CVSprite::loadFromStream(std::istream& is)
{
    is.read((char*)&width, sizeof(int32_t));
    is.read((char*)&height, sizeof(int32_t));

    if (!isCvImage() && pColData)
        delete[] pColData;

    if (m_image.cols != width || m_image.rows != height)
    {
        m_image.release();
        m_image = cv::Mat(width, height, CV_8UC4);
    }

    if (m_image.isContinuous())
    {
        uchar* p = m_image.ptr<uchar>(0);
        is.read((char*)p, (size_t)width * (size_t)height * sizeof(uint32_t));
        pColData = (olc::Pixel*)p;
        return olc::OK;
    }
    return olc::FAIL;
}
//-----------------------------------------------------------------------------

olc::rcode CVSprite::loadImage(const std::string& sImageFile)
{
    cv::Mat img = cv::imread(sImageFile, cv::IMREAD_COLOR);
    if (img.channels() > 4)
        return olc::FAIL;

    cv::Mat aux(img.size(), CV_MAKE_TYPE(img.depth(), 4));
    switch (img.channels())
    {
    case 1:
        cv::mixChannels({ { img } }, { { aux } }, { 0,0, 0,1, 0,2 });
        //-- set Alpha channel to 255 (fully opaque)
        aux.reshape(1, aux.rows * aux.cols).col(3).setTo(cv::Scalar(255));
        break;

    case 2:
        cv::mixChannels({ { img } }, { { aux } }, { 0,0, 0,1, 0,2, 1,3 });
        break;

    case 3:
        cv::mixChannels({ { img } }, { { aux } }, { 0,2, 1,1, 2,0 });
        //-- set Alpha channel to 255 (fully opaque)
        aux.reshape(1, aux.rows * aux.cols).col(3).setTo(cv::Scalar(255));
        break;

    case 4:
        cv::mixChannels({ { img } }, { { aux } }, { 0,2, 1,1, 2,0, 3,3 });
        break;

    default:
        break;
    }
    m_image = aux;
    //cv::namedWindow("Display reshaped", cv::WINDOW_AUTOSIZE); // Create a window for display.
    //cv::imshow("Display reshaped", newSrc); // Show our image inside it.
    return olc::OK;
}
//-----------------------------------------------------------------------------

void CVSprite::scaleImage(float scale)
{
    if (scale != 1.0)
    {
        cv::resize(m_image, m_image, cv::Size(), scale, scale);
    }
}
//-----------------------------------------------------------------------------

void CVSprite::resize(int32_t w, int32_t h)
{
    if (w != m_image.cols || h != m_image.rows)
    {
        cv::resize(m_image, m_image, cv::Size(w, h));
    }
}
//-----------------------------------------------------------------------------

olc::rcode CVSprite::LoadFromFile(const std::string& sImageFile, olc::ResourcePack* pack)
{
    olc::rcode rtn = olc::FAIL;

    if (!pack)
    {
        if ((rtn = loadImage(sImageFile)) == olc::OK)
            updateSprite();
    }
    else
    {
        // Load sprite from input stream
        ResourceBuffer rb = pack->GetFileBuffer(sImageFile);
        std::istream   is(&rb);
        rtn = loadFromStream(is);
    }

    return rtn;
}
//-----------------------------------------------------------------------------

olc::rcode CVSprite::LoadFromPGESprFile(const std::string& sImageFile, olc::ResourcePack* pack)
{
    olc::rcode rtn = olc::FAIL;

    if (!pack)
    {
        std::ifstream ifs;
        ifs.open(sImageFile, std::ifstream::binary);
        if (ifs.is_open())
            rtn = loadFromStream(ifs);
    }
    else
    {
        // Load sprite from input stream
        ResourceBuffer rb = pack->GetFileBuffer(sImageFile);
        std::istream   is(&rb);
        rtn = loadFromStream(is);
    }

    return rtn;
}
//-----------------------------------------------------------------------------

olc::rcode CVSprite::SaveToFile(const std::string& sImageFile)
{
    if (isCvImage())
        return cv::imwrite(sImageFile, olcToCv(m_image)) ? olc::OK : olc::FAIL;
    return olc::FAIL;
}
//-----------------------------------------------------------------------------

olc::rcode CVSprite::SaveToPGESprFile(const std::string& sImageFile)
{
    if (pColData == nullptr)
        return olc::FAIL;

    std::ofstream ofs;
    ofs.open(sImageFile, std::ifstream::binary);
    if (ofs.is_open())
    {
        ofs.write((char*)&width, sizeof(int32_t));
        ofs.write((char*)&height, sizeof(int32_t));
        ofs.write((char*)pColData, (size_t)width * (size_t)height * sizeof(uint32_t));
        ofs.close();
        return olc::OK;
    }

    return olc::FAIL;
}
//-----------------------------------------------------------------------------
