#pragma once
#include "wsn.h"
#include "Zigbee.h"
class Wifi :
	public wsn
{
private:
	enum WifiStatus
	{
		REQUEST_SENDING = 5 * NOISE_PARA,
		ASK_RECVING = 3 * NOISE_PARA,
		CONFIRM_SENDING = 4 * NOISE_PARA
	};

private:
	WifiStatus status;
	WifiStatus lastStatus;
	bool rssi[ASK_RECVING][SAMPLE_TIME] = { false };
	int foldSum[SAMPLE_TIME];
	int neighborTable[ZIGBEE_NUM] = { -1 };
	int waitToConfirm = 0;
	int waitTime = -1;
	int isSendAlready;

public:
	Wifi();
	~Wifi();

	bool IsDiscoverZigbee();
	void showAllNeighbors();
	bool IsDiscoverCorrect(Zigbee zigbees[]);

	bool SendSignalToEnv(int time);
	void RecvSignalFromEnv(int time, bool signal);

	int GetMaxFoldSum();
};

