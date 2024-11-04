#pragma once
#include <QMatrix4x4> 
#include <QPoint>
#include <QRect>

#ifndef PI
#define PI 3.141592653
#endif

#ifndef RAD
#define RAD (PI/180.0f)
#endif

class MatrixTransform
{
public:
	void setViewMatrix(const QMatrix4x4& matrix);
	QMatrix4x4 getViewMatrix() const;
	void setViewMatrixToIdentity();

	void setProjectMatrix(const QMatrix4x4& matrix);
	QMatrix4x4 getProjectMatrix() const;
	void setProjectMatrixToIdentity();

	void setCanvasRect(const QRectF rc);
	QRectF getCanvasRect() const;

	void setViewRect(const QRectF rc);
	QRectF getViewRect() const;

	void rotate(const int angle);
	int getRotateAngle() const;

	void scale(const float factor, const QPointF zoom_center);//x、y方向缩放倍数一致
	float getScaleFactor() const;
	void fitView();
	void originalScale(bool btrans2center = true);

	void translate(const double dx, const double dy);
	void translate(const QPointF offset);
	QPointF getTranslate() const;
	double getTranslateX() const;
	double getTranslateY() const;

	bool ptInImg(const QPointF& pt) const;

	QPointF ClientToImg(const QPointF& pt);//窗口坐标映射到图像坐标
	QPointF ImgToClient(const QPointF& pt); //把图像坐标转换为窗口坐标
	QPointF ClientToWorld(const QPointF& pt);	//计算窗口的点对应的世界坐标, 世界坐标的X方向向右, Y方向向上
	QPointF WorldToClient(const QPointF& pt); //把世界坐标转换为窗口坐标
	QPointF ImgToWorld(const QPointF& pt); //把图像坐标转换为世界坐标
	QPointF WorldToImg(const QPointF& pt); //把世界坐标转换为图像坐标

private:
	void SetupMatrices(int width, int height);//矩阵设置
	void zoom(const QPointF center);
	void translate2ImgCenter();

private:
	QMatrix4x4 mat_model_view_, mat_projection_;
	QRectF canvas_rect_;
	QRectF view_rect_;

	float fscale_ = 1.0;//缩放比例
	int irotate_angle_ = 0;//旋转角度
	QPointF translate_offset_;//平移

	QRect img_rect_;	// 针对bottom和left判断 canvas_rect_包含QPointF计算与img_rect_包含QPoint的结果不同
};

