#pragma once
#include <QFont>
#include <QPainter>
class UIHelper
{
public:
	enum PosType {
		LT,
		LB,
		RT,
		RB,
		CC
	};

	static QRect getBoundingRect(
		QPainter& painter,
		const QRect& win_rect,
		const QString&
	);

	static QRect getBoundingRect(
		QPainter& painter,
		const QFont& font,
		const QRect& win_rect,
		const QString& text
	);

	static QRect getBoundingRect(
		QPainter& painter,
		const QRect& win_rect,
		const QString& text,
		PosType pos_type,
		int kPadding = 0
	);

	static void drawText(
		QPainter& painter,
		const QFont& font,
		const QColor& color,
		const QRect& win_rect,
		const QString& text,
		PosType pos_type,
		int kPadding = 0
	);

	static void drawHtmlText(
		QPainter& painter,
		const QFont& font,
		const QRect& win_rect,
		const QString& html,
		PosType pos_type,
		int kPadding = 0
	);
};

