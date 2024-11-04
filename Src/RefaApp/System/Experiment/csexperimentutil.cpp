#include "csexperimentutil.h"
#include <QDateTime>
QString CSExperimentUtil::GenerateExpCode()
{
	return QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
}
