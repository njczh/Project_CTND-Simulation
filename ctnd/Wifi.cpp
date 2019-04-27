#include "pch.h"
#include "Wifi.h"


Wifi::Wifi()
{
	addr = SAMPLE_TIME - 1;
	startupTime = 0; // TODO
	referencePosition = 0;
	status = lastStatus = REQUEST_SENDING;
	sendBeaconTime = 0;
	for (int i = 0; i < ZIGBEE_NUM; i++)
		neighborTable[i] = -1;
	waitToConfirm = 0;	// 等待确认的数量
	waitTime = -1;		// 等待时间
	isSendAlready = 0; // 用于记录某周期是否已经发送过信号
}


Wifi::~Wifi()
{
}

bool Wifi::IsDiscoverZigbee()
{
	for (int i = 0; i < ZIGBEE_NUM; i++)
		if (neighborTable[i] <= 0)
			return false;
	return true;
}

void Wifi::showAllNeighbors()
{
	std::cout << "WIFI's neighbor:";
	for (int i = 0; i < ZIGBEE_NUM; i++)
	{
		std::cout << "\t" << i + 1 << ".Zigbee_" << neighborTable[i] << "\t";
	}
	std::cout << std::endl;
}

bool Wifi::IsDiscoverCorrect(Zigbee zigbees[])
{
	int correctNum = 0;
	for (int i = 0; i < ZIGBEE_NUM; i++)
	{
		for (int j = 0; j < ZIGBEE_NUM; j++)
		{
			if (neighborTable[i] == zigbees[j].addr){
				correctNum++; break;
			}
		}
	}
	if (correctNum == ZIGBEE_NUM)
		return true;
	else
		return false;
}

bool Wifi::SendSignalToEnv(int envTime)
{
	bool signalToSend = false;	// 待发送的信号

	int signalPosition = (envTime - startupTime) % SAMPLE_TIME;

	switch (status)
	{
	case Wifi::REQUEST_SENDING:
		// 在每个信标周期的reference position时间位置广播信标
		if (signalPosition == referencePosition) {
			lastStatus = REQUEST_SENDING;
#ifdef DEBUG 
			std::cout << "WI-FI(RS)  ->\t";//REQUEST_SENDING
			//std::cout << "In (envTime:) " << envTime
			//	<< ", WIFI send a signal at (position:) " << signalPosition
			//	<< " as REQUEST_SENDING" << std::endl;
#endif	
			signalToSend = true;
		}
		else
			status = ASK_RECVING;	// 发送完立刻转入接收状态
		break;
	case Wifi::ASK_RECVING:
		break;
	case Wifi::CONFIRM_SENDING:
		// 在每个信标周期的addr时间位置广播信标
		if (signalPosition == addr ) {
			lastStatus = CONFIRM_SENDING;
#ifdef DEBUG 
			std::cout << "WI-FI(CS)  ->\t";//CONFIRM_SENDING
			//std::cout << "In (envTime:) " << envTime
			//	<< ", WIFI send a signal at (position:) " << signalPosition 
			//	<< " as CONFIRM_SENDING" << std::endl;
#endif	
			signalToSend = true;
			if (++sendBeaconTime == CONFIRM_SENDING) {
				if (--waitToConfirm > 0) {
					waitTime = NOISE_PARA * 2;
				}
				sendBeaconTime = 0;
				status = REQUEST_SENDING;
			}
		}
		else
			status = ASK_RECVING;	// 发送完立刻转入接收状态
		break;
	default:
		break;
	}

	return signalToSend;
}

void Wifi::RecvSignalFromEnv(int envTime, bool signal)
{
	switch (status)
	{
	case Wifi::REQUEST_SENDING:
		break;
	case Wifi::ASK_RECVING: {
		int nextPosition = (envTime + 1 - startupTime) % SAMPLE_TIME;
		int signalPosition = (envTime - startupTime) % SAMPLE_TIME;
		int cyclePosition = ((envTime - startupTime) / SAMPLE_TIME) % status;
		rssi[cyclePosition][signalPosition] = signal;
#ifdef DEBUG 
		if (signal)
			std::cout << "WI-FI(AR)\t";//ASK_RECVING

			//std::cout << "In (envTime:) " << envTime
			//<< ", WIFI received a signal at (position:) " << signalPosition
			//<< " as ASK_RECVING" << std::endl;
#endif
		if (nextPosition == referencePosition) {
			if (GetMaxFoldSum() == ASK_RECVING) { // 收到回复信标组
				lastStatus = CONFIRM_SENDING;
				waitToConfirm++;
			}
			else if(waitTime > 0 && --waitTime == 0)
				lastStatus = CONFIRM_SENDING;
			else
				status = REQUEST_SENDING;
			
		}
		else if (nextPosition == addr)
			if (lastStatus == CONFIRM_SENDING)
				status = CONFIRM_SENDING;
	}
		break;
	case Wifi::CONFIRM_SENDING:
		if (GetMaxFoldSum() == ASK_RECVING) { // 收到回复信标组
			lastStatus = CONFIRM_SENDING;
			waitToConfirm++;
		}
		break;
	default:
		break;
	}
}

int Wifi::GetMaxFoldSum()
{
	int max = -1;

	for (int j = 0; j < SAMPLE_TIME; j++)
		foldSum[j] = 0;

	for (int i = 0; i < ASK_RECVING; i++)
	{
		for (int j = 0; j < SAMPLE_TIME; j++)
		{
			foldSum[j] += rssi[i][j];
			//std::cout << rssi[i][j] << " ";
		}
		//std::cout << std::endl;
	}

	for (int j = 0; j < SAMPLE_TIME; j++)
		if (foldSum[j] == ASK_RECVING) {
			max = ASK_RECVING;
			for (int k = 0; k < ZIGBEE_NUM; k++) {
				int temp = j - referencePosition;
				if (temp == neighborTable[k]) break;
				if (neighborTable[k] <= 0) {
					neighborTable[k] = temp;
					//std::cout << "WIFI discover:" << neighborTable[k] << "\t";
					break;
				}
			}
		}
	return max;
}
