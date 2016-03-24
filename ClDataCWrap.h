#include "clusterData.h"

//#ifndef __ClDataCWrap
//#define __ClDataCWrap
#include <VrLib/ClusterData.h>


class ClDataCWrap
{
public:
	ClDataCWrap(void);
	~ClDataCWrap(void);

	void init(void);
	bool isLocal(void);

	void setData(float x, float y, float z, float az, float el, float roll, double time, int cluster, bool);
	
	float getX();
	float getY();
	float getZ();
	float getAz();
	float getEl();
	float getRoll();
	double getTime();
	bool getStop();
	int getCluster();

private:
	//Declararatie gedeelde netwerk variabelen.
	//cluster::UserData<clusterData> gedeeldeData;

	vrlib::ClusterData<clusterData> gedeeldeData;
	

};

extern "C" bool CD_isLocal(void);
extern "C" void CD_init(void);
extern "C" void CD_setData(float x, float y, float z, float az, float el, float roll, double time, int cluster, bool stop);
extern "C" float CD_getX();
extern "C" float CD_getY();
extern "C" float CD_getZ();
extern "C" float CD_getAz();
extern "C" float CD_getEl();
extern "C" float CD_getRoll();
extern "C" double CD_getTime();
extern "C" int CD_getCluster();
extern "C" bool CD_getStop();


//#endif