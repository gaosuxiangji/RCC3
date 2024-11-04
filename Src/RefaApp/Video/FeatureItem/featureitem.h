#ifndef FEATUREITEM_H
#define FEATUREITEM_H

#include <QVariant>

/**
 * @brief 特征项类
 */
class FeatureItem
{
public:
    /**
     * @brief 特征类型枚举
     */
    enum Type
    {
        FeatureInvalid, // 无效
        FeaturePoint, // 特征点
        FeatureLine, // 特征线
    };

    FeatureItem();
    FeatureItem(const QVariant & id, const QVariant & value);
    FeatureItem(const QVariant & id, const QVariant & value, const QVariant & user_data);

    /**
     * @brief 设置ID
     * @param id ID
     */
    void setId(const QVariant & getId);

    /**
     * @brief 获取ID
     * @return ID
     */
    QVariant getId() const;

    /**
     * @brief 设置值
     * @param value 值：特征点-QPoint，特征线-QLine
     */
    void setValue(const QVariant & getValue);

    /**
     * @brief 获取值
     * @return 值
     */
    QVariant getValue() const;

    /**
     * @brief 获取类型
     * @return 类型
     */
    int getType() const;

    /**
     * @brief 是否有效
     * @return true-有效, false-无效
     */
    bool isValid() const;

    /**
     * @brief 设置用户数据
     * @param 用户数据：测量结果-QStringList
     */
    void setUserData(const QVariant & user_data);

    /**
     * @brief 获取用户数据
     * @return 用户数据：测量结果-QStringList
     */
    QVariant getUserData() const;

private:
    QVariant id_;
    QVariant value_;
    QVariant user_data_;
};

#endif // FEATUREITEM_H
