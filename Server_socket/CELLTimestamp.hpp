#ifndef _CELLTIMESTAMP_HPP_
#define _CELLTIMESTAMP_HPP_

#include<chrono>

class CELLTimestamp {
public:
	CELLTimestamp();
	~CELLTimestamp();

	void update();
	long long getElapsedTiemLnMicroSec();	//获取微秒
	double getElapsedTimeInMilliSec();	//获取毫秒
	double getElapsedSecond();	//获取秒
private:
	std::chrono::time_point<std::chrono::high_resolution_clock>_begin;
};
CELLTimestamp::CELLTimestamp() {
	update();
}
CELLTimestamp::~CELLTimestamp() {
}
//初始化_begin成员变量为当前时间
void CELLTimestamp::update() {
	_begin = std::chrono::high_resolution_clock::now();
}
//获取微秒
long long CELLTimestamp::getElapsedTiemLnMicroSec() {
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - _begin).count();
}
//获取毫秒
double CELLTimestamp::getElapsedTimeInMilliSec() {
	return this->getElapsedTiemLnMicroSec() * 0.001;
}
//获取秒
double CELLTimestamp::getElapsedSecond() {
	return  this->getElapsedTiemLnMicroSec() * 0.000001;
}

#endif