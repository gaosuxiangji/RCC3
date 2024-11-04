#pragma once

#include <opencv2/opencv.hpp> 
#include <vector>
#include <unordered_map>

struct CalibrationParams
{
	cv::Point2f worldCenterPos;//�������꣬��������ϵ���ꡣ��λ��
	cv::Point2f imageCenterPos;//�������꣬��������
	cv::Point2f burstPoint;//ը�����꣬��������

	cv::Mat homographyMatrix_camera_2_plane;//��������浽�궨ƽ��֮���͸�Ӿ���
	cv::Mat rotationMatrix_plane_2_world;//�궨ƽ�浽��������ϵ����ת����
	cv::Mat translateMatrix_plane_2_world;//�궨ƽ�浽��������ϵ��ƽ�ƾ���

	cv::Mat homographyMatrix_plane_2_camera;//�궨ƽ�浽���������֮���͸�Ӿ���
	cv::Mat rotationMatrix_world_2_plane;//��������ϵ���궨ƽ�����ת����
	cv::Mat translateMatrix_world_2_plane;//��������ϵ���궨ƽ���ƽ�ƾ���

	//cv::Mat distCoeffs;//�������ϵ��

	enum ResultPlaneType
	{
		RES_PLANE_TYPE_CALI,    //�궨ƽ��
		RES_PLANE_TYPE_GROUND   //����
	};
	ResultPlaneType plane;

	std::string fileName;//�궨�ļ�����
	std::string cameraIp;//���IP
	std::string cameraType;//����ͺ�
};

enum LegendsType
{
	GRID_LEGEND,
	CIRCLE_LEGEND
};

struct EllipseLegendsParams
{
	cv::Point2f centerPos; //��Բ��������
	cv::Size2f axes;//���̰���
	float rotationAngle;//��Բ��ǣ���λ��
	float startAngle;//��ʼ�Ƕȣ���λ��
	float endAngle;//�����Ƕȣ���λ��
	float distanceTick;//����̶ȣ���λ��
};

struct LegendsParams
{
	int minDist;
	int maxDist;
	int distInterval;
	LegendsType legendsType;
	std::vector<std::vector<cv::Point2f>> lineLegends;//ֱ�߲���
	std::vector<EllipseLegendsParams> ellipseLegends;//���߲���

	void clear()
	{
		lineLegends.clear();
		ellipseLegends.clear();
	}

	void pushBackLineLegend(std::vector<cv::Point2f> lineLegend)
	{
		lineLegends.emplace_back(lineLegend);
	}

	void pushBackEllipseLegend(EllipseLegendsParams ellipseLegend)
	{
		ellipseLegends.emplace_back(ellipseLegend);
	}
};

struct CameraParams
{
	CalibrationParams* pCalibrationParams=nullptr;//�궨����
	LegendsParams* pLegendsParams= nullptr;//�����߲���

	CameraParams()
	{
		pCalibrationParams = new CalibrationParams();
		pLegendsParams = new LegendsParams();
	}

	~CameraParams()
	{
		if (pCalibrationParams != nullptr)
		{
			delete pCalibrationParams; pCalibrationParams = nullptr;
		}

		if (pLegendsParams != nullptr)
		{
			delete pLegendsParams; pLegendsParams = nullptr;
		}
	}
};

struct RetrievalCalibrationInfo
{
	std::string fileName;
	std::string cameraType;
	std::string cameraIp;

	RetrievalCalibrationInfo(std::string _fileName, std::string _cameraType,std::string _cameraIp)
		:fileName(_fileName),
		cameraType(_cameraType),
		cameraIp(_cameraIp)
	{
	}
};

class  __declspec(dllexport)  FallPointMeasure
{
private:
	FallPointMeasure();

public:
	~FallPointMeasure();

	static FallPointMeasure& GetInstance();

	//ʹ������������
	void EnableFallPointMeasure(bool b);

	//�ж��Ƿ�ʹ������������
	bool isEnableFallPointMeasure() const;

	//����궨����
	int ImportCalibrationParams(const std::string& filePath, std::string& cameraID, std::string& cameraType);

	//�����������ģ�centerPos���������ꡣ�˺������������������߲���
	bool setCenterPos(cv::Point2f centerPos);

	//��ȡ�������ģ�centerPos����������
	bool getCenterPos(cv::Point2f& centerPos) const;

	//���ùؼ�֡���
	void setKeyFrameIndex(int64_t keyFrameIndex);

	//����ը�����꣬��λ���ء�������ը���⺯����Ҳ���޸�ը��λ��
	void setBurstPoint(const cv::Point2f& point);

	//��ȡը�����꣬��λ���ء�
	cv::Point2f getBurstPoint() const;

