#include "qiplineedit.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>
#include <QRegExpValidator>

QIPLineEdit::QIPLineEdit(int type, QWidget *parent) : QLineEdit(parent)
,m_type(type){
  QRegExp rx("(2[0-5]{2}|2[0-4][0-9]|1?[0-9]{1,2})");
  QHBoxLayout *pHBox = new QHBoxLayout(this);
  pHBox->setSpacing(10);
  pHBox->setContentsMargins(0, 0, 0, 0);
  

  for (int i = 0; i < 4; i++) {
    m_lineEdit[i] = new QLineEdit(this);
	switch (i)
	{
	case 0:
		m_lineEdit[i]->setStyleSheet("QLineEdit{border-left-width:1;border-right-width:0;border-top-width:1;border-bottom-width:1;border-style:outset}");
		break;
	case 1:
	case 2:
		m_lineEdit[i]->setStyleSheet("QLineEdit{border-left-width:0;border-right-width:0;border-top-width:1;border-bottom-width:1;border-style:outset}");
		break;
	case 3:
		m_lineEdit[i]->setStyleSheet("QLineEdit{border-left-width:0;border-right-width:1;border-top-width:1;border-bottom-width:1;border-style:outset}");
		break;
	default:
		break;
	}
	m_lineEdit[i]->setFrame(false);
	m_lineEdit[i]->setMaxLength(3);
	m_lineEdit[i]->setAlignment(Qt::AlignCenter);
	m_lineEdit[i]->installEventFilter(this);
	m_lineEdit[i]->setValidator(new QRegExpValidator(rx, this));
	m_lineEdit[i]->setSizePolicy(QSizePolicy::Preferred,
		QSizePolicy::Preferred);

    pHBox->addWidget(m_lineEdit[i]);
  }

  //    this->setReadOnly(true);
}

QIPLineEdit::~QIPLineEdit() {}

void QIPLineEdit::setReadOnly(bool b)
{
	for (int i = 0; i < 4; i++) {
		m_lineEdit[i]->setReadOnly(b);
	}
	QLineEdit::setReadOnly(b);
}

void QIPLineEdit::paintEvent(QPaintEvent *event) {
  QLineEdit::paintEvent(event);

  QPainter painter(this);

  QPen pen;
  pen.setWidth(2);
  pen.setColor(Qt::black);
  painter.setPen(pen);

  int width = 0;
  for (int i = 0; i < 3; i++) {
    width += m_lineEdit[i]->width() + (i == 0 ? 3 : 10);
	painter.drawPoint(width, height() / 2);
  }
}

int QIPLineEdit::getIndex(QLineEdit *pEdit) {
  int index = -1;
  for (int i = 0; i < 4; i++) {
    if (pEdit == m_lineEdit[i]) {
      index = i;
      break;
    }
  }

  return index;
}

bool QIPLineEdit::eventFilter(QObject *obj, QEvent *ev) {
  if (children().contains(obj) && QEvent::KeyPress == ev->type()) {
    QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(ev);
    QLineEdit *pEdit = qobject_cast<QLineEdit *>(obj);
	if (!keyEvent || !pEdit)
	{
		return false;
	}

    switch (keyEvent->key()) {
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9: {
      QString strText = pEdit->text();
      if (pEdit->selectedText().length()) {
        pEdit->text().replace(pEdit->selectedText(), QChar(keyEvent->key()));
      } else if (strText.length() == 3 ||
                 (strText.length() < 3 && strText.toInt() * 10 > 255)) {
        int index = getIndex(pEdit);
        if (index != -1 && index != 3) {
          m_lineEdit[index + 1]->setFocus();
          m_lineEdit[index + 1]->selectAll();
        }
      } else if (strText.length() == 2 && strText.toInt() * 10 < 255) {
        if (Qt::Key_0 == keyEvent->key() && strText.toInt()) {
          pEdit->setText(
              strText.insert(pEdit->cursorPosition(), QChar(Qt::Key_0)));
        }
      }

      return QLineEdit::eventFilter(obj, ev);

      break;
    }
    case Qt::Key_degree: {
      qDebug("delete");

      return QLineEdit::eventFilter(obj, ev);

      break;
    }
    case Qt::Key_Backspace: {
      QString strText = pEdit->text();
      if (!strText.length() || (strText.length() && !pEdit->cursorPosition())) {
        int index = getIndex(pEdit);
        if (index != -1 && index != 0) {
          m_lineEdit[index - 1]->setFocus();
          int length = m_lineEdit[index - 1]->text().length();
          m_lineEdit[index - 1]->setCursorPosition(length ? length : 0);
        }
      }
      return QLineEdit::eventFilter(obj, ev);
      break;
    }
    case Qt::Key_Left: {
      if (!pEdit->cursorPosition()) {
        int index = getIndex(pEdit);
        if (index != -1 && index != 0) {
          m_lineEdit[index - 1]->setFocus();
          int length = m_lineEdit[index - 1]->text().length();
          m_lineEdit[index - 1]->setCursorPosition(length ? length : 0);
        }
      }
      return QLineEdit::eventFilter(obj, ev);
      break;
    }
      //        case Qt::Key_Delete:
	case Qt::Key_Period:
    case Qt::Key_Right: {
      if (pEdit->cursorPosition() == pEdit->text().length()) {
        int index = getIndex(pEdit);
        if (index != -1 && index != 3) {
          m_lineEdit[index + 1]->setFocus();
          m_lineEdit[index + 1]->setCursorPosition(0);
        }
      }
      return QLineEdit::eventFilter(obj, ev);
      break;
    }
    default:
      break;
    }
  }

  return false;
}

void QIPLineEdit::setText(const QString &strIP) {
  if (!isTextValid(strIP)) {
	  if (0 == m_type) {
		  QMessageBox::warning(this, "Attention", "Your IP Address is Invalid",
			  QMessageBox::Ok);
		  return;
	  }
  } else {
	  int i = 0;
	  QStringList ipList = strIP.split(".");

	  foreach(QString ip, ipList) {
		  m_lineEdit[i]->setText(ip);
		  i++;
	  }
  }
}

void QIPLineEdit::clearText() {
  for (int i = 0; i < 4; i++)
    m_lineEdit[i]->clear();
}

bool QIPLineEdit::isTextValid(const QString &strIP) {
  QRegExp rx2("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|"
              "2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");

  if (!rx2.exactMatch(strIP))
    return false;
  return true;
}

QString QIPLineEdit::text() const {
  QString strIP;
  for (int i = 0; i < 4; i++) {
    strIP.append(m_lineEdit[i]->text());
    if (i < 3)
      strIP.append(".");
  }
  return strIP;
}
