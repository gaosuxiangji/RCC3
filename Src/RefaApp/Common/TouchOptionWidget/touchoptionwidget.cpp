#include "touchoptionwidget.h"
#include "ui_touchoptionwidget.h"

TouchOptionWidget::TouchOptionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TouchOptionWidget)
{
    ui->setupUi(this);
}

TouchOptionWidget::~TouchOptionWidget()
{
    delete ui;
}

void TouchOptionWidget::setText(const QString &text)
{
    ui->radioButton->setText(text);
}

void TouchOptionWidget::setValue(const QVariant &value)
{
    value_ = value;
}

QVariant TouchOptionWidget::value() const
{
    return value_;
}

QAbstractButton *TouchOptionWidget::button() const
{
    return ui->radioButton;
}
