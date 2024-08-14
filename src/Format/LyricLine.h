#ifndef NEOLRCEDITORAPP_LYRICLINE_H
#define NEOLRCEDITORAPP_LYRICLINE_H

#include <QSharedDataPointer>

class LyricLineData;

class LyricLine {
public:
    LyricLine();
    LyricLine(int centisecond, const QString &lyric);
    LyricLine(const LyricLine &o);
    ~LyricLine();

    LyricLine &operator=(const LyricLine &o);
    bool operator<(const LyricLine &o) const;

    bool isNull() const;

    void setCentisecond(int centisecond);
    int centisecond() const;

    void setLyric(const QString &lyric);
    QString lyric() const;

    QString time() const;
    QString toString() const;
    static QList<LyricLine> parse(const QString &lyricLineStr);
    static bool isValidLine(const QString &lyricLineStr);

private:
    QSharedDataPointer<LyricLineData> d;
};


#endif //NEOLRCEDITORAPP_LYRICLINE_H
