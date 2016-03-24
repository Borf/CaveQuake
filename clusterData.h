#pragma once

#include <VrLib/BinaryStream.h>

class clusterData : public vrlib::SerializableObject
{
public:
	clusterData(void);
	~clusterData(void);
	
//	vpr::ReturnStatus writeObject(vpr::ObjectWriter* writer);
//	vpr::ReturnStatus readObject(vpr::ObjectReader* reader);
	virtual void writeObject(vrlib::BinaryStream& writer);
	virtual void readObject(vrlib::BinaryStream& reader);
	
	void setData(float x, float y, float z, float az, float el, float roll, double time, int cluster, bool);
	float getX(void);
	float getY(void);
	float getZ(void);
	float getAz(void);
	float getEl(void);
	float getRoll(void);
	double getTime(void);
	int getCluster(void);
	bool getStop();
	
private:
	float x,y,z,az,el;

	double time;
	int cluster;

	bool stop;
	float roll;

	
};
