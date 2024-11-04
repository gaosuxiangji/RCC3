#ifndef KEYBOARDCONTROL_H
#define KEYBOARDCONTROL_H

#include <QRect>

/**
*@brief 软键盘显示控制类
**/
class KeyBoardControl
{
public:
	/**
	*@brief 打开软键盘
	**/
	static void OpenKeyBoard();

	/**
	*@brief 关闭软键盘
	**/
	static void CloseKeyBoard();

	/**
	*@brief 移动软键盘
	*param [in] : rc : const QRect &，键盘区域
	**/
	static void MoveKeyBoard(const QRect & rc);
};

#endif // KEYBOARDCONTROL_H