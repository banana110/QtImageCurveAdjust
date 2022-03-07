#ifndef IMAGEGLOBALDEF_H
#define IMAGEGLOBALDEF_H

//曲线调整模式
enum class ICAChannelMode:int
{
    eChannelModeAll,//灰度图使用，或彩色图使用是RGB整体调整
    eChannelModeSingle,//单通道调整（彩色图使用）
    eChannelModeMulti,//多通道调整（彩色图使用）
};

//曲线调整的色彩通道
enum class ICAChannel:int
{
    eChannelAll,//灰度图使用，或彩色图使用是RGB整体调整
    eChannelR,//红色通道
    eChannelG,//绿色通道
    eChannelB,//蓝色通道
};

#endif // IMAGEGLOBALDEF_H
