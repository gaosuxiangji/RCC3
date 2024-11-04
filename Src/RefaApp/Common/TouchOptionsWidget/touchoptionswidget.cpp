#include "touchoptionswidget.h"
#include "ui_touchoptionswidget.h"

#include <QButtonGroup>
#include <QSpinBox>

#include "Common/TouchOptionWidget/touchoptionwidget.h"

TouchOptionsWidget::TouchOptionsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TouchOptionsWidget)
{
    ui->setupUi(this);

	ui->widgetCustomOption->setText(tr("Custom"));
    ui->widgetCustomOption->setVisible(false);
	ui->labelTip->setVisible(false);
    ui->frameCustomWidget->setVisible(false);

    button_group_options_ = new QButtonGroup(this);
    button_group_options_->addButton(ui->widgetCustomOption->button(), INT_MAX);
    connect(button_group_options_, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &TouchOptionsWidget::onOptionClicked);
}

TouchOptionsWidget::~TouchOptionsWidget()
{
    delete ui;
}

void TouchOptionsWidget::addOption(const QVariant &value, const QString &option)
{
    TouchOptionWidget *option_widget = new TouchOptionWidget();
	option_widget->setText(option);
	option_widget->setValue(value);
    ui->verticalLayoutOptions->addWidget(option_widget);
    button_group_options_->addButton(option_widget->button(), ui->verticalLayoutOptions->count() - 1);
}

void TouchOptionsWidget::setCurrentOption(const QVariant &value)
{
    int index = -1;
    for (int i = 0; i < ui->verticalLayoutOptions->count(); i++)
    {
        TouchOptionWidget *option = qobject_cast<TouchOptionWidget*>(ui->verticalLayoutOptions->itemAt(i)->widget());
        if (option && option->value() == value)
        {
            index = i;
            break;
        }
    }

    if (index >= 0)
    {
        button_group_options_->button(index)->setChecked(true);
    }
    else
    {
        if (custom_widget_)
        {
            button_group_options_->button(INT_MAX)->setChecked(true);

            QSpinBox* spin_box = qobject_cast<QSpinBox*>(custom_widget_);
            if (spin_box)
            {
                spin_box->setValue(value.toInt());
            }

			ui->widgetCustomOption->setValue(value);
            ui->widgetCustomOption->setVisible(true);
			ui->frameCustomWidget->setVisible(true);
        }
    }
}

QString TouchOptionsWidget::getCurrentOptionText() const
{
	QAbstractButton *checked_button = button_group_options_->checkedButton();
	if (checked_button)
	{
		if (button_group_options_->id(checked_button) == INT_MAX)
		{
			return ui->widgetCustomOption->value().toString();
		}
		return checked_button->text();
	}
	
	return "";
}

QVariant TouchOptionsWidget::getCurrentOptionValue() const
{
	QAbstractButton *checked_button = button_group_options_->checkedButton();
	if (checked_button)
	{
		int id = button_group_options_->id(checked_button);
		if (id == INT_MAX)
		{
			return ui->widgetCustomOption->value();
		}
		else
		{
			return getCurrentOptionWidget(id)->value();
		}
	}

	return QVariant();
}

void TouchOptionsWidget::clearOptions()
{
    QLayoutItem *child = nullptr;
    while ((child = ui->verticalLayoutOptions->takeAt(0)) != nullptr)
    {
		
        TouchOptionWidget *option = qobject_cast<TouchOptionWidget*>(child->widget());
		if (option)
		{
			button_group_options_->removeButton(option->button());

			delete option;
			option = nullptr;
		}

        delete child;
        child = nullptr;
    }
}

void TouchOptionsWidget::setCustomWidget(QWidget *custom_widget, QString tip)
{
    if (custom_widget)
    {
        ui->horizontalLayoutCustom->addWidget(custom_widget);
        ui->widgetCustomOption->setVisible(true);
		ui->labelTip->setVisible(true);

        QSpinBox *spin_box = qobject_cast<QSpinBox*>(custom_widget);
        if (spin_box)
        {
            connect(spin_box, QOverload<int>::of(&QSpinBox::valueChanged), this, &TouchOptionsWidget::onSpinBoxValueChanged);
        }
    }
    else
    {
        if (custom_widget_)
        {
            QSpinBox *spin_box = qobject_cast<QSpinBox*>(custom_widget_);
            if (spin_box)
            {
                disconnect(spin_box, nullptr, nullptr, nullptr);
            }
        }
        ui->widgetCustomOption->setVisible(false);
        ui->frameCustomWidget->setVisible(false);
		ui->labelTip->setVisible(false);
    }

    custom_widget_ = custom_widget;
}

void TouchOptionsWidget::setCustomTip(QString tip)
{
	ui->labelTip->setText(tip);
}

void TouchOptionsWidget::on_pushButtonBack_clicked()
{
    emit backButtonClicked();
}

void TouchOptionsWidget::onOptionClicked(int id)
{
    button_group_options_->button(id)->setChecked(true);

    TouchOptionWidget* option = getCurrentOptionWidget(id);
    if (option)
    {
		ui->frameCustomWidget->setVisible(false);
        emit currentOptionChanged(option->value());
		emit backButtonClicked();//点击选项自动返回
    }
	else
	{
		ui->frameCustomWidget->setVisible(true);
		QSpinBox *spin_box = qobject_cast<QSpinBox*>(custom_widget_);
		if (spin_box)
		{
			spin_box->setFocus();
		}
		emit currentOptionChanged(ui->widgetCustomOption->value());
	}
}

void TouchOptionsWidget::onSpinBoxValueChanged(int value)
{
	ui->widgetCustomOption->setValue(value);
	emit currentOptionChanged(ui->widgetCustomOption->value());
}

TouchOptionWidget *TouchOptionsWidget::getCurrentOptionWidget(int id) const
{
    TouchOptionWidget* current_option = nullptr;
    for (int i = 0; i < ui->verticalLayoutOptions->count(); i++)
    {
        TouchOptionWidget *option = qobject_cast<TouchOptionWidget*>(ui->verticalLayoutOptions->itemAt(i)->widget());
        if (option && button_group_options_->id(option->button()) == id)
        {
            current_option = option;
            break;
        }
    }

    return current_option;
}
