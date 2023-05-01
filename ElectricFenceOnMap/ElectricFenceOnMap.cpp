#include "ElectricFenceOnMap.h"

ElectricFenceOnMap::ElectricFenceOnMap(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	showRunningInfo_slot(cn("��������"));

	//ui.pBtn_start->setEnabled(false);
	id_compare = "defaultId";
	clicked_count = 0;

	connect(this, SIGNAL(showRunningInfo_signal(QString)), this, SLOT(showRunningInfo_slot(QString)));
	connect(this, SIGNAL(loadMap_signal()), this, SLOT(loadMap_slot()), Qt::QueuedConnection);

	//���ص�ͼ
	emit loadMap_signal();
}

ElectricFenceOnMap::~ElectricFenceOnMap()
{

}

/*���ص�ͼ */
void ElectricFenceOnMap::loadMap_slot()
{
	emit showRunningInfo_signal(cn("��ͼ������"));
	
	_myChannel = new myChannel(this);
	web_channel = new QWebChannel(this);
	web_channel->registerObject("qtChannel", _myChannel);

	_myChannel->page = ui.webEngineView->page();
	_myChannel->page->setWebChannel(web_channel);
	//ui.webEngineView->page()->setWebChannel(web_channel);

	TRYAGAIN:
	if (!checkNetworkConnection()) {
		ui.webEngineView->load(QUrl("qrc:/mymap.html"));
		emit showRunningInfo_signal(cn("��ͼ����ʧ�ܣ�������������"));
		QThread::sleep(3);
		goto TRYAGAIN;
	}
	else {
		ui.webEngineView->load(QUrl("qrc:/mymap.html"));
		emit showRunningInfo_signal(cn("��ͼ���سɹ�"));
		/*���ӵ����ͼ��ȡ�������ź���ۺ���*/
		connect(_myChannel, SIGNAL(receiveCoordinates_signal(QString, QString)), this, SLOT(saveElectricFenceFile_slot(QString, QString)));
		return;
	}

}

/*��������Ƿ�������*/
bool ElectricFenceOnMap::checkNetworkConnection()
{
	QNetworkAccessManager manager;
	QNetworkRequest request(QUrl("http://www.baidu.com"));
	QNetworkReply *reply = manager.get(request);
	QEventLoop loop;
	QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	loop.exec();
	if (reply->error() == QNetworkReply::NoError) {
		qDebug() << cn("����������");
		return true;
	}
	else {
		qDebug() << cn("����δ����");
		return false;
	}
}

/*���ر��ص���Χ���ļ��������������ڵ�ͼ��*/
void ElectricFenceOnMap::on_pBtn_loadFile_clicked()
{
	switch (readElectricFenceFile()) 
	{
	case CommonStatus::success:
		emit showRunningInfo_signal(cn("Χ���ļ���ȡ�ɹ�"));
		showPolygonOnMap();
		emit showRunningInfo_signal(cn("����Χ���ڵ�ͼ���ѱ��"));
		ui.pBtn_loadFile->setEnabled(false);
		ui.pBtn_start->setEnabled(true);
		break;

	case CommonStatus::file_not_exist:
		emit showRunningInfo_signal(cn("Χ���ļ�������"));
		break;

	case CommonStatus::file_open_fail:
		emit showRunningInfo_signal(cn("Χ���ļ���ʧ��"));
		break;

	case CommonStatus::file_wrong:
		emit showRunningInfo_signal(cn("Χ���ļ�����"));
		break;

	default:
		break;
	}
}

/*��������ľ�γ�ȵ㣬�ж����Ƿ���Χ����*/
void ElectricFenceOnMap::on_pBtn_start_clicked()
{
	double longitude = 0.0;
	double latitude = 0.0;
	vector<Vec2d> polygon_mem;

	longitude = ui.lineEdit_lon->text().toDouble();
	latitude = ui.lineEdit_lat->text().toDouble();
	//qDebug() << "longitude" << longitude << "latitude" << latitude;
	if (longitude == 0.0 || latitude == 0.0) {
		emit showRunningInfo_signal(cn("������һ����γ�ȵ�"));
		return;
	}
	clicked_count++;
	_myChannel->sendCoordinates(QString::number(longitude, 'f', 8), QString::number(latitude, 'f', 8), QString::number(clicked_count));
	
	/*�ж��Ƿ��Ѿ����ص���Χ���ļ�*/
	if (elec_fence_vector.size() == 0) {
		emit showRunningInfo_signal(cn("�������Χ���Ƿ����"));
		return;
	}

	for (int i = 0; i < elec_fence_vector.size(); i++) {	//����ÿ������Σ�������ţ�
		polygon_mem.clear();
		polygon_mem = elec_fence_vector[i];	//ȡ��һ������Σ�������ţ�

		for (int j = 0; j < polygon_mem.size(); j++) {	//���������ÿһ����
			
			if (Point_In_Polygon_2D(longitude, latitude, polygon_mem)) {
				emit showRunningInfo_signal(cn("�ڵ���Χ���ڣ�Χ����ţ�%1").arg(polygon_mem[j].id));
				return;
			}
			
		}

		if (i == elec_fence_vector.size() - 1) {
			emit showRunningInfo_signal(cn("�ڵ���Χ����"));
			return;
		}
	}
}

