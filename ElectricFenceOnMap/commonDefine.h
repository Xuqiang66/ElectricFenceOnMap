#pragma once
#include "ElectricFenceOnMap.h"
#include <iostream>
#include <vector>

using namespace std;

constexpr auto PI = 3.1415926535897932384626;
constexpr auto a = 6378245.0;
constexpr auto ee = 0.00669342162296594323;
constexpr auto ELECTRIC_FILE = "./electric_fence.csv";

#define EPSILON 0.000001

typedef struct _POSITION
{
	double longitude;
	double latitude;
}POSITION;

typedef struct {
	double lon;
	double lat;
	int id;
}elec_fence_info_t;

namespace CommonStatus {
	enum CommonStatus {
		success,
		file_not_exist,
		file_open_fail,
		file_wrong
	};
}

//¶þÎ¬doubleÊ¸Á¿
struct  Vec2d
{
	double x, y;
	QString id;

	Vec2d()
	{
		x = 0.0;
		y = 0.0;
		id = "";
	}
	Vec2d(double dx, double dy, QString did)
	{
		x = dx;
		y = dy;
		id = did;
	}
	void Set(double dx, double dy, QString did)
	{
		x = dx;
		y = dy;
		id = did;
	}
};