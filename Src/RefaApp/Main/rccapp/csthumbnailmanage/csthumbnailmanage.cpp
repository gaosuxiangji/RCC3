#include "csthumbnailmanage.h"
#include "ui_csthumbnailmanage.h"
#include <QMouseEvent>
#include <QtMath>

CSThumbnailManage::CSThumbnailManage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CSThumbnailManage)
{
    ui->setupUi(this);
	Init();
}

CSThumbnailManage::~CSThumbnailManage()
{
    delete ui;
}

void CSThumbnailManage::SlotThumbnailUpdated(int thumbnail_index, const RccFrameInfo & thumbnail_image)
{
#if TRUN_ON_THUMBNAIL_ANIMATION_MODE
	if (15 < (end_frame_no_ - start_frame_no_))
	{
		emit SignalFrameStepStatus(1, true);
		emit SignalFrameStepStatus(2, true);
	}
	else
	{
		emit SignalFrameStepStatus(1, false);
		emit SignalFrameStepStatus(2, false);
	}
	start_frame_no_ = thumbnail_image.playback_info.start_frame_no;
	end_frame_no_ = thumbnail_image.playback_info.end_frame_no;
	thumbnail_interval_ = (end_frame_no_ - start_frame_no_) / (THUMBNAIL_MAX_NUM - 1);
	if (thumbnail_interval_ < 1)
	{
		thumbnail_interval_ = 1;
	}
	thumbnail_end_frame_no_ = start_frame_no_ + (THUMBNAIL_MAX_NUM - 1) * thumbnail_interval_;
#endif

	auto thumbnail_ctrl_ptr = getThumbnailCtrl(thumbnail_index);
	if (thumbnail_ctrl_ptr)
	{
		if ((thumbnail_ctrl_ptr->GetFrameNo() != thumbnail_image.playback_info.frame_no) 
			|| (thumbnail_ctrl_ptr->GetThumbnailState() != thumbnail_image.playback_info.thumb_nail_state)
			|| thumbnail_image.playback_info.m_image_changed)
		{
			CSLOG_INFO("SlotThumbnailUpdated state:{}, thumb_idx:{}, frame_no:{}", thumbnail_image.playback_info.thumb_nail_state, thumbnail_index, thumbnail_image.playback_info.frame_no);
			thumbnail_ctrl_ptr->UpdateThumbnail(thumbnail_index, thumbnail_image);
		}

		bool bBorder = thumbnail_image.playback_info.is_highlight_frame ? true : false;
		thumbnail_ctrl_ptr->DrawBorderLine(bBorder);
	}
}

void CSThumbnailManage::SlotHighlightThumbnail(int thumbnail_index)
{
	if (highlight_thumbnail_index_ != thumbnail_index)
	{
		setThumbnailHighlighted(highlight_thumbnail_index_, false);
		setThumbnailHighlighted(thumbnail_index, true);

		highlight_thumbnail_index_ = thumbnail_index;
	}
}

void CSThumbnailManage::SlotVideoRangeChanged(int64_t start_frame_index, int64_t end_frame_index)
{
	if (start_frame_index > end_frame_index)
	{
		return;
	}
	if (start_frame_no_ != start_frame_index || end_frame_no_ != end_frame_index)
	{
		start_frame_no_ = start_frame_index;
		end_frame_no_ = end_frame_index;
	}
}
void CSThumbnailManage::SlotMouseInLeftBtn()
{
	m_leftBtnTimer->start(TIME_INTERVAL);
}

void CSThumbnailManage::SlotMouseOutLeftBtn()
{
	m_leftBtnTimer->stop();

	SlotHighlightThumbnail(-1);
}

void CSThumbnailManage::SlotMouseInRightBtn()
{
	m_rightBtnTimer->start(TIME_INTERVAL);
}

void CSThumbnailManage::SlotMouseOutRightBtn()
{
	m_rightBtnTimer->stop();
	
	SlotHighlightThumbnail(-1);
}