/*��������ľ�γ�ȵ㣬�ж����Ƿ���Χ����*/
void ElectricFenceOnMap::on_pBtn_draw_clicked()
{
	QMessageBox::StandardButton reply;
	reply = QMessageBox::warning(this, cn("����Χ��"), cn("�Ƿ�ȷ�ϻ���Χ����"), QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes) {	// �û�ѡ��ȷ��
		/*�ж��Ƿ���ع�Χ���ļ�*/
		if (elec_fence_vector.size() > 0) {
			reply = QMessageBox::warning(this, cn("����Χ��"), cn("���»�����Ҫ�������ͼ����Χ������ȷ��������"), QMessageBox::Yes | QMessageBox::No);
			if (reply == QMessageBox::Yes) {	//�û�ѡ��ȷ��
				/*�����ͼ����Χ����*/
				_myChannel->clearMarkerSignal();
			}
			else {	//�û�ѡ��ȡ��
				return;
			}
		}

		/*�����һ�λ��Ƶ�Χ����������*/
		qstrList_elec.clear();
		/*����״̬�ɲ��ɻ��Ƹ�Ϊ�ɻ���*/
		_myChannel->sendChangeStatusSignal();
		/*�����ʼ����֮�󣬽����ư�ť��Ϊ�����ٴε��*/
		ui.pBtn_draw->setEnabled(false);
		/*�����ʼ����֮�󣬽�������ļ���ť��Ϊ�ɵ��*/
		ui.pBtn_save->setEnabled(true);
		/*�����ʼ����֮�󣬽���ʼ��ť��Ϊ���ɵ��*/
		ui.pBtn_start->setEnabled(false);
	}
	else {	// �û�ѡ��ȡ��
		return;
	}

}

void ElectricFenceOnMap::on_pBtn_save_clicked()
{
	qDebug() << "qstrList_elec" << qstrList_elec;
	/*�ж��Ƿ��������Χ��*/
	if (qstrList_elec.size() == 0) {
		QMessageBox::information(this, cn("��ʾ"), cn("���Ȼ���Χ���ٵ�����棡"));
		return;
	}

	QFile file(ELECTRIC_FILE);
	if (file.exists()) {
		QMessageBox::StandardButton reply;
		reply = QMessageBox::warning(this, cn("�����ļ�"), cn("�ļ��Ѵ��ڣ��Ƿ񸲸�ԭΧ����"), QMessageBox::Yes | QMessageBox::No);
		if (reply == QMessageBox::Yes) {	// �û�ѡ��ȷ��
			remove(ELECTRIC_FILE);	//�Ƴ�ԭ��Χ���ļ�
		}
		else {	//�û�ѡ��ȡ��
			/*��ջ��Ƶ�Χ���ַ����б�*/
			qstrList_elec.clear();
			return;
		}
	}

	/*���»���Χ������д���ļ���*/
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
	{
		qDebug() << "Failed to open file for writing:" << ELECTRIC_FILE;
		return;
	}
	QStringListIterator strIt(qstrList_elec);
	QTextStream out(&file);
	while (strIt.hasNext()) {
		out << strIt.next() << "\n";
	}
	out << qstrList_elec.at(0) << "\n";

	file.close();
	
	/*��������ļ�֮�󣬽����ư�ť��Ϊ���ٴε��*/
	ui.pBtn_draw->setEnabled(true);
	/*��������ļ�֮�󣬽�������ļ���ť��Ϊ�����ٴε��*/
	ui.pBtn_save->setEnabled(false);
	/*��������ļ�֮�󣬽�����Χ���ļ���ť��Ϊ�ɵ��*/
	ui.pBtn_loadFile->setEnabled(true);
	
	QMessageBox::information(this, cn("��ʾ"), cn("Χ�������»��ƣ���������Χ���ļ���ť���¼���Χ����"));
}

void ElectricFenceOnMap::saveElectricFenceFile_slot(QString longitude, QString latitude)
{
	//qDebug() << "[longitude,latitude]" << longitude << "," << latitude;
	QString elec_row = longitude.append(",") + latitude.append(",") + QString::number(1);
	qstrList_elec.append(elec_row);
}

