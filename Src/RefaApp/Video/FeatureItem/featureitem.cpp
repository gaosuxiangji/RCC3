#include "featureitem.h"

FeatureItem::FeatureItem()
{

}

FeatureItem::FeatureItem(const QVariant &id, const QVariant &value) : id_(id), value_(value)
{

}

FeatureItem::FeatureItem(const QVariant &id, const QVariant &value, const QVariant &user_data) : id_(id), value_(value), user_data_(user_data)
{

}

void FeatureItem::setId(const QVariant &id)
{
    id_ = id;
}

QVariant FeatureItem::getId() const
{
    return id_;
}

void FeatureItem::setValue(const QVariant &value)
{
    value_ = value;
}

QVariant FeatureItem::getValue() const
{
    return value_;
}

int FeatureItem::getType() const
{
    if (value_.type() == QVariant::Point)
    {
        return Type::FeaturePoint;
    }
    else if (value_.type() == QVariant::Line)
    {
        return Type::FeatureLine;
    }
    else
    {
        return Type::FeatureInvalid;
    }
}

bool FeatureItem::isValid() const
{
    return id_.isValid() && getType() != Type::FeatureInvalid;
}

void FeatureItem::setUserData(const QVariant &user_data)
{
    user_data_ = user_data;
}

QVariant FeatureItem::getUserData() const
{
    return user_data_;
}
