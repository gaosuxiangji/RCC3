#ifndef CSDLGNEWEXP_H
#define CSDLGNEWEXP_H

#include <QDialog>
#include "csexperimentutil.h"
namespace Ui {
class CSDlgNewExp;
}

/**
 * @brief 新建项目对话框
 */
class CSDlgNewExp : public QDialog
{
    Q_OBJECT

public:
    explicit CSDlgNewExp(QWidget *parent = 0);
    ~CSDlgNewExp();

	/**
	* @brief 获取对话框中的实验信息
	* @return CSExperiment 实验信息
	*/
	CSExperiment getExperimentInfo() { return m_exp; }

	
private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_textEditDescription_textChanged();

private:
	void InitUI();
private:
    Ui::CSDlgNewExp *ui;
	CSExperiment m_exp;//实验信息

	const int m_maxNameCount = 64;
	const int m_maxWordCount = 512;
};

#endif // CSDLGNEWEXP_H
