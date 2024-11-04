#include "uiutils.h"

#include <QMessageBox>

#include "Common/UIExplorer/uiexplorer.h"

void UIUtils::showInfoMsgBox(QWidget *parent, const QString &text)
{
    QMessageBox::information(parent, UIExplorer::instance().getProductName(), text);
}

void UIUtils::showWarnMsgBox(QWidget *parent, const QString &text)
{
    QMessageBox::warning(parent, UIExplorer::instance().getProductName(), text);
}

void UIUtils::showErrorMsgBox(QWidget *parent, const QString &text)
{
    QMessageBox::critical(parent, UIExplorer::instance().getProductName(), text);
}

bool UIUtils::showQuestionMsgBox(QWidget *parent, const QString &text, int button_type)
{
    if (button_type == 0)
    {
        return QMessageBox::question(parent, UIExplorer::instance().getProductName(), text, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes;
    }
    else
    {
        return QMessageBox::question(parent, UIExplorer::instance().getProductName(), text, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok) == QMessageBox::Ok;
    }
}
