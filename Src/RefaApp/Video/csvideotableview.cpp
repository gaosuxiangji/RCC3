#include "csvideotableview.h"
#include <QScrollBar>
#include <QHeaderView>

CSVideoTableView::CSVideoTableView(QWidget *parent) :QTableView(parent)
{
	
}

CSVideoTableView::~CSVideoTableView()
{
	delete m_frozenTableView;
}

void CSVideoTableView::init(QStandardItemModel *myModel, QHeaderView *myHeader, QAbstractItemDelegate *myItemDelegate)
{
	setModel(myModel);
	m_frozenTableView = new QTableView(this);
	m_frozenTableView->setModel(model());
	m_frozenTableView->setHorizontalHeader(myHeader);
	m_frozenTableView->setItemDelegate(myItemDelegate);
	
	initFreezeTable();
}

void CSVideoTableView::initFreezeTable()
{	
	viewport()->stackUnder(m_frozenTableView);

	setStyleSheet("QHeaderView{border: 0px solid ; border-bottom: 1px solid #d8d8d8;} \
		QTableView::item:selected {color:rgb(255,255,255); background-color:rgb(0,120,215);}");
	m_frozenTableView->setStyleSheet("QTableView{border: 0px}\
		QHeaderView{border: 1px solid #d8d8d8; border-left: 0px;border-top: 0px;} \
		QTableView::item:selected {color:rgb(255,255,255); background-color:rgb(0,120,215);}");
	m_frozenTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_frozenTableView->verticalHeader()->hide();
	m_frozenTableView->setSelectionModel(selectionModel());
	m_frozenTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_frozenTableView->horizontalHeader()->setDefaultSectionSize(m_default_section_size);
	m_frozenTableView->horizontalHeader()->resizeSection(0, 30);
	m_frozenTableView->setVerticalScrollMode(ScrollPerPixel);
	setVerticalScrollMode(ScrollPerPixel);
	horizontalHeader()->setDefaultSectionSize(m_default_section_size);
	horizontalHeader()->resizeSection(0, 30);

	for (int col = m_iFreezeColCounts; col < model()->columnCount(); ++col)
	{
		m_frozenTableView->setColumnHidden(col, true);
	}
	for (int i = 0; i < m_iFreezeColCounts; i++)
	{
		m_frozenTableView->setColumnWidth(i, columnWidth(0));
	}
	m_frozenTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_frozenTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_frozenTableView->show();
	updateFrozenTableGeometry();

	setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
	m_frozenTableView->setVerticalScrollMode(ScrollPerPixel);
	connect(m_frozenTableView->verticalScrollBar(), &QAbstractSlider::valueChanged, verticalScrollBar(), &QAbstractSlider::setValue);
	connect(verticalScrollBar(), &QAbstractSlider::valueChanged, m_frozenTableView->verticalScrollBar(), &QAbstractSlider::setValue);
	connect(m_frozenTableView, &QTableView::pressed, this, &CSVideoTableView::updateSection);
	connect(m_frozenTableView, &QTableView::clicked, this, &CSVideoTableView::updateSection);
	connect(horizontalHeader(), &QHeaderView::sectionResized, this, &CSVideoTableView::updateSectionWidth);
	connect(verticalHeader(), &QHeaderView::sectionResized, this, &CSVideoTableView::updateSectionHeight);
}

void CSVideoTableView::updateFrozenTableGeometry()
{
	int width = 0;
	for (int i = 0; i < m_iFreezeColCounts; i++)
	{
		width += columnWidth(i);
	}
	m_frozenTableView->setGeometry(verticalHeader()->width() + frameWidth(),
		frameWidth(), width, viewport()->height() + horizontalHeader()->height());
}

void CSVideoTableView::updateSectionWidth(int logicalIndex, int lodSize, int newSize)
{
	if (logicalIndex == m_iFreezeColCounts - 1)
	{
		int width = 0;
		for (int i = 0; i < m_iFreezeColCounts - 1; i++)
		{
			width += columnWidth(i);
		}
		for (int i = 0; i < m_iFreezeColCounts; i++)
		{
			m_frozenTableView->setColumnWidth(i, (newSize + width) / m_iFreezeColCounts);
		}
		updateFrozenTableGeometry();
	}
}

void CSVideoTableView::updateSectionHeight(int logicalIndex, int lodSize, int newSize)
{
	m_frozenTableView->setRowHeight(logicalIndex, newSize);
}

void CSVideoTableView::updateSection(const QModelIndex current)
{
	setCurrentIndex(current);
}

void CSVideoTableView::resizeEvent(QResizeEvent *event)
{
	QTableView::resizeEvent(event);
	if (m_frozenTableView)
	{
		updateFrozenTableGeometry();
	}
}

void CSVideoTableView::scrollTo(const QModelIndex & index, ScrollHint hint)
{
	QTableView::scrollTo(index, hint);
}

void CSVideoTableView::mouseMoveEvent(QMouseEvent *event)
{
	emit tableviewMoveEvent(this->indexAt(event->pos()));
	QTableView::mouseMoveEvent(event);
}

void CSVideoTableView::leaveEvent(QEvent *event)
{
	emit tableviewLeaveEvent();
	QTableView::leaveEvent(event);
}

void CSVideoTableView::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Space)//空格触发
	{
		QWidget::keyPressEvent(event);
		return;
	}
	QTableView::keyPressEvent(event);
}