#include "bgmeasuremanage.h"
#include <QVector>
#include <QDebug>
#include <QMenu>
#include <QSpinBox>
#include <QWidgetAction>

LineMeasureItemManage::LineMeasureItemManage()
{
	qRegisterMetaType<PointInfo>("PointInfo");
}

void LineMeasureItemManage::pushPointSlot(const QPoint& pt, bool cancel)
{
	m_draw_point_cancel = cancel;
	m_point_vec.append(pt);

	if (cancel)
	{
		m_point_vec.clear();
	}
}

void LineMeasureItemManage::measureLineItemSlot(uint32_t index, const PointInfo& item)
{
	if (!isLineMeasureItemExist(index, item)) {
		QSharedPointer<BGLineMeasureItem> line_measure_item = QSharedPointer<BGLineMeasureItem>(new BGLineMeasureItem(this, item));
		connect(line_measure_item.data(), &BGLineMeasureItem::updateMeasureInfoSignal, this, &LineMeasureItemManage::updateMeasureItemInfoSignal, Qt::QueuedConnection);
		m_mesure_item_list.push_back(line_measure_item);
	}
}

void LineMeasureItemManage::pushMultiPtSlot(const QPoint& pt, bool feature, bool cancel)
{
	if (feature && !cancel) {
		m_multi_pt_feature_vec.append(pt);
	}
	else if (!feature && !cancel) {
		m_multi_pt_common_vec.append(pt);
	}

	if (cancel)
	{
		m_multi_pt_feature_vec.clear();
		m_multi_pt_common_vec.clear();
	}
}

void LineMeasureItemManage::multiMeasureLineItemSlot(uint32_t index, const MultiPointInfo& item)
{
	if (!isMultiLineMeasureItemExist(index, item)) {
		QSharedPointer<BGLinesMeasureItem> multi_line_measure_item = QSharedPointer<BGLinesMeasureItem>(new BGLinesMeasureItem(this, item));
		connect(multi_line_measure_item.data(), &BGLinesMeasureItem::updateMultiMeasureInfoSignal, this, &LineMeasureItemManage::updateMultiMeasureItemInfoSignal, Qt::QueuedConnection);
		m_multi_mesure_item_list.push_back(multi_line_measure_item);
	}
}

void LineMeasureItemManage::updateMeasureModeType(int type)
{
	for (int i = 0; i < m_mesure_item_list.size(); ++i) {
		if (MMT_Modify == type) {
			m_mesure_item_list.at(i)->setFeatureRectVisible(true);
		}
		else {
			m_mesure_item_list.at(i)->setFeatureRectVisible(false);
		}
	}
}

void LineMeasureItemManage::updateMeasureItemInfo(const PointInfo& item)
{
	emit updateMeasureItemInfoSignal(item);
}

void LineMeasureItemManage::deleteMeasureLines(const QList<int> vctLine)
{
	if (m_mesure_item_list.size() > 0) {
		for (int i = 0; i < vctLine.size(); i++)
		{
			for (int j = 0; j < m_mesure_item_list.size(); j++)
			{
				if (vctLine[i] == m_mesure_item_list.at(j)->getItemInfo().point_index)
				{
					m_mesure_item_list.removeAt(j);
					break;
				}
			}
		}
	}
}

void LineMeasureItemManage::setMeasureItemVisible(bool visible)
{
	for (int i = 0; i < m_mesure_item_list.size(); ++i) {
		m_mesure_item_list.at(i)->setLineVisible(visible);
	}
}

void LineMeasureItemManage::updateMultiMeasureModeType(int type)
{
	for (int i = 0; i < m_multi_mesure_item_list.size(); ++i) {
		if (MMT_Modify == type) {
			m_multi_mesure_item_list.at(i)->setFeatureRectVisible(true);
		}
		else {
			m_multi_mesure_item_list.at(i)->setFeatureRectVisible(false);
		}
	}
}

void LineMeasureItemManage::updateMultiMeasureItemInfo(const MultiPointInfo& item)
{
	emit updateMultiMeasureItemInfoSignal(item);
}

void LineMeasureItemManage::deleteMultiMeasureLines(const QList<int> vctLine)
{
	if (m_multi_mesure_item_list.size() > 0) {
		for (int i = 0; i < vctLine.size(); i++)
		{
			for (int j = 0; j < m_multi_mesure_item_list.size(); j++)
			{
				if (vctLine[i] == m_multi_mesure_item_list.at(j)->getItemInfo().multi_point_index)
				{
					m_multi_mesure_item_list.removeAt(j);
					break;
				}
			}
		}
	}
}

void LineMeasureItemManage::setMultiMeasureItemVisible(bool visible)
{
	for (int i = 0; i < m_multi_mesure_item_list.size(); ++i) {
		m_multi_mesure_item_list.at(i)->setLineVisible(visible);
	}
}

QRectF LineMeasureItemManage::boundingRect() const
{
	return QRectF{};
}

void LineMeasureItemManage::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QPen pen;
	pen.setWidth(2);
	if (m_point_vec.size() > 1) {
		painter->save();
		pen.setColor(Qt::green);
		painter->setPen(pen);
		QBrush brush;
		brush.setStyle(Qt::SolidPattern);
		brush.setColor(Qt::green);
		painter->setBrush(brush);
		painter->drawEllipse(m_point_vec.at(0), 4, 4);
		painter->restore();

		painter->save();
		pen.setColor(Qt::red);
		painter->setPen(pen);
		painter->drawLine(m_point_vec.at(0), m_point_vec.at(m_point_vec.size() - 1));
		painter->restore();
	}

	if (m_multi_pt_feature_vec.size() > 0 && m_multi_pt_common_vec.size() > 0) {
		painter->save();
		pen.setColor(Qt::green);
		painter->setPen(pen);
		QBrush brush;
		brush.setStyle(Qt::SolidPattern);
		brush.setColor(Qt::green);
		painter->setBrush(brush);
		for (auto feature_pt : m_multi_pt_feature_vec) {
			painter->drawEllipse(feature_pt, 4, 4);
		}
		painter->restore();

		painter->save();
		pen.setColor(Qt::red);
		painter->setPen(pen);
		painter->drawPolyline(m_multi_pt_feature_vec);
		painter->restore();

		painter->save();
		pen.setColor(Qt::red);
		painter->setPen(pen);
		painter->drawLine(m_multi_pt_feature_vec.at(m_multi_pt_feature_vec.size() - 1), m_multi_pt_common_vec.at(m_multi_pt_common_vec.size() - 1));
		painter->restore();
	}
}

bool LineMeasureItemManage::isLineMeasureItemExist(uint32_t index, const PointInfo& item)
{
	for (int i = 0; i < m_mesure_item_list.size(); ++i) {
		if (index == m_mesure_item_list.at(i)->getItemInfo().point_index) {
			m_mesure_item_list.at(i)->updatePointInfo(item);
			return true;
		}
	}
	return false;
}

bool LineMeasureItemManage::isMultiLineMeasureItemExist(uint32_t index, const MultiPointInfo& item)
{
	for (int i = 0; i < m_multi_mesure_item_list.size(); ++i) {
		if (index == m_multi_mesure_item_list.at(i)->getItemInfo().multi_point_index) {
			m_multi_mesure_item_list.at(i)->updatePointInfo(item);
			return true;
		}
	}
	return false;
}
