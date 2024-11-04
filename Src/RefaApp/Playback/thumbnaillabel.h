#ifndef THUMBNAILLABEL_H
#define THUMBNAILLABEL_H

#include <QLabel>
#include <RMAImage.h>
#include <QPixmap>

class QMouseEvent;
class QPaintEvent;
class QResizeEvent;

/**
*@brief 缩略图Label类
**/
class ThumbnailLabel : public QLabel
{
	Q_OBJECT

public:
    ThumbnailLabel(QWidget *parent = Q_NULLPTR);
    ThumbnailLabel(const RMAImage &img, QWidget *parent = Q_NULLPTR);

	/**
	*@brief 设置缩略图
	*@param [in] : img : const RMAImage&，缩略图图像
	**/
    void setThumbnailImage(const RMAImage& img);

	/**
	*@brief 获取缩略图
	*@return : RMAImage，缩略图图像
	**/
    RMAImage getThumbnailImage() const;

	/**
	*@brief 清空缩略图
	**/
    void clearThumbnailImage();

	/**
	*@brief 设置选中状态
	*@param [in] : bselected : bool，是否选中，true-选中，false-未选中
	**/
    void setSelected(bool bselected);

	/**
	*@brief 获取缩略图是否选中
	*@return : bool : 是否选中，true-选中，false-未选中
	**/
    bool isSelected() const;

	/**
	*@brief 设置缩略图lable编号
	*@param [in] : no : uint，编号
	**/
    void setLabelNo(uint no);
	
	/**
	*@brief 获取缩略图lable编号
	*@return : uint : 编号
	**/
    uint getLabelNo() const;

	/**
	*@brief 设置当前label是否被使用
	*@param [in] : bselected : bused，true-使用，false-不使用
	*@note 当视频帧数较少，不能将所有缩略图都用到时设置为false
	**/
	void setUsed(bool bused);

	/**
	*@brief 获取当前label是否被使用
	*@return : bool : true-使用，false-不使用
	**/
	bool isUsed() const;

	/**
	*@brief 设置缩略图加载状态
	*@param [in] : bfinished : bool，是否加载完成，true-已完成，false-未完成
	**/
	void setLoadStatus(bool bfinished);

	/**
	*@brief 获取缩略图加载状态
	*@return : bool : 是否加载完成，true-已完成，false-未完成
	**/
	bool isLoaded() const;

signals:
	/**
	*@brief 选中当前label信号
	*@param [in] : img : const RMAImage&，缩略图图像
	**/
    void selected(const RMAImage&);

protected:
	/**
	*@brief 鼠标释放事件
	*@param [in] : event : QMouseEvent*，鼠标事件指针
	**/
    void mouseReleaseEvent(QMouseEvent* event) override;

	/**
	*@brief 绘制事件
	*@param [in] : event : QPaintEvent*，绘制事件指针
	**/
    void paintEvent(QPaintEvent *event) override;

	/**
	*@brief 调整大小事件
	*@param [in] : event : QResizeEvent*，事件指针
	**/
    void resizeEvent(QResizeEvent *event) override;

private:
	/**
	*@brief 制作绘制图像
	**/
	void MakePaintBuffer();

private:
    RMAImage thumb_image_;
    QPixmap paint_buffer_;
    bool bselected_{ false };
    uint label_number_{ 0 };
	bool bused_{ true };//是否使用
	bool bload_finished_{ false };//是否加载完成

    static const int kFrameNoTextPadding{ 5 };
    static const int kBorder{ 3 };
	const QColor kBorderColor{ Qt::red };
};

#endif // THUMBNAILLABEL_H
