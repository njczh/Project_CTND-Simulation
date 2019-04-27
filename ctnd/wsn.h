#pragma once
//#define DEBUG
#include <iostream>

constexpr auto SAMPLE_TIME = 800;	// 每个信标周期的采样
constexpr auto NOISE_PARA = 2;	// 噪声参数
constexpr auto ZIGBEE_NUM = 4;	// 环境中参与邻居发现的zigbee设备数量

class wsn
{
public:
	int addr;				// 物联网设备物理地址（唯一）
	int startupTime;		// 设备启动时间
	int referencePosition;	// freebee通讯所需的参考位置
	int sendBeaconTime;		// 用于统计在不同阶段发送的信标次数

public:
	wsn();
	~wsn();

	// 设置随机地址
	int SetRandomAddr();
	
	/* 广播信号，返回true表示，广播了信号；返回false表示没有广播信号 */
	virtual bool SendSignalToEnv(int time) = 0;
	/* 接收信号 */
	virtual void RecvSignalFromEnv(int time, bool Signal) = 0;
	/* 计算折叠和并返回最大折叠和列 */
	virtual int GetMaxFoldSum() = 0;
};

