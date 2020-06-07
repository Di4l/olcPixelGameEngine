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

CVSprite::CVSprite(cv::Mat& image) : Sprite(), m_image(image)
{
    updateSprite();
}
//-----------------------------------------------------------------------------

CVSprite::~CVSprite()
{
    if (m_image.rows > 0 || m_image.cols > 0)
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

bool CVSprite::loadImage(const std::string& sImageFile)
{
    cv::Mat img = cv::imread(sImageFile, cv::IMREAD_COLOR);
    if (img.channels() > 4)
        return false;

    cv::Mat aux(img.size(), CV_MAKE_TYPE(img.depth(), 4));
    switch (img.channels())
    {
    case 1:
        cv::mixChannels({ { img } }, { { aux } }, { 0,0, 0,1, 0,2 });
        //-- set Alpha channel to 255 (opaque in CV, transparent in olc)
        aux.reshape(1, aux.rows * aux.cols).col(3).setTo(cv::Scalar(255));
        break;

    case 2:
        cv::mixChannels({ { img } }, { { aux } }, { 0,0, 0,1, 0,2, 1,3 });
        break;

    case 3:
        cv::mixChannels({ { img } }, { { aux } }, { 0,2, 1,1, 2,0 });
        //-- set Alpha channel to 255 (opaque in CV, transparent in olc)
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
    return true;
}
//-----------------------------------------------------------------------------


Sprite* pgexOpenCV::toSprite(cv::Mat& image)
{
    Sprite* sprt = nullptr;

    if (image.channels() == 3)
    {
        cv::Mat aux(image.size(), CV_MAKE_TYPE(image.depth(), 4));
        cv::mixChannels({ { image } }, { { aux } }, { 0,2, 1,1, 2,0 });
        //-- set Alpha channel to 255 (opaque in CV, transparent in olc)
        aux.reshape(1, aux.rows * aux.cols).col(3).setTo(cv::Scalar(255));
        //cv::namedWindow("Display reshaped", cv::WINDOW_AUTOSIZE); // Create a window for display.
        //cv::imshow("Display reshaped", newSrc); // Show our image inside it.
        cv::swap(aux, image);
    }
    else if (image.channels() == 2)
    {
        cv::Mat aux(image.size(), CV_MAKE_TYPE(image.depth(), 4));
        cv::mixChannels({ { image } }, { { aux } }, { 0,0, 0,1, 0,2, 1,3 });
        cv::swap(aux, image);
    }
    else if (image.channels() == 1)
    {
        cv::Mat aux(image.size(), CV_MAKE_TYPE(image.depth(), 4));
        cv::mixChannels({ { image } }, { { aux } }, { 0,0, 0,1, 0,2 });
        aux.reshape(1, aux.rows * aux.cols).col(3).setTo(cv::Scalar(255));
        cv::swap(aux, image);
    }
    else if(image.channels() > 4)
    {
        return nullptr;
    }

    if (image.isContinuous())
    {
        sprt = new olc::Sprite(image.cols, image.rows);
        uchar* p = image.ptr<uchar>(0);
        if (sprt->pColData)
            delete sprt->pColData;
        sprt->pColData = (olc::Pixel*)p;
    }

    return sprt;
}
//-----------------------------------------------------------------------------