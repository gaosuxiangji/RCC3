#ifndef VIDEOITEM_H
#define VIDEOITEM_H

#include <QMap>
#include <QList>
#include <QMutex>
#include <QRect>

#include "Video/FeatureItem/featureitem.h"

/**
 * @brief 视频项
 */
class VideoItem
{
public:
    /**
     * @brief 视频类型枚举
     */
    enum VideoType
    {
        Local,
        Remote
    };

    /**
     * @brief 属性类型枚举
     */
    enum PropType
    {
        Name, // 视频名称: QString
        Roi, // 裁剪窗口：QRect
		RotationType, // 旋转类型 : int
        Luminance, // 亮度：int
        Contrast, // 对比度：int
		AntiColorEnable, // 反色使能 : bool
		DisplayMode, //色彩模式: int
        OsdVisible, // OSD可见：bool
        FocusPoint, // 窗口中心点：QPoint
        VideoFormat, // 视频格式：int
        StreamType, // 协议格式：int
        ExportPath, // 导出路径：QString
		VideoFrameCount, // 视频总帧数 : qint64
        BeginFrameIndex, // 起始帧：qint64
        EndFrameIndex, // 结束帧：qint64
		KeyFrameIndex, // 关键帧帧号 :qint64
		FrameStep, // 帧步长：qint64
		ValidBitsPerPix, // 有效位深：qint8
		FPS, // fps : qint64
		ExposureTime, // 曝光时间 : qint64
		FocusPointVisible, // 窗口中心点可见：bool
		TimeStamp, //起始录制时间戳
		VideoSize, //视频大小
		ColorCorrectInfo, //颜色校正
		ExportFrameRate, // 播放帧率
		EnablePiv, // piv是否使能
		VideoNameType, // 视频名称类型
		AVIcompress, // avi视频视频压缩
		BinningMode, // 开启像素融合
		RuleName // 规则名称: QString
    };

    VideoItem();
    VideoItem(const VideoItem &other);
    VideoItem(const QVariant & id, int type = VideoType::Local);

    bool isValid() const;

    /**
     * @brief 获取ID
     * @return ID
     */
    QVariant getId() const;

    /**
     * @brief 设置类型
     * @param type
     */
    void setType(int type);

    /**
     * @brief getType
     * @return
     */
    int getType() const;

    /**
     * @brief 设置名称
     * @param name 名称
     */
    void setName(const QString & name);

    /**
     * @brief 获取名称
     * @return 名称
     */
    QString getName() const;

    /**
     * @brief 设置格则名称，为了导出自定义名称使用
     * @param name 名称
     */
    void setRuleName(const QString & name);

    /**
     * @brief 获取格则名称
     * @return 名称
     */
    QString getRuleName() const;

	/**************************
	* @brief: 获取当前时间
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	QString getTimeStamp() const;

	QString getRecordDate() const;

	QString getOriginalTimeStamp() const;

	/**************************
	* @brief: 获取时长
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	QString getTimeLength() const;

	/**************************
	* @brief: 获取帧率
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	QString getFrameRate() const;

	/**************************
	* @brief: 获取分辨率
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	QString getResolution() const;

	/**************************
	* @brief: 获取总帧数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	QString getTotalFrame() const;

    /**
     * @brief 设置ROI
     * @param roi ROI
     */
    void setRoi(const QRect & roi);

    /**
     * @brief 获取ROI
     * @return ROI
     */
    QRect getRoi() const;

	/**
	* @brief 设置旋转类型
	* @param type 旋转类型
	*/
	void setRotationType(int type);

	/**
	* @brief 获取旋转类型
	* @return 旋转类型
	*/
	int getRotationType() const;

    /**
     * @brief 设置窗口中心线
     * @param point 窗口中心线：QPoint
     */
    void setFocusPoint(const QVariant & point);

    /**
     * @brief 获取窗口中心线
     * @return 窗口中心线
     */
    QVariant getFocusPoint() const;

    /**
     * @brief 设置亮度
     * @param luminance 亮度
     */
    void setLuminance(int luminance);

    /**
     * @brief 获取亮度
     * @return 亮度
     */
    int getLuminance() const;

    /**
     * @brief 设置对比度
     * @param contrast 对比度
     */
    void setContrast(int contrast);

    /**
     * @brief 获取对比度
     * @return 对比度
     */
    int getContrast() const;

	/**
	* @brief 设置反色使能
	* @param enable true-开启, false-关闭
	*/
	void setAntiColorEnable(bool enable);
	/**
	* @brief 反色使能
	* @return true-开启, false-关闭
	*/
	bool isAntiColorEnable() const;

    /**
     * @brief 设置OSD是否显示
     * @param visible true-显示, false-隐藏
     */
    void setOsdVisible(int8_t visible);
    /**
     * @brief OSD是否显示
     * @return true-显示, false-隐藏
     */
    bool isOsdVisible() const;

	/**
	* @brief 设置协议格式
	* @param type 协议格式
	*/
	void setStreamType(int type);

	/**
	* @brief 获取协议格式
	* @return 协议格式
	*/
	int getStreamType() const;

    /**
     * @brief 设置视频格式
     * @param format 视频格式
     */
    void setVideoFormat(int format);

