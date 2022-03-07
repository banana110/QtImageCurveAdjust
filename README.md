# QtImageCurveAdjust
Image Brightness Contrast Curve Adjust Qt impl demo

Base on qt just like photoshop

Features：

1、Multi points curve control

2、Histogram show

3、Gray image curve adjust

4、Color image curve adjust with 3 mode

（1）all channel total adjust

（2）single channel adjust

（3）Multi channel adjust

Demo manual： 1、top menu “Adjust” -》“Open” to open an image file

2、top menu “Adjust” -》“Curve Adjust” to adjust image

spline lib is from ： https://people.sc.fsu.edu/~jburkardt/c_src/spline/spline.html

and i modify some method return type(origin is exit(-1) change to return nullptr)

aaCurve impl is from： https://github.com/pyzhangxiang/qt-curve-editor

and i add function: aaCurvePtr createCurve(const std::vector<std::pair<unsigned char, unsigned char> >& spline_data, SplineType type) use to load data from std vector
