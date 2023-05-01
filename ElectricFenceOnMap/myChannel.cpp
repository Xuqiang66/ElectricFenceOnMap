#include "myChannel.h"

myChannel::myChannel(QObject *parent)
    :QObject(parent)
{

}

void myChannel::addOneMark(QString lng, QString lat, QString p_time)
{
	emit addOneMarkSingal(lng, lat, p_time);
}

void myChannel::addWithoutOneMark(QString lng, QString lat)
{
	emit addWithoutOneMarkSingal(lng, lat);
}

void myChannel::addPolyLine(QString lng, QString lat)
{
	emit addPolyLineSingal(lng, lat);
}

/*���Ͷ����������ʾ�ڵ�ͼ��--����δ������*/
void myChannel::sendPolygon_type1(QString longitude, QString latitude, QString title)
{
	QVariantMap coordinates;
	coordinates.insert("latitude", latitude);
	coordinates.insert("longitude", longitude);
	coordinates.insert("titl", title);
	//qDebug() << "coordinates" << coordinates;
	QJsonObject obj = QJsonObject::fromVariantMap(coordinates);
	QJsonDocument doc(obj);
	QString script = QString("setPolygon(%1)").arg(QString(doc.toJson()));
	//qDebug() << "script" << script;
	//page = qobject_cast<QWebEnginePage*>(sender());
	if (page) {
		qDebug() << "send Polygon to the map";
		page->runJavaScript(script);
	}
}

/*���Ͷ����������ʾ�ڵ�ͼ��--������ʹ��*/
void myChannel::sendPolygon_typeInuse(QString longitude, QString latitude, QString title)
{
	QString script = QString("setPolygon(%1,%2,%3)").arg(longitude).arg(latitude).arg(title);
	//qDebug() << "script" << script;
	if (page) {
		qDebug() << "send Polygon to the map";
		page->runJavaScript(script);
	}
}

/*����������ʾ�ڵ�ͼ��*/
void myChannel::sendCoordinates(QString longitude, QString latitude, QString title)
{
	QString script = QString("setCoordinates(%1,%2,%3)").arg(longitude).arg(latitude).arg(title);
	
	if (page) {
		qDebug() << "send coordinate to the map";
		page->runJavaScript(script);
	}
}

void myChannel::clearMarkerSignal()
{
	QString script = QString("clearMarker()");

	if (page) {
		qDebug() << "send signal to clear marker";
		page->runJavaScript(script);
	}
}

/*��������Ĭ�ϵ����ͼ�����ȡ�õ����꣬������ư�ť����*/
void myChannel::sendChangeStatusSignal()
{
	QString script = QString("changeStatus()");

	if (page) {
		qDebug() << "send signal to change status";
		page->runJavaScript(script);
	}
}

void myChannel::receiveCoordinates_slot(QString longitude, QString latitude)
{
	qDebug() << "[longitude,latitude]" << longitude << "," << latitude;
	emit receiveCoordinates_signal(longitude, latitude);
}

