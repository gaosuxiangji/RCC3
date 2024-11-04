#include "tabelheaderview.h"

TableHeaderView::TableHeaderView(Qt::Orientation orientation, QWidget *parent)
	: QHeaderView(orientation, parent)
	, m_bPressed(false)
	, m_bChecked(false)
	, m_bTristate(false)
	, m_bNoChange(false)
	, m_bMoving(false)
	, m_iCheckBoxColumn(0)
{
	setHighlightSections(false);
	setMouseTracking(true);

	// 响应鼠标
	setSectionsClickable(true);
}

TableHeaderView::~TableHeaderView()
{}

void TableHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
	painter->save();
	QHeaderView::paintSection(painter, rect, logicalIndex);
	painter->restore();

	if (m_iCheckBoxColumn == logicalIndex)
	{
		QStyleOptionButton option;
		option.initFrom(this);

		if (m_bChecked)
			option.state |= QStyle::State_Sunken;

		if (m_bTristate && m_bNoChange)
			option.state |= QStyle::State_NoChange;
		else
			option.state |= m_bChecked ? QStyle::State_On : QStyle::State_Off;
		if (testAttribute(Qt::WA_Hover) && underMouse()) {
			if (m_bMoving)
				option.state |= QStyle::State_MouseOver;
			else
				option.state &= ~QStyle::State_MouseOver;
		}

		QCheckBox checkBox;
        option.iconSize = QSize(20, 20);
#ifdef _WINDOWS
        option.rect = rect;
#else
        int nX = 0, nY = 0;
        if(rect.width() > 20)
        {
            nX = rect.x() + (rect.width() - 20) / 2;
        }
        if(rect.height() > 20)
        {
            nY = rect.y() + (rect.height() - 20) / 2;
        }
        option.rect = QRect(nX, nY, 20, 20);
#endif
        style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, painter);
	}
}

void TableHeaderView::mousePressEvent(QMouseEvent *event)
{
	int nColumn = logicalIndexAt(event->pos());
	if ((event->buttons() & Qt::LeftButton) && (m_iCheckBoxColumn == nColumn))
	{
		m_bPressed = true;
	}
	else
	{
		QHeaderView::mousePressEvent(event);
	}
}

void TableHeaderView::mouseReleaseEvent(QMouseEvent *event)
{
	if (m_bPressed)
	{
		if (m_bTristate && m_bNoChange)
		{
			m_bChecked = true;
			m_bNoChange = false;
		}
		else
		{
			m_bChecked = !m_bChecked;
		}

		update();

		Qt::CheckState state = m_bChecked ? Qt::Checked : Qt::Unchecked;
		emit SignalStateChanged(state);
	}
	else
	{
		QHeaderView::mouseReleaseEvent(event);
	}

	m_bPressed = false;
}

bool TableHeaderView::event(QEvent *event)
{
//	updateSection(0); //解决点击表头，鼠标移开后表头才会更新状态问题
	if (event->type() == QEvent::Enter || event->type() == QEvent::Leave)
	{
		QMouseEvent *pEvent = static_cast<QMouseEvent *>(event);
		int nColumn = logicalIndexAt(pEvent->x());
		if (m_iCheckBoxColumn == nColumn)
		{
			m_bMoving = (event->type() == QEvent::Enter);

			update();
			return true;
		}
	}

	return QHeaderView::event(event);
}

//////////////////////////////////////////** 槽函数-begin **////////////////////////////////////////////////////////
void TableHeaderView::SlotStateChanged(int state)
{
	if (state == Qt::PartiallyChecked) {
		m_bTristate = true;
		m_bNoChange = true;
	}
	else {
		m_bNoChange = false;
	}

	m_bChecked = (state != Qt::Unchecked);
	update();
}
//////////////////////////////////////////** 槽函数-end **////////////////////////////////////////////////////////
