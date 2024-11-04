#include "touchcheckwidget.h"
#include "ui_touchcheckwidget.h"

TouchCheckWidget::TouchCheckWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TouchCheckWidget)
{
    ui->setupUi(this);
	setAttribute(Qt::WA_StyledBackground);
    connect(ui->checkBox, &QCheckBox::clicked, this, &TouchCheckWidget::clicked);
}

TouchCheckWidget::~TouchCheckWidget()
{
    delete ui;
}

void TouchCheckWidget::setText(const QString &text)
{
    ui->checkBox->setText(text);
}

QString TouchCheckWidget::text() const
{
    return ui->checkBox->text();
}

void TouchCheckWidget::setChecked(bool checked)
{
    ui->checkBox->setChecked(checked);
}

bool TouchCheckWidget::isChecked() const
{
    return ui->checkBox->isChecked();
}
