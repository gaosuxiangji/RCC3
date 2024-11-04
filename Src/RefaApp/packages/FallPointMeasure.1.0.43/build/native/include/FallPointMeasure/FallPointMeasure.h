#pragma once

#include <opencv2/opencv.hpp> 
#include <vector>
#include <unordered_map>

struct CalibrationParams
{
	cv::Point2f worldCenterPos;//靶心坐标，世界坐标系坐标。单位米
	cv::Point2f imageCenterPos;//靶心坐标，像素坐标
	cv::Point2f burstPoint;//炸点坐标，像素坐标

	cv::Mat homographyMatrix_camera_2_plane;//相机成像面到标定平面之间的透视矩阵
	cv::Mat rotationMatrix_plane_2_world;//标定平面到世界坐标系的旋转矩阵
	cv::Mat translateMatrix_plane_2_world;//标定平面到世界坐标系的平移矩阵

	cv::Mat homographyMatrix_plane_2_camera;//标定平面到相机成像面之间的透视矩阵
	cv::Mat rotationMatrix_world_2_plane;//世界坐标系到标定平面的旋转矩阵
	cv::Mat translateMatrix_world_2_plane;//世界坐标系到标定平面的平移矩阵

	//cv::Mat distCoeffs;//相机畸变系数

	enum ResultPlaneType
	{
		RES_PLANE_TYPE_CALI,    //标定平面
		RES_PLANE_TYPE_GROUND   //地面
	};
	ResultPlaneType plane;

	std::string fileName;//标定文件名称
	std::string cameraIp;//相机IP
	std::string cameraType;//相机型号
};

enum LegendsType
{
	GRID_LEGEND,
	CIRCLE_LEGEND
};

struct EllipseLegendsParams
{
	cv::Point2f centerPos; //椭圆中心坐标
	cv::Size2f axes;//长短半轴
	float rotationAngle;//椭圆倾角，单位度
	float startAngle;//起始角度，单位度
	float endAngle;//结束角度，单位度
	float distanceTick;//距离刻度，单位米
};

struct LegendsParams
{
	int minDist;
	int maxDist;
	int distInterval;
	LegendsType legendsType;
	std::vector<std::vector<cv::Point2f>> lineLegends;//直线参数
	std::vector<EllipseLegendsParams> ellipseLegends;//曲线参数

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
	CalibrationParams* pCalibrationParams=nullptr;//标定参数
	LegendsParams* pLegendsParams= nullptr;//网格线参数

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

	//使能落点测量功能
	void EnableFallPointMeasure(bool b);

	//判断是否使能落点测量功能
	bool isEnableFallPointMeasure() const;

	//导入标定参数
	int ImportCalibrationParams(const std::string& filePath, std::string& cameraID, std::string& cameraType);

	//设置网格中心，centerPos是像素坐标。此函数会重新生成网格线参数
	bool setCenterPos(cv::Point2f centerPos);

	//获取网格中心，centerPos是像素坐标
	bool getCenterPos(cv::Point2f& centerPos) const;

	//设置关键帧编号
	void setKeyFrameIndex(int64_t keyFrameIndex);

	//设置炸点坐标，单位像素。若调用炸点检测函数，也会修改炸点位置
	void setBurstPoint(const cv::Point2f& point);

	//获取炸点坐标，单位像素。
	cv::Point2f getBurstPoint() const;

	//绘制网格线
	void DrawLegends(cv::Mat& srcImg,bool legendsVisible, int64_t frameIndex/*,bool isColorPic*/) const;

	//炸点检测，若检测成功，会修改炸点位置。炸点为像素坐标
	bool DetectBurstPoint(const std::vector<cv::Mat>& imgs, int64_t firstFrameIndex);

	//计算落点相对于靶心的偏移量。burstPoint为落点的像素坐标，fallPoint为落点相对于靶心的偏移量，物理坐标，单位米
	bool CalcFallPoint(const cv::Point2f& burstPoint, cv::Point2f& fallPoint) const;

