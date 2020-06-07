//-----------------------------------------------------------------------------
#include "open_cv.h"
#include "openCV/pgexOpenCV.h"
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
//-----------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << " Usage: " << argv[0] << " ImageToLoadAndDisplay" << std::endl;
        return -1;
    }
    cv::Mat image;
    image = cv::imread(argv[1], cv::IMREAD_COLOR); // Read the file
    if (image.empty()) // Check for invalid input
    {
        std::cout << "Could not open or find the image" << std::endl;
        return -1;
    }

    cv::Mat aux;
    cv::resize(image, aux, cv::Size(), 0.5, 0.5);
    cv::swap(aux, image);

    int64_t cnt = image.rows * image.cols;
    olcOpenCv demo;
    if (demo.Construct(image.cols, image.rows, 1, 1))
    {
        demo.sprite   = olc::pgexOpenCV::toSprite(image);
        //demo.cvSprite = new olc::CVSprite(image);
        demo.cvSprite = new olc::CVSprite(argv[1]);
        demo.Start();

        //-- OpenCV takes care of the actual image data
        demo.sprite->pColData = nullptr;
    }

	return 0;
}
//-----------------------------------------------------------------------------
