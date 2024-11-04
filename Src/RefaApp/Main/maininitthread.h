#ifndef MAININITTHREAD_H
#define MAININITTHREAD_H

#include <QThread>

/**
 * @brief 主初始化线程类
 */
class MainInitThread : public QThread
{
    Q_OBJECT
public:
    MainInitThread();

private:
    void run() override;
};

#endif // MAININITTHREAD_H
