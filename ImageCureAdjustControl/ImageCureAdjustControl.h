#ifndef IMAGECURVEADJUSTCONTROL_H
#define IMAGECURVEADJUSTCONTROL_H

#include <QWidget>
#include <QMap>
#include <vector>
#include <list>
#include <utility>
#include <QPainterPath>
#include "ImageGlobalDef.h"

namespace Ui {
class ImageCureAdjustControl;
}
using namespace std;

class ImageCureAdjustControl : public QWidget
{
    Q_OBJECT

public:
    explicit ImageCureAdjustControl(QWidget *parent = nullptr);
    ~ImageCureAdjustControl();
public:
	//用于处理彩色图片
	bool Init(const QImage& img);

	//获取当前处理的色彩通道模式
	ICAChannelMode GetAdjustChannelMode();

    //获取当前处理的色彩通道
    ICAChannel GetCurrentChannel();

	//设置当前处理的色彩通道模式  若用灰度图初始化控件则不要调用设置别的模式，灰度图只有一种模式和一个通道
	void SetAdjustChannelMode(ICAChannelMode mode);

	//获取对应色彩通道的颜色检索表 vector的大小为定长256
	vector<unsigned char> GetColorLookUpTable(ICAChannel channel);

	//获取对应色彩通道的调整控制点参数
	vector<pair<unsigned char, unsigned char> > GetColorAdjustPoint(ICAChannel channel);

	//设置对应色彩通道的调整控制点参数，控件没有被初始化时会失败
	bool SetColorAdjustPoint(ICAChannel channel, const vector<pair<unsigned char, unsigned char> > &points);

	//设置只有灰度图的布局
	void SetGrayImageLayout(bool bGray);

	//清理当前控件
    void Clear();

public:signals:
	void SigCurveChanged();
	void SigChannelModeChanged();
	void SigChannelChanged();

private slots:
	void OnCBChannelModeChanged(int index);
	void OnCBChannelChanged(int index);
	void OnChannelModeChanged(ICAChannelMode mode);
	void OnChannelChanged(ICAChannel channel);
	void OnReset();
private:
    void SetColorIndicator(ICAChannel channel);
	QPoint WndPos2CoordinatePos(QPoint pos);
	QPoint CoordinatePos2WndPos(QPoint pos);
    ICAChannel Ui2Channel();
    QRect GetKnotRect(int x, int y);
    void CalcCurveData(ICAChannel channel);
	bool TestNewPos(QPoint pos, ICAChannel channel, int index);
	ICAChannelMode CBIndex2ChannelMode(int i);
	int ChannelMode2CBIndex(ICAChannelMode mode);

protected:
    virtual void paintEvent(QPaintEvent* event)override;
	virtual void mousePressEvent(QMouseEvent* event)override;
	virtual void mouseReleaseEvent(QMouseEvent* event)override;
    virtual void mouseMoveEvent(QMouseEvent* event)override;
private:
    Ui::ImageCureAdjustControl *ui;

private:
    //处理的源图像：
	QImage m_QImage;

    //处理的数据
    ICAChannelMode m_channelMode = ICAChannelMode::eChannelModeAll;
	QMap<ICAChannel, vector<pair<unsigned char, unsigned char> > > m_channel2Knots;
	QMap<ICAChannel, list<pair<unsigned char, unsigned char> > > m_channel2CurveData;
	QMap<ICAChannel, vector<long long> > m_channel2Histogram;
	QMap<ICAChannel, long long> m_channel2HistogramMaxVal;

	QPainterPath m_curCurve;

	int m_curKnotIndex = -1;
	int m_curDragKnotIndex = -1;
    bool m_noMove2Del = false;
    bool m_hasMove = false;
};

#endif // IMAGECURVEADJUSTCONTROL_H
