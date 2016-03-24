#include ".\clusterdata.h"
#include <iostream>

clusterData::clusterData(void)
{
	std::cout << "Ik ben er!!!" << std::endl;
	stop = false;
}

clusterData::~clusterData(void)
{
}


/*vpr::ReturnStatus clusterData::writeObject(vpr::ObjectWriter* writer){
	writer->writeFloat(x);
	writer->writeFloat(y);
	writer->writeFloat(z);
	writer->writeFloat(az);
	writer->writeFloat(el);
	writer->writeDouble(time);
	writer->writeUint32(cluster);

	return vpr::ReturnStatus();
}*/
void clusterData::writeObject(vrlib::BinaryStream& writer)
{
	writer<<x<<y<<z<<az<<el<<roll<<time<<stop<<cluster;
}

void clusterData::readObject(vrlib::BinaryStream& reader)
{
	reader>>x>>y>>z>>az>>el>>roll>>time>>stop>>cluster;
}


/*vpr::ReturnStatus clusterData::readObject(vpr::ObjectReader* reader){
	x = reader->readFloat();
	y = reader->readFloat();
	z = reader->readFloat();
	az = reader->readFloat();
	el = reader->readFloat();

	time = reader->readDouble();
	cluster = reader->readUint32();

	return vpr::ReturnStatus();
}*/

float clusterData::getX(void){
	return x;
}

float clusterData::getY(void){
	return y;
}

float clusterData::getZ(void){
	return z;
}

float clusterData::getAz(void){
	return az;
}

float clusterData::getEl(void){
	return el;
}
float clusterData::getRoll(void){
	return roll;
}

double clusterData::getTime(void){
	return time;
}

int clusterData::getCluster(void){
	return cluster;
}

bool clusterData::getStop()
{
	return stop;
}

void clusterData::setData(float x, float y, float z, float az, float el, float roll, double time, int cluster, bool stop){
	this->x = x;
	this->y = y;
	this->z = z;
	this->az = az;
	this->el = el;
	this->roll = roll;
	this->time = time;
	this->cluster = cluster;
	this->stop = stop;
}