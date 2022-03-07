#include "ImageCureAdjustControl.h"
#include "ui_ImageCureAdjustControl.h"
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QPen>
#include "internal/aaCurve.h"
//根据ui设计规定出界面上的曲线坐标系范围
constexpr int g_uiMapLeft = 61;
constexpr int g_uiMapTop = 71;
constexpr int g_uiMapRight = 316;
constexpr int g_uiMapBottom = 326;

//调整点正方形的大小    不宜过大  必须为奇数且不得小于3
constexpr int g_uiKnotSize = 5;

//测试是否靠近曲线的接近距离
constexpr int g_testOffset = 2;

ImageCureAdjustControl::ImageCureAdjustControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageCureAdjustControl)
{
    Q_ASSERT(g_uiKnotSize >= 3 && (g_uiKnotSize % 2) == 1);
    ui->setupUi(this);
    setMouseTracking(true);
	connect(ui->chk_baseLine, &QCheckBox::stateChanged, this, static_cast<void(QWidget::*)()>(&QWidget::repaint));
	connect(ui->chk_Histogram, &QCheckBox::stateChanged, this, static_cast<void(QWidget::*)()>(&QWidget::repaint));
    connect(ui->chk_combine, &QCheckBox::stateChanged, this, static_cast<void(QWidget::*)()>(&QWidget::repaint));
	connect(ui->cb_channelMode, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ImageCureAdjustControl::OnCBChannelModeChanged);
	connect(ui->cb_channel, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ImageCureAdjustControl::OnCBChannelChanged);
	connect(ui->pb_reset, &QPushButton::clicked, this, &ImageCureAdjustControl::OnReset);

    OnChannelModeChanged(m_channelMode);
	ui->cb_channelMode->setEnabled(false);
	ui->pb_reset->setEnabled(false);
}

ImageCureAdjustControl::~ImageCureAdjustControl()
{
    delete ui;
}

bool ImageCureAdjustControl::Init(const QImage& img)
{
	Clear();
    if (img.isNull()) return false;
    QImage::Format fmt = img.format();
    switch (fmt)
    {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
        ui->cb_channelMode->setEnabled(true);
        break;
    case QImage::Format_Grayscale8:
        ui->cb_channelMode->setEnabled(false);
        break;
    default:
        return false;//格式不支持
    }
    m_QImage = img;


    std::list<ICAChannel> channels;
    channels.push_back(ICAChannel::eChannelAll);
    if(fmt != QImage::Format_Grayscale8)
    {
        channels.push_back(ICAChannel::eChannelR);
        channels.push_back(ICAChannel::eChannelG);
        channels.push_back(ICAChannel::eChannelB);
    }

    for(auto channel:channels)
    {
         //曲线起始控制点
        auto& knots = m_channel2Knots[channel];
        knots.push_back(pair<unsigned char, unsigned char>(0, 0));
        knots.push_back(pair<unsigned char, unsigned char>(255, 255));

        //计算曲线数据
        CalcCurveData(channel);

        //直方图
        auto& his = m_channel2Histogram[channel];
        long long maxLevelCount = 0;
        his.resize(256);
        for (int x = 0; x < m_QImage.width(); ++x)
        {
            for (int y = 0; y < m_QImage.height(); ++y)
            {
                QColor color = m_QImage.pixelColor(x,y);
                unsigned char level = 0;
                switch (channel)
                {
                case ICAChannel::eChannelAll:
                {
                    if(fmt == QImage::Format_Grayscale8)
                    {
                        level = color.red();//灰度图取出来rgb都是一样的
                    }
                    else
                    {
                        level = qGray(color.red(),color.green(),color.blue());
                    }
                    break;
                }
                case ICAChannel::eChannelR:
                {
                    level = color.red();
                    break;
                }
                case ICAChannel::eChannelG:
                {
                    level = color.green();
                    break;
                }
                case ICAChannel::eChannelB:
                {
                    level = color.blue();
                    break;
                }
                default:
                    Q_ASSERT(false);
                    break;
                }
                his[level] += 1;
                maxLevelCount = (maxLevelCount < his[level] ? his[level] : maxLevelCount);
            }
        }
        m_channel2HistogramMaxVal[channel] = maxLevelCount;
    }

    //刷新界面
    if (ui->cb_channelMode->currentIndex() == 0)
    {
        OnChannelModeChanged(ICAChannelMode::eChannelModeAll);
    }
    else
    {
        ui->cb_channelMode->setCurrentIndex(0);
    }

    ui->pb_reset->setEnabled(true);
    return true;
}

