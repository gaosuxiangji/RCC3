#include "videolistwidget.h"
#include "ui_videolistwidget.h"

#include <QStandardItemModel>
#include <QMouseEvent>

#include "Common/AgCheckBoxHeaderView/agcheckboxheaderview.h"
#include "Common/AgCheckBoxItemDelegate/agcheckboxitemdelegate.h"
#include "Common/UIUtils/uiutils.h"

VideoListWidget::VideoListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoListWidget)
{
    ui->setupUi(this);

    initUi();
	ui->tableView->viewport()->installEventFilter(this);
}

VideoListWidget::~VideoListWidget()
{
    delete ui;
}

void VideoListWidget::addItem(const VideoListItem &item)
{
    QStandardItemModel* model_ptr = qobject_cast<QStandardItemModel*>(ui->tableView->model());
    if (!model_ptr)
    {
        return;
    }

    int row = model_ptr->rowCount();
    model_ptr->insertRow(row);

    int column = 0;

    // 复选框
    QStandardItem *item_ptr = new QStandardItem();
    item_ptr->setCheckable(true);
    item_ptr->setCheckState(Qt::Unchecked);
    item_ptr->setData(item.id, Qt::UserRole);
    model_ptr->setItem(row, column++, item_ptr);

    // 编号
	auto numberItem = new QStandardItem(QString::number(row + 1));
	numberItem->setTextAlignment(Qt::AlignCenter);
    model_ptr->setItem(row, column++, numberItem);

    // 名称
    model_ptr->setItem(row, column++, new QStandardItem(item.name));
	ui->tableView->resizeColumnToContents(column-1);

    // 更新提示信息
    updateTip();
}

void VideoListWidget::setCurrentItem(const QVariant &video_id)
{
    QStandardItemModel* model_ptr = qobject_cast<QStandardItemModel*>(ui->tableView->model());
    if (!model_ptr){
        return;
    }

    for (int i = 0; i < model_ptr->rowCount(); i++) {
		auto item = model_ptr->item(i, 0);
		if (item){
			if (item->index().data(Qt::UserRole) == video_id){
				ui->tableView->selectRow(i);
				break;
			}
		}
    }
}

QVariant VideoListWidget::getCurrentItem() const
{
    QModelIndex current_index = ui->tableView->selectionModel()->currentIndex();
    if (current_index.isValid())
    {
        return ui->tableView->model()->index(current_index.row(), 0).data(Qt::UserRole);
    }

    return QVariant();
}

void VideoListWidget::addItems(const QList<VideoListItem> &items)
{
    for (auto item : items)
    {
        addItem(item);
    }
}

QVariantList VideoListWidget::getSelectedItems() const
{
    return toVideoItems(selectedIndexes());
}

void VideoListWidget::clear()
{
    QStandardItemModel* model_ptr = qobject_cast<QStandardItemModel*>(ui->tableView->model());
    if (!model_ptr)
    {
        return;
    }

    model_ptr->setRowCount(0);
}

void VideoListWidget::onMouseClickedEnabled()
{
	benabled_mouse_clicked_ = true;
}

void VideoListWidget::initUi()
{
	// 删除按钮默认禁用
	ui->pushButtonDelete->setEnabled(false);

    // 设置模型
    QStandardItemModel *model_ptr = new QStandardItemModel(this);
    ui->tableView->setModel(model_ptr);

    // 设置行表头
    QStringList labels;
    labels << "" << tr("No.") << tr("Name");
    model_ptr->setHorizontalHeaderLabels(labels);

    // 设置复选样式表头（第1列）
    AgCheckBoxHeaderView* check_box_header_ptr = new AgCheckBoxHeaderView(Qt::Horizontal, ui->tableView);
    check_box_header_ptr->setSectionCheckable(0, true);
    ui->tableView->setHorizontalHeader(check_box_header_ptr);

    // 列宽度布局
    ui->tableView->resizeColumnToContents(0);
    ui->tableView->resizeColumnToContents(1);
    //ui->tableView->horizontalHeader()->setStretchLastSection(true);

    // 设置复选列（第1列）
    AgCheckBoxItemDelegate *check_box_delegatte_ptr = new AgCheckBoxItemDelegate(this);
    ui->tableView->setItemDelegateForColumn(0, check_box_delegatte_ptr);

    // 更新提示信息
    updateTip();

    // 项变化关联
    connect(model_ptr, &QStandardItemModel::itemChanged, this, &VideoListWidget::onItemChanged);

    // 当前行关联
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &VideoListWidget::onCurrentRowChanged);
}

