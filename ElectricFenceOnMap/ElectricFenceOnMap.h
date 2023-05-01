#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ElectricFenceOnMap.h"
#include "myChannel.h"
#include <qwebchannel.h>
#include <qwebengineview.h>
#include <qwebenginepage.h>
#include <qthread.h>
#include <qdebug.h>
#include <qurl.h>
#include <qfile.h>
#include <qvector.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkrequest.h>
#include <qnetworkreply.h>
#include "commonDefine.h"
#include <qmessagebox.h>

#define cn(x) QString::fromLocal8Bit(x)

class ElectricFenceOnMap : public QMainWindow
{
    Q_OBJECT

public:
    ElectricFenceOnMap(QWidget *parent = nullptr);
    ~ElectricFenceOnMap();

public slots:
	void showRunningInfo_slot(QString);
	void on_pBtn_loadFile_clicked();
	void on_pBtn_start_clicked();
	void on_pBtn_draw_clicked();
	void on_pBtn_save_clicked();
	void saveElectricFenceFile_slot(QString longitude, QString latitude);

private slots:
	void loadMap_slot();

signals:
	void showRunningInfo_signal(QString);
	void loadMap_signal();

private:
	bool checkNetworkConnection();
	CommonStatus::CommonStatus readElectricFenceFile();
	void showPolygonOnMap();
	bool IsPointOnLine(double px0, double py0, double px1, double py1, double px2, double py2);
	bool IsIntersect(double px1, double py1, double px2, double py2, double px3, double py3, double px4, double py4);
	bool Point_In_Polygon_2D(double x, double y, const vector<Vec2d> &POL);
	double translate_lon(double lon, double lat);
	double translate_lat(double lon, double lat);
	POSITION wgs84togcj02(double wgs_lon, double wgs_lat);
	POSITION gcj02towgs84(double gcj_lon, double gcj_lat);

private:
    Ui::ElectricFenceOnMapClass ui;

	myChannel *_myChannel;
	QWebChannel *web_channel;

	QString id_compare;
	Vec2d elec_fence;
	vector<Vec2d> elec_fence_rect;
	vector<vector<Vec2d>> elec_fence_vector;

	QStringList qstrList_elec;

	int clicked_count;

	POSITION gcj_pos;
	POSITION wgs_pos;

};