void CSThumbnailManage::updateSelectedThumbnail(int frame_no)
{
	m_iNewestFrameNo = frame_no;
	int thumbnails_frame_no = start_frame_no_ + ((frame_no - start_frame_no_) / thumbnail_interval_) * thumbnail_interval_;
	if (thumbnails_frame_no > thumbnail_end_frame_no_)
	{
		thumbnails_frame_no = thumbnail_end_frame_no_;
	}

	QRect r = this->rect();
	float delta = float(r.width()) / THUMBNAIL_NUM;
	int total_frame_count = thumbnail_interval_ * thumbnail_ctrl_count_;

	uint64_t mid_thumbnail_index = qCeil(THUMBNAIL_NUM *1.0 / 2) - 1;

	uint64_t mid_thumbnail_no = frame_no;
	if (mid_thumbnail_no < start_frame_no_ + mid_thumbnail_index * thumbnail_interval_)
	{
		mid_thumbnail_no = start_frame_no_ + mid_thumbnail_index * thumbnail_interval_;
	}
	else if (mid_thumbnail_no > thumbnail_end_frame_no_ - mid_thumbnail_index * thumbnail_interval_)
	{
		mid_thumbnail_no = thumbnail_end_frame_no_ - mid_thumbnail_index * thumbnail_interval_;
	}

	int x = (mid_thumbnail_index - float(mid_thumbnail_no - start_frame_no_) / thumbnail_interval_) * delta;

	for (int i = 0; i < thumbnail_ctrl_count_; i++)
	{
		auto thumbnail_ctrl_ptr = getThumbnailCtrl(i);
		if (thumbnail_ctrl_ptr)
		{
			thumbnail_ctrl_ptr->setGeometry(x + i * delta, r.y(), delta, r.height());
#if TRUN_ON_FLOAT_THUMBNAIL
			if (thumbnail_ctrl_ptr->IsHeightLight())
			{
				SlotUpdateShowHotWidget(i, true);
			}
#endif
		}
	}

	uint64_t iStart{ 0 };
	uint64_t iEnd{ 0 };
	GetThumbnailRange(iStart, iEnd);
	iStart = (-1 == iStart) ? start_frame_no_ : iStart;
	iEnd = (-1 == iEnd) ? end_frame_no_ : iEnd;
	emit SignalThumbnailRange(iStart, iEnd);
}

void CSThumbnailManage::SetImgMoveDirection(const bool bMove)
{
	if (bMove)
	{
		uint64_t mid_thumbnail_index = qCeil(THUMBNAIL_NUM *1.0 / 2) - 1;
		if (m_iNewestFrameNo < (thumbnail_end_frame_no_ - mid_thumbnail_index * thumbnail_interval_))
		{
			m_iNewestFrameNo = ((m_iNewestFrameNo + thumbnail_interval_) > end_frame_no_) ? end_frame_no_ : (m_iNewestFrameNo + thumbnail_interval_);
		}
	}
	else
	{
		if (m_iNewestFrameNo > (start_frame_no_ + thumbnail_interval_))
		{
			m_iNewestFrameNo = ((m_iNewestFrameNo - thumbnail_interval_) < start_frame_no_) ? start_frame_no_ : (m_iNewestFrameNo - thumbnail_interval_);
		}	
	}

	if (0 <= m_iNewestFrameNo)
	{
		updateSelectedThumbnail(m_iNewestFrameNo);
	}
}

void CSThumbnailManage::GetThumbnailRange(uint64_t & start_frame_no, uint64_t & iEnd)
{
	start_frame_no = -1;
	iEnd = -1;
	int startIndex = ReturnThumbnailIndex(QPointF(5,this->height()/2));
	int endIndex = ReturnThumbnailIndex(QPointF(this->width()-5, this->height() / 2));
	if (-1 != startIndex)
	{
		auto thumbnail_ctrl_ptr = getThumbnailCtrl(startIndex);
		if (thumbnail_ctrl_ptr)
		{
			//直接获取缩略图帧号
			start_frame_no = thumbnail_ctrl_ptr->GetFrameNo();
		}
	}

	if (-1 != endIndex)
	{
		auto thumbnail_ctrl_ptr = getThumbnailCtrl(endIndex);
		if (thumbnail_ctrl_ptr)
		{
			//直接获取缩略图帧号
			iEnd = thumbnail_ctrl_ptr->GetFrameNo();
		}
	}
}

void CSThumbnailManage::Init()
{
	InitUI();

	m_leftBtnTimer = new QTimer(this);
	bool ok = connect(m_leftBtnTimer, &QTimer::timeout, this, &CSThumbnailManage::SlotLeftBtnTimer);

	m_rightBtnTimer = new QTimer(this);
	ok = connect(m_rightBtnTimer, &QTimer::timeout, this, &CSThumbnailManage::SlotRightBtnTimer);
	Q_UNUSED(ok);
}

