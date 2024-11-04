#ifndef CSDLGSETOPTION_H
#define CSDLGSETOPTION_H

#include <QDialog>
#include <QButtonGroup>
namespace Ui {
class CSDlgSetOption;
}

/**
 * @brief 设置选项对话框
 */
class CSDlgSetOption : public QDialog
{
    Q_OBJECT

public:
    explicit CSDlgSetOption(QWidget *parent = 0);
    ~CSDlgSetOption();


private slots:

    void on_pushButtonBrowseWorkingDir_clicked();

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
	void InitUI();

	QButtonGroup* m_groupBoxAcquisitionSetting;

    Ui::CSDlgSetOption *ui;
};

#endif // CSDLGSETOPTION_H
