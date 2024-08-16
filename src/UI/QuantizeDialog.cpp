#include "QuantizeDialog.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>

QuantizeDialog::QuantizeDialog(QWidget *parent) {
    auto mainLayout = new QVBoxLayout;
    auto formLayout = new QFormLayout;
    m_tempoSpinBox = new QDoubleSpinBox;
    m_tempoSpinBox->setRange(1.0, std::numeric_limits<double>::max());
    m_tempoSpinBox->setValue(120.0);
    formLayout->addRow(tr("&Tempo"), m_tempoSpinBox);
    m_alignComboBox = new QComboBox;
    m_alignComboBox->addItem(tr("Whole note"), 0.25);
    m_alignComboBox->addItem(tr("Half note"), 0.5);
    m_alignComboBox->addItem(tr("Half note triplet"), 0.75);
    m_alignComboBox->addItem(tr("Quarter note"), 1);
    m_alignComboBox->addItem(tr("Quarter note triplet"), 1.5);
    m_alignComboBox->addItem(tr("Eighth note"), 2);
    m_alignComboBox->addItem(tr("Eighth note triplet"), 3);
    m_alignComboBox->addItem(tr("1/16 note"), 4);
    m_alignComboBox->addItem(tr("1/16 note triplet"), 6);
    m_alignComboBox->addItem(tr("1/32 note"), 8);
    m_alignComboBox->addItem(tr("1/32 note triplet"), 12);
    formLayout->addRow(tr("&Alignment"), m_alignComboBox);
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
    setWindowTitle(tr("Quantize"));

    connect(okButton, &QAbstractButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QAbstractButton::clicked, this, &QDialog::reject);
}

QuantizeDialog::~QuantizeDialog() = default;

double QuantizeDialog::div() const {
    return 6000.0 / m_tempoSpinBox->value() / m_alignComboBox->currentData().toDouble();
}
