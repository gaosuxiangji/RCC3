#ifndef QIPLINEEDIT_H
#define QIPLINEEDIT_H

#include <QEvent>
#include <QLabel>
#include <QLineEdit>

class QIPLineEdit : public QLineEdit {
  Q_OBJECT
public:
  explicit QIPLineEdit(int type=0, QWidget *parent = 0);
  ~QIPLineEdit();

  void setReadOnly(bool);

  void setText(const QString &strIP);
  QString text() const;
  void clearText();

  bool isTextValid(const QString &strIP);

protected:
  void paintEvent(QPaintEvent *event);
  bool eventFilter(QObject *obj, QEvent *ev);

  int getIndex(QLineEdit *pEdit);

private:
  QLineEdit *m_lineEdit[4];
  int m_type{ 0 }; //1-设备列表，添加
signals:

public slots:
};

#endif // QIPLINEEDIT_H
