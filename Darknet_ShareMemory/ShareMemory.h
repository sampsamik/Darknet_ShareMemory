#ifndef _SHAREMEMORY_H
#define _SHAREMEMORY_H

#include <opencv2\opencv.hpp>
#include <Windows.h>

extern LPVOID pBuffer;

//Mat header
typedef struct
{
	int width;
	int height;
	int type;
}MatHeader;

// The size of each block
#define FLAG_SIZE			sizeof(char)
#define MAT_HEADER_SIZE		sizeof(MatHeader)
#define MAT_SIZE			1920*1080*3*8

#define MEMORY_SIZE			FLAG_SIZE + MAT_HEADER_SIZE + MAT_SIZE

// memory first address
#define FLAG_ADDRESS		(char*)pBuffer
#define MAT_HEADER_ADDRESS	FLAG_ADDRESS + FLAG_SIZE
#define MAT_DATA_ADDRESS	MAT_HEADER_ADDRESS + MAT_HEADER_SIZE



bool initShareString();

//func
bool initShareMemory();
void writeFlag2ShareMemory(char* FLAG_Address, char value);
void readFlagFromShareMemory(char* FLAG_Address, char& value);
void writeMat2ShareMemory(char* MatHeadAddress, char* MatDataAddress, cv::Mat image);
void ReadMatFromShareMemory(char* MatHeadAddress, char* MatDataAddress, cv::Mat& image);

void writeString2ShareMemory(char* STRING_Address, std::string strValue);
void readStringFromShareMemory(std::string& strValue);

struct detectedObject
{
	int id;
	float w;
	float h;
	float x;
	float y;
	float conf;
};

void drawBbox(cv::Mat& frame, detectedObject detObj);
float get_color(int c, int x, int max);
void getDetectedObjects(std::string objects, std::vector<detectedObject> &detObjs, std::string delimiter = "-");

#endif // !_SHAREMEMORY_H
