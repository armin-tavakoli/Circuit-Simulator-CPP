// TransientAnalysisDialog.cpp
#include "TransientAnalysisDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>

TransientAnalysisDialog::TransientAnalysisDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Edit Simulation Command"));

    // ایجاد ویجت‌های ورودی
    stopTimeEdit = new QLineEdit("1s", this);
    startTimeEdit = new QLineEdit("0s", this);
    maxTimestepEdit = new QLineEdit("1ms", this);

    // ایجاد دکمه‌های OK و Cancel
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // چیدمان فرم
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("Stop time:"), stopTimeEdit);
    formLayout->addRow(tr("Time to start saving data:"), startTimeEdit);
    formLayout->addRow(tr("Maximum Timestep:"), maxTimestepEdit);

    // چیدمان اصلی دیالوگ
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
}

// پیاده‌سازی توابع getter
double TransientAnalysisDialog::stopTime() const {
    // در اینجا باید واحدها (s, ms, us) را نیز پردازش کنیم.
    // فعلا برای سادگی، فقط عدد را می‌خوانیم.
    return stopTimeEdit->text().toDouble();
}

double TransientAnalysisDialog::startTime() const {
    return startTimeEdit->text().toDouble();
}

double TransientAnalysisDialog::maxTimestep() const {
    return maxTimestepEdit->text().toDouble();
}