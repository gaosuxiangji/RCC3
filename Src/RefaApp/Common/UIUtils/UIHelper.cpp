#include "UIHelper.h"
#include <QFontMetrics>
#include <QTextDocument>
QRect UIHelper::getBoundingRect(
	QPainter& painter, 
	const QRect& win_rect,
	const QString& text)
{
	QFontMetrics metrics = painter.fontMetrics();
	return metrics.boundingRect(win_rect, Qt::TextWordWrap, text);
}



QRect UIHelper::getBoundingRect(QPainter& painter, const QFont& font, const QRect& win_rect, const QString& text)
{
	painter.save();
	painter.setFont(font);
	auto rect = getBoundingRect(painter, win_rect, text);
	painter.restore();
	return rect;
}

QRect UIHelper::getBoundingRect(QPainter& painter, const QRect& win_rect, const QString& text, PosType pos_type, int kPadding /*= 10*/)
{
	QRect rect_text = UIHelper::getBoundingRect(painter, win_rect, text);

	if (PosType::LT==pos_type){
		rect_text.moveTo(kPadding, kPadding);
	}
	else if (PosType::LB == pos_type) {
		rect_text.moveTo(kPadding, win_rect.height()-kPadding- rect_text.height());
	}
	else if(PosType::RT == pos_type)
		rect_text.moveTo(win_rect.width() - kPadding - rect_text.width(), kPadding);
	else if (PosType::RB == pos_type)
		rect_text.moveTo(win_rect.width() - kPadding - rect_text.width(), win_rect.height() - kPadding - rect_text.height());
	else
		rect_text.moveCenter(win_rect.center());
	return rect_text;
}

void UIHelper::drawText(QPainter& painter, const QFont& font, const QColor& color, const QRect& win_rect, const QString& text, PosType pos_type, int kPadding /*= 0 */)
{
	painter.save();
	painter.setFont(font);
	painter.setPen(color);
	QTextDocument td;
	td.setHtml(text);
	QRect text_rect = getBoundingRect(painter, win_rect, td.toPlainText(), pos_type,kPadding);

	painter.drawText(text_rect, td.toPlainText());
	painter.restore();
}

void UIHelper::drawHtmlText(QPainter& painter, const QFont& font, const QRect& win_rect, const QString& html, PosType pos_type, int kPadding /*= 0 */)
{
	painter.save();
 	painter.setFont(font);
	QTextDocument td;
	td.setHtml(html);
	QRect text_rect = getBoundingRect(painter, win_rect, td.toPlainText(), pos_type, kPadding);
	painter.translate(text_rect.topLeft());
	td.drawContents(&painter, win_rect);
	painter.restore();
}
