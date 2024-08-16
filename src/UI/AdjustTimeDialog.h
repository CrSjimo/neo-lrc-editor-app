#ifndef NEOLRCEDITORAPP_ADJUSTTIMEDIALOG_H
#define NEOLRCEDITORAPP_ADJUSTTIMEDIALOG_H

#include <QDialog>

class QDoubleSpinBox;

class TimeSpinBox;

class AdjustTimeDialog : public QDialog {
    Q_OBJECT
public:
    explicit AdjustTimeDialog(QWidget *parent = nullptr);
    ~AdjustTimeDialog() override;

    double ratio() const;
    int offset() const;

private:
    QDoubleSpinBox *m_multiplyRatioSpinBox;
    TimeSpinBox *m_offsetSpinBox;
};


#endif //NEOLRCEDITORAPP_ADJUSTTIMEDIALOG_H