	//选择当前使用的相机参数
	bool SelectCameraParam(const std::string& cameraID);

	//判断指定相机的标定参数是否存在
	bool CameraParamExisted(const std::string& cameraID) const;

	//计算指定点到靶心的距离，输入像素坐标，输出单位米
	bool CalcDistToCenter(const cv::Point2f& pt,float& dist) const;

	void getCalibrationParamsInfo(std::vector<RetrievalCalibrationInfo>& info) const;

	//删除指定的标定参数和网格线参数
	bool RemoveCalibrationParam(const std::string& cameraID);

	int64_t GetBurstPointFrameIndex() const;

	//根据当前设置的靶心坐标计算最佳网格线间距（单位米），网格线条数默认为5
	float FindBestLegendInterval();

	bool SetLegendInterval(float interval);//设置物理圆间隔，并刷新网格线参数

	void SetImageSize(const cv::Size& size);//设置图像尺寸

private:
	//生成单条网格点
	bool PrepareLegendsPts(std::vector<cv::Point2f>& pts, LegendsType legendsType, cv::Point2f bullEyePos,float radius) const;//计算物理圆上的点

#if 0   //根据最大值、最小值和间隔生成多条网格点
	__declspec(deprecated)
	bool PrepareLegendsPts(std::vector<std::vector<cv::Point2f>>& pts, std::vector<int>& distanceTicks,LegendsType legendsType, 
		cv::Point2f bullEyePos, int minDist,int maxDist, int distInterval);
#endif

	bool FitEllipse(const std::vector<cv::Point2f>& pts, EllipseLegendsParams* ellipseLegends) const;

	bool ImageDiff(const cv::Mat& srcImg1, const cv::Mat& srcImg2, cv::Mat& dstImg, double threshold) const;

	bool FindContour(const cv::Mat& srcImg, std::vector<cv::Point>& contour) const;

	bool FindLowerPoint(const std::vector<cv::Point>& contour, cv::Point2f& lowerPoint, float& ratio) const;

	//将图像坐标映射为物理坐标，输出单位米
	bool Image2World(const cv::Point2f& imagePts, cv::Point2f& worldPts) const;

	//将物理坐标映射为图像坐标，输入单位米
	bool World2Image(const cv::Point2f& worldPt, const CalibrationParams* pCalibrationParams, cv::Point2f& imagePt) const;

	//批量将物理坐标映射为图像坐标，输入单位米
	bool World2Image(const std::vector<cv::Point2f>& worldPts, const CalibrationParams* pCalibrationParams,std::vector<cv::Point2f>& imagePts);

#if 0
	void CreateEllipseLegendsParams(const std::vector<std::vector<cv::Point2f>>& worldPts, const std::vector<int>& distanceTicks, const CalibrationParams* pCalibrationParams, LegendsParams* pLegendsParams);
#endif

	//启发式生成椭圆网格线
	bool CreateEllipseLegendsParams(const std::string& cameraID);

private:
	bool isFallPointMeasureEnabled;//落点测量功能是否使能
	std::string curCameraID;//当前选择的相机ID
	CalibrationParams* pCurCalibrationParams = nullptr;//当前选择的标定参数。仅作为数据的指针，不需要释放
	LegendsParams* pCurLegendsParams = nullptr;//当前选择的网格线参数。仅作为数据的指针，不需要释放
	std::unordered_map<std::string, CameraParams*> mapCameraParams;//所有标定参数和网格线参数的汇总
	int64_t burstPointFrameIndex;//炸点所在的帧
	float legendInterval = -1;//刻度线间隔，单位米，即物理圆间隔
	cv::Size imageSize;//图像尺寸
	const cv::Scalar red = cv::Scalar(0, 0, 255);//红色，网格线、靶点中心、炸点的颜色
	const cv::Scalar gray = cv::Scalar(177, 177, 34);//灰色，刻度线的颜色
};