#if 0
bool ImageCureAdjustControl::Init(const MYImage& img)
{
    Clear();
    m_isMYImage = true;
    if (img.isEmpty() || img.type != MYImageType::RAW || img.bitPerPixel != 8)
    {
        m_isMYImage = false;
		return false;
    }
    m_MYImage = img;
    
    ui->cb_channelMode->setEnabled(false);
    //曲线起始控制点
    auto& knots = m_channel2Knots[ICAChannel::eChannelAll];
	knots.push_back(pair<unsigned char, unsigned char>(0, 0));
	knots.push_back(pair<unsigned char, unsigned char>(255, 255));

    //计算曲线数据
    CalcCurveData(ICAChannel::eChannelAll);

    //直方图
    auto& his = m_channel2Histogram[ICAChannel::eChannelAll];
    int sz = m_MYImage.image.size();
    Q_ASSERT(sz == (m_MYImage.height * m_MYImage.width));
    auto pData = (unsigned char*)m_MYImage.image.data();
    long long maxLevelCount = 0;
    his.resize(256);
    for (int i = 0; i < sz; ++i)
    {
        unsigned char level = pData[i];
        his[level] += 1;
        maxLevelCount = (maxLevelCount < his[level] ? his[level] : maxLevelCount);
    }
    m_channel2HistogramMaxVal[ICAChannel::eChannelAll] = maxLevelCount;

    //刷新界面
    if (ui->cb_channelMode->currentIndex() == 0)
    {
        OnChannelModeChanged(ICAChannelMode::eChannelModeAll);
    }
    else
    {
        ui->cb_channelMode->setCurrentIndex(0);
    }

	ui->pb_reset->setEnabled(true);
    return true;
}
#endif

ICAChannelMode ImageCureAdjustControl::GetAdjustChannelMode()
{
    return CBIndex2ChannelMode(ui->cb_channelMode->currentIndex());
}

void ImageCureAdjustControl::SetAdjustChannelMode(ICAChannelMode mode)
{
    ui->cb_channelMode->setCurrentIndex(ChannelMode2CBIndex(mode));
}

vector<unsigned char> ImageCureAdjustControl::GetColorLookUpTable(ICAChannel channel)
{
    vector<unsigned char> lookUpTable;
    lookUpTable.resize(256);
	auto& curveData = m_channel2CurveData[channel];
    for (int i=0;i<curveData.begin()->first;++i)
    {
        lookUpTable[i] = i;
    }
    for (int i=curveData.rbegin()->first + 1;i<256;++i)
    {
        lookUpTable[i] = i;
    }
    for (auto const& data : curveData)
    {
		Q_ASSERT(data.first >= 0 && data.first < 256);
		Q_ASSERT(data.second >= 0 && data.second < 256);
        lookUpTable[data.first] = data.second;
    }
    return lookUpTable;
}

vector<std::pair<unsigned char, unsigned char> > ImageCureAdjustControl::GetColorAdjustPoint(ICAChannel channel)
{
    return m_channel2Knots[channel];
}

bool ImageCureAdjustControl::SetColorAdjustPoint(ICAChannel channel, const vector<pair<unsigned char, unsigned char> >& points)
{
    if (points.size() < 2)
    {
        return false;//控制点少于两个（开始点和结束点）
    }

    auto& knots = m_channel2Knots[channel];
    if (knots.size() == 0)
    {
        return false;//当前颜色通道没有被初始化  （当前图像不支持这个通道的调整或者还没设置图像进行初始化）
    }
    Q_ASSERT(knots.size() >= 2);
    QRect tCoordinate(0, 0, 256, 256);
    for (int i=0;i<points.size();++i)
    {
        if (!tCoordinate.contains(points[i].first,points[i].second))
        {
            return false;//超出坐标系
        }
		if (i > 0)
		{
            if ((points[i].first - g_uiKnotSize) <= points[i-1].first)
            {
                return false;//后一个点和前一个点间距小于控制点绘画的大小（本程序设定不能小于这个间隔 不然没太大意义也给显示造成重叠）
            }
		}
    }
    knots = points;
	bool bCombineShow = (ui->chk_combine->isChecked() && ui->chk_combine->isEnabled());
    if (channel == GetCurrentChannel() || bCombineShow)//组合显示也要重新计算刷新
    {
        CalcCurveData(channel);
        repaint();
    }
    return true;
}