/*��ȡ���ص���Χ��CSV�ļ�*/
CommonStatus::CommonStatus ElectricFenceOnMap::readElectricFenceFile()
{
	QFile inFile(ELECTRIC_FILE);
	QStringList lines;

	if (inFile.exists()) {
		if (inFile.open(QIODevice::ReadOnly)) {
			/*���ļ�֮ǰ�Ƚ�����洢Χ�����ݵ�vector���*/
			elec_fence_rect.clear();
			elec_fence_vector.clear();

			QTextStream stream_text(&inFile);
			while (!stream_text.atEnd()) {
				lines.push_back(stream_text.readLine());	//�ж�ȡ
			}

			for (int i = 0; i < lines.size(); i++) {
				QString line = lines.at(i);	//��ȡһ�е�����
				QStringList list_split = line.split(",");	//�Զ��Ų��
				if (list_split.size() < 3) {
					return CommonStatus::file_wrong;
				}

				elec_fence.x = list_split.at(0).toDouble();
				elec_fence.y = list_split.at(1).toDouble();
				elec_fence.id = list_split.at(2);

				if (id_compare == "defaultId") {
					id_compare = elec_fence.id;
				}
				if (elec_fence.id != id_compare) {
					elec_fence_vector.push_back(elec_fence_rect);
					elec_fence_rect.clear();
					id_compare = elec_fence.id;
				}
				elec_fence_rect.push_back(elec_fence);
				
				if (i == lines.size() - 1) {
					elec_fence_vector.push_back(elec_fence_rect);
				}
			}
		}
		else {
			return CommonStatus::file_open_fail;
		}
	}
	else {
		return CommonStatus::file_not_exist;
	}

	return CommonStatus::success;
}

/*������Χ���ļ��еĶ���ο���ʾ�ڵ�ͼ��*/
void ElectricFenceOnMap::showPolygonOnMap()
{
	vector<Vec2d> polygon_mem;

	/*�������ͼ�ѻ��Ƶ�Χ����*/
	_myChannel->clearMarkerSignal();

	for (int i = 0; i < elec_fence_vector.size(); i++) {	//����ÿ������Σ�������ţ�
		polygon_mem = elec_fence_vector[i];	//ȡ��һ������Σ�������ţ�

		for (int j = 0; j < polygon_mem.size(); j++) {	//���������ÿһ����
			QString lon = QString::number(polygon_mem[j].x, 'f', 8);
			QString lat = QString::number(polygon_mem[j].y, 'f', 8);
			QString id = polygon_mem[j].id;

			_myChannel->sendPolygon_typeInuse(lon, lat, id);
		}
		
	}
}

//�жϵ����߶���
bool ElectricFenceOnMap::IsPointOnLine(double px0, double py0, double px1, double py1, double px2, double py2)
{
	bool flag = false;
	double d1 = (px1 - px0) * (py2 - py0) - (px2 - px0) * (py1 - py0);
	if ((abs(d1) < EPSILON) && ((px0 - px1) * (px0 - px2) <= 0) && ((py0 - py1) * (py0 - py2) <= 0))
	{
		flag = true;
	}
	return flag;
}

//�ж����߶��ཻ
bool ElectricFenceOnMap::IsIntersect(double px1, double py1, double px2, double py2, double px3, double py3, double px4, double py4)
{
	bool flag = false;
	double d = (px2 - px1) * (py4 - py3) - (py2 - py1) * (px4 - px3);
	if (d != 0)
	{
		double r = ((py1 - py3) * (px4 - px3) - (px1 - px3) * (py4 - py3)) / d;
		double s = ((py1 - py3) * (px2 - px1) - (px1 - px3) * (py2 - py1)) / d;
		if ((r >= 0) && (r <= 1) && (s >= 0) && (s <= 1))
		{
			flag = true;
		}
	}
	return flag;
}

