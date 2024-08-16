#ifndef NEOLRCEDITORAPP_TIMESPINBOX_H
#define NEOLRCEDITORAPP_TIMESPINBOX_H

#include <QSpinBox>

class TimeSpinBox : public QSpinBox {
    Q_OBJECT
public:
    explicit TimeSpinBox(QWidget *parent = nullptr);
    ~TimeSpinBox() override;

protected:
    QValidator::State validate(QString &input, int &pos) const override;
    void fixup(QString &str) const override;
    QString textFromValue(int val) const override;
    int valueFromText(const QString &text) const override;
};


#endif //NEOLRCEDITORAPP_TIMESPINBOX_H
