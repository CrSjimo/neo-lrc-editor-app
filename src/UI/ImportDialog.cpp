#include "ImportDialog.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QMessageBox>

#include <NeoLrcEditorApp/TimeSpinBox.h>

ImportDialog::ImportDialog(QWidget *parent) : QDialog(parent) {
    auto mainLayout = new QVBoxLayout;
    auto formLayout = new QFormLayout;
    auto browseLayout = new QHBoxLayout;
    auto fileNameLineEdit = new QLineEdit;
    fileNameLineEdit->setReadOnly(true);
    browseLayout->addWidget(fileNameLineEdit);
    auto browseButton = new QPushButton(tr("&Browse..."));
    browseLayout->addWidget(browseButton);
    formLayout->addRow(tr("File"), browseLayout);
    m_initialTimeSpinBox = new TimeSpinBox;
    m_initialTimeSpinBox->setValue(500000);
    formLayout->addRow(tr("&Initial time"), m_initialTimeSpinBox);
    mainLayout->addLayout(formLayout);
    m_editor = new QPlainTextEdit;
    formLayout->addRow(m_editor);
    auto buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    auto okButton = new QPushButton(tr("OK"));
    buttonLayout->addWidget(okButton);
    auto cancelButton = new QPushButton(tr("Cancel"));
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
    setWindowTitle(tr("Import"));

    connect(okButton, &QAbstractButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QAbstractButton::clicked, this, &QDialog::reject);

    connect(browseButton, &QAbstractButton::clicked, this, [=] {
        auto fileName = QFileDialog::getOpenFileName(this, {}, {}, tr("Plain Text (*.txt)"));
        if (fileName.isEmpty())
            return;
        QFile f(fileName);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, {}, tr("Cannot open file %1").arg(fileName));
            return;
        }
        fileNameLineEdit->setText(fileName);
        m_editor->setPlainText(QString::fromUtf8(f.readAll()));
    });
}

ImportDialog::~ImportDialog() = default;

QString ImportDialog::text() const {
    return m_editor->toPlainText();
}

int ImportDialog::initialTime() const {
    return m_initialTimeSpinBox->value();
}
