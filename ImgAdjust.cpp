#include "ImgAdjust.h"
#include "ui_ImgAdjust.h"
#include "ImageCureAdjustControl/ImageCureAdjustControl.h"
#include "ImageCureAdjustControl/ImageTools.h"
#include <QFile>
#include <QFileDialog>

ImgAdjust::ImgAdjust(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ImgAdjust)
{
    ui->setupUi(this);
    connect(ui->actionCurve_Adjust,&QAction::triggered,this,&ImgAdjust::OnAdjust);
    connect(ui->actionOpen,&QAction::triggered,this,&ImgAdjust::OnOpen);
}

ImgAdjust::~ImgAdjust()
{
    delete ui;
}

void ImgAdjust::OnAdjust()
{
    ui->label->setPixmap(QPixmap::fromImage(m_img));
    ImageCureAdjustControl *adjCtr = new ImageCureAdjustControl(this);
    adjCtr->setWindowFlag(Qt::Tool);
    adjCtr->setFixedSize(430, 365);
    adjCtr->setAttribute(Qt::WA_DeleteOnClose);

    if(adjCtr->Init(m_img))
    {
        adjCtr->show();
        connect(adjCtr,&ImageCureAdjustControl::SigCurveChanged,this,&ImgAdjust::OnRefresh);
        connect(adjCtr,&ImageCureAdjustControl::SigChannelModeChanged,this,&ImgAdjust::OnRefresh);
        connect(adjCtr,&ImageCureAdjustControl::SigChannelChanged,this,&ImgAdjust::OnRefresh);
    }
    else
    {
        delete adjCtr;
    }
}

void ImgAdjust::OnRefresh()
{
    ImageCureAdjustControl *adjCtr = static_cast<ImageCureAdjustControl*>(sender());
    QImage showImg = m_img;
    ICAChannelMode mode = adjCtr->GetAdjustChannelMode();


    switch (mode)
    {
    case ICAChannelMode::eChannelModeAll:
    {
        std::vector<unsigned char> lookUpTable = adjCtr->GetColorLookUpTable(ICAChannel::eChannelAll);
        ImageTools::ImgCurveAdjust(showImg,lookUpTable,ICAChannel::eChannelAll);
        break;
    }
    case ICAChannelMode::eChannelModeSingle:
    {
        ICAChannel channel = adjCtr->GetCurrentChannel();
        std::vector<unsigned char> lookUpTable = adjCtr->GetColorLookUpTable(channel);
        ImageTools::ImgCurveAdjust(showImg,lookUpTable,channel);
        break;
    }
    case ICAChannelMode::eChannelModeMulti:
    {
        std::vector<unsigned char> lookUpTable = adjCtr->GetColorLookUpTable(ICAChannel::eChannelR);
        ImageTools::ImgCurveAdjust(showImg,lookUpTable,ICAChannel::eChannelR);
        lookUpTable = adjCtr->GetColorLookUpTable(ICAChannel::eChannelG);
        ImageTools::ImgCurveAdjust(showImg,lookUpTable,ICAChannel::eChannelG);
        lookUpTable = adjCtr->GetColorLookUpTable(ICAChannel::eChannelB);
        ImageTools::ImgCurveAdjust(showImg,lookUpTable,ICAChannel::eChannelB);
    }
    }
    ui->label->setPixmap(QPixmap::fromImage(showImg));

}

void ImgAdjust::OnOpen()
{
    QString filePath = QFileDialog::getOpenFileName(this,tr("Image"));
    QFile file(filePath);
    if(file.open(QFile::ReadOnly))
    {
        QByteArray buf = file.readAll();
        if(m_img.loadFromData(buf))
        {
            if(m_img.format() == QImage::Format_Indexed8)
            {
                m_img = m_img.convertToFormat(QImage::Format_Grayscale8);
            }

            ui->label->setPixmap(QPixmap::fromImage(m_img));
        }
        else
        {
            ui->label->clear();
        }
    }
}
