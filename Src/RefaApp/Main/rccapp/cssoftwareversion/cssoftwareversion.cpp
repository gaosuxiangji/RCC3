#include "cssoftwareversion.h"
#include "../../../version/version.h"
#include "Common/UIExplorer/uiexplorer.h"
#include "csopensourcelicensedlg.h"

CSSoftwareVersion::CSSoftwareVersion(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	InitUI();
	ConnectSignalAndSlot();
}

void CSSoftwareVersion::InitUI()
{
	//设置弹框的尺寸
	this->setFixedSize(420, 190);
	setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
	setWindowTitle(tr("About"));
	ui.label->setText(UIExplorer::instance().getProductFullName());
	ui.ProductVersionLabel->setText(VER_PRODUCT_VERSION_STR);
	ui.CopyrightLabel->setText(VER_COPYRIGHT_STR);

	// [2023/12/20 rgq]: 暂时隐藏开源代码
	ui.LicenseBtn->setHidden(true);
}

void CSSoftwareVersion::ConnectSignalAndSlot()
{
	connect(ui.CertainBtn, &QPushButton::clicked, [this]() {close(); });
	connect(ui.LicenseBtn, &QPushButton::clicked, this, &CSSoftwareVersion::slotLicense);
}

void CSSoftwareVersion::slotLicense()
{
	CSOpenSourceLicenseDlg openSourceLicense(this);
	openSourceLicense.resize(1066, 600);
	openSourceLicense.exec();
}