void ImageCureAdjustControl::SetGrayImageLayout(bool bGray)
{
	ui->label->setVisible(!bGray);
	ui->label_2->setVisible(!bGray);
	ui->chk_combine->setVisible(!bGray);
	ui->cb_channelMode->setVisible(!bGray);
	ui->cb_channel->setVisible(!bGray);

    if (!bGray)
    {
		ui->chk_baseLine->move(220, 3);
		ui->chk_Histogram->move(220, 29);
		ui->pb_reset->move(310, 33);
    }
    else
    {
		ui->chk_baseLine->move(6, 3);
		ui->chk_Histogram->move(6, 29);
		ui->pb_reset->move(93, 33);
	}
}

void ImageCureAdjustControl::Clear()
{
	m_QImage = QImage();
    m_channelMode = ICAChannelMode::eChannelModeAll;
    m_channel2Knots.clear();
    m_channel2CurveData.clear();
    m_channel2Histogram.clear();
    m_channel2HistogramMaxVal.clear();
    m_curCurve = QPainterPath();
	ui->cb_channelMode->setEnabled(false);
	ui->cb_channelMode->setCurrentIndex(0);
	ui->cb_channel->setCurrentIndex(0);
    ui->chk_combine->setChecked(false);
    ui->pb_reset->setEnabled(false);
    m_curKnotIndex = -1;
    m_curDragKnotIndex = -1;
    m_noMove2Del = false;
    m_hasMove = false;
}

void ImageCureAdjustControl::OnCBChannelModeChanged(int index)
{
	OnChannelModeChanged(CBIndex2ChannelMode(index));
    emit SigChannelModeChanged();
}

void ImageCureAdjustControl::OnCBChannelChanged(int index)
{
    if (ui->cb_channelMode->currentIndex() == 0) return;
	switch (index)
	{
	case 0:
		OnChannelChanged(ICAChannel::eChannelR);
		break;
	case 1:
		OnChannelChanged(ICAChannel::eChannelG);
		break;
	case 2:
		OnChannelChanged(ICAChannel::eChannelB);
		break;
	default:
		break;
	}
    emit SigChannelChanged();
}

void ImageCureAdjustControl::OnChannelModeChanged(ICAChannelMode mode)
{
    ui->cb_channel->setEnabled(mode != ICAChannelMode::eChannelModeAll);
    ui->chk_combine->setEnabled(mode == ICAChannelMode::eChannelModeMulti);
    switch (mode)
    {
    case ICAChannelMode::eChannelModeAll:
    {
        SetColorIndicator(ICAChannel::eChannelAll);
        break;
    }
    case ICAChannelMode::eChannelModeSingle:
    case ICAChannelMode::eChannelModeMulti:
    {
        OnCBChannelChanged(ui->cb_channel->currentIndex());
        break;
    }
    default:
        Q_ASSERT(false);
        break;
    }
    repaint();
    return;
}

void ImageCureAdjustControl::OnChannelChanged(ICAChannel channel)
{
	SetColorIndicator(channel);
	repaint();
}

void ImageCureAdjustControl::OnReset()
{
    ICAChannel currentChannel = GetCurrentChannel();
	auto& knots = m_channel2Knots[currentChannel];
    if (knots.size() == 0)
    {
        return;
    }
    if (knots.size() == 2 && knots[0] == pair<unsigned char, unsigned char>(0, 0)
        && knots[1] == pair<unsigned char, unsigned char>(255, 255))
    {
        return;
    }
    knots.clear();
	knots.push_back(pair<unsigned char, unsigned char>(0, 0));
	knots.push_back(pair<unsigned char, unsigned char>(255, 255));

    CalcCurveData(currentChannel);
    repaint();
    emit SigCurveChanged();
}

void ImageCureAdjustControl::SetColorIndicator(ICAChannel channel)
{
	ui->wd_colorBlack1->setVisible(channel == ICAChannel::eChannelAll);
	ui->wd_colorBlack2->setVisible(channel == ICAChannel::eChannelAll);
	ui->wd_colorRed1->setVisible(channel == ICAChannel::eChannelR);
	ui->wd_colorRed2->setVisible(channel == ICAChannel::eChannelR);
	ui->wd_colorGreen1->setVisible(channel == ICAChannel::eChannelG);
	ui->wd_colorGreen2->setVisible(channel == ICAChannel::eChannelG);
	ui->wd_colorBlue1->setVisible(channel == ICAChannel::eChannelB);
	ui->wd_colorBlue2->setVisible(channel == ICAChannel::eChannelB);
}

