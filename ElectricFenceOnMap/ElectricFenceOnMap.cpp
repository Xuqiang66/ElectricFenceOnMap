#include "ElectricFenceOnMap.h"

ElectricFenceOnMap::ElectricFenceOnMap(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	showRunningInfo_slot(cn("程序启动"));

	//ui.pBtn_start->setEnabled(false);
	id_compare = "defaultId";
	clicked_count = 0;

	connect(this, SIGNAL(showRunningInfo_signal(QString)), this, SLOT(showRunningInfo_slot(QString)));
	connect(this, SIGNAL(loadMap_signal()), this, SLOT(loadMap_slot()), Qt::QueuedConnection);

	//加载地图
	emit loadMap_signal();
}

ElectricFenceOnMap::~ElectricFenceOnMap()
{

}

/*加载地图 */
void ElectricFenceOnMap::loadMap_slot()
{
	emit showRunningInfo_signal(cn("地图加载中"));
	
	_myChannel = new myChannel(this);
	web_channel = new QWebChannel(this);
	web_channel->registerObject("qtChannel", _myChannel);

	_myChannel->page = ui.webEngineView->page();
	_myChannel->page->setWebChannel(web_channel);
	//ui.webEngineView->page()->setWebChannel(web_channel);

	TRYAGAIN:
	if (!checkNetworkConnection()) {
		ui.webEngineView->load(QUrl("qrc:/mymap.html"));
		emit showRunningInfo_signal(cn("地图加载失败，请检查网络连接"));
		QThread::sleep(3);
		goto TRYAGAIN;
	}
	else {
		ui.webEngineView->load(QUrl("qrc:/mymap.html"));
		emit showRunningInfo_signal(cn("地图加载成功"));
		/*连接点击地图获取坐标点的信号与槽函数*/
		connect(_myChannel, SIGNAL(receiveCoordinates_signal(QString, QString)), this, SLOT(saveElectricFenceFile_slot(QString, QString)));
		return;
	}

}

/*检查网络是否有连接*/
bool ElectricFenceOnMap::checkNetworkConnection()
{
	QNetworkAccessManager manager;
	QNetworkRequest request(QUrl("http://www.baidu.com"));
	QNetworkReply *reply = manager.get(request);
	QEventLoop loop;
	QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	loop.exec();
	if (reply->error() == QNetworkReply::NoError) {
		qDebug() << cn("网络已连接");
		return true;
	}
	else {
		qDebug() << cn("网络未连接");
		return false;
	}
}

/*加载本地电子围栏文件，并将其撒点在地图上*/
void ElectricFenceOnMap::on_pBtn_loadFile_clicked()
{
	switch (readElectricFenceFile()) 
	{
	case CommonStatus::success:
		emit showRunningInfo_signal(cn("围栏文件读取成功"));
		showPolygonOnMap();
		emit showRunningInfo_signal(cn("电子围栏在地图上已标出"));
		ui.pBtn_loadFile->setEnabled(false);
		ui.pBtn_start->setEnabled(true);
		break;

	case CommonStatus::file_not_exist:
		emit showRunningInfo_signal(cn("围栏文件不存在"));
		break;

	case CommonStatus::file_open_fail:
		emit showRunningInfo_signal(cn("围栏文件打开失败"));
		break;

	case CommonStatus::file_wrong:
		emit showRunningInfo_signal(cn("围栏文件错误"));
		break;

	default:
		break;
	}
}

/*根据输入的经纬度点，判断其是否在围栏内*/
void ElectricFenceOnMap::on_pBtn_start_clicked()
{
	double longitude = 0.0;
	double latitude = 0.0;
	vector<Vec2d> polygon_mem;

	longitude = ui.lineEdit_lon->text().toDouble();
	latitude = ui.lineEdit_lat->text().toDouble();
	//qDebug() << "longitude" << longitude << "latitude" << latitude;
	if (longitude == 0.0 || latitude == 0.0) {
		emit showRunningInfo_signal(cn("请输入一个经纬度点"));
		return;
	}
	clicked_count++;
	_myChannel->sendCoordinates(QString::number(longitude, 'f', 8), QString::number(latitude, 'f', 8), QString::number(clicked_count));
	
	/*判断是否已经加载电子围栏文件*/
	if (elec_fence_vector.size() == 0) {
		emit showRunningInfo_signal(cn("请检查电子围栏是否存在"));
		return;
	}

	for (int i = 0; i < elec_fence_vector.size(); i++) {	//遍历每个多边形（包含编号）
		polygon_mem.clear();
		polygon_mem = elec_fence_vector[i];	//取出一个多边形（包含编号）

		for (int j = 0; j < polygon_mem.size(); j++) {	//遍历多边形每一个点
			
			if (Point_In_Polygon_2D(longitude, latitude, polygon_mem)) {
				emit showRunningInfo_signal(cn("在电子围栏内，围栏编号：%1").arg(polygon_mem[j].id));
				return;
			}
			
		}

		if (i == elec_fence_vector.size() - 1) {
			emit showRunningInfo_signal(cn("在电子围栏外"));
			return;
		}
	}
}