    /**
     * @brief 获取视频格式
     * @return 视频格式
     */
    int getVideoFormat() const;

	/**
	* @brief 设置色彩模式
	* @param mode 色彩模式
	*/
	void setDisplayMode(int mode);

	/**
	* @brief 获取色彩模式
	* @return 色彩模式
	*/
	int getDisplayMode() const;


    /**
     * @brief 设置导出路径
     * @param path 导出路径
     */
    void setExportPath(const QString & path);

    /**
     * @brief 获取导出路径
     * @return 导出路径
     */
    QString getExportPath() const;


	/**
	* @brief 设置视频总帧数
	* @param count 视频总帧数
	*/
	void setVideoFrameCount(qint64 count);

	/**
	* @brief 获取视频总帧数
	* @return 视频总帧数
	*/
	qint64 getVideoFrameCount() const;

    /**
     * @brief 设置起始帧索引
     * @param index 起始帧索引
     */
    void setBeginFrameIndex(qint64 index);

    /**
     * @brief 获取起始帧索引
     * @return 起始帧索引
     */
    qint64 getBeginFrameIndex() const;

    /**
     * @brief 设置结束帧索引
     * @param index 结束帧索引
     */
    void setEndFrameIndex(qint64 index);

    /**
     * @brief 获取结束帧索引
     * @return 结束帧索引
     */
    qint64 getEndFrameIndex() const;

	/**
	* @brief 设置关键帧索引
	* @param index 关键帧索引
	*/
	void setKeyFrameIndex(qint64 index);

	/**
	* @brief 获取关键帧索引
	* @return 关键帧索引
	*/
	qint64 getKeyFrameIndex() const;

    /**
     * @brief 设置帧步长
     * @param step 帧步长
     */
    void setFrameStep(qint64 step);

    /**
     * @brief 获取帧步长
     * @return 帧步长
     */
    qint64 getFrameStep() const;

	/**
	* @brief 设置有效位深
	* @param bpp 有效位深
	*/
	void setValidBitsPerPixel(qint8 bpp);

	/**
	* @brief 获取有效位深
	* @return 有效位深
	*/
	qint8 getValidBitsPerPixel() const;

	/**
	* @brief 设置FPS
	* @param step FPS
	*/
	void setFPS(qint64 fps);


	/**
	* @brief 获取FPS
	* @return FPS
	*/
	qint64 getFPS() const;

	/**
	* @brief 设置曝光时间
	* @param time 曝光时间
	*/
	void setExposureTime(qint64 time);


	/**
	* @brief 获取曝光时间
	* @return 曝光时间
	*/
	qint64 getExposureTime() const;

    /**
     * @brief 设置属性
     * @param type 属性类型
     * @param value 属性值
     */
    void setProperty(int type, const QVariant & value);

    /**
     * @brief 获取属性
     * @param type 属性类型
     * @param default_value 缺省值
     * @return 属性值
     */
    QVariant getProperty(int type, const QVariant & default_value = QVariant()) const;

    /**
     * @brief 获取特征项列表
     * @return 特征项列表
     */
    QList<FeatureItem> getFeatureItems() const;

    /**
     * @brief 添加特征项
     * @param item 特征项
     */
    void addFeatureItem(const FeatureItem & item);

    /**
     * @brief 获取特征项
     * @param feature_id 特征项ID
     * @return 特征项
     */
    FeatureItem getFeatureItem(const QVariant & feature_id) const;

    /**
     * @brief 设置特征项
     * @param feature_id 特征项ID
     * @param item 特征项
     */
    void setFeatureItem(const QVariant & feature_id, const FeatureItem & item);

    /**
     * @brief 删除特征项
     * @param feature_id 特征项ID
     */
    void removeFeatureItem(const QVariant & feature_id);

    /**
     * @brief 清空特征项
     */
    void clearFeatureItems();

	/**
	* @brief 设置播放速率
	* @param index 播放速率
	*/
	void setExportFrameRate(qint64 frame_rate);

	/**
	* @brief 获取播放速率
	* @return 播放速率
	*/
	qint64 getExportFrameRate() const;

	/**
	* @brief 设置视频名称类型
	* @param index 视频名称类型
	*/
	void setVideoNameType(int32_t type);

	/**
	* @brief 获取视频名称类型
	* @return 视频名称类型
	*/
	int32_t getVideoNameType() const;

	/**
	* @brief 设置avi视频压缩与否
	* @param enable 
	*/
	void setAVIcompress(bool enable);

	/**
	* @brief 获取avi视频压缩与否
	* @return 
	*/
	bool getAVIcompress() const;

	/**
	* @brief 设置像素融合模式
	* @param enable 像素融合模式
	*/
	void setBinningModeEnable(bool enable);

	/**
	* @brief 获取像素融合模式
	* @return 像素融合模式
	*/
	bool getBinningModeEnable() const;

    VideoItem & operator=(const VideoItem& other);

private:
    QVariant id_;
    int type_{ VideoType::Local };

    QMap<QVariant, FeatureItem> feature_item_map_;
    mutable QMutex feature_item_mutex_;

    QMap<int, QVariant> property_map_;
    mutable QMutex property_mutex_;
};

#endif // VIDEOITEM_H
