#ifndef PLAYBACKCACHERTYPES_H
#define PLAYBACKCACHERTYPES_H

#include <QtGlobal>

/**
 * @brief 回放参数结构体
 */
struct PlaybackCacherParams
{
    qint64 startFrameNo{ -1 };
    qint64 endFrameNo{ -1 };
    qint64 frameInterval{ -1 };
    qint64 curFrameNo{ -1 };
};

#endif // PLAYBACKCACHERTYPES_H
