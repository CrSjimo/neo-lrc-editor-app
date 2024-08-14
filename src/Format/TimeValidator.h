#ifndef NEOLRCEDITORAPP_TIMEVALIDATOR_H
#define NEOLRCEDITORAPP_TIMEVALIDATOR_H

#include <QValidator>

class TimeValidator : public QValidator {
    Q_OBJECT
public:
    explicit TimeValidator(QObject *parent = nullptr);
    ~TimeValidator() override;

    State validate(QString &input, int &) const override;
    void fixup(QString &input) const override;

    static int stringToTime(const QString &);
    static QString timeToString(int);
};


#endif //NEOLRCEDITORAPP_TIMEVALIDATOR_H
