#ifndef _CELLTIMESTAMP_HPP_
#define _CELLTIMESTAMP_HPP_

#include<chrono>

class CELLTimestamp {
public:
	CELLTimestamp();
	~CELLTimestamp();

	void update();
	long long getElapsedTiemLnMicroSec();	//��ȡ΢��
	double getElapsedTimeInMilliSec();	//��ȡ����
	double getElapsedSecond();	//��ȡ��
private:
	std::chrono::time_point<std::chrono::high_resolution_clock>_begin;
};
CELLTimestamp::CELLTimestamp() {
	update();
}
CELLTimestamp::~CELLTimestamp() {
}
//��ʼ��_begin��Ա����Ϊ��ǰʱ��
void CELLTimestamp::update() {
	_begin = std::chrono::high_resolution_clock::now();
}
//��ȡ΢��
long long CELLTimestamp::getElapsedTiemLnMicroSec() {
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - _begin).count();
}
//��ȡ����
double CELLTimestamp::getElapsedTimeInMilliSec() {
	return this->getElapsedTiemLnMicroSec() * 0.001;
}
//��ȡ��
double CELLTimestamp::getElapsedSecond() {
	return  this->getElapsedTiemLnMicroSec() * 0.000001;
}

#endif