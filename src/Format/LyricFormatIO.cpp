#include "LyricFormatIO.h"

#include <algorithm>

#include <QList>
#include <QIODevice>
#include <QTextStream>
#include <QRegularExpression>

#include <NeoLrcEditorApp/LyricLine.h>

QList<LyricLine> LyricFormatIO::read(QIODevice *stream, bool *ok) {
    QTextStream t(stream);
    t.setEncoding(QStringConverter::Utf8);
    QList<LyricLine> ret;
    QString str;
    while (t.readLineInto(&str)) {
        if (!LyricLine::isValidLine(str)) {
            if (ok)
                *ok = false;
            return {};
        }
        ret.append(LyricLine::parse(str));
    }
    std::sort(ret.begin(), ret.end());
    if (ok)
        *ok = true;
    return ret;
}

void LyricFormatIO::write(QIODevice *stream, const QList<LyricLine> &lyricLines) {
    QTextStream t(stream);
    t.setEncoding(QStringConverter::Utf8);
    for (const auto &lyricLine : lyricLines) {
        t << lyricLine.toString() << "\n";
    }
    t.flush();
}