/*根据输入的经纬度点，判断其是否在围栏内*/
void ElectricFenceOnMap::on_pBtn_draw_clicked()
{
	QMessageBox::StandardButton reply;
	reply = QMessageBox::warning(this, cn("绘制围栏"), cn("是否确认绘制围栏？"), QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes) {	// 用户选择确认
		/*判断是否加载过围栏文件*/
		if (elec_fence_vector.size() > 0) {
			reply = QMessageBox::warning(this, cn("绘制围栏"), cn("重新绘制需要先清除地图已有围栏，你确定继续吗？"), QMessageBox::Yes | QMessageBox::No);
			if (reply == QMessageBox::Yes) {	//用户选择确认
				/*清除地图已有围栏点*/
				_myChannel->clearMarkerSignal();
			}
			else {	//用户选择取消
				return;
			}
		}

		/*清空上一次绘制的围栏数据链表*/
		qstrList_elec.clear();
		/*绘制状态由不可绘制改为可绘制*/
		_myChannel->sendChangeStatusSignal();
		/*点击开始绘制之后，将绘制按钮设为不可再次点击*/
		ui.pBtn_draw->setEnabled(false);
		/*点击开始绘制之后，将保存成文件按钮设为可点击*/
		ui.pBtn_save->setEnabled(true);
		/*点击开始绘制之后，将开始按钮设为不可点击*/
		ui.pBtn_start->setEnabled(false);
	}
	else {	// 用户选择取消
		return;
	}

}

void ElectricFenceOnMap::on_pBtn_save_clicked()
{
	qDebug() << "qstrList_elec" << qstrList_elec;
	/*判断是否绘制了新围栏*/
	if (qstrList_elec.size() == 0) {
		QMessageBox::information(this, cn("提示"), cn("请先绘制围栏再点击保存！"));
		return;
	}

	QFile file(ELECTRIC_FILE);
	if (file.exists()) {
		QMessageBox::StandardButton reply;
		reply = QMessageBox::warning(this, cn("保存文件"), cn("文件已存在，是否覆盖原围栏？"), QMessageBox::Yes | QMessageBox::No);
		if (reply == QMessageBox::Yes) {	// 用户选择确认
			remove(ELECTRIC_FILE);	//移除原有围栏文件
		}
		else {	//用户选择取消
			/*清空绘制的围栏字符串列表*/
			qstrList_elec.clear();
			return;
		}
	}

	/*将新画的围栏数据写入文件中*/
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
	
	/*点击保存文件之后，将绘制按钮设为可再次点击*/
	ui.pBtn_draw->setEnabled(true);
	/*点击保存文件之后，将保存成文件按钮设为不可再次点击*/
	ui.pBtn_save->setEnabled(false);
	/*点击保存文件之后，将加载围栏文件按钮设为可点击*/
	ui.pBtn_loadFile->setEnabled(true);
	
	QMessageBox::information(this, cn("提示"), cn("围栏已重新绘制，请点击加载围栏文件按钮重新加载围栏！"));
}

void ElectricFenceOnMap::saveElectricFenceFile_slot(QString longitude, QString latitude)
{
	//qDebug() << "[longitude,latitude]" << longitude << "," << latitude;
	QString elec_row = longitude.append(",") + latitude.append(",") + QString::number(1);
	qstrList_elec.append(elec_row);
}

