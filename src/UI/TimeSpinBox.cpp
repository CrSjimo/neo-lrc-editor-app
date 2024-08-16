#include "TimeSpinBox.h"

#include <NeoLrcEditorApp/TimeValidator.h>

TimeSpinBox::TimeSpinBox(QWidget *parent) : QSpinBox(parent) {
    setMaximum(599999);
}

TimeSpinBox::~TimeSpinBox() = default;

static TimeValidator m_timeValidator;

QValidator::State TimeSpinBox::validate(QString &input, int &pos) const {
    return m_timeValidator.validate(input, pos);
}

void TimeSpinBox::fixup(QString &str) const {
    m_timeValidator.fixup(str);
}

QString TimeSpinBox::textFromValue(int val) const {
    return TimeValidator::timeToString(val);
}

int TimeSpinBox::valueFromText(const QString &text) const {
    return TimeValidator::stringToTime(text);
}
