#ifndef VIDEOLISTWIDGET_H
#define VIDEOLISTWIDGET_H

#include <QWidget>
#include <QVariant>
#include <QModelIndexList>

namespace Ui {
class VideoListWidget;
}

class QStandardItem;

/**
 * @brief 视频列表项结构体
 */
struct VideoListItem
{
    QVariant id; // ID：录制回放时，表示视频片段ID，类型为int；本地回放时，表示视频文件路径，类型为QString
    QString name; // 名称
    QString ip; // 设备IP
};

Q_DECLARE_METATYPE(VideoListItem)

/**
 * @brief 视频列表界面类
 */
class VideoListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoListWidget(QWidget *parent = 0);
    ~VideoListWidget();

    /**
     * @brief 添加视频列表项
     * @param item 视频列表项
     */
    void addItem(const VideoListItem & item);

    /**
     * @brief 设置当前视频项
     * @param video_id 视频ID
     */
    void setCurrentItem(const QVariant & video_id);

    /**
     * @brief 获取当前视频项
     * @return 当前视频ID
     */
    QVariant getCurrentItem() const;

    /**
     * @brief 添加视频列表项
     * @param items 视频列表项
     */
    void addItems(const QList<VideoListItem> & items);

    /**
     * @brief 获取选中的视频项
     * @return 视频ID列表
     */
    QVariantList getSelectedItems() const;

signals:
    /**
     * @brief 视频删除
     * @param video_id_list 视频ID列表
     */
    void itemsRemoved(const QVariantList & video_id_list);

    /**
     * @brief 当前视频项改变
     * @param video_id 视频ID
     */
    void currentItemChanged(const QVariant & video_id);

    /**
     * @brief 选中视频项改变
     * @param video_id_list 视频ID列表
     */
    void selectedItemsChanged(const QVariantList & video_id_list);

public slots:
	/**
	*@brief 清空视频列表
	**/
    void clear();

	void onMouseClickedEnabled();

private slots:
    void on_pushButtonDelete_clicked();

    void onItemChanged(QStandardItem *item);

    void onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    /**
     * @brief 初始化界面
     */
    void initUi();

    /**
     * @brief 更新提示信息
     */
    void updateTip();

    /**
     * @brief 更新删除界面
     * @param selected_indexes 选中的索引列表
     */
    void updateDeleteUi(const QModelIndexList & selected_indexes);

    /**
     * @brief 获取选中的索引列表
     * @return 选中的索引列表
     */
    QModelIndexList selectedIndexes() const;

    /**
     * @brief 索引列表=>视频ID列表
     * @param indexes 索引列表
     * @return 视频ID列表
     */
    QVariantList toVideoItems(const QModelIndexList & indexes) const;

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::VideoListWidget *ui;
	bool benabled_mouse_clicked_{ true };//是否允许鼠标点击
};

#endif // VIDEOLISTWIDGET_H
