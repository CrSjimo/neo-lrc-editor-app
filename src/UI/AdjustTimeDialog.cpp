#include "AdjustTimeDialog.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>

#include <NeoLrcEditorApp/TimeSpinBox.h>

AdjustTimeDialog::AdjustTimeDialog(QWidget *parent) : QDialog(parent) {
    auto mainLayout = new QVBoxLayout;

    auto formLayout = new QFormLayout;
    m_multiplyRatioSpinBox = new QDoubleSpinBox;
    m_multiplyRatioSpinBox->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
    m_multiplyRatioSpinBox->setValue(1.0);
    formLayout->addRow(tr("&Multiply"), m_multiplyRatioSpinBox);
    auto timeLayout = new QHBoxLayout;
    m_offsetSpinBox = new TimeSpinBox;
    timeLayout->addWidget(m_offsetSpinBox);
    auto backwardCheckBox = new QCheckBox(tr("&Backward"));
    timeLayout->addWidget(backwardCheckBox);
    auto offsetLabel = new QLabel(tr("&Offset (post multiply)"));
    offsetLabel->setBuddy(m_offsetSpinBox);
    formLayout->addRow(offsetLabel, timeLayout);
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    auto buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    auto okButton = new QPushButton(tr("OK"));
    buttonLayout->addWidget(okButton);
    auto cancelButton = new QPushButton(tr("Cancel"));
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
    setWindowTitle(tr("Adjust Time"));

    connect(okButton, &QAbstractButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QAbstractButton::clicked, this, &QDialog::reject);
}

AdjustTimeDialog::~AdjustTimeDialog() = default;

double AdjustTimeDialog::ratio() const {
    return m_multiplyRatioSpinBox->value();
}

int AdjustTimeDialog::offset() const {
    return m_offsetSpinBox->value();
}
