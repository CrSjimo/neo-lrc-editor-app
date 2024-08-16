#ifndef NEOLRCEDITORAPP_IMPORTDIALOG_H
#define NEOLRCEDITORAPP_IMPORTDIALOG_H

#include <QDialog>

class QPlainTextEdit;

class TimeSpinBox;

class ImportDialog : public QDialog {
    Q_OBJECT
public:
    explicit ImportDialog(QWidget *parent = nullptr);
    ~ImportDialog() override;

    QString text() const;
    int initialTime() const;

private:
    QPlainTextEdit *m_editor;
    TimeSpinBox *m_initialTimeSpinBox;
};


#endif //NEOLRCEDITORAPP_IMPORTDIALOG_H
