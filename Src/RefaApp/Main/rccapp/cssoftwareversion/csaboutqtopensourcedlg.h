#ifndef CSABOUTQTOPENSOURCEDLG_H_
#define CSABOUTQTOPENSOURCEDLG_H_

#include <QDialog>
namespace Ui { class CSAboutQtOpenSourceDlg; };

class CSAboutQtOpenSourceDlg : public QDialog
{
	Q_OBJECT

public:
	CSAboutQtOpenSourceDlg(QWidget *parent = Q_NULLPTR);
	~CSAboutQtOpenSourceDlg();
private:
	void Init();

	QString GetQtOpenSourceText();
private:
	Ui::CSAboutQtOpenSourceDlg *ui;
};
#endif //CSABOUTQTOPENSOURCEDLG_H_
