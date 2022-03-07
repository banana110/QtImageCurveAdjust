#include "ImageTools.h"
#include <QImage>

ImageTools::ImageTools()
{

}

void ImageTools::ImgCurveAdjust(QImage &img, const std::vector<unsigned char> &lookUpTable, ICAChannel channel)
{
    if (img.isNull()) return;
    QImage::Format fmt = img.format();
    switch (fmt)
    {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_Grayscale8:
        break;
    default:
        return;//格式不支持
    }

    if (fmt == QImage::Format_Grayscale8)
    {
        uchar* pData = img.bits();
        int pixelCt = img.width() * img.height();
        for (int i = 0; i < pixelCt; ++i)
        {
            pData[i] = lookUpTable[pData[i]];
        }
    }
    else
    {
        for (int x = 0; x < img.width(); ++x)
        {
            for (int y = 0; y < img.height(); ++y)
            {
                QColor color = img.pixelColor(x, y);
                switch (channel)
                {
                case ICAChannel::eChannelAll:
                    color.setRed(lookUpTable[color.red()]);
                    color.setGreen(lookUpTable[color.green()]);
                    color.setBlue(lookUpTable[color.blue()]);
                    break;
                case ICAChannel::eChannelR:
                    color.setRed(lookUpTable[color.red()]);
                    break;
                case ICAChannel::eChannelG:
                    color.setGreen(lookUpTable[color.green()]);
                    break;
                case ICAChannel::eChannelB:
                    color.setBlue(lookUpTable[color.blue()]);
                    break;
                default:
                    Q_ASSERT(false);
                    break;
                }
                img.setPixelColor(x, y, color);
            }
        }
    }
}
