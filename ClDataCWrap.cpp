#include ".\cldatacwrap.h"

ClDataCWrap::ClDataCWrap(void)
{

}

ClDataCWrap::~ClDataCWrap(void)
{
}

void ClDataCWrap::init(void)
{
	//Ken een unieke identifier to aan de data die over het netwerk gedeeld gaat worden.
//	vpr::GUID onzeGUID("1dc19050-efd8-11da-8ad9-0800200c9a66");
	gedeeldeData.init();

}

bool ClDataCWrap::isLocal(){
	return gedeeldeData.isLocal();
}

void ClDataCWrap::setData(float x, float y, float z, float az, float el, float roll, double time, int cluster, bool stop){
	gedeeldeData->setData(x,y,z,az,el,roll, time,cluster, stop);
}

float ClDataCWrap::getX(void){
	return gedeeldeData->getX();
}

float ClDataCWrap::getY(void){
	return gedeeldeData->getY();
}

float ClDataCWrap::getZ(void){
	return gedeeldeData->getZ();
}
float ClDataCWrap::getAz(void){
	return gedeeldeData->getAz();
}
float ClDataCWrap::getEl(void){
	return gedeeldeData->getEl();
}
float ClDataCWrap::getRoll(void){
	return gedeeldeData->getRoll();
}
double ClDataCWrap::getTime(void){
	return gedeeldeData->getTime();
}

int ClDataCWrap::getCluster(void){
	return gedeeldeData->getCluster();
}
bool ClDataCWrap::getStop()
{
	return gedeeldeData->getStop();
}


ClDataCWrap dataCWrap;

bool CD_isLocal(){
	return dataCWrap.isLocal();
}

void CD_init(){
	dataCWrap.init();
}

void CD_setData(float x, float y, float z, float az, float el, float roll, double time, int cluster, bool stop){
	dataCWrap.setData(x,y,z,az,el,roll, time,cluster, stop);
}

float CD_getX(){
	return dataCWrap.getX();
}

float CD_getY(){
	return dataCWrap.getY();
}

float CD_getZ(){
	return dataCWrap.getZ();
}

float CD_getAz(){
	return dataCWrap.getAz();
}

float CD_getEl(){
	return dataCWrap.getEl();
}
float CD_getRoll(){
	return dataCWrap.getRoll();
}

double CD_getTime(){
	return dataCWrap.getTime();
}

int CD_getCluster(){
	return dataCWrap.getCluster();
}

bool CD_getStop()
{
	return dataCWrap.getStop();
}