#include "widgetthumbnail.h"
#include "ui_widgetthumbnail.h"
#include "thumbnaillabel.h"

WidgetThumbnail::WidgetThumbnail(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetThumbnail)
{
    ui->setupUi(this);

    thumbnail_labels_<< ui->thumbnailLabel_1;
    thumbnail_labels_<< ui->thumbnailLabel_2;
    thumbnail_labels_<< ui->thumbnailLabel_3;
    thumbnail_labels_<< ui->thumbnailLabel_4;
    thumbnail_labels_<< ui->thumbnailLabel_5;
    thumbnail_labels_<< ui->thumbnailLabel_6;
    thumbnail_labels_<< ui->thumbnailLabel_7;
    thumbnail_labels_<< ui->thumbnailLabel_8;

    for(int i = 0, size = thumbnail_labels_.size(); i < size; ++i)
    {
        auto label = thumbnail_labels_[i];
        if(!label)
            continue;
        label->setLabelNo(i);
        connect(label, &ThumbnailLabel::selected, this, &WidgetThumbnail::onSelectThumbnailLabelChanged);
    }
	enable_thumb_cnt_ = thumbnail_labels_.size();
}

WidgetThumbnail::~WidgetThumbnail()
{
    delete ui;
}

void WidgetThumbnail::setThumbnail(uint thumb_no, const RMAImage &img)
{
    for(auto e : thumbnail_labels_)
    {
        if(e->getLabelNo() == thumb_no)
        {
            e->setThumbnailImage(img);
            break;
        }
    }
}

void WidgetThumbnail::clearThumbnail()
{
    for(auto e : thumbnail_labels_)
    {
		e->setUsed(false);
        e->clearThumbnailImage();
        e->setSelected(false);
    }
}

void WidgetThumbnail::setThumbnailCount(int cnt)
{
	if (cnt > thumbnail_labels_.size())
		cnt = thumbnail_labels_.size();
	enable_thumb_cnt_ = cnt;
	for (auto e : thumbnail_labels_)
	{
		e->setUsed(e->getLabelNo() < enable_thumb_cnt_);
	}
}

int WidgetThumbnail::getThumbnailCount() const
{
	return enable_thumb_cnt_;
}

int WidgetThumbnail::getThumbnailCountMax() const
{
	return thumbnail_labels_.size();
}

void WidgetThumbnail::setLoadThumbnailStatus(bool bfinished)
{
	for (auto e : thumbnail_labels_)
	{
		e->setLoadStatus(bfinished);
	}
}

void WidgetThumbnail::clearThumbnailSelect()
{
	for (auto e : thumbnail_labels_)
	{
		e->setSelected(false);
	}
}

void WidgetThumbnail::onSelectThumbnailLabelChanged(const RMAImage &img)
{
    auto plabel = dynamic_cast<ThumbnailLabel*>(sender());
    if(!plabel)
        return;
    if(!thumbnail_labels_.contains(plabel))
        return;
    for(auto e : thumbnail_labels_)
    {
        if(e != plabel)
        {
            e->setSelected(false);
        }
        else
        {
			emit selectedImageChanged(e->getLabelNo(), img);
        }
    }
}
