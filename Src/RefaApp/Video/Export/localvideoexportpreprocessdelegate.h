#ifndef LOCALVIDEOEXPORTPREPROCESSDELEGATE_H
#define LOCALVIDEOEXPORTPREPROCESSDELEGATE_H

#include "SingleVideoExportPreprocessDelegate.h"
#include <memory>
#include <QRect>
#include <QStringList>

class ISPUtil;

class LocalVideoExportPreprocessDelegate : public SingleVideoExportPreprocessDelegate
{
	Q_OBJECT

public:
	struct LocalExtraParams
	{
		QRect roi;// ͼ��ü�roi����
		bool benabled_watermark{ false };// �Ƿ����ˮӡ
		QString video_name;// ��Ƶ����
	};

public:
    LocalVideoExportPreprocessDelegate(std::shared_ptr<SingleVideoExportParam> param, QObject *parent = Q_NULLPTR);

	/** @brief ����������������ü��󴰿ڣ��궨���
	@param [in] : params : const LocalExtraParams &��ԭʼ����
	*/
	void setExtraParams(const LocalExtraParams & params);

	/** @brief ��ȡһ֡ͼ��
	@param [in] : FRAME_INDEX frameIndex : ֡��
		   [out] : RMAImage& image : ͼ��
	@return : bool : ��ȡͼ���Ƿ�ɹ�
	@note : �ú��������ISPģ���ͼ����д���
	*/
	bool GetFrame(FRAME_INDEX frameIndex, RMAImage& image) const override;

private:
	/**
	*@brief ����osd��Ϣ
	*@param [in/out] : RMAImage& image : ͼ��
	**/
	void paintOSD(RMAImage& image) const;

	/**
	*@brief ��ȡosd��Ϣ
	*@param [in] : const RMAImage& image : ͼ��
	*@return : QStringList : osd��Ϣ
	**/
	inline QStringList getOsdInfo(const RMAImage& image) const;

private:
	LocalExtraParams extra_params_;
	static const int kWatermarkPadding{ 5 };
};

#endif // LOCALVIDEOEXPORTPREPROCESSDELEGATE_H