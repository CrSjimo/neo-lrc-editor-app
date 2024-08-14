#include "LyricLine.h"
#include "LyricLine_p.h"

#include <QList>
#include <QRegularExpression>

LyricLine::LyricLine() : d(new LyricLineData) {
}

LyricLine::LyricLine(int centisecond, const QString &lyric) : LyricLine() {
    d->centisecond = centisecond;
    d->lyric = lyric;
}

LyricLine::LyricLine(const LyricLine &o) = default;
LyricLine::~LyricLine() = default;
LyricLine &LyricLine::operator=(const LyricLine &o) = default;

bool LyricLine::operator<(const LyricLine &o) const {
    return d->centisecond < o.d->centisecond;
}

bool LyricLine::isNull() const {
    return d->centisecond == -1;
}

void LyricLine::setCentisecond(int centisecond) {
    Q_ASSERT(centisecond >= 0);
    d->centisecond = centisecond;
}

int LyricLine::centisecond() const {
    return d->centisecond;
}

void LyricLine::setLyric(const QString &lyric) {
    d->lyric = lyric;
}

QString LyricLine::lyric() const {
    return d->lyric;
}

QString LyricLine::time() const {
    return QString::number(d->centisecond / 6000) + ":" + QString::number(d->centisecond % 6000 / 100) + "." + QString::number(d->centisecond % 100);
}

QString LyricLine::toString() const {
    return "[" + time() + "]" + d->lyric;
}

QList<LyricLine> LyricLine::parse(const QString &lyricLineStr) {
    static QRegularExpression rx(R"(^(\[\d\d:\d\d\.\d\d\])+(.*)$)");
    auto match = rx.match(lyricLineStr);
    if (!match.hasMatch())
        return {};
    QList<LyricLine> ret;
    auto lyric = match.captured(match.lastCapturedIndex());
    for (int i = 1; i < match.lastCapturedIndex(); i++) {
        auto captured = match.capturedView(i);
        auto minuteStr = captured.mid(1, 2);
        auto secondStr = captured.mid(4, 2);
        auto centisecondStr = captured.mid(7, 2);
        auto centisecond = minuteStr.toInt() * 6000 + secondStr.toInt() * 100 + centisecondStr.toInt();
        ret.append({centisecond, lyric});
    }
    return ret;
}

bool LyricLine::isValidLine(const QString &lyricLineStr) {
    static QRegularExpression lineRx(R"(^(\[\d\d:\d\d\.\d\d\])+(.*)$)");
    static QRegularExpression metadataRx(R"(^\[([a-z#]*):(.*)\]$)");
    static QRegularExpression spaceRx(R"(^\s*$)");
    return lineRx.match(lyricLineStr).hasMatch() || metadataRx.match(lyricLineStr).hasMatch() || spaceRx.match(lyricLineStr).hasMatch();
}
