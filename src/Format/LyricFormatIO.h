#ifndef NEOLRCEDITORAPP_LYRICFORMATIO_H
#define NEOLRCEDITORAPP_LYRICFORMATIO_H

#include <QtGlobal>

class QIODevice;

class LyricLine;

class LyricFormatIO {
public:
    static QList<LyricLine> read(QIODevice *stream, bool *ok = nullptr);
    static void write(QIODevice *stream, const QList<LyricLine> &lyricLines);
};


#endif //NEOLRCEDITORAPP_LYRICFORMATIO_H
