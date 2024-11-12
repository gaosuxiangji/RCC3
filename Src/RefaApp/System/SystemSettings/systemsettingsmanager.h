#ifndef SYSTEMSETTINGSMANAGER_H
#define SYSTEMSETTINGSMANAGER_H

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QLocale>
#include <atomic>
#include "System/Experiment/csexperimentutil.h"
//实验项目相关
static const QString kExperimentNameKey("Experiment/Name");//项目名称
static const QString kExperimentCodeKey("Experiment/Code");//项目代号
static const QString kExperimentDescKey("Experiment/Desc");//项目描述
/**
 * @brief 系统配置管理类
 */
class SystemSettingsManager
{
public:
    static SystemSettingsManager & instance();

	// 录制后动作类型
	enum ActionTypesAfterRecording
	{
		kAcquire,
		kAutoExport,
		kPlayback,
		kAutoExportAndTrigger,
	};

    /**
     * @brief 初始化
     */
    void initialize();

	/**
	* @brief 记录当前系统是否正在睡眠
	*/
	void setSystemSleepState(bool sleeping);

	/**
	* @brief 获取当前系统是否正在睡眠状态
	*/
	bool isSystemSleeping();

    /**
     * @brief 设置本地IP
     * @param addr ip地址
     */
    void setLocalIp(const QString & addr);

    /**
     * @brief 获取本地IP
     * @return 本地IP
     */
    QString getLocalIp() const;

    /**
     * @brief 获取所有本地IP
     * @return 本地IP列表
     */
    QStringList getAllLocalIp() const;

    /**
     * @brief 设置工作路径
     * @param dir 工作路径
     */
    void setWorkingDirectory(const QString & dir);

    /**
     * @brief 获取工作路径
     * @return 工作路径
     */
    QString getWorkingDirectory() const;

	//标定相关设置
	/**
	* @brief 设置标定文件路径
	* @param dir 标定文件路径
	*/
	void setCalibrationDirectory(const QString & dir);

	/**
	* @brief 获取标定文件路径
	* @return 标定文件路径
	*/
	QString getCalibrationDirectory() const;

	/**
	* @brief 加载已经记录的标定文件
	*/
	void loadCalibrationParamFiles();

	//添加和删除对标定文件dir的记录
	void addCalibrationFile(QString dir,QString camera_id);
	void removeCalibrationFile(QString camera_id);

    /**
     * @brief 设置自动回放是否开启
     * @param enable true-开启，false-关闭
     */
    void setAutoPlaybackEnabled(bool enable);

    /**
     * @brief 判定自动回放是否开启
     * @return true-开启，false-关闭
     */
    bool isAutoPlaybackEnabled() const;

	/**
	* @brief 设置自动连接全部设备是否开启
	* @param enable true-开启，false-关闭
	*/
	void setAutoConnectEnabled(bool enable);

	/**
	* @brief 判定自动连接全部设备是否开启
	* @return true-开启，false-关闭
	*/
	bool isAutoConnectEnabled() const;

	/**
	* @brief 设置即录即停功能是否开启
	* @param enable true-开启，false-关闭
	*/
	void setRecordingStopEnabled(bool enable);

	/**
	* @brief 判定即录即停功能是否开启
	* @return true-开启，false-关闭
	*/
	bool isRecordingStopEnabled() const;

	/**
	* @brief 设置录制后动作
	* @param type 枚举值
	*/
	void setActionTypesAfterRecording(int type);

	/**
	* @brief 判定录制后动作
	* @return int 枚举值
	*/
	int getActionTypesAfterRecording() const;

    /**
     * @brief 设置语言
     * @param lang 语言
     */
    void setLanguage(QLocale::Language lang);

    /**
     * @brief 获取语言
     * @return 语言
     */
    QLocale::Language getLanguage() const;

	/**
	*@brief 设置是否全屏
	*@param benabled true-开启，false-关闭
	*@return
	**/
	void setFullScreenEnabled(bool benabled);

	/**
	*@brief 获取是否全屏
	*@return true-全屏，false-非全屏
	**/
	bool isFullScreenEnabled() const;


	/**
	*@brief 保存窗口状态
	*@param data 窗口状态数据
	*@return
	**/
	void saveWindowState(QByteArray data);

	/**
	*@brief 获取窗口状态
	*@return QByteArray 窗口状态数据
	**/
	QByteArray getWindowState() const;


	/**
	*@brief 设置自动连接设备
	*@param devices_ip 设备ip
	**/
	void setAutoConnectedDevice(const QStringList & devices_ip) const;

	/**
	*@brief 获取自动连接相机
	*@return 自动连接相机的ip
	**/
	QStringList getAutoConnectedDevice() const;

	/**
	* @brief 获取当前的实验项目信息
	* @return CSExperiment 当前的实验项目
	*/
	CSExperiment getCurrentExperiment() { return m_current_exp; }

	/**
	* @brief 设置当前的实验项目信息
	* @return exp 实验项目
	*/
	void setCurrentExperiment(CSExperiment exp) { m_current_exp = exp; }