QPoint ImageCureAdjustControl::WndPos2CoordinatePos(QPoint pos)
{
    int x = pos.x() - g_uiMapLeft;
    int y = g_uiMapBottom - pos.y();
    if (x >= 0 && y >= 0 && x < 256 && y < 256)
    {
		return QPoint(x, y);
    }
    return QPoint(-1, -1);
}

QPoint ImageCureAdjustControl::CoordinatePos2WndPos(QPoint pos)
{
    int x = pos.x() + g_uiMapLeft;
    int y = g_uiMapBottom - pos.y();
    return QPoint(x, y);
}

ICAChannel ImageCureAdjustControl::Ui2Channel()
{
	switch (ui->cb_channel->currentIndex())
	{
	case 0:
		return ICAChannel::eChannelR;
	case 1:
		return ICAChannel::eChannelG;
	case 2:
		return ICAChannel::eChannelB;
	default:
		break;
	}
	Q_ASSERT(false);
	return ICAChannel::eChannelAll;
}

QRect ImageCureAdjustControl::GetKnotRect(int x, int y)
{
    QPoint wndPos = CoordinatePos2WndPos(QPoint(x, y));

	int x1 = wndPos.x() - (g_uiKnotSize / 2);
	int y1 = wndPos.y() - (g_uiKnotSize / 2);
	return QRect(x1, y1, g_uiKnotSize, g_uiKnotSize);
}

void ImageCureAdjustControl::CalcCurveData(ICAChannel channel)
{
    auto const & srcData = m_channel2Knots[channel];
    aaAaa::aaCurvePtr pCurve = aaAaa::aaCurveFactory::createCurve(srcData,aaAaa::SplineType::SPLINE_CUBIC);
    int s = srcData.begin()->first;
    int e = srcData.rbegin()->first;
    auto& curveData = m_channel2CurveData[channel];
    curveData.clear();
    for (int i=s;i<=e;++i)
    {
        double y = 0;
        pCurve->getValue(i, y);
        unsigned char y1 = 0;
        if (y <= 255 && y >= 0) y1 = y;
        else if (y > 255) y1 = 255;
        else if (y < 0) y1 = 0;
        curveData.push_back(pair<unsigned char, unsigned char>(i, y1));
    }
}

ICAChannel ImageCureAdjustControl::GetCurrentChannel()
{
	ICAChannel currentChannel = ICAChannel::eChannelAll;
	if (ui->cb_channelMode->currentIndex() != 0)
	{
		currentChannel = Ui2Channel();
	}
    return currentChannel;
}

bool ImageCureAdjustControl::TestNewPos(QPoint pos, ICAChannel channel, int index)
{
	auto const & knotsData = m_channel2Knots[channel];
	int i1 = (index == 0 ? 0 : index - 1);
	int i2 = (index == (knotsData.size() - 1) ? index : index + 1);
	int x1 = knotsData[i1].first;
	int x2 = knotsData[i2].first;
	if ((pos.x() > (x1 + g_uiKnotSize) || index == 0) &&
		(pos.x() < (x2 - g_uiKnotSize) || index == (knotsData.size() - 1)))
	{
        return true;
	}
    return false;
}

ICAChannelMode ImageCureAdjustControl::CBIndex2ChannelMode(int i)
{
	switch (i)
	{
	case 0://RGB整体调整
        return ICAChannelMode::eChannelModeAll;
	case 1://单通道调整
		return ICAChannelMode::eChannelModeSingle;
	case 2://多通道调整
		return ICAChannelMode::eChannelModeMulti;
	default:
		break;
	}
    Q_ASSERT(false);
    return ICAChannelMode::eChannelModeAll;
}

int ImageCureAdjustControl::ChannelMode2CBIndex(ICAChannelMode mode)
{
    switch (mode)
    {
    case ICAChannelMode::eChannelModeAll:
        return 0;
    case ICAChannelMode::eChannelModeSingle:
		return 1;
    case ICAChannelMode::eChannelModeMulti:
		return 2;
    default:
        break;
    }
    Q_ASSERT(false);
    return 0;
}