	//����������
	void DrawLegends(cv::Mat& srcImg,bool legendsVisible, int64_t frameIndex/*,bool isColorPic*/) const;

	//ը���⣬�����ɹ������޸�ը��λ�á�ը��Ϊ��������
	bool DetectBurstPoint(const std::vector<cv::Mat>& imgs, int64_t firstFrameIndex);

	//�����������ڰ��ĵ�ƫ������burstPointΪ�����������꣬fallPointΪ�������ڰ��ĵ�ƫ�������������꣬��λ��
	bool CalcFallPoint(const cv::Point2f& burstPoint, cv::Point2f& fallPoint) const;

	//ѡ��ǰʹ�õ��������
	bool SelectCameraParam(const std::string& cameraID);

	//�ж�ָ������ı궨�����Ƿ����
	bool CameraParamExisted(const std::string& cameraID) const;

	//����ָ���㵽���ĵľ��룬�����������꣬�����λ��
	bool CalcDistToCenter(const cv::Point2f& pt,float& dist) const;

	void getCalibrationParamsInfo(std::vector<RetrievalCalibrationInfo>& info) const;

	//ɾ��ָ���ı궨�����������߲���
	bool RemoveCalibrationParam(const std::string& cameraID);

	int64_t GetBurstPointFrameIndex() const;

	//���ݵ�ǰ���õİ������������������߼�ࣨ��λ�ף�������������Ĭ��Ϊ5
	float FindBestLegendInterval();

	bool SetLegendInterval(float interval);//��������Բ�������ˢ�������߲���

	void SetImageSize(const cv::Size& size);//����ͼ��ߴ�

private:
	//���ɵ��������
	bool PrepareLegendsPts(std::vector<cv::Point2f>& pts, LegendsType legendsType, cv::Point2f bullEyePos,float radius) const;//��������Բ�ϵĵ�

#if 0   //�������ֵ����Сֵ�ͼ�����ɶ��������
	__declspec(deprecated)
	bool PrepareLegendsPts(std::vector<std::vector<cv::Point2f>>& pts, std::vector<int>& distanceTicks,LegendsType legendsType, 
		cv::Point2f bullEyePos, int minDist,int maxDist, int distInterval);
#endif

	bool FitEllipse(const std::vector<cv::Point2f>& pts, EllipseLegendsParams* ellipseLegends) const;

	bool ImageDiff(const cv::Mat& srcImg1, const cv::Mat& srcImg2, cv::Mat& dstImg, double threshold) const;

	bool FindContour(const cv::Mat& srcImg, std::vector<cv::Point>& contour) const;

	bool FindLowerPoint(const std::vector<cv::Point>& contour, cv::Point2f& lowerPoint, float& ratio) const;

	//��ͼ������ӳ��Ϊ�������꣬�����λ��
	bool Image2World(const cv::Point2f& imagePts, cv::Point2f& worldPts) const;

	//����������ӳ��Ϊͼ�����꣬���뵥λ��
	bool World2Image(const cv::Point2f& worldPt, const CalibrationParams* pCalibrationParams, cv::Point2f& imagePt) const;

	//��������������ӳ��Ϊͼ�����꣬���뵥λ��
	bool World2Image(const std::vector<cv::Point2f>& worldPts, const CalibrationParams* pCalibrationParams,std::vector<cv::Point2f>& imagePts);

#if 0
	void CreateEllipseLegendsParams(const std::vector<std::vector<cv::Point2f>>& worldPts, const std::vector<int>& distanceTicks, const CalibrationParams* pCalibrationParams, LegendsParams* pLegendsParams);
#endif

	//����ʽ������Բ������
	bool CreateEllipseLegendsParams(const std::string& cameraID);

private:
	bool isFallPointMeasureEnabled;//�����������Ƿ�ʹ��
	std::string curCameraID;//��ǰѡ������ID
	CalibrationParams* pCurCalibrationParams = nullptr;//��ǰѡ��ı궨����������Ϊ���ݵ�ָ�룬����Ҫ�ͷ�
	LegendsParams* pCurLegendsParams = nullptr;//��ǰѡ��������߲���������Ϊ���ݵ�ָ�룬����Ҫ�ͷ�
	std::unordered_map<std::string, CameraParams*> mapCameraParams;//���б궨�����������߲����Ļ���
	int64_t burstPointFrameIndex;//ը�����ڵ�֡
	float legendInterval = -1;//�̶��߼������λ�ף�������Բ���
	cv::Size imageSize;//ͼ��ߴ�
	const cv::Scalar red = cv::Scalar(0, 0, 255);//��ɫ�������ߡ��е����ġ�ը�����ɫ
	const cv::Scalar gray = cv::Scalar(177, 177, 34);//��ɫ���̶��ߵ���ɫ
};



