#include "pch.h"
#include "Zigbee.h"

int Zigbee::addrs[ZIGBEE_NUM] = {-1};	// zigbee设备地址集
int Zigbee::nowNum = 0;					// 当前环境中zigbee设备数量

Zigbee::Zigbee()
{
	addr = SetTheOnlyAddr();
	startupTime = rand() % (SAMPLE_TIME * 12 * NOISE_PARA); // TODO
	referencePosition = -1;
	status = REQUEST_RECVING;
	neighborTable = -1;
	sendBeaconTime = 0;
	wakeupTime = 0;
	RRCycleTime = 0;

#ifdef DEBUG 
	std::cout << "Zigbee No." << addr << " will be start up at (time): " << startupTime << std::endl;
#endif	

}

Zigbee::~Zigbee()
{
}

int Zigbee::SetTheOnlyAddr()
{
	bool flag = false;
	addr = SetRandomAddr() % (SAMPLE_TIME - 2) + 1; // TODO 采样频率的信标位移粒度不一样，是1.024ms和128us的关系
	while (!flag) {
		flag = true;
		for (int i = 0; i < nowNum; i++)
		{
			if (addrs[i] == addr) {
				addr = SetRandomAddr() % (SAMPLE_TIME - 2) + 1;
				flag = false;
				break;
			}
		}
	}
	addrs[nowNum++] = addr;

	return addr;
}

bool Zigbee::IsDiscoverWifi()
{
	if (neighborTable == -1)
		return false;
	else
		return true;
}

bool Zigbee::SendSignalToEnv(int time)
{
	bool signalToSend = false;

	if (status == ASK_SENDING && time >= startupTime && neighborTable == -1)
	{
		if ((time - startupTime) % SAMPLE_TIME == (addr + referencePosition) % SAMPLE_TIME) {
			if (sendBeaconTime++ == ASK_SENDING-1) {
				sendBeaconTime = -1;
			}
			signalToSend = true;
			wakeupTime++; //std::cout << wakeupTime;

#ifdef DEBUG 
			std::cout << "ZB_" << addr << " (AS)  ->\t";//ASK_SENDING
#endif	

		}
		if ((sendBeaconTime == -1) && ((time-startupTime)%SAMPLE_TIME == referencePosition)) {
			status = CONFIRM_RECVING;
			for (int i = 0; i < status; i++)
				for (int j = 0; j < SAMPLE_TIME; j++)
					rssi[i][j]=0;
		}
	}

	return signalToSend;
}

void Zigbee::RecvSignalFromEnv(int envTime, bool signal)
{
	if (envTime >= startupTime) {

		int nextPosition = (envTime + 1 - startupTime) % SAMPLE_TIME;
		int signalPosition = (envTime - startupTime) % SAMPLE_TIME;
		int cyclePosition = ((envTime - startupTime) / SAMPLE_TIME) % status;

		if (neighborTable != -1)
			return;

		switch (status)
		{
		case Zigbee::REQUEST_RECVING:
#ifdef OPTIMIZE
			if (OptRRDutyCycle(envTime, signalPosition)) {
				rssi[cyclePosition][signalPosition] = signal;
				wakeupTime++; //std::cout << wakeupTime; // DEBUG
			}
			else
				rssi[cyclePosition][signalPosition] = false;
#else
			rssi[cyclePosition][signalPosition] = signal;
			wakeupTime++;
#endif // OPTIMIZE

#ifdef DEBUG 
			if (rssi[cyclePosition][signalPosition])
				std::cout << "ZB_"<<addr<<" (RR)\t"; //REQUEST_RECVING
#endif	

			if (nextPosition == 0) {
				int temp = GetMaxFoldSum();
				if (temp != -1) {
					referencePosition = temp;
					// std::cout << "referencePosition"<<referencePosition ;
					status = ASK_SENDING;
				}
			}
			break;
		case Zigbee::ASK_SENDING:
			break;
		case Zigbee::CONFIRM_RECVING:
#ifdef OPTIMIZE
			if (signalPosition == (referencePosition + SAMPLE_TIME - 1) % SAMPLE_TIME) {
				rssi[cyclePosition][signalPosition] = signal;
				wakeupTime++; //std::cout << wakeupTime;
			}
			else {
				rssi[cyclePosition][signalPosition] = false;
			}
#else
			rssi[cyclePosition][signalPosition] = signal;
			wakeupTime++;
#endif // OPTIMIZE

#ifdef DEBUG 
			if (rssi[cyclePosition][signalPosition])
				std::cout << "ZB_" << addr << " (CR)\t";//CONFIRM_RECVING
#endif	

			if (nextPosition == 0) {
				int temp = GetMaxFoldSum();
				if (temp != -1) {
					neighborTable = (referencePosition - temp) % SAMPLE_TIME;
					status = REQUEST_RECVING;
				}
			}
			break;
		default:
			break;
		}
	}
}

int Zigbee::GetMaxFoldSum()
{
	bool flag = false; // 检查是否有多于一列达到额阈值范围
	int max = -1;

	for (int j = 0; j < SAMPLE_TIME; j++)
		foldSum[j] = 0;

	for (int i = 0; i < status; i++)
	{
		for (int j = 0; j < SAMPLE_TIME; j++)
		{
			foldSum[j] += rssi[i][j];
			//std::cout << rssi[i][j] << " ";
		}
		//std::cout << std::endl;
	}

	for (int j = 0; j < SAMPLE_TIME; j++) {
		//std::cout << foldSum[j] << " ";
		if (foldSum[j] >= status && foldSum[j]<status+NOISE_PARA) {
			switch (status)
			{
			case Zigbee::REQUEST_RECVING:
				if (!flag) {
					max = j; flag = true;
				}
				else
					std::cout << "bug！#1" << std::endl;
				break;
			case Zigbee::ASK_SENDING:
				break;
			case Zigbee::CONFIRM_RECVING:
				if (j != referencePosition)
					max = j;
				//else
					//std::cout << "bug！#2" << std::endl;
				break;
			default:
				break;
			}

		}
	}
	//std::cout << std::endl;

	return max;
}

int Zigbee::GetWakeupTime()
{
	return wakeupTime;
}

bool Zigbee::OptRRDutyCycle(int envTime, int signalPosition)
{
	int cycleTime = (envTime - startupTime) / SAMPLE_TIME;
	int foldSum = 0;
	if (cycleTime < 3)
		return true;
	else {
		for (int i = 0; i < REQUEST_RECVING; i++)
			foldSum += rssi[i][signalPosition];
		if (foldSum == cycleTime)
			return true;
		else	
			return false;
	}

}
