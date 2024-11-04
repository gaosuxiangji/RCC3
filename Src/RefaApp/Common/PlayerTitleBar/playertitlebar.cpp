#include "playertitlebar.h"
#include "ui_playertitlebar.h"

#include <QPixmap>

PlayerTitleBar::PlayerTitleBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayerTitleBar)
{
    ui->setupUi(this);
}

PlayerTitleBar::~PlayerTitleBar()
{
    delete ui;
}

void PlayerTitleBar::setTitle(const QString &text)
{
	ui->titleText->setText(text);
}

void PlayerTitleBar::setIcon(const QIcon &ico)
{
	ui->icon->setPixmap(ico.pixmap(ui->icon->sizeHint()));
}

void PlayerTitleBar::setFullScreen(bool benabled)
{
	setVisible(benabled);	
}

void PlayerTitleBar::on_restoreButton_clicked()
{
	emit fullScreen(false);
}
