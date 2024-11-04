#include "csaboutqtopensourcedlg.h"
#include "ui_csaboutqtopensourcedlg.h"

CSAboutQtOpenSourceDlg::CSAboutQtOpenSourceDlg(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui::CSAboutQtOpenSourceDlg();
	ui->setupUi(this);

	Init();
}

CSAboutQtOpenSourceDlg::~CSAboutQtOpenSourceDlg()
{
	delete ui;
}

void CSAboutQtOpenSourceDlg::Init()
{
	setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
	ui->textBrowser->setText(GetQtOpenSourceText());

	connect(ui->pushButton_OK, &QPushButton::clicked, [this]() {close(); });
}

QString CSAboutQtOpenSourceDlg::GetQtOpenSourceText()
{
	QString strText;

	strText = "About Qt\n";
	strText += "This program uses Qt version 5.7.0.\n";

	strText += "Qt is a C++ toolkit for cross-platform application development.\n";

	strText += R"(Qt is available under different licensing options designed to accommodate the needs of our various users.)";

	strText += R"(Qt licensed under commercial licenses are appropriate for development of proprietary/commercial software where you do not want to share any source code with third parties or otherwise cannot comply with the terms of the GNU LGPL version 3.)";

	strText += R"(Qt licensed under the GNU Lesser General Public License (LGPL) version 3 is appropriate for the development of Qt applications provided you can comply with the terms and conditions of the GNU LGPL version 3 (or GNU GPL version 3).)";

	strText += R"(Note: Some specific parts (modules) of the Qt framework are not available under the GNU LGPL version 3, but under the GNU General Public License (GPL) instead. See Licenses Used in Qt for details.)";

	strText += R"(Qt documentation is licensed under the terms of the GNU Free Documentation License (FDL) version 1.3, as published by the Free Software Foundation. Alternatively, you may use the documentation in accordance with the terms contained in a written agreement between you and The Qt Company.)";

	strText += R"(See http://qt.io/licensing/ for an overview of Qt licensing. )";
	return strText;
}