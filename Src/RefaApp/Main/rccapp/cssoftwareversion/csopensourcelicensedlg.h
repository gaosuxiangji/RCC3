#ifndef CSOPENSOURCELICENSEDLG_H_
#define CSOPENSOURCELICENSEDLG_H_

#include <QDialog>
namespace Ui { class CSOpenSourceLicenseDlg; };

class CSOpenSourceLicenseDlg : public QDialog
{
	Q_OBJECT

public:
	CSOpenSourceLicenseDlg(QWidget *parent = Q_NULLPTR);
	~CSOpenSourceLicenseDlg();

public slots:
	void soltAboutQt();
private:
	void Init();
	// 获取开源文案
	QString GetOpenSourceText();

	// 获取BSD相关
	QString getStringOfBSD();

	// 获取FTL相关
	QString getStringOfFTL();

	// 获取LGPL相关
	QString getStringOfLGPL();

	// 获取MIT相关
	QString getStringOfMIT();

	// 获取boost相关
	QString getStringOfBoost();
private:
	Ui::CSOpenSourceLicenseDlg *ui;
};
#endif //CSOPENSOURCELICENSEDLG_H_