/*读取本地电子围栏CSV文件*/
CommonStatus::CommonStatus ElectricFenceOnMap::readElectricFenceFile()
{
	QFile inFile(ELECTRIC_FILE);
	QStringList lines;

	if (inFile.exists()) {
		if (inFile.open(QIODevice::ReadOnly)) {
			/*读文件之前先将负责存储围栏数据的vector清空*/
			elec_fence_rect.clear();
			elec_fence_vector.clear();

			QTextStream stream_text(&inFile);
			while (!stream_text.atEnd()) {
				lines.push_back(stream_text.readLine());	//行读取
			}

			for (int i = 0; i < lines.size(); i++) {
				QString line = lines.at(i);	//获取一行的数据
				QStringList list_split = line.split(",");	//以逗号拆分
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

/*将电子围栏文件中的多边形框显示在地图上*/
void ElectricFenceOnMap::showPolygonOnMap()
{
	vector<Vec2d> polygon_mem;

	/*先清除地图已绘制的围栏点*/
	_myChannel->clearMarkerSignal();

	for (int i = 0; i < elec_fence_vector.size(); i++) {	//遍历每个多边形（包含编号）
		polygon_mem = elec_fence_vector[i];	//取出一个多边形（包含编号）

		for (int j = 0; j < polygon_mem.size(); j++) {	//遍历多边形每一个点
			QString lon = QString::number(polygon_mem[j].x, 'f', 8);
			QString lat = QString::number(polygon_mem[j].y, 'f', 8);
			QString id = polygon_mem[j].id;

			_myChannel->sendPolygon_typeInuse(lon, lat, id);
		}
		
	}
}

//判断点在线段上
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

//判断两线段相交
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

//判断点在多边形内
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
	double linePoint2x = minX - 10;			//取最小的X值还小的值作为射线的终点
	double linePoint2y = y;

	//遍历每一条边
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

		if (fabs(cy2 - cy1) < EPSILON)   //平行则不相交
		{
			continue;
		}

		if (IsPointOnLine(cx1, cy1, linePoint1x, linePoint1y, linePoint2x, linePoint2y))
		{
			if (cy1 > cy2)			//只保证上端点+1
			{
				count++;
			}
		}
		else if (IsPointOnLine(cx2, cy2, linePoint1x, linePoint1y, linePoint2x, linePoint2y))
		{
			if (cy2 > cy1)			//只保证上端点+1
			{
				count++;
			}
		}
		else if (IsIntersect(cx1, cy1, cx2, cy2, linePoint1x, linePoint1y, linePoint2x, linePoint2y))   //已经排除平行的情况
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

/*显示运行信息槽函数*/
void ElectricFenceOnMap::showRunningInfo_slot(QString p_text)
{
	QString m_text = p_text;
	qDebug() << "m_text" << m_text;
	ui.textBrowser->append(m_text);
	ui.textBrowser->moveCursor(QTextCursor::End);
}

//经度转换
double ElectricFenceOnMap::translate_lon(double lon, double lat)
{
	double ret = 300.0 + lon + 2.0*lat + 0.1*lon*lon + 0.1*lon*lat + 0.1*sqrt(abs(lon));
	ret += (20.0 * sin(6.0*lon*PI) + 20.0*sin(2.0*lon*PI)) *2.0 / 3.0;
	ret += (20.0 * sin(lon*PI) + 40.0*sin(lon / 3.0 *PI)) *2.0 / 3.0;
	ret += (150 * sin(lon / 12.0 *PI) + 300.0*sin(lon / 30.0 * PI)) *2.0 / 3.0;
	return ret;
}

//纬度转换
double ElectricFenceOnMap::translate_lat(double lon, double lat)
{
	double ret = -100 + 2.0*lon + 3.0*lat + 0.2*lat*lat + 0.1*lon*lat + 0.2*sqrt((abs(lon)));
	ret += (20.0 *sin(6.0*lon*PI) + 20 * sin(2.0*lon*PI)) *2.0 / 3.0;
	ret += (20.0 *sin(lat*PI) + 40.0*sin(lat / 3.0*PI)) *2.0 / 3.0;
	ret += (160.0*sin(lat / 12.0*PI) + 320.0*sin(lat / 30.0 *PI)) *2.0 / 3.0;
	return ret;
}

/*84坐标系转高德坐标*/
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

/*高德坐标转84坐标*/
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
