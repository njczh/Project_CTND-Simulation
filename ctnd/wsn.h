#pragma once
//#define DEBUG
#include <iostream>

constexpr auto SAMPLE_TIME = 800;	// ÿ���ű����ڵĲ���
constexpr auto NOISE_PARA = 2;	// ��������
constexpr auto ZIGBEE_NUM = 4;	// �����в����ھӷ��ֵ�zigbee�豸����

class wsn
{
public:
	int addr;				// �������豸�����ַ��Ψһ��
	int startupTime;		// �豸����ʱ��
	int referencePosition;	// freebeeͨѶ����Ĳο�λ��
	int sendBeaconTime;		// ����ͳ���ڲ�ͬ�׶η��͵��ű����

public:
	wsn();
	~wsn();

	// ���������ַ
	int SetRandomAddr();
	
	/* �㲥�źţ�����true��ʾ���㲥���źţ�����false��ʾû�й㲥�ź� */
	virtual bool SendSignalToEnv(int time) = 0;
	/* �����ź� */
	virtual void RecvSignalFromEnv(int time, bool Signal) = 0;
	/* �����۵��Ͳ���������۵����� */
	virtual int GetMaxFoldSum() = 0;
};

