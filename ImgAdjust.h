#ifndef IMGADJUST_H
#define IMGADJUST_H

#include <QMainWindow>
#include <QImage>

namespace Ui {
class ImgAdjust;
}

class ImgAdjust : public QMainWindow
{
    Q_OBJECT

public:
    explicit ImgAdjust(QWidget *parent = nullptr);
    ~ImgAdjust();

private slots:
    void OnAdjust();
    void OnRefresh();
    void OnOpen();
private:
    Ui::ImgAdjust *ui;

    QImage m_img;
};

#endif // IMGADJUST_H
