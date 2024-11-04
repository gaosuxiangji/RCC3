#include "cswindowcrosslinesetting.h"
#include "ui_cswindowcrosslinesetting.h"
#include <QRegExpValidator>
CSWindowCrossLineSetting::CSWindowCrossLineSetting(/*QSharedPointer<Device> device, */QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::CSWindowCrossLineSetting)
{
    ui->setupUi(this);
    InitUI();
	ConnectSignalSlot();
}

CSWindowCrossLineSetting::~CSWindowCrossLineSetting()
{
    delete ui;
}

void CSWindowCrossLineSetting::SetCustomCrossLineCenterPointLineEditText(const QPointF& centerpoint)
{
	auto devicePtr = DeviceManager::instance().getCurrentDevice();

	if (devicePtr->getState() == Acquiring) {
		_offset = devicePtr->getProperty(Device::PropRoi).toRectF().topLeft().toPoint();
	}
	else {
		_offset = QPoint{ 0,0, };
	}
	if (devicePtr->isGrabberDevice()) {
		_offset = devicePtr->getProperty(Device::PropRoi).toRectF().bottomRight().toPoint();
		QPoint point(_offset.x() / 2, _offset.y() / 2);
		_offset = point;
		ui->CrossLineCenterPoint_X_LineEdit->setText(QString::number(_offset.x()));
		ui->CrossLineCenterPoint_Y_LineEdit->setText(QString::number(_offset.y()));
	}
	else {
		ui->CrossLineCenterPoint_X_LineEdit->setText(QString::number(centerpoint.x() - _offset.x()));
		ui->CrossLineCenterPoint_Y_LineEdit->setText(QString::number(centerpoint.y() - _offset.y()));
	}

	if (centerpoint.x() < 0) {
		ui->CrossLineCenterPoint_X_LineEdit->setText(QString::number(0));
	}

	if (centerpoint.y() < 0) {
		ui->CrossLineCenterPoint_Y_LineEdit->setText(QString::number(0));
	}
}

void CSWindowCrossLineSetting::UpdateTranslate()
{
	ui->TitleLabel->setText(tr("Center Point Pixel Coordinates"));
	ui->CenterLocationBtn->setText(tr("Central Location"));
	this->setWindowTitle(tr("Center Line Setting"));
}

void CSWindowCrossLineSetting::SlotWindowCrossLineSettingOpened()
{
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (nullptr != device_ptr)
	{
		if (device_ptr->getState() == Acquiring) {
			_offset = device_ptr->getProperty(Device::PropRoi).toRectF().topLeft().toPoint();
		}
		else {
			_offset = QPoint{ 0,0, };
		}
		if (device_ptr->isGrabberDevice()) {
			_offset = device_ptr->getProperty(Device::PropRoi).toRectF().bottomRight().toPoint();
			QPoint point(_offset.x() / 2, _offset.y() / 2);
			_offset = point;
			ui->CrossLineCenterPoint_X_LineEdit->setText(QString::number(_offset.x()));
			ui->CrossLineCenterPoint_Y_LineEdit->setText(QString::number(_offset.y()));
		}
		else {
			QPointF centerPointF = device_ptr->getProperty(Device::PropType::PropFocusPoint).toPointF() - _offset;
			ui->CrossLineCenterPoint_X_LineEdit->setText(QString::number(centerPointF.x()));
			ui->CrossLineCenterPoint_Y_LineEdit->setText(QString::number(centerPointF.y()));
		}
		//if (centerPointF.x() < 0) {
		//	ui->CrossLineCenterPoint_X_LineEdit->setText(QString::number(0));
		//}

		//if (centerPointF.y() < 0) {
		//	ui->CrossLineCenterPoint_Y_LineEdit->setText(QString::number(0));
		//}
	}
}

void CSWindowCrossLineSetting::InitUI()
{
    //设置弹框的尺寸
    this->setFixedSize(240, 140);

    //隐藏标题栏的问号和图标，WindowStaysOnTopHint-让窗口浮在最上方
    //this->setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint | Qt::Drawer);
	this->setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint | Qt::Drawer);

	//只允许输入数字
	//ui->CrossLineCenterPoint_X_LineEdit->setValidator(new QRegExpValidator(QRegExp("^[0-9]*$")));
	//ui->CrossLineCenterPoint_Y_LineEdit->setValidator(new QRegExpValidator(QRegExp("^[0-9]*$")));
	ui->CrossLineCenterPoint_X_LineEdit->setValidator(new QRegExpValidator(QRegExp("(^-?[0-9]+$)|([-])")));
	ui->CrossLineCenterPoint_Y_LineEdit->setValidator(new QRegExpValidator(QRegExp("(^-?[0-9]+$)|([-])")));
}

