#include "ViewMatrixTransform.h"
#include <cmath>
void MatrixTransform::setViewMatrix(const QMatrix4x4& matrix)
{
	mat_model_view_ = matrix;
}

QMatrix4x4 MatrixTransform::getViewMatrix() const
{
	return mat_model_view_;
}

void MatrixTransform::setViewMatrixToIdentity()
{
	mat_model_view_.setToIdentity();
}

void MatrixTransform::setProjectMatrix(const QMatrix4x4& matrix)
{
	mat_projection_ = matrix;
}

QMatrix4x4 MatrixTransform::getProjectMatrix() const
{
	return mat_projection_;
}

void MatrixTransform::setProjectMatrixToIdentity()
{
	mat_projection_.setToIdentity();
}

void MatrixTransform::setCanvasRect(const QRectF rc)
{
	canvas_rect_ = rc;
	img_rect_ = canvas_rect_.toRect();
}

QRectF MatrixTransform::getCanvasRect() const
{
	return canvas_rect_;
}

void MatrixTransform::setViewRect(const QRectF rc)
{
	view_rect_ = rc;
	SetupMatrices(view_rect_.width(), view_rect_.height());
}

QRectF MatrixTransform::getViewRect() const
{
	return view_rect_;
}

void MatrixTransform::rotate(const int angle)
{
	double originX = translate_offset_.x() + canvas_rect_.width() / 2;
	double originY = translate_offset_.y() + canvas_rect_.height() / 2;
	double real_angle = angle - irotate_angle_;

	double dstX, dstY;
	dstX = originX * std::cos(real_angle * RAD) + originY * std::sin(real_angle * RAD);
	dstY = originY * std::cos(real_angle * RAD) - originX * std::sin(real_angle * RAD);

	translate_offset_ = QPointF(dstX - canvas_rect_.width() / 2, dstY - canvas_rect_.height() / 2);
	irotate_angle_ = angle;

	SetupMatrices(view_rect_.width(), view_rect_.height());
}

int MatrixTransform::getRotateAngle() const
{
	return irotate_angle_;
}

void MatrixTransform::scale(const float factor, const QPointF zoom_center)
{
	fscale_ = factor;
	zoom(zoom_center);
	zoom(zoom_center);
}

float MatrixTransform::getScaleFactor() const
{
	return fscale_;
}

void MatrixTransform::fitView()
{
	int w = (std::abs)(canvas_rect_.width());
	int h = (std::abs)(canvas_rect_.height());
	if (w * h == 0)
		return;

	int angle_abs = (std::abs)(irotate_angle_);
	if (angle_abs == 90.0 || angle_abs == 270.0)
	{
		int tmp = h;
		h =  w;
		w = tmp;
	}
	float scalex = view_rect_.width() / w;
	float scaley = view_rect_.height() / h;
	fscale_ = (std::min)(scalex, scaley);

	translate2ImgCenter();
}

void MatrixTransform::originalScale(bool btrans2center/* = true*/)
{
	fscale_ = 1.0;
	if(btrans2center)
		translate2ImgCenter();
}

void MatrixTransform::translate(const double dx, const double dy)
{
	QPointF add_pt(dx, dy);
	translate(add_pt);
}

void MatrixTransform::translate(const QPointF offset)
{
	translate_offset_ += offset;
	SetupMatrices(view_rect_.width(), view_rect_.height());
}

double MatrixTransform::getTranslateX() const
{
	return translate_offset_.x();
}

double MatrixTransform::getTranslateY() const
{
	return translate_offset_.y();
}

bool MatrixTransform::ptInImg(const QPointF& pt) const
{
	return img_rect_.contains(pt.toPoint());
}

QPointF MatrixTransform::getTranslate() const
{
	return translate_offset_;
}

QPointF MatrixTransform::ClientToImg(const QPointF& pt)
{
	return WorldToImg(ClientToWorld(pt));
}

QPointF MatrixTransform::ImgToClient(const QPointF& pt)
{
	return WorldToClient(ImgToWorld(pt));
}

QPointF MatrixTransform::ClientToWorld(const QPointF& pt)
{
	QVector3D client3d(pt.x(), view_rect_.height() - pt.y() - 1, 0);
	QVector3D worldPos = client3d.unproject(mat_model_view_, mat_projection_, view_rect_.toRect());
	return QPointF(worldPos.x(), worldPos.y());
}

QPointF MatrixTransform::WorldToClient(const QPointF& pt)
{
	QVector3D worldPos(pt.x(), pt.y(), 0);
	QVector3D ClientPos = worldPos.project(mat_model_view_, mat_projection_, view_rect_.toRect());
	return QPointF(ClientPos.x(), view_rect_.height() - ClientPos.y() - 1);
}

QPointF MatrixTransform::ImgToWorld(const QPointF& pt)
{
	QPointF p = pt;
	p.setY(std::abs(canvas_rect_.height()) - p.y() - 1);
	return p;
}

QPointF MatrixTransform::WorldToImg(const QPointF& pt)
{
	QPointF p = pt;
	p.setY(std::abs(canvas_rect_.height()) - p.y() - 1);
	return p;
}

void MatrixTransform::SetupMatrices(int width, int height)
{
	mat_model_view_.setToIdentity();
	if (irotate_angle_ != 0) {
		mat_model_view_.rotate(irotate_angle_, 0, 0, 1);//逆时针旋转，当irotate_angle_<0时则顺时针旋转
	}

	mat_model_view_.scale(fscale_);
	mat_model_view_.translate(translate_offset_.x(), translate_offset_.y());

	mat_projection_.setToIdentity();
	mat_projection_.ortho(0, width, 0, height, 1.0f, -1.0f);
}

void MatrixTransform::zoom(const QPointF center)
{
	QPoint ImgPoint = ClientToImg(center).toPoint();
	QPoint pt(center.x(), view_rect_.height() - center.y());
#if 1
	ImgPoint.setX(std::abs(canvas_rect_.width()) - ImgPoint.x());
	translate_offset_ = pt / fscale_ - ImgPoint;
#else
	qDebug() << "ImgPoint : " << ImgPoint;
	QPointF tmp_pt = ClientToWorld(center);
	QPointF pt_img2world_new = tmp_pt;// QPointF(round(tmp_pt.x()), round(tmp_pt.y()));
	translate_offset_ = pt / fscale_ - pt_img2world_new;//平移后误差较大

	qDebug() << "pt_img2world_new : " << pt_img2world_new;
	qDebug() << "translate_offset : " << translate_offset_;
#endif
	SetupMatrices(view_rect_.width(), view_rect_.height());
}

void MatrixTransform::translate2ImgCenter()
{
	translate_offset_ = (view_rect_.center() / fscale_ - QPoint((std::abs)(canvas_rect_.width()) / 2, (std::abs)(canvas_rect_.height()) / 2));
	float angle = irotate_angle_;
	irotate_angle_ = 0;
	rotate(angle);
}
