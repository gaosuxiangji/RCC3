#include "bgtextitem.h"
#include <QFont>
#include <QFontMetrics>

BTextItem::BTextItem()
{
	init();
}

BTextItem::~BTextItem()
{

}

void BTextItem::init()
{
	{//×ÖÌå
		QFont font;
		font.setFamily("ËÎÌå");
		font.setPixelSize(25);
		font.setBold(true);
		setFont(font);
		setDefaultTextColor(Qt::red);
	}
}

void BTextItem::updateInfo(const QString& info)
{
	m_current_info = info;
	setPlainText(info);
	m_info_width = QFontMetrics(this->font()).width(toPlainText()) + m_info_offset_const;
	m_info_height = QFontMetrics(this->font()).height() + m_info_offset_const;
}

void BTextItem::switchColor(const QColor &c)
{
	setDefaultTextColor(c);
}