void ImageCureAdjustControl::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	QPen pen;

    ICAChannel currentChannel = GetCurrentChannel();

    //基线
    if (ui->chk_baseLine->isChecked())
    {
        pen.setColor(Qt::gray);
        painter.setPen(pen);
        painter.drawLine(g_uiMapLeft, g_uiMapBottom, g_uiMapRight, g_uiMapTop);
    }

    //直方图
    if (ui->chk_Histogram->isChecked())
    {
        
        long long maxVal = m_channel2HistogramMaxVal.value(currentChannel, -1);
        auto &his = m_channel2Histogram[currentChannel];
		if (maxVal != -1 && his.size() == 256)
        {
            switch (currentChannel)
            {
            case ICAChannel::eChannelAll:
				pen.setColor(Qt::gray);
				break;
            case ICAChannel::eChannelR:
                pen.setColor(QColor(250, 205, 185));
                break;
            case ICAChannel::eChannelG:
				pen.setColor(QColor(205, 255, 185));
				break;
            case ICAChannel::eChannelB:
				pen.setColor(QColor(215, 215, 250));
				break;
            default:
                break;
            }
			painter.setPen(pen);
            long long stepVal = (maxVal > 256 ? (maxVal / 256) : 1);//在曲线的坐标系中y轴最多到256个
            for (int i=0; i<256; ++i)
            {
                long long y = his[i];
                if (y > 0)
                {
                    y = (y / stepVal);
                    if (y == 0) y = 1;
                    QPoint wndPosS = CoordinatePos2WndPos(QPoint(i, 0));
                    QPoint wndPosE = CoordinatePos2WndPos(QPoint(i, y - 1));
                    painter.drawLine(wndPosS, wndPosE);
                }
            }
        }
    }
    auto GetChannelColor = [](ICAChannel channel)->QColor
    {
		switch (channel)
		{
		case ICAChannel::eChannelAll:
			return QColor(Qt::black);
			break;
		case ICAChannel::eChannelR:
            return QColor(Qt::red);
			break;
		case ICAChannel::eChannelG:
            return QColor(Qt::green);
			break;
		case ICAChannel::eChannelB:
            return QColor(Qt::blue);
			break;
		default:
			break;
		}
        Q_ASSERT(false);
        return QColor(Qt::black);
    };
    //曲线
    m_curCurve = QPainterPath();
    QList<ICAChannel> chs;
    currentChannel == ICAChannel::eChannelAll ? (chs << ICAChannel::eChannelAll) :
        (chs << ICAChannel::eChannelR << ICAChannel::eChannelG << ICAChannel::eChannelB);
    bool bCombineShow = (ui->chk_combine->isChecked() && ui->chk_combine->isEnabled());
    for (auto channel : chs)
    {
        if(channel != currentChannel && !bCombineShow) continue;
        auto& curveData = m_channel2CurveData[channel];
        int sz = curveData.size();
        if (sz > 1)
        {
			QColor color = GetChannelColor(channel);
			pen.setColor(color);
			painter.setPen(pen);
            QPainterPath curCurvePath;
			auto it = curveData.begin();
            curCurvePath.moveTo(CoordinatePos2WndPos(QPoint(it->first,it->second)));
            ++it;
            while(it!= curveData.end())
            {
                //pair<unsigned char, unsigned char> tXY = curveData[i];
                //path.lineTo(g_uiMapLeft + tXY.first, g_uiMapBottom - tXY.second);
                curCurvePath.lineTo(CoordinatePos2WndPos(QPoint(it->first, it->second)));
                ++it;
            }
            painter.drawPath(curCurvePath);
            if(channel == currentChannel) m_curCurve = curCurvePath;
        }
    }

	//控制点
	auto &knotsData = m_channel2Knots[currentChannel];
    int sz = knotsData.size();
    if (sz > 0)
    {
        Q_ASSERT(sz >= 2);
		QColor color = GetChannelColor(currentChannel);
        pen.setColor(color);
		painter.setPen(pen);
        
        for (int i = 0;i < sz;++i)
        {
            QRect rect = GetKnotRect(knotsData[i].first, knotsData[i].second);
            painter.drawRect(rect);
            if (i == m_curKnotIndex)
            {
				painter.fillRect(rect, color);
            }
        }
    }
    return;
}

