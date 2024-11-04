#ifndef CSDLGFALLPOINTMEASURESETUP_H
#define CSDLGFALLPOINTMEASURESETUP_H

#include <QDialog>
#include <QStandardItemModel>
#include <QPointer>
namespace Ui {
class CSDlgFallPointMeasureSetup;
}

//落点测量标定参数导入对话框
class CSDlgFallPointMeasureSetup : public QDialog
{
    Q_OBJECT

public:
    explicit CSDlgFallPointMeasureSetup(QWidget *parent = 0);
    ~CSDlgFallPointMeasureSetup();

private slots:
    void on_checkBox_selete_all_clicked(bool checked);

    void on_pushButton_delete_clicked();

    void on_pushButton_import_clicked();


    void on_pushButton_close_clicked();

    void on_tableView_cali_params_clicked(const QModelIndex &index);

private:
	void InitUI();

	//从落点测量库中获取已经加载的标定参数信息
	bool UpdateParamsInfo();

	//根据列表中的数据来判断当前的选中状态
	Qt::CheckState GetCurrentCheckState();
private:
    Ui::CSDlgFallPointMeasureSetup *ui;

	QPointer< QStandardItemModel> m_table_model_ptr;//列表数据
};

#endif // CSDLGFALLPOINTMEASURESETUP_H