void CSThumbnailManage::InitUI()
{
	//去除标题框
	setWindowFlags(Qt::FramelessWindowHint);

	// 初始化缩略图控件
	InitThumbnailCtrls();
}

void CSThumbnailManage::InitThumbnailCtrls()
{
	thumbnail_ctrl_count_ = THUMBNAIL_MAX_NUM;
#if TRUN_ON_FLOAT_THUMBNAIL
	if (m_pThumbnailCtrl)
	{
		delete m_pThumbnailCtrl;
		m_pThumbnailCtrl = nullptr;
	}
	m_pThumbnailCtrl = new CSThumbnailControl(constFloatControlIndex, parentWidget()->parentWidget());
	m_pThumbnailCtrl->setWindowFlags(m_pThumbnailCtrl->windowFlags() | Qt::Dialog);
	bool ok = connect(m_pThumbnailCtrl, &CSThumbnailControl::SignalShowImgInBigScreen, this, &CSThumbnailManage::SignalShowImgInBigScreen);
	ok = connect(m_pThumbnailCtrl, &CSThumbnailControl::SignalKeyFrame, this, &CSThumbnailManage::SignalKeyFrame);
	ok = connect(m_pThumbnailCtrl, &CSThumbnailControl::SignalBeginFrame, this, &CSThumbnailManage::SignalBeginFrame);
	ok = connect(m_pThumbnailCtrl, &CSThumbnailControl::SignalEndFrame, this, &CSThumbnailManage::SignalEndFrame);
	ok = connect(m_pThumbnailCtrl, &CSThumbnailControl::SignalSingleThumbnailClicked, this, &CSThumbnailManage::SignalSingleThumbnailClicked);
	ok = connect(m_pThumbnailCtrl, &CSThumbnailControl::SignalUpdateShowHotWidget, this, &CSThumbnailManage::SlotUpdateShowHotWidget);
	ok = connect(m_pThumbnailCtrl, &CSThumbnailControl::SignalMoveOutThumbnailArea, this, &CSThumbnailManage::SignalMoveOutThumbnailArea);
#endif
	thumbnail_ctrls_.resize(thumbnail_ctrl_count_);
	for (int i = 0; i < thumbnail_ctrl_count_; i++)
	{
		auto singleThumbnail = new CSThumbnailControl(i, this);
		setThumbnailCtrl(i, singleThumbnail);
		bool ok = connect(singleThumbnail, &CSThumbnailControl::SignalShowImgInBigScreen, this, &CSThumbnailManage::SignalShowImgInBigScreen);
		ok = connect(singleThumbnail, &CSThumbnailControl::SignalKeyFrame, this, &CSThumbnailManage::SignalKeyFrame);
		ok = connect(singleThumbnail, &CSThumbnailControl::SignalBeginFrame, this, &CSThumbnailManage::SignalBeginFrame);
		ok = connect(singleThumbnail, &CSThumbnailControl::SignalEndFrame, this, &CSThumbnailManage::SignalEndFrame);
		ok = connect(singleThumbnail, &CSThumbnailControl::SignalSingleThumbnailClicked, this, &CSThumbnailManage::SignalSingleThumbnailClicked);

#if TRUN_ON_FLOAT_THUMBNAIL
		ok = connect(singleThumbnail, &CSThumbnailControl::SignalUpdateShowHotWidget, this, &CSThumbnailManage::SlotUpdateShowHotWidget);
#endif
		Q_UNUSED(ok);
	}
}

void CSThumbnailManage::mousePressEvent(QMouseEvent * event)
{
	if (event->buttons() == Qt::LeftButton)
	{
		m_bLeftBtnPressed = true;
#if TRUN_ON_THUMBNAIL_ANIMATION_MODE
		int index = ReturnThumbnailIndex(event->pos());
		if (-1 != index)
		{
			m_iDragIndexStart = index;
			SlotHighlightThumbnail(m_iDragIndexStart);
		}
#else
		m_pressPos = mapFromGlobal(QCursor().pos()).x();
		int singleLength = this->width() / THUMBNAIL_NUM;
		m_iDragIndexStart = m_pressPos / singleLength;
		m_iDragIndexStart = (m_iDragIndexStart > thumbnail_ctrl_count_) ? (thumbnail_ctrl_count_) : m_iDragIndexStart;
		SlotHighlightThumbnail(m_iDragIndexStart);
#endif	
	}
}

