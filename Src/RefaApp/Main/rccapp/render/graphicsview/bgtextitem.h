#pragma once
#include <QGraphicsTextItem>

class BTextItem : public QGraphicsTextItem {
	Q_OBJECT
public:
	BTextItem();
	~BTextItem();
private:
	void init();
public:
	void updateInfo(const QString& info);
	void switchColor(const QColor &c);
	int getInfoWidth() { return m_info_width; }
	int getInfoHeight() { return m_info_height; }
	int getInfoOffset() { return 5; }
	QString getInfo() { return m_current_info; }
private:
	int m_info_width{ 0 };
	int m_info_height{ 0 };
	const int m_info_offset_const{ 15 };
	QString m_current_info{};
};
