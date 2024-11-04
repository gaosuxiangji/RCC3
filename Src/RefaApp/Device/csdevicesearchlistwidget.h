#pragma once

#include <QtWidgets/QWidget>
#include "ui_csdevicesearchlistwidget.h"
#include <QPushButton>
#include <QTimer>
#include <QMainWindow>
#include <QPointer>
#include <QSharedPointer>
#include <QLabel>
#include "System/DeviceList/devicerefreshwidget.h"

class DeviceManager;

class CSDeviceSearchListWidget : public QWidget
{
    Q_OBJECT

public:
    CSDeviceSearchListWidget(QWidget *parent = Q_NULLPTR);
public:
    void startSearch();
    void refreshSearch();
signals:
    void signalUpdateDeviceRotate(const QString strIP);
    void signalSearchDeviceFinished();
	void signalItemClicked();
public slots:
	void slotItemClicked();
private:
    void init();
    void initUi();
    void bind();
private slots:
    void slotRefreshBtnClicked();
    void slotAddBtnClicked();
    void slotCountdownTimer();
    void slotRefreshSearchFinished(const int devicecount);
    void slot_tree_item_clicked(QTreeWidgetItem *item, int column);
    void slotUpdateTree(const QString& ip);

	/**
	**@ Brife	设备树被双击
	**@ Param	index 被双击的itemIndex
	**@ Note	需要执行连接操作
	*/
	void on_device_tree_widget_itemDoubleClicked(QTreeWidgetItem *item, int column);
private:
    void titleBarClicked(bool checked);
    void mousePressEvent(QMouseEvent *event);
    void updateTitleIconStyle(bool checked);
private:
    void updateRefreshLabel(const uint8_t count);
    void refresh(bool brefresh = false);
    void updateDeviceTree(bool reget = true);
    QMainWindow * getMainWindow();
    void searchFinished(int device_count);
    void setRefreshBtnEnable(bool enable);
    void setAddBtnEnable(bool enable);
protected:
	/**
	*@brief 变化事件
	*@param [in] : * event : QEvent，事件指针
	**/
	void changeEvent(QEvent *event) override;
private:
    const uint8_t m_const_count_down{ 60 };
    QLabel* m_visible_icon{};
    uint8_t m_countdown{ m_const_count_down };
    QTimer* m_countdown_timer{};
    QPointer<DeviceManager> m_device_magager_ptr{};//设备管理器
    bool m_title_bar_checked{ false };
    DeviceRefreshWidget m_device_refresh_widget{};
    QList<QSharedPointer<Device>> m_search_device_list{};
    QMap<QString, QList<QSharedPointer<Device>>> m_device_map{};
    QString m_current_device_ip{};
	QLabel* m_title_name{};
private:
    Ui::CSDeviceSearchListWidgetClass ui;
};
