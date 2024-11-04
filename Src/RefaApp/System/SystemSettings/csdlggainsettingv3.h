#ifndef CSDLGGAINSETTINGV3_H
#define CSDLGGAINSETTINGV3_H

#include <QDialog>
#include <QSharedPointer>

namespace Ui {
class CSDlgGainSettingV3;
}
class Device;

class CSDlgGainSettingV3 : public QDialog
{
    Q_OBJECT

public:
	explicit CSDlgGainSettingV3(QSharedPointer<Device> device_ptr, QWidget *parent = 0);
    ~CSDlgGainSettingV3();

	private slots:
	void on_comboBox_analogGain_IndexChanged(int index);

	void on_pushButton_default_clicked();

	void on_pushButton_ok_clicked();

	void slot_digital_gain_valueChanged(int index);

private:
	void InitUI();

private:
	Ui::CSDlgGainSettingV3 *ui;

	//double constDIGITALGAIN[64] = { 1,	1.0625,	1.125,	1.1875,	1.25,	1.3125,	1.375,	1.4375,	
	//	1.5,	1.5625,	1.625,	1.6875,	1.75,	1.8125,	1.875,	1.9375,	
	//	2.0,	2.125,	2.25,	2.375,	2.5,	2.625,	2.75,	2.875,	
	//	3.0,	3.125,	3.25,	3.375,	3.5,	3.625,	3.75,	3.875,	
	//	4.0,	4.25,	4.5,	4.75,	5.0,	5.25,	5.5,	5.75,	
	//	6.0,	6.25,	6.5,	6.75,	7.0,	7.25,	7.5,	7.75,	
	//	8.0,	8.5,	9.0,	9.5,	10.0,	10.5,	11.0,	11.5,
	//	12.0,	12.5,	13,		13.5,	14.0,	14.5,	15.0,	15.5
	//};

	double constDIGITALGAIN[33] = {
		1,		1.0625,	1.125,	1.1875,	1.25,	1.3125,	1.375,	1.4375,
		1.5,	1.5625,	1.625,	1.6875,	1.75,	1.8125,	1.875,	1.9375,
		2.0,	2.125,	2.25,	2.375,	2.5,	2.625,	2.75,	2.875,
		3.0,	3.125,	3.25,	3.375,	3.5,	3.625,	3.75,	3.875,
		4.0
	};
	int constDigitalMax = 33;

	QString constGainText[4] = { "x1", "x1.3", "x1.9", "x2.8" };
	QSharedPointer<Device> m_device_ptr;

	int m_current_index;//当前模拟增益选项
};

#endif // CSDLGGAINSETTINGV3_H
