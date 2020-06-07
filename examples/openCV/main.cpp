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

    olcOpenCv demo;
    if (demo.Construct(480, 270, 2, 2))
    {
        demo.cvSprite = new olc::CVSprite(argv[1], demo.ScreenWidth(), demo.ScreenHeight());
        demo.Start();
    }

	return 0;
}
//-----------------------------------------------------------------------------
