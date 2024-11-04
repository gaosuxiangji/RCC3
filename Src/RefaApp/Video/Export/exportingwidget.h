#ifndef EXPORTINGWIDGET_H
#define EXPORTINGWIDGET_H

#include <QDialog>
#include <QList>
#include <memory>
#include <QMap>

#include "Video/VideoItem/videoitem.h"

class VideoExportController;
class ISPUtil;
struct VideoExportParams;
class PlayerController;

namespace Ui {
class ExportingWidget;
}

class ExportingWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ExportingWidget(QWidget *parent = 0);
    ~ExportingWidget();

    /**
     * @brief 开始导出
     * @param video_items 视频项列表
     */
    void startExport(const QList<VideoItem> & video_items);

public slots:

    /**
     * @brief 停止导出
     */
    void stopExport();

signals:
    /**
     * @brief 结束信号
     * @param result 结果码
     */
    void finished(quint64 result);

protected:
    void closeEvent(QCloseEvent *) override;

private slots:
    void onExportProgressChanged(float progress);
    void onExportFinished(bool res, float progress);

private:
    /**
     * @brief 初始化UI
     */
    void initUi();

    /**
     * @brief 导出视频
     * @param video_item 视频项
     */
    void exportVideo(const VideoItem & video_item);

private:
    Ui::ExportingWidget *ui;

    std::shared_ptr<VideoExportController> export_ctrl_ptr_;
    QList<VideoItem> video_items_;
    QMap<int, bool> result_map_;
    int current_video_index_{ 0 };
	//std::shared_ptr<VideoExportParams> export_param_ptr_;
	bool bexport_finished_{ false };//是否完成导出，导出完成和导出出错时设为true
};

#endif // EXPORTINGWIDGET_H
