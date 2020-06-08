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
    if (demo.Construct(960, 540, 1, 1))
    {
        demo.cvSprite = new olc::CVSprite(argv[1], demo.ScreenWidth(), demo.ScreenHeight());
        //demo.cvSprite->SaveToPGESprFile("file.pgr");
        //demo.cvSprite->SaveToFile("file.png");
        //demo.cvSprite = new olc::CVSprite();
        //demo.cvSprite->LoadFromPGESprFile("file.pgr");
        //demo.cvSprite->LoadFromFile("file.png");

        demo.Start();
    }

	return 0;
}
//-----------------------------------------------------------------------------
