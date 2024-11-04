#ifndef CSDLGHEALTHMANAGERSETTING_H
#define CSDLGHEALTHMANAGERSETTING_H

#include <QDialog>
class QValidator;
namespace Ui {
class CSDlgHealthManagerSetting;
}

class CSDlgHealthManagerSetting : public QDialog
{
    Q_OBJECT

public:
    explicit CSDlgHealthManagerSetting(QWidget *parent = 0);
    ~CSDlgHealthManagerSetting();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
	void InitUI();

    Ui::CSDlgHealthManagerSetting *ui;

	QValidator * m_validator_ip;
	QValidator * m_validator_port;
	QValidator * m_validator_period;

};

#endif // CSDLGHEALTHMANAGERSETTING_H