	/**
	* @brief 获取当前项目路径
	* @return 项目文件路径
	*/
	QString getCurrentExperimentDir() const;
	QString getCurrentDeviceParamDir() const;

	/**
	* @brief 保存当前项目信息到文件
	*/
	void saveCurrentExperimentInfo();
	void saveCurrentDeviceParamInfo();

	/**
	* @brief 从文件读取当前项目信息
	*/
	void loadCurrentExperimentInfo();

	/**
	* @brief 设置窗口中心线是否开启
	* @param bEnable true-开启，false-关闭
	*/
	void setWindow_Centerline(bool bEnable);

	/**
	* @brief 判定窗口中心线是否开启
	* @return true-开启，false-关闭
	*/
	bool isWindow_Centerline() const;

	/**
	* @brief 设置视频导出默认保存格式
	* @param videoFormat 视频格式文本
	*/
	void setVideoFormat(const QString videoFormat);

	/**
	* @brief 获取视频导出默认保存格式
	* @param 视频格式文本
	*/
	bool getVideoFormat(QString& videoFormat);

	/**
	* @brief 设置视频导出默认色彩模式
	* @param videoColor 色彩模式文本
	*/
	void setVideoCorlor(const QString videoColor);

	/**
	* @brief 获取视频导出默认色彩模式
	* @param 色彩模式文本
	*/
	bool getVideoCorlor(QString& videoColor);

	/**
	* @brief 设置视频导出默认命名规则
	* @param ruleName 默认命名规则
	*/
	void setVideoRuleName(const QString ruleName);

	/**
	* @brief 获取视频导出默认命名规则
	* @return 默认命名规则
	*/
	bool getVideoRuleName(QString& ruleName);

	/**
	* @brief 设置视频默认播放速率
	* @param fps 播放速率
	*/
	void setDisplayfps(int fps);

	/**
	* @brief 获取视频默认播放速率
	* @return 播放速率
	*/
	int getDisplayfps()const;

	/**
	* @brief 设置视频默认抽帧值
	* @param skipFrame 抽帧值
	*/
	void setSkipFrame(double skipFrame);

	/**
	* @brief 获取视频默认抽帧值，在抽帧单位成功获取后才使用此值
	* @return 抽帧值
	*/
	double getSkipFrame()const;

	/**
	* @brief 设置视频默认抽帧单位
	* @param skipFrame 抽帧单位
	*/
	void setSkipUnit(int skipUnit);

	/**
	* @brief 获取视频默认抽帧单位
	* @return 是否有效
	*/
	bool getSkipUnit(int &skipUnit)const;

	/**
	* @brief 设置自定义水印信息
	* @param watermarkParam 水印参数列表 <文件名，文件索引、水印名、位置、宽、高、透明度、旋转角、大小>
	*/
	void setVideoWatermarkInfo(QMap<QString, QStringList>);
	
	/**
	* @brief 获取自定义水印信息
	* @return <文件名，文件索引、水印名、位置、宽、高、透明度、旋转角、大小>
	*/
	QMap<QString ,QVariant> getVideoWatermarkInfo()const;

	/**
	* @brief 设置avi压缩是否开启
	* @param enable true-开启，false-关闭
	*/
	void setAviCompressEnabled(bool enable);

	/**
	* @brief 判定avi压缩是否开启
	* @return true-开启，false-关闭
	*/
	bool isAviCompressEnabled() const;

	/**
	* @brief 设置默认水印是否开启
	* @param enable true-开启，false-关闭
	*/
	void setDefaultVideoWatermarkEnabled(bool enable);

	/**
	* @brief 判定默认水印是否开启
	* @return true-开启，false-关闭
	*/
	bool isDefaultVideoWatermarkEnabled() const;
	
	void setCurrentDeviceIP(const QString& iporsn);
	QString get12PxStyle() const;

	QMap<QString, std::pair<int, int>> getTemperatureThreshold() const;
private:
    SystemSettingsManager();

    QStringList local_ip_addresses_; // 本地IP地址列表

    QString default_working_directory_; // 缺省工作目录

	CSExperiment m_current_exp;//当前的实验项目

	QString default_current_exp_directory_; // 默认实验项目目录

	std::atomic_bool m_system_sleeping {false};
	QString m_device_param_save_path{};
	QString m_current_device_ip{};
	QString m_12px_style{ "font-size:12px;font-family:Microsoft YaHei;QTabBar::tab{font-size:12px;};" };

	const QMap<QString, std::pair<int, int>> map_range_{
		{ "ARM",std::make_pair(-20, 60) },
		{ "ARMCHIP",std::make_pair(-20, 60) },
		{ "MAINBOARD",std::make_pair(-20, 80) },
		{ "SLAVEBOARD",std::make_pair(-20, 80) },
		{ "FPGA",std::make_pair(-20, 80) },
		{ "CMOS",std::make_pair(-20, 80) },
		{ "CHAMBER",std::make_pair(10, 30) }
	};
};

#endif // SYSTEMSETTINGSMANAGER_H
