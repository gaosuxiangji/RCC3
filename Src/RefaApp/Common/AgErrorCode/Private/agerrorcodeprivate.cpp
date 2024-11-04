#include "agerrorcodeprivate.h"

#include "qtcsv/reader.h"

AgErrorCodePrivate::AgErrorCodePrivate()
{

}

bool AgErrorCodePrivate::reset(int lang)
{
    error_code_map_.clear();

    QString csv_path = QString(":/errorcode/%1/error_code.csv").arg(lang == 1 ? "en_US" : "zh_CN");
	QList<QStringList> data = QtCSV::Reader::readToList(csv_path);
    for (auto line_list : data)
    {
        if (line_list.isEmpty())
        {
            continue;
        }

        if (line_list.first().startsWith("#"))
        {
            continue;
        }

        int index = 0;
        bool ok = false;
        uint32_t value = line_list.at(index++).toUInt(&ok, 16);
        if (!ok)
        {
            continue;
        }

        QSharedPointer<AgErrorCodeInfo> info_ptr(new AgErrorCodeInfo);
        info_ptr->value = value;

        QString type;
        if (index < line_list.size())
        {
            type = line_list.at(index++).trimmed();
        }

        int level = 0;
        if (index < line_list.size())
        {
            level = line_list.at(index++).toInt();
        }
        if (level == 2)
        {
            info_ptr->level = 2;
        }
        else if (level == 3)
        {
            info_ptr->level = 1;
        }
        else
        {
            info_ptr->level = 0;
        }

        QString desc;
        if (index < line_list.size())
        {
            desc = line_list.at(index++).trimmed();
        }



        if (!desc.isEmpty())
        {

            info_ptr->desc = desc;
        }
        else
        {
            info_ptr->desc = type;
        }

        error_code_map_.insert(info_ptr->value, info_ptr);
    }

    return true;
}

bool AgErrorCodePrivate::get(uint32_t value, AgErrorCodeInfo &info)
{
	uint32_t query_error_code = value;
	if (isArmError(query_error_code))
	{
		query_error_code &= 0xffff00ff;
	}
	
    auto info_ptr = error_code_map_.value(query_error_code);
    if (info_ptr)
    {
        info = *info_ptr;

		uint8_t dev_addr = (value >> 8) & 0xff;
		if (dev_addr != 0)
		{
			info.desc += QString("(0x%1)").arg((uint)dev_addr,2,16,QChar('0'));
		}

        return true;
    }

    return false;
}

bool AgErrorCodePrivate::isArmError(uint32_t error_code) const
{
	return ((error_code >> 24) & 0x7F) == 1;
}