void ImageCureAdjustControl::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::MouseButton::RightButton)//右键删除控制点
    {
		auto outPos = WndPos2CoordinatePos(event->pos());
        if (outPos != QPoint(-1, -1))
        {
			m_curKnotIndex = -1;
			ICAChannel currentChannel = GetCurrentChannel();
            auto& knotsData = m_channel2Knots[currentChannel];
            //掐头去尾查找   不删除头尾两个点
            auto it = knotsData.begin()+1;
            bool finded = false;
			for (;it!= knotsData.end() -1 ;)
            {
                QPoint knotPos(it->first, it->second);
                QRect rect = GetKnotRect(knotPos.x(),knotPos.y());
                if (rect.contains(event->pos()))
                {
                    knotsData.erase(it);
                    finded = true;
                    break;
                }
                ++it;
            }
            if (finded)
            {
                CalcCurveData(currentChannel);
				repaint();
                emit SigCurveChanged();
			}
		}
    }
    else if (event->buttons() == Qt::MouseButton::LeftButton)//左键添加控制点并开启拖拽
    {
		auto outPos = WndPos2CoordinatePos(event->pos());
		if (outPos != QPoint(-1, -1))
		{
			m_curKnotIndex = -1;
			//先判定是否点到了已有控制点  同时查找新插入的下标
            int newIndex = -1;
			ICAChannel currentChannel = GetCurrentChannel();
			auto& knotsData = m_channel2Knots[currentChannel];
			int sz = knotsData.size();
			for (int i = 0; i< sz; ++i)
			{
				QRect rect = GetKnotRect(knotsData[i].first, knotsData[i].second);
				if (rect.contains(event->pos()) && m_curKnotIndex == -1)
				{
					m_curDragKnotIndex = i;
					m_curKnotIndex = i;
                    m_noMove2Del = false;
				}
                
                if (knotsData[i].first < outPos.x())
                {
                    newIndex = i + 1;
                }
			}
            //找不到则继续判定是否要添加新的控制点
			if (m_curDragKnotIndex == -1)
			{
                if (newIndex > 0 && newIndex < sz)
                {
                    QPainterPath testArea;
                    auto pos = QPoint(event->pos().x() - g_testOffset, event->pos().y() - g_testOffset);
                    testArea.addEllipse(pos.x(), pos.y(), g_uiKnotSize, g_uiKnotSize);
                    //测试鼠标是否接近曲线
                    if (testArea.intersects(m_curCurve))
                    {
                        int x1 = knotsData[newIndex - 1].first;
                        int x2 = knotsData[newIndex].first;
                        if (outPos.x() > (x1 + g_uiKnotSize) && outPos.x() < (x2 - g_uiKnotSize))
                        {
                            auto it = knotsData.begin() + newIndex;
                            knotsData.insert(it, pair<unsigned char, unsigned char>(outPos.x(), outPos.y()));
                            m_curDragKnotIndex = newIndex;
                            m_noMove2Del = true;
                        }
                    }
					else
					{
						repaint();
					}
                }
                else
                {
                    repaint();
                }
			}
            else//找到则更新界面刷新当前控制点显示
            {
				repaint();
            }
		}
    }
	return QWidget::mousePressEvent(event);
}

void ImageCureAdjustControl::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::MouseButton::LeftButton)
	{
        //停止拖拽
        if (m_curDragKnotIndex != -1)
        {
            //处理新添加但是没动过的控制点直接删除
            if (m_noMove2Del && !m_hasMove)
            {
                ICAChannel currentChannel = GetCurrentChannel();
                auto& knotsData = m_channel2Knots[currentChannel];
                auto it = knotsData.begin() + m_curDragKnotIndex;
                knotsData.erase(it);
            }

            //动过控制点的发送信号
            if (m_hasMove)
            {
                emit SigCurveChanged();
            }

			m_curDragKnotIndex = -1;
		}
		m_hasMove = false;
	}
}

void ImageCureAdjustControl::mouseMoveEvent(QMouseEvent* event)
{
    auto outPos = WndPos2CoordinatePos(event->pos());
    if (outPos != QPoint(-1, -1))
    {

		//设置界面显示坐标系的值
        ui->label_inNum->setText(QString::number(outPos.x()));
        ui->label_outNum->setText(QString::number(outPos.y()));

        //处理拖拽
		if (event->buttons() == Qt::MouseButton::LeftButton && m_curDragKnotIndex != -1)
		{
			ICAChannel currentChannel = GetCurrentChannel();
			if (TestNewPos(outPos,currentChannel,m_curDragKnotIndex))
			{
				m_hasMove = true;//有效的移动才标记为真
				auto& knotsData = m_channel2Knots[currentChannel];
				knotsData[m_curDragKnotIndex].first = outPos.x();
				knotsData[m_curDragKnotIndex].second = outPos.y();
                CalcCurveData(currentChannel);
                repaint();
			}
		}


    }
    else
    {
        ui->label_inNum->clear();
        ui->label_outNum->clear();
    }

    return QWidget::mouseMoveEvent(event);
}
