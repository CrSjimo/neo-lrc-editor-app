#ifndef NEOLRCEDITORAPP_LYRICLINE_P_H
#define NEOLRCEDITORAPP_LYRICLINE_P_H

#include <QSharedData>

struct LyricLineData : QSharedData {
    QString lyric;
    int centisecond = -1;
};

#endif //NEOLRCEDITORAPP_LYRICLINE_P_H
