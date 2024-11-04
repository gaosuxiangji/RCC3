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
	// ��ȡ��Դ�İ�
	QString GetOpenSourceText();

	// ��ȡBSD���
	QString getStringOfBSD();

	// ��ȡFTL���
	QString getStringOfFTL();

	// ��ȡLGPL���
	QString getStringOfLGPL();

	// ��ȡMIT���
	QString getStringOfMIT();

	// ��ȡboost���
	QString getStringOfBoost();
private:
	Ui::CSOpenSourceLicenseDlg *ui;
};
#endif //CSOPENSOURCELICENSEDLG_H_