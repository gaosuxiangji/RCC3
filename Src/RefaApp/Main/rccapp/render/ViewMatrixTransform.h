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

	void scale(const float factor, const QPointF zoom_center);//x��y�������ű���һ��
	float getScaleFactor() const;
	void fitView();
	void originalScale(bool btrans2center = true);

	void translate(const double dx, const double dy);
	void translate(const QPointF offset);
	QPointF getTranslate() const;
	double getTranslateX() const;
	double getTranslateY() const;

	bool ptInImg(const QPointF& pt) const;

	QPointF ClientToImg(const QPointF& pt);//��������ӳ�䵽ͼ������
	QPointF ImgToClient(const QPointF& pt); //��ͼ������ת��Ϊ��������
	QPointF ClientToWorld(const QPointF& pt);	//���㴰�ڵĵ��Ӧ����������, ���������X��������, Y��������
	QPointF WorldToClient(const QPointF& pt); //����������ת��Ϊ��������
	QPointF ImgToWorld(const QPointF& pt); //��ͼ������ת��Ϊ��������
	QPointF WorldToImg(const QPointF& pt); //����������ת��Ϊͼ������

private:
	void SetupMatrices(int width, int height);//��������
	void zoom(const QPointF center);
	void translate2ImgCenter();

private:
	QMatrix4x4 mat_model_view_, mat_projection_;
	QRectF canvas_rect_;
	QRectF view_rect_;

	float fscale_ = 1.0;//���ű���
	int irotate_angle_ = 0;//��ת�Ƕ�
	QPointF translate_offset_;//ƽ��

	QRect img_rect_;	// ���bottom��left�ж� canvas_rect_����QPointF������img_rect_����QPoint�Ľ����ͬ
};

