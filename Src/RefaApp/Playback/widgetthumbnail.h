#ifndef WIDGETTHUMBNAIL_H
#define WIDGETTHUMBNAIL_H

#include <QWidget>
#include <QList>
#include <RMAImage.h>

class ThumbnailLabel;

namespace Ui {
class WidgetThumbnail;
}

/**
*@brief 缩略图控件类
**/
class WidgetThumbnail : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetThumbnail(QWidget *parent = 0);
    ~WidgetThumbnail();

	/**
	*@brief 设置缩略图
	*@param [in] : thumb_no : uint，缩略图索引
				   img : const RMAImage&，缩略图图像
	**/
    void setThumbnail(uint thumb_no, const RMAImage &img);

	/**
	*@brief 清空缩略图
	**/
    void clearThumbnail();

	/**
	*@brief 设置缩略图数量
	*@param [in] : cnt : int，缩略图数量
	**/
	void setThumbnailCount(int cnt);

	/**
	*@brief 获取缩略图数量
	*@return : int，缩略图数量
	**/
    int getThumbnailCount() const;

	/**
	*@brief 获取缩略图最大数量
	*@return : int，缩略图最大数量
	**/
	int getThumbnailCountMax() const;

	/**
	*@brief 设置缩略图加载状态
	*@param [in] : bfinished : bool，是否加载完成，true-已完成，false-未完成
	**/
	void setLoadThumbnailStatus(bool bfinished);

public slots:
	/**
	*@brief 清空所有选择的缩略图槽函数
	**/
	void clearThumbnailSelect();

private slots:
	/**
	*@brief 选中缩略图变化响应槽函数
	*@param [in] : img : const RMAImage&，选中缩略图图像
	**/
    void onSelectThumbnailLabelChanged(const RMAImage &img);

signals:
	/**
	*@brief 选中缩略图变化信号
	*@param [in] : thumb_no : uint，缩略图索引
				   img : const RMAImage&，缩略图图像
	**/
    void selectedImageChanged(uint thumb_no, const RMAImage &img);

private:
    Ui::WidgetThumbnail *ui;
    QList<ThumbnailLabel*> thumbnail_labels_;
	uint enable_thumb_cnt_{ 0 };
};

#endif // WIDGETTHUMBNAIL_H
