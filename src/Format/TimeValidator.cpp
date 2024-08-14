#include "TimeValidator.h"

#include <QRegularExpression>

TimeValidator::TimeValidator(QObject *parent) : QValidator(parent) {
}

TimeValidator::~TimeValidator() = default;

int TimeValidator::stringToTime(const QString &input) {
    static QRegularExpression rx(R"(^\s*(\d*)\s*([:\x{ff1a}]?)\s*(\d*)\s*([\.\x{3002}\x{ff0e}]?)\s*(\d*)\s*$)");
    auto match = rx.match(input);
    auto capDigit1 = match.captured(1);
    auto capColon = match.captured(2);
    auto capDigit2 = match.captured(3);
    auto capDot = match.captured(4);
    auto capDigit3 = match.captured(5);

    if (capDigit3.size() == 1)
        capDigit3 += "0";
    else if (capDigit3.size() > 2)
        capDigit3 = capDigit3.mid(0, 2);

    if (capColon.isEmpty() && capDot.isEmpty()) {
        return capDigit1.toInt() * 100;
    } else if (capColon.isEmpty()) {
        return capDigit1.toInt() * 100 + capDigit3.toInt();
    } else {
        return capDigit1.toInt() * 6000 + capDigit2.toInt() * 100 + capDigit3.toInt();
    }
}

QString TimeValidator::timeToString(int t) {
    if (t >= 600000)
        t = 599999;
    return QStringLiteral("%1:%2.%3").arg(t / 6000, 2, 10, QChar('0'))
                                     .arg(t % 6000 / 100, 2, 10, QChar('0'))
                                     .arg(t % 100, 2, 10, QChar('0'));
}

QValidator::State TimeValidator::validate(QString &input, int &) const {
    if (timeToString(stringToTime(input)) == input)
        return Acceptable;
    else
        return Intermediate;
}

void TimeValidator::fixup(QString &input) const {
    input = timeToString(stringToTime(input));
}
