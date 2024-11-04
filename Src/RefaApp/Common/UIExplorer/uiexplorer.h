#ifndef UIEXPLORER_H
#define UIEXPLORER_H

#include <QString>

class QSettings;

/**
 * @brief 界面资源管理类
 */
class UIExplorer
{
public:
    static UIExplorer& instance();

	~UIExplorer();

    /**
     * @brief 获取产品名称
     * @return 产品名称
     */
    QString getProductName() const;

	/**
	* @brief 获取产品名称
	* @return 产品名称
	*/
	QString getProductFullName() const;

    /**
     * @brief 获取产品版本
     * @return 产品版本
     */
    QString getProductVersion() const;

    /**
     * @brief 获取公司名称
     * @return 公司名称
     */
    QString getCompanyName() const;

	/**
	* @brief 重置
	* @param lang 语言类型：0-中文，1-英文
	* @return true-成功，false-失败
	*/
	void reset(int lang);

	/**
	* @brief 获取id对应的字符串
	* @param id 字符串ID
	* @return id对应字符串
	*/
	QString getStringById(const QString id) const;

private:
    UIExplorer();

	UIExplorer(const UIExplorer &p) = delete;
	UIExplorer &operator=(const UIExplorer &p) = delete;

	QSettings * m_setting_ptr;
};

#endif // UIEXPLORER_H
