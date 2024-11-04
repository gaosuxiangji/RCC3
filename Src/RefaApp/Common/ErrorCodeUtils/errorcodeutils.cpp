#include "errorcodeutils.h"

#include <QString>

#include "Common/UIUtils/uiutils.h"
#include "Video/VideoItemManager/videoitemmanager.h"
#include "Common/AgErrorCode/agerrorcode.h"

void ErrorCodeUtils::handle(QWidget *parent_ptr, quint64 error_code, const QVariant & video_id,QString extTip)
{
    QString caption = QObject::tr("Error Code");
    QString value = QString("0x%1").arg(error_code, 8, 16, QChar('0'));
	QString video_desc = VideoItemManager::instance().getVideoItem(video_id).getName();

	QString error_desc;
	int level = 0;
	AgErrorCodeInfo error_code_info{};
	if (AgErrorCode::instance().get(error_code, error_code_info))
	{
		error_desc = error_code_info.desc;
		level = error_code_info.level;
	}

    QString msg_text = QString("%1(%2): %3 %4 %5").arg(caption).arg(value).arg(video_desc).arg(error_desc).arg(extTip);
    msg_text = msg_text.trimmed();
    if (msg_text.endsWith(":"))
    {
        msg_text.chop(1);
    }

   
    switch(level)
    {
    case 1: // 警告
        UIUtils::showWarnMsgBox(parent_ptr, msg_text);
        break;
    case 2: // 错误
        UIUtils::showErrorMsgBox(parent_ptr, msg_text);
        break;
    default:
        UIUtils::showInfoMsgBox(parent_ptr, msg_text);
        break;
    }
}
