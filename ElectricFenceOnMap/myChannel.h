#ifndef __MYCHANNEL_H__
#define __MYCHANNEL_H__

#include <QWebChannel>
#include <QJsonObject>
#include <qjsondocument.h>
#include <qwebenginepage.h>
#include <qdebug.h>

class myChannel :public QObject 
{
	Q_OBJECT
public:
    explicit myChannel(QObject* parent=nullptr);

	void addOneMark(QString lng, QString lat, QString p_time);
	void addWithoutOneMark(QString lng, QString lat);
	void addPolyLine(QString lng, QString lat/*QStringList p_pth*/);

public slots:
	void sendCoordinates(QString longitude, QString latitude, QString title);
	void sendPolygon_type1(QString longitude, QString latitude, QString title);
	void sendPolygon_typeInuse(QString longitude, QString latitude, QString title);
	void clearMarkerSignal();
	void sendChangeStatusSignal();
	void receiveCoordinates_slot(QString longitude, QString latitude);

signals:
	void addOneMarkSingal(QString p_lngStr, QString p_latStr, QString p_time);
	void addWithoutOneMarkSingal(QString p_lngStr, QString p_latStr);
	void addPolyLineSingal(QString p_lngStr, QString p_latStr);
	void receiveCoordinates_signal(QString longitude, QString latitude);

public:
	QWebEnginePage* page;


};


#endif
