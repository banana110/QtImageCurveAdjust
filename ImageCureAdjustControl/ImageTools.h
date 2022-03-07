#ifndef IMAGETOOLS_H
#define IMAGETOOLS_H
#include "ImageGlobalDef.h"
#include <vector>

class QImage;

class ImageTools
{
public:
    ImageTools();

    static void ImgCurveAdjust(QImage& img, const std::vector<unsigned char>& lookUpTable, ICAChannel channel);
};

#endif // IMAGETOOLS_H