void CSThumbnailManage::mouseReleaseEvent(QMouseEvent * event)
{
	if (m_bLeftBtnPressed)
	{
		m_bLeftBtnPressed = false;
#if TRUN_ON_THUMBNAIL_ANIMATION_MODE
		if (m_iDragIndexEnd > m_iDragIndexStart)
		{
			auto drag_start_thumbnail_ctrl_ptr = getThumbnailCtrl(m_iDragIndexStart);
			auto drag_end_thumbnail_ctrl_ptr = getThumbnailCtrl(m_iDragIndexEnd);
			if (drag_start_thumbnail_ctrl_ptr && drag_end_thumbnail_ctrl_ptr)
			{
				emit SignalUapdateFrameRange(drag_start_thumbnail_ctrl_ptr->GetFrameNo(), drag_end_thumbnail_ctrl_ptr->GetFrameNo());
				SlotHighlightThumbnail(-1);
			}
		}
#else
		int singleLength = this->width() / THUMBNAIL_NUM;
		m_iDragIndexEnd = (mapFromGlobal(QCursor().pos()).x()) / singleLength;
		m_iDragIndexEnd = (m_iDragIndexEnd > thumbnail_ctrl_count_) ? thumbnail_ctrl_count_ : m_iDragIndexEnd;
		if (m_iDragIndexEnd > m_iDragIndexStart)
		{
			auto drag_start_thumbnail_ctrl_ptr = getThumbnailCtrl(m_iDragIndexStart);
			auto drag_end_thumbnail_ctrl_ptr = getThumbnailCtrl(m_iDragIndexEnd);
			if (drag_start_thumbnail_ctrl_ptr && drag_end_thumbnail_ctrl_ptr)
			{
				emit SignalUapdateFrameRange(drag_start_thumbnail_ctrl_ptr->GetFrameNo(), drag_end_thumbnail_ctrl_ptr->GetFrameNo());
				SlotHighlightThumbnail(-1);
			}
		}
#endif
		ResetVariablesValue();
	}
}

void CSThumbnailManage::mouseMoveEvent(QMouseEvent * event)
{
	if (event->buttons() == Qt::LeftButton)
	{
#if TRUN_ON_THUMBNAIL_ANIMATION_MODE
		int index = ReturnThumbnailIndex(event->pos());
		if (-1 != index)
		{
			if (m_iDragIndexStart <= index)
			{
				if (index >= m_indexMax)
				{
					m_indexMax = index;
				}
				else
				{
					//鼠标回退时，取消高亮缩略图
					for (int i = index + 1; i <= m_indexMax; ++i)
					{
						setThumbnailHighlighted(i, false);
					}
				}

				auto thumbnail_ctrl_ptr = getThumbnailCtrl(index);
				if (thumbnail_ctrl_ptr && !thumbnail_ctrl_ptr->IsCurrentImgEmpty())
				{
					m_iDragIndexEnd = index;
					setThumbnailHighlighted(index, true);
				}
			}
		}
#else
		CalThumbnailIndex(mapFromGlobal(QCursor().pos()).x());
		if (m_iDragIndexStart <= m_indexNewest)
		{
			if (m_indexNewest >= m_indexMax)
			{
				m_indexMax = m_indexNewest;
			}
			else
			{
				//鼠标回退时，取消之前的红框
				for (int i = m_indexNewest+1; i<=m_indexMax; ++i)
				{
					setThumbnailHighlighted(i, false);
				}
			}

			setThumbnailHighlighted(m_indexNewest, true);
		}
#endif	
	}
}

void CSThumbnailManage::CalThumbnailIndex(const int xPos)
{
	if (m_pressPos>xPos)
	{
		return;
	}
	int singleLength = this->width() / THUMBNAIL_NUM;
	m_indexNewest = xPos / singleLength;
	m_indexNewest = (m_indexNewest > thumbnail_ctrl_count_) ? thumbnail_ctrl_count_ : m_indexNewest;
}

void CSThumbnailManage::ResetVariablesValue()
{
	m_pressPos = -1;
	m_indexNewest = -1;
	m_iDragIndexStart = -1;
	m_iDragIndexEnd = -1;
	m_indexMax = 0;
}

void CSThumbnailManage::leaveEvent(QEvent * event)
{
	emit SignalMoveOutThumbnailArea();
}

void CSThumbnailManage::resizeEvent(QResizeEvent * e)
{
#if TRUN_ON_THUMBNAIL_ANIMATION_MODE
	m_iNewestFrameNo = start_frame_no_;
	updateSelectedThumbnail(start_frame_no_);
#endif
}

