#ifndef CSDEVICETEMPERATUREPANEL_H
#define CSDEVICETEMPERATUREPANEL_H

#include <QWidget>
#include <QMap>
#include <QLineEdit>
namespace Ui {
class CSDeviceTemperaturePanel;
}

class CSDeviceTemperaturePanel : public QWidget
{
    Q_OBJECT

public:
    explicit CSDeviceTemperaturePanel(QWidget *parent = 0);
    ~CSDeviceTemperaturePanel();

	//更新界面的温度信息
	void setTemperatureInfo(QString temperature_info);

protected:
	/**
	*@brief 变化事件
	*@param [in] : * event : QEvent，事件指针
	**/
	void changeEvent(QEvent *event) override;
private:
	QMap<QString, QLineEdit*> map_type2Widget_;
	const QString init_str_{ "—— —— ——" };
	QMap<QString, std::pair<int, int>> map_range_;

	Ui::CSDeviceTemperaturePanel *ui;
};

#endif // CSDEVICETEMPERATUREPANEL_H