void VideoListWidget::updateTip()
{
    int total_count = ui->tableView->model()->rowCount();
    int selected_count = selectedIndexes().count();

    QString text = QString(tr("<font color=red>*</font>Total:%1, Selected:%2")).arg(total_count).arg(selected_count);
    ui->labelVideoListTip->setText(text);
}

void VideoListWidget::updateDeleteUi(const QModelIndexList & selected_indexes)
{
    ui->pushButtonDelete->setEnabled(!selected_indexes.empty());
}

QModelIndexList VideoListWidget::selectedIndexes() const
{
    QModelIndexList indexes;

    QStandardItemModel* model_ptr = qobject_cast<QStandardItemModel*>(ui->tableView->model());
    if (model_ptr)
    {
        for (int i = 0; i < model_ptr->rowCount(); i++)
        {
            QStandardItem *item_ptr = model_ptr->item(i, 0);
            if (item_ptr && item_ptr->checkState() == Qt::Checked)
            {
                indexes << item_ptr->index();
            }
        }
    }

    return indexes;
}

QVariantList VideoListWidget::toVideoItems(const QModelIndexList &indexes) const
{
    QVariantList video_item_list;

    for (auto index : indexes)
    {
        video_item_list << index.data(Qt::UserRole);
    }

    return video_item_list;
}

bool VideoListWidget::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == ui->tableView->viewport() &&
		(event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease))
	{
		QMouseEvent *pMouse = dynamic_cast<QMouseEvent *>(event);
		if (pMouse && !benabled_mouse_clicked_)
		{
			pMouse->ignore();
			return true;
		}
	}
	return QWidget::eventFilter(watched, event);
}

void VideoListWidget::on_pushButtonDelete_clicked()
{
    QString msg = QString("%1\r\n%2")
            .arg(tr("Delete Video?"))
            .arg(tr("After clicking OK, the system will delete the selected video files."));
    if (UIUtils::showQuestionMsgBox(this, msg, 1))
    {
        QModelIndexList indexes = selectedIndexes();
        QVariantList video_item_list = toVideoItems(indexes);
		bool bcur_selected_video_removed = indexes.contains(ui->tableView->selectionModel()->currentIndex());

		// 删除过程中不发送信号
		ui->tableView->selectionModel()->blockSignals(true);
        for (int i = indexes.size() - 1; i >= 0; i--)
        {
            ui->tableView->model()->removeRow(indexes.at(i).row());
        }

		// 重新设置当前选中行
		if (bcur_selected_video_removed)
		{
			int current_row = ui->tableView->model()->rowCount() - 1;
			ui->tableView->selectRow(current_row);
			onCurrentRowChanged(ui->tableView->model()->index(current_row, 0), QModelIndex());
		}
		ui->tableView->selectionModel()->blockSignals(false);

        // 更新提示信息
        updateTip();

        // 更新删除界面
        updateDeleteUi(selectedIndexes());

        emit itemsRemoved(video_item_list);
    }
}

void VideoListWidget::onItemChanged(QStandardItem *item)
{
    QModelIndexList selected_indexes = selectedIndexes();
    updateDeleteUi(selected_indexes);

    // 更新提示信息
    updateTip();

    emit selectedItemsChanged(toVideoItems(selected_indexes));
}

void VideoListWidget::onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
	// 修改视频时设置为不可用
	if (!benabled_mouse_clicked_)
		return;

	benabled_mouse_clicked_ = false;
    emit currentItemChanged(ui->tableView->model()->index(current.row(), 0).data(Qt::UserRole));
}
