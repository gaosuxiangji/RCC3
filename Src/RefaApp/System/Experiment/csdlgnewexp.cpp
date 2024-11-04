#include "csdlgnewexp.h"
#include "ui_csdlgnewexp.h"

CSDlgNewExp::CSDlgNewExp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSDlgNewExp)
{
    ui->setupUi(this);
	InitUI();
}

CSDlgNewExp::~CSDlgNewExp()
{
    delete ui;
}

void CSDlgNewExp::on_buttonBox_accepted()
{
	m_exp.name = ui->lineEditName->text();
	m_exp.info = ui->textEditDescription->toPlainText();
	accept();
}

void CSDlgNewExp::on_buttonBox_rejected()
{
	reject();
}

void CSDlgNewExp::InitUI()
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QString exp_code = CSExperimentUtil::GenerateExpCode();
	ui->lineEditId->setText(exp_code);
	m_exp.code = exp_code;

	ui->lineEditName->setMaxLength(m_maxNameCount);
	ui->lineEditName->setFocus();//初始焦点
	

}

void CSDlgNewExp::on_textEditDescription_textChanged()
{
	//文本框最大值限制
	auto textEdit = static_cast<QPlainTextEdit*>(sender());

	int currentCount = textEdit->toPlainText().size();
	if (currentCount > m_maxWordCount)
	{
		QString text = textEdit->toPlainText();
		text = text.mid(0, m_maxWordCount);

		int position = textEdit->textCursor().position();

		textEdit->setPlainText(text);
		
		QTextCursor cursor = textEdit->textCursor();
		if (position > m_maxWordCount)
		{
			position = m_maxWordCount;
		}
		cursor.setPosition(position);
		textEdit->setTextCursor(cursor);
	}
}