int CSThumbnailManage::ReturnThumbnailIndex(const QPointF & posPointf) const
{
	int index = -1;

	if (posPointf.x() < 0 || posPointf.x() > this->width())
	{
		index = -1;
	}
	else
	{
		for (int i = 0; i< thumbnail_ctrl_count_; ++i)
		{
			auto thumbnail_ctrl_ptr = getThumbnailCtrl(i);
			if (thumbnail_ctrl_ptr)
			{
				QPointF TL = thumbnail_ctrl_ptr->geometry().topLeft();
				QPointF BR = thumbnail_ctrl_ptr->geometry().bottomRight();
				if ((posPointf.x() >= TL.x()) && (posPointf.x() <= BR.x()))
				{
					index = i;
					break;
				}
			}
		}
	}
	return index;
}

void CSThumbnailManage::SlotLeftBtnTimer()
{
	SetImgMoveDirection(false);
	int startIndex = ReturnThumbnailIndex(QPointF(5, this->height() / 2));
	SlotHighlightThumbnail(startIndex);
}

void CSThumbnailManage::SlotRightBtnTimer()
{
	SetImgMoveDirection(true);
	int endIndex = ReturnThumbnailIndex(QPointF(this->width() - 5, this->height() / 2));
	SlotHighlightThumbnail(endIndex);
}

void CSThumbnailManage::SlotUpdateShowHotWidget(int nIndex, bool bShow)
{
#if TRUN_ON_FLOAT_THUMBNAIL
	if (nIndex != -1 && nIndex < thumbnail_ctrl_count_ && bShow)
	{
		m_nThumbnailCtrl = nIndex;
		CSThumbnailControl *pThumb = getThumbnailCtrl(nIndex);
		QRect rc = pThumb->geometry();
		int nWidth = rc.width();
		int nHeight = rc.height();
		QPoint globalPos = pThumb->mapToGlobal(QPoint(0, 0));
		rc.setLeft(globalPos.x() - 5);
		rc.setTop(globalPos.y() - 5);
		rc.setWidth(nWidth + 10);
		rc.setHeight(nHeight + 10);
		m_pThumbnailCtrl->SetCurrentImage(pThumb->GetCurrentImage());
		m_pThumbnailCtrl->setKeyFrame(pThumb->IsKeyFrame());
		m_pThumbnailCtrl->setFrameNo(pThumb->GetFrameNo());
		m_pThumbnailCtrl->setThumbnailIndex(pThumb->getThumbnailIndex());
		m_pThumbnailCtrl->setGeometry(rc);
		m_pThumbnailCtrl->updateUI();
		m_pThumbnailCtrl->setFixedSize(QSize(rc.width(), rc.height()));
		m_pThumbnailCtrl->show();
	}
	else if (nIndex == constFloatControlIndex)
	{
		m_nThumbnailCtrl = nIndex;
		if (bShow)
		{
			m_pThumbnailCtrl->show();
		}
		else
		{
			m_pThumbnailCtrl->hide();
		}
	}
	else if (nIndex == m_nThumbnailCtrl)
	{
		if (bShow)
		{
			m_pThumbnailCtrl->show();
		}
		else
		{
			m_pThumbnailCtrl->hide();
		}
	}
	else
	{
		m_nThumbnailCtrl = -1;
		m_pThumbnailCtrl->hide();
	}
#endif
}

CSThumbnailControl* CSThumbnailManage::getThumbnailCtrl(int index) const
{
	if (0 <= index && index < thumbnail_ctrls_.size())
	{
		return thumbnail_ctrls_[index];
	}

	return nullptr;
}

void CSThumbnailManage::setThumbnailCtrl(int index, CSThumbnailControl* ctrl_ptr)
{
	if (0 <= index && index < thumbnail_ctrls_.size())
	{
		thumbnail_ctrls_[index] = ctrl_ptr;
	}
}

int CSThumbnailManage::getThumbnailCtrlCount() const
{
	return thumbnail_ctrls_.count();
}

void CSThumbnailManage::setThumbnailHighlighted(int index, bool highlighted)
{
	auto thumbnail_ctrl_ptr = getThumbnailCtrl(index);
	if (thumbnail_ctrl_ptr)
	{
		thumbnail_ctrl_ptr->DrawBorderLine(highlighted);
	}
}

