#ifndef NEOLRCEDITORAPP_QUANTIZEDIALOG_H
#define NEOLRCEDITORAPP_QUANTIZEDIALOG_H

#include <QDialog>

class QComboBox;
class QDoubleSpinBox;

class QuantizeDialog : public QDialog {
    Q_OBJECT
public:
    explicit QuantizeDialog(QWidget *parent = nullptr);
    ~QuantizeDialog() override;

    double div() const;

private:
    QDoubleSpinBox *m_tempoSpinBox;
    QComboBox *m_alignComboBox;
};


#endif //NEOLRCEDITORAPP_QUANTIZEDIALOG_H