//�жϵ��ڶ������
bool ElectricFenceOnMap::Point_In_Polygon_2D(double x, double y, const vector<Vec2d> &POL)
{
	bool isInside = false;
	int count = 0;

	//
	double minX = DBL_MAX;
	for (int i = 0; i < POL.size(); i++)
	{
		minX = std::min(minX, POL[i].x);
	}

	//
	double px = x;
	double py = y;
	double linePoint1x = x;
	double linePoint1y = y;
	double linePoint2x = minX - 10;			//ȡ��С��Xֵ��С��ֵ��Ϊ���ߵ��յ�
	double linePoint2y = y;

	//����ÿһ����
	for (int i = 0; i < POL.size() - 1; i++)
	{
		double cx1 = POL[i].x;
		double cy1 = POL[i].y;
		double cx2 = POL[i + 1].x;
		double cy2 = POL[i + 1].y;

		if (IsPointOnLine(px, py, cx1, cy1, cx2, cy2))
		{
			return true;
		}

		if (fabs(cy2 - cy1) < EPSILON)   //ƽ�����ཻ
		{
			continue;
		}

		if (IsPointOnLine(cx1, cy1, linePoint1x, linePoint1y, linePoint2x, linePoint2y))
		{
			if (cy1 > cy2)			//ֻ��֤�϶˵�+1
			{
				count++;
			}
		}
		else if (IsPointOnLine(cx2, cy2, linePoint1x, linePoint1y, linePoint2x, linePoint2y))
		{
			if (cy2 > cy1)			//ֻ��֤�϶˵�+1
			{
				count++;
			}
		}
		else if (IsIntersect(cx1, cy1, cx2, cy2, linePoint1x, linePoint1y, linePoint2x, linePoint2y))   //�Ѿ��ų�ƽ�е����
		{
			count++;
		}
	}

	if (count % 2 == 1)
	{
		isInside = true;
	}

	return isInside;
}

/*��ʾ������Ϣ�ۺ���*/
void ElectricFenceOnMap::showRunningInfo_slot(QString p_text)
{
	QString m_text = p_text;
	qDebug() << "m_text" << m_text;
	ui.textBrowser->append(m_text);
	ui.textBrowser->moveCursor(QTextCursor::End);
}

//����ת��
double ElectricFenceOnMap::translate_lon(double lon, double lat)
{
	double ret = 300.0 + lon + 2.0*lat + 0.1*lon*lon + 0.1*lon*lat + 0.1*sqrt(abs(lon));
	ret += (20.0 * sin(6.0*lon*PI) + 20.0*sin(2.0*lon*PI)) *2.0 / 3.0;
	ret += (20.0 * sin(lon*PI) + 40.0*sin(lon / 3.0 *PI)) *2.0 / 3.0;
	ret += (150 * sin(lon / 12.0 *PI) + 300.0*sin(lon / 30.0 * PI)) *2.0 / 3.0;
	return ret;
}

//γ��ת��
double ElectricFenceOnMap::translate_lat(double lon, double lat)
{
	double ret = -100 + 2.0*lon + 3.0*lat + 0.2*lat*lat + 0.1*lon*lat + 0.2*sqrt((abs(lon)));
	ret += (20.0 *sin(6.0*lon*PI) + 20 * sin(2.0*lon*PI)) *2.0 / 3.0;
	ret += (20.0 *sin(lat*PI) + 40.0*sin(lat / 3.0*PI)) *2.0 / 3.0;
	ret += (160.0*sin(lat / 12.0*PI) + 320.0*sin(lat / 30.0 *PI)) *2.0 / 3.0;
	return ret;
}

/*84����ϵת�ߵ�����*/
POSITION ElectricFenceOnMap::wgs84togcj02(double wgs_lon, double wgs_lat)
{
	double dlat = translate_lat(wgs_lon - 105.0, wgs_lat - 35.0);
	double dlon = translate_lon(wgs_lon - 105.0, wgs_lat - 35.0);
	double radlat = wgs_lat / 180.0 * PI;
	double magic = sin(radlat);
	magic = 1 - ee * magic*magic;
	double squrtmagic = sqrt(magic);
	dlon = (dlon *180.0) / (a / squrtmagic * cos(radlat)*PI);
	dlat = (dlat *180.0) / ((a*(1 - ee)) / (magic * squrtmagic)*PI);
	gcj_pos.longitude = wgs_lon + dlon;
	gcj_pos.latitude = wgs_lat + dlat;
	return gcj_pos;
}

/*�ߵ�����ת84����*/
POSITION ElectricFenceOnMap::gcj02towgs84(double gcj_lon, double gcj_lat)
{

	double dlat = translate_lat(gcj_lon - 105.0, gcj_lat - 35.0);
	double dlon = translate_lon(gcj_lon - 105.0, gcj_lat - 35.0);
	double radlat = gcj_lat / 180.0 *PI;
	double magic = sin(radlat);
	magic = 1 - ee * magic*magic;
	double squrtmagic = sqrt(magic);
	dlon = (dlon *180.0) / (a / squrtmagic * cos(radlat)*PI);
	dlat = (dlat *180.0) / ((a*(1 - ee)) / (magic * squrtmagic)*PI);
	wgs_pos.longitude = gcj_lon - dlon;
	wgs_pos.latitude = gcj_lat - dlat;
	return wgs_pos;

}
