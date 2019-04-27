// ctnd.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <iomanip>
#include <windows.h>
#include <fstream>


//#define NOISE
using namespace std;

constexpr auto SIMULATION_TIMES = 500;
constexpr auto FAIL_THRESHOLD = SAMPLE_TIME * 1000;


void sysInit();
bool IsNeighborDiscoverFinished(Wifi, Zigbee[]);
bool GetNoiseSignal(int envTime);
int oneTimeSimulation(int &delay, double&dutycycle,bool&isDiscoverCorrect);
double CaluAllZigbeeWuTime(Zigbee zigbees[],int envTime);

int main()
{
	int successTime = 0;
	int failTime = 0;
	int totalDelayTime = 0;
	double totalDutycycle = 0;
	ofstream saveFile("result.txt");

	srand((unsigned)time(NULL));

	for (int i = 0; i < SIMULATION_TIMES; i++)
	{
		if (SIMULATION_TIMES != 1)
			cout << "第 " << setw(4) << setfill(' ') << i + 1 << " 次模拟仿真" << endl;
		int tempDl;
		double tempDc;
		bool isDiscoverCorrect;
		oneTimeSimulation(tempDl, tempDc, isDiscoverCorrect);
		if (tempDl != -1 && isDiscoverCorrect) {
			successTime++; 
			totalDelayTime += tempDl;
			totalDutycycle += tempDc; 
			saveFile << tempDc<<"\n";
		}
		else
			failTime++;
		if (SIMULATION_TIMES != 1)
			system("cls");
	}

	cout << "\n================================== SYSTEM RUNNING COMPLETE!  SIMULATION FINISH!  ==================================\n";
	cout << "SAMPLE_TIME: " << SAMPLE_TIME
		<< ",\t\tNOISE_PARA: " << NOISE_PARA
		<< ",\t\t\tZIGBEE_NUM: " << ZIGBEE_NUM << endl;
	cout << "TOTAL SIMULATION TIME: " << SIMULATION_TIMES
		<< ", \tSUCCESS time: " << successTime << "(" << (double)successTime / SIMULATION_TIMES * 100 << "%)"
		<< ", \tFAIL time: " << failTime << endl;
	cout << "Average discovery delay: " << (double)totalDelayTime / SIMULATION_TIMES * 0.1024 << "s";
	cout << "\t\t\t\tAverage Zigbee DutyCycle: " << (double)totalDutycycle / SIMULATION_TIMES << "%";
	cout << "\n===================================================================================================================\n\n";

	saveFile.close();
}

int oneTimeSimulation(int &delay, double&dutycycle, bool&isDiscoverCorrect)
{
	//srand((unsigned)time(NULL));

	int envTime = 0;				// 环境时钟
	bool envToZigbee = false;		// 某时刻zigbee设备可以感测到的能量信号
									// 来源包括①WiFi设备、②其他zigbee设备以及③环境噪声
	bool envToWifi = false;			// 某时刻WiFi感测的能量信号，来源包括zigbee设备与环境噪声

	Wifi wifi;
	Zigbee zigbees[ZIGBEE_NUM];

	/*--------------------------------------------------------------------------------*/

	sysInit();

	while (!IsNeighborDiscoverFinished(wifi, zigbees))
	{
#ifdef DEBUG 
		if (envTime % SAMPLE_TIME == 0)
			cout << "---------------------------------------------------------"
			<< "--------------------------------------------------------- "
			<< envTime / SAMPLE_TIME << endl;
		std::cout << "Env Time: " << setw(3) << setfill('0') << envTime << "\t";
#endif 
		/****************************** 发送信号 *********************************/
#ifdef NOISE
		bool noiseSignal = GetNoiseSignal(envTime);
#else
		bool noiseSignal = false;
#endif // NOISE

#ifdef DEBUG 
		if (noiseSignal)
			std::cout << "  [NOISE]  =>\t";
		else
			std::cout << "\t\t";
#endif
		envToZigbee = wifi.SendSignalToEnv(envTime) || noiseSignal; // ①、③

		for (int i = 0; i < ZIGBEE_NUM; i++)
		{
			bool temp = zigbees[i].SendSignalToEnv(envTime);
			envToWifi |= temp;			// ① 对WiFi而言来自zigbee设备的信号
			envToZigbee |= temp;		// ② 对zigbee而言其他zigbee设备的信号
		}
		envToWifi |= noiseSignal;		// ②


		/****************************** 接收信号 *********************************/

		wifi.RecvSignalFromEnv(envTime, envToWifi);

		for (int i = 0; i < ZIGBEE_NUM; i++)
			zigbees[i].RecvSignalFromEnv(envTime, envToZigbee);


		/********************************************************************/

#ifdef DEBUG 
		std::cout << std::endl;
#endif	

		// 环境时间递增1，环境信号重置	
		envTime++;	
		//Sleep(10);
		envToZigbee = false;
		envToWifi = false;
		if (envTime > FAIL_THRESHOLD) return -1;
	}

#ifdef DEBUG 
	std::cout << "Env Time: " << setw(3) << setfill('0') << envTime << "\t";
	std::cout << "  [ O K ] NEIGHBOR DISCOVER FINISHED!" << std::endl;
	if (SIMULATION_TIMES != 1) system("cls");
	wifi.showAllNeighbors();
	cout << "Average Zigbees Wakeup TIME :" << CaluAllZigbeeWuTime(zigbees, envTime) << "%" << endl;
#endif	
	delay = envTime / SAMPLE_TIME;
	dutycycle = CaluAllZigbeeWuTime(zigbees, envTime);
	isDiscoverCorrect = wifi.IsDiscoverCorrect(zigbees);
}


void sysInit()
{
	for (int i = 0; i < ZIGBEE_NUM; i++)
		Zigbee::addrs[i] = -1;			// zigbee设备地址集
	Zigbee::nowNum = 0;					// 当前环境中zigbee设备数量

#ifdef DEBUG 
	cout << "\n================================== SYSTEM INITIALIZE COMPLETE! SIMULATION BEGIN! ==================================\n\n";
#endif	

}

bool IsNeighborDiscoverFinished(Wifi wifi, Zigbee zigbees[])
{
	if (wifi.IsDiscoverZigbee() == false)
		return false;

	for (int i = 0; i < ZIGBEE_NUM; i++)
		if (zigbees[i].IsDiscoverWifi() == false)
			return false;

	return true;
}

bool GetNoiseSignal(int envTime)
{
	if (NOISE_PARA == 0)
		return false;
	int temp = rand() % (SAMPLE_TIME / NOISE_PARA);
	if (temp == 0)
		return true;
	else
		return false;
}

double CaluAllZigbeeWuTime(Zigbee zigbees[],int envTime)
{
	double total = 0;
	for (int i = 0; i < ZIGBEE_NUM; i++)
		total += ((double)zigbees[i].GetWakeupTime() / (envTime-zigbees[i].startupTime));
	return total / ZIGBEE_NUM * 100;
}