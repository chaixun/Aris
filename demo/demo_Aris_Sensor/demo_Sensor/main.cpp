﻿#include "Platform.h"

#include <iostream>
#include <iomanip>

#include "Aris_Core.h"
#include "Aris_IMU.h"
#include "Aris_DynKer.h"
#ifdef PLATFORM_IS_LINUX
#include "Aris_Vision.h"
#endif

class SENSOR :public Aris::Sensor::SENSOR_BASE<double>
{
	virtual void UpdateData(double &data)
	{
		static double sensorData = 0;
		data = sensorData;
		sensorData++;
		Aris::Core::Sleep(10);
	}
};


#ifdef PLATFORM_IS_LINUX
Aris::Sensor::KINECT kinect;
#endif

int main()
{
	Aris::Core::DOCUMENT doc;
#ifdef PLATFORM_IS_WINDOWS
	doc.LoadFile("C:\\Robots\\resource\\Robot_Type_I\\Robot_III.xml");
#endif
#ifdef PLATFORM_IS_LINUX
	doc.LoadFile("/usr/Robots/resource/Robot_Type_I/Robot_III.xml");
#endif

/*
#ifdef PLATFORM_IS_LINUX
	kinect.Start();

	for (int i = 0; i < 1000; ++i)
	{
		auto data = kinect.GetSensorData();
		std::cout<<"data:"<<data.Get().gridMap[100][100]<<std::endl;
		Aris::Core::Sleep(100);
	}
	
	kinect.Stop();
#endif
*/
	/*
	auto p = doc.RootElement()->FirstChildElement("Server")->FirstChildElement("Sensors")->FirstChildElement("IMU");
	
	Aris::Sensor::IMU imu(p);

	imu.Start();
	
	for (int i = 0; i < 1000;++i)
	{
		auto data = imu.GetSensorData();

		double eul[3];
		//data.Get().ToBodyEul(eul);
		//Aris::DynKer::dsp(eul, 1, 3);
		
		data.Get().ToEulBody2Ground(eul, PI, "321");
		Aris::DynKer::dsp(eul, 1, 3);

		//double pm[16];
		//data.Get().ToBodyPm(pm, 0.0);
		//Aris::DynKer::dsp(pm, 4, 4);

		//Aris::DynKer::dsp(data.Get().eul321, 1, 3);

		Aris::Core::Sleep(1);
	}

	imu.Stop();
	*/

	//SENSOR sensor;

	//sensor.Start();

	//for (int i = 0; i < 200;++i)
	//{
	//	
	//	
	//	auto data = sensor.GetSensorData();

	//	//std::cout << data.Get()<< std::endl;

	//	Aris::Core::Sleep(1);
	//}

	//{
	//	auto data = sensor.GetSensorData();

	//	std::cout << data.Get() << std::endl;
	//}
	//sensor.Stop();



	Aris::Core::RT_MSG::instance[0].Copy("123");


	char aaa;
	std::cin>>aaa;
	return 0;
}

