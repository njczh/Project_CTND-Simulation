#pragma once
#include "wsn.h"
class Zigbee :
	public wsn
{
private:
	enum ZigbeeStatus
	{
		REQUEST_RECVING = 5 * NOISE_PARA,
		ASK_SENDING = 3 * NOISE_PARA,
		CONFIRM_RECVING = 4 * NOISE_PARA
	};

public:
	static int addrs[ZIGBEE_NUM];	// zigbee设备地址集
	static int nowNum;				// 当前环境中zigbee设备数量

private:
	ZigbeeStatus status;
	bool rssi[REQUEST_RECVING][SAMPLE_TIME] = { false };
	int foldSum[SAMPLE_TIME];
	int neighborTable;
	int sendBeaconTime;
	int wakeupTime = 0;

public:
	Zigbee();
	~Zigbee();

	int SetTheOnlyAddr();
	bool IsDiscoverWifi();
	bool SendSignalToEnv(int time);
	void RecvSignalFromEnv(int time, bool signal);
	int GetMaxFoldSum();
	int GetWakeupTime();
};

