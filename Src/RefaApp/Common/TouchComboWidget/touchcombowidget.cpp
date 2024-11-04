#include "touchcombowidget.h"
#include "ui_touchcombowidget.h"

TouchComboWidget::TouchComboWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TouchComboWidget)
{
    ui->setupUi(this);
	setAttribute(Qt::WA_StyledBackground);
    ui->labelTitle->installEventFilter(this);
    ui->labelValue->installEventFilter(this);
    ui->labelIcon->installEventFilter(this);
}

TouchComboWidget::~TouchComboWidget()
{
    delete ui;
}

void TouchComboWidget::setTitle(const QString &title)
{
    ui->labelTitle->setText(title);
}

QString TouchComboWidget::title() const
{
    return ui->labelTitle->text();
}

void TouchComboWidget::setCurrentText(const QString &text)
{
    ui->labelValue->setText(text);
}

QString TouchComboWidget::currentText() const
{
    return ui->labelValue->text();
}

bool TouchComboWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        if (isEnabled())
        {
            emit clicked();

            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}