void CSWindowCrossLineSetting::ConnectSignalSlot()
{
	bool ok = connect(ui->CrossLineCenterPoint_X_LineEdit, &QLineEdit::editingFinished, this, &CSWindowCrossLineSetting::on_CrossLineCenterPoint_X_LineEdit_editingFinished);
	ok = connect(ui->CrossLineCenterPoint_X_LineEdit, &QLineEdit::textChanged, this, [=](const QString & text) {
		if (text.isEmpty()) {
			ui->CrossLineCenterPoint_X_LineEdit->setText(QString::number(0));
		}
	});
	ok = connect(ui->CrossLineCenterPoint_Y_LineEdit, &QLineEdit::editingFinished, this, &CSWindowCrossLineSetting::on_CrossLineCenterPoint_Y_LineEdit_editingFinished);
	ok = connect(ui->CrossLineCenterPoint_Y_LineEdit, &QLineEdit::textChanged, this, [=](const QString & text) {
		if (text.isEmpty()) {
			ui->CrossLineCenterPoint_Y_LineEdit->setText(QString::number(0));
		}
	});
	ok = connect(this, &CSWindowCrossLineSetting::SignalWindowCrossLineSettingOpened, this, &CSWindowCrossLineSetting::SlotWindowCrossLineSettingOpened);
}

QPointF CSWindowCrossLineSetting::GetCrossLineCenterPointF() const
{
	QPointF point = { QPointF(ui->CrossLineCenterPoint_X_LineEdit->text().toInt() ,ui->CrossLineCenterPoint_Y_LineEdit->text().toInt()) };
	return point;
}

void CSWindowCrossLineSetting::on_CrossLineCenterPoint_X_LineEdit_editingFinished()
{
	//多个字符字且首字符为0时，去除0
	if (ui->CrossLineCenterPoint_X_LineEdit->text().toInt() + _offset.x() < 0 || ui->CrossLineCenterPoint_X_LineEdit->text().isEmpty()) {
		ui->CrossLineCenterPoint_X_LineEdit->setText("0");
	}
	else {
		ui->CrossLineCenterPoint_X_LineEdit->setText(QString::number(ui->CrossLineCenterPoint_X_LineEdit->text().toInt()));
	}
	auto devicePtr = DeviceManager::instance().getCurrentDevice();
	if (nullptr != devicePtr)
	{
		devicePtr->setProperty(Device::PropFocusPoint, QVariant::fromValue(GetCrossLineCenterPointF() + _offset));
	}
}

void CSWindowCrossLineSetting::on_CrossLineCenterPoint_Y_LineEdit_editingFinished()
{
	//多个字符字且首字符为0时，去除0
	if (ui->CrossLineCenterPoint_Y_LineEdit->text().toInt() + _offset.y() < 0 || ui->CrossLineCenterPoint_Y_LineEdit->text().isEmpty()) {
		ui->CrossLineCenterPoint_Y_LineEdit->setText("0");
	}
	else {
		ui->CrossLineCenterPoint_Y_LineEdit->setText(QString::number(ui->CrossLineCenterPoint_Y_LineEdit->text().toInt()));
	}
	auto devicePtr = DeviceManager::instance().getCurrentDevice();
	if (nullptr != devicePtr)
	{
		devicePtr->setProperty(Device::PropFocusPoint, QVariant::fromValue(GetCrossLineCenterPointF() + _offset));
	}
}

void CSWindowCrossLineSetting::on_CenterLocationBtn_clicked()
{
	emit SignalGetPictureCenterPoint();
}

void CSWindowCrossLineSetting::on_CrossLineCenterPoint_X_LineEdit_textChanged(const QString &arg1)
{
	if (arg1.isEmpty()) return;
	auto devicePtr = DeviceManager::instance().getCurrentDevice();
	if (!devicePtr) return;

	HscIntRange width_range{};
	if (!devicePtr->getRoiWidthRange(0, width_range))
	{
		return;
	}
	int64_t nMax = width_range.max - _offset.x();
	if (nMax > 0)
	{
		nMax--;
	}
	int nValue = arg1.toInt();
	if (nValue > nMax)
	{
		ui->CrossLineCenterPoint_X_LineEdit->setText(QString::number(nMax));
	}
	else if (nValue < -_offset.x())
	{
		ui->CrossLineCenterPoint_X_LineEdit->setText(QString::number(-_offset.x()));
	}
}

void CSWindowCrossLineSetting::on_CrossLineCenterPoint_Y_LineEdit_textChanged(const QString &arg1)
{
	if (arg1.isEmpty()) return;
	auto devicePtr = DeviceManager::instance().getCurrentDevice();
	if (!devicePtr) return;

	HscIntRange height_range{};
	if (!devicePtr->getRoiHeightRange(0, height_range))
	{
		return;
	}

	int64_t nMax = height_range.max - _offset.y();
	if (nMax > 0)
	{
		nMax--;
	}
	int nValue = arg1.toInt();
	if (nValue > nMax)
	{
		ui->CrossLineCenterPoint_Y_LineEdit->setText(QString::number(nMax));
	}
	else if (nValue < -_offset.y())
	{
		ui->CrossLineCenterPoint_Y_LineEdit->setText(QString::number(-_offset.y()));
	}
}
