#ifndef CSDLGOPENEXP_H
#define CSDLGOPENEXP_H

#include <QDialog>
#include <QPointer>
#include <QStandardItemModel>
#include "csexperimentutil.h"

namespace Ui {
class CSDlgOpenExp;
}

/**
 * @brief 打开项目对话框
 */
class CSDlgOpenExp : public QDialog
{
    Q_OBJECT

public:
    explicit CSDlgOpenExp(QWidget *parent = 0);
    ~CSDlgOpenExp();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();


private:
	void InitUI();

	//填充表格数据
	void FillList();
private:
    Ui::CSDlgOpenExp *ui;
	QPointer<QStandardItemModel> m_list_model;//列表数据
};

#endif // CSDLGOPENEXP_H
