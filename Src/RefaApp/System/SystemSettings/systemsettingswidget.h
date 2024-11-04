#ifndef SYSTEMSETTINGSWIDGET_H
#define SYSTEMSETTINGSWIDGET_H

#include <QWidget>

namespace Ui {
class SystemSettingsWidget;
}

/**
 * @brief 系统配置界面类
 */
class SystemSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SystemSettingsWidget(QWidget *parent = 0);
    ~SystemSettingsWidget();

private slots:
    void on_pushButtonBrowseWorkingDirectory_clicked();

    void on_comboBoxLocalIp_activated(const QString &arg1);

    void on_checkBoxAutoPlayback_clicked(bool checked);

private:
    /**
     * @brief 初始化界面
     */
    void initUi();

private:
    Ui::SystemSettingsWidget *ui;
};

#endif // SYSTEMSETTINGSWIDGET_H
