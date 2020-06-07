#include "ShareMemory.h"

HANDLE hMapFile;
LPVOID pBuffer;


//Send strings
#define BUF_SIZE 4800    //100 objects per image
HANDLE hMapString;
LPCSTR pBufString;

bool initShareString()
{
	std::string strMemoryName = "ShareStringMemory";

	// Open a memory mapped file object named "ShareStringMemory"
	hMapString = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		(LPCSTR)strMemoryName.c_str());               // name of mapping object
	if (hMapString == NULL)
	{
		printf(TEXT("Could not open file mapping object (%d).\n"));
		return 0;
	}

	pBufString = (LPTSTR)MapViewOfFile(hMapString,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		BUF_SIZE);

	if (pBufString == NULL)
	{
		printf(TEXT("Could not map view of file (%d).\n"));

		CloseHandle(hMapString);

		return 0;
	}

	return 1;
}

bool initShareMemory()
{
	std::string memeryName = "SharePicture";

	hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		MEMORY_SIZE,
		(LPCSTR)memeryName.c_str()
	);

	if (hMapFile == NULL)
	{
		return false;
	}

	pBuffer = (LPTSTR)MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		MEMORY_SIZE
	);

	if (pBuffer == NULL)
	{
		CloseHandle(hMapFile);
		return false;
	}
	// Initialize the shared memory content is all 0
	memset((char*)pBuffer, 0, MEMORY_SIZE);
	return true;
}

void writeFlag2ShareMemory(char * FLAG_Address, char value)
{
	memcpy(FLAG_Address, &value, sizeof(value));
}

void readFlagFromShareMemory(char * FLAG_Address, char& value)
{
	memcpy(&value, FLAG_Address, sizeof(value));
}

void writeString2ShareMemory(char* STRING_Address, std::string strValue)
{
	//memcpy(STRING_ADDRESS, &strValue, sizeof(strValue));
}

void readStringFromShareMemory(std::string& strValue)
{
	strValue = pBufString;
}

void writeMat2ShareMemory(char * MatHeadAddress, char * MatDataAddress, cv::Mat image)
{
	MatHeader matHead;
	matHead.width = image.cols;
	matHead.height = image.rows;
	matHead.type = image.type();

	// Write the information header to the memory
	memcpy(MatHeadAddress, &matHead, sizeof(MatHeader));

	// Calculate the number of bytes occupied by the Mat element according to the remainder of 8 by Mat type()
	int ByteNum = 1;
	switch (image.type() % 8)
	{
	case 0:
	case 1:
		ByteNum = 1;
		break;
	case 2:
	case 3:
		ByteNum = 2;
		break;
	case 4:
	case 5:
		ByteNum = 4;
		break;
	case 6:
		ByteNum = 8;
		break;
	}

	int write_bits = 0;
	for (int row = 0; row < image.rows; ++row)
	{
		write_bits = row * image.cols * image.channels() * ByteNum;
		memcpy(MatDataAddress + write_bits, image.ptr(row), image.cols * image.channels() * ByteNum);
	}
}

void ReadMatFromShareMemory(char * MatHeadAddress, char * MatDataAddress, cv::Mat & image)
{
	// Read the information header
	MatHeader matHead;
	memcpy(&matHead, MatHeadAddress, sizeof(MatHeader));

	// Calculate the number of bytes occupied by the Mat element according to the remainder of 8 by Mat type()
	int ByteNum = 1;
	switch (matHead.type % 8)
	{
	case 0:
	case 1:
		ByteNum = 1;
		break;
	case 2:
	case 3:
		ByteNum = 2;
		break;
	case 4:
	case 5:
		ByteNum = 4;
		break;
	case 6:
		ByteNum = 8;
		break;
	}

	// Create image
	image.create(matHead.height, matHead.width, matHead.type);

	// Copy data from memory
	int write_bits = 0;
	for (int row = 0; row < matHead.height; ++row)
	{
		write_bits = row * image.cols * image.channels() * ByteNum;
		memcpy(image.ptr(row), MatDataAddress + write_bits, image.cols * image.channels() * ByteNum);
	}
}

void drawBbox(cv::Mat& frame, detectedObject detObj)
{
	int class_id = detObj.id;
	float w = detObj.w;
	float h = detObj.h;
	float x = detObj.x;
	float y = detObj.y;

	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << detObj.conf;
	std::string conf = stream.str();

	int width = max(1.0f, frame.rows * .002f);

	int classes = 10;

	int offset = class_id * 123457 % classes;
	float red = get_color(2, offset, classes);
	float green = get_color(1, offset, classes);
	float blue = get_color(0, offset, classes);
	float rgb[3];

	rgb[0] = red;
	rgb[1] = green;
	rgb[2] = blue;

	int left = (x - w / 2.)*frame.cols;
	int right = (x + w / 2.)*frame.cols;
	int top = (y - h / 2.)*frame.rows;
	int bot = (y + h / 2.)*frame.rows;

	if (left < 0) left = 0;
	if (right > frame.cols - 1) right = frame.cols - 1;
	if (top < 0) top = 0;
	if (bot > frame.rows - 1) bot = frame.rows - 1;

	float const font_size = frame.rows / 1000.F;
	cv::Size const text_size = cv::getTextSize("0-" + conf, cv::FONT_HERSHEY_COMPLEX_SMALL, font_size, 1, 0);
	cv::Point pt1, pt2, pt_text, pt_text_bg1, pt_text_bg2;
	pt1.x = left;
	pt1.y = top;
	pt2.x = right;
	pt2.y = bot;
	pt_text.x = left;
	pt_text.y = top - 4;// 12;
	pt_text_bg1.x = left;
	pt_text_bg1.y = top - (3 + 18 * font_size);
	pt_text_bg2.x = right;
	if ((right - left) < text_size.width) pt_text_bg2.x = left + text_size.width;
	pt_text_bg2.y = top;
	cv::Scalar color;
	color.val[0] = red * 256;
	color.val[1] = green * 256;
	color.val[2] = blue * 256;

	cv::rectangle(frame, pt1, pt2, color, width, 8, 0);

	cv::rectangle(frame, pt_text_bg1, pt_text_bg2, color, width, 8, 0);
	cv::rectangle(frame, pt_text_bg1, pt_text_bg2, color, cv::FILLED, 8, 0);    // filled
	cv::Scalar black_color = CV_RGB(0, 0, 0);
	cv::putText(frame, std::to_string(class_id) + "-" + conf, pt_text, cv::FONT_HERSHEY_COMPLEX_SMALL, font_size, black_color, 2 * font_size);
	// cv::FONT_HERSHEY_COMPLEX_SMALL, cv::FONT_HERSHEY_SIMPLEX
}

float get_color(int c, int x, int max)
{
	float colors[6][3] = { {1,0,1}, {0,0,1},{0,1,1},{0,1,0},{1,1,0},{1,0,0} };

	float ratio = ((float)x / max) * 5;
	int i = floor(ratio);
	int j = ceil(ratio);
	ratio -= i;
	float r = (1 - ratio) * colors[i][c] + ratio * colors[j][c];
	//printf("%f\n", r);
	return r;
}

//Parse the string into detected objects
void getDetectedObjects(std::string objects, std::vector<detectedObject> &detObjs, std::string delimiter)
{

	if (objects.length() > 0)
	{
		size_t pos = 0;
		size_t pos2 = 0;
		while ((pos = objects.find(delimiter)) != std::string::npos)
		{
			detectedObject detObj;
			detObj.id = stoi(objects.substr(0, pos));
			objects.erase(0, pos + delimiter.length());

			pos = objects.find(delimiter);
			detObj.w = stof(objects.substr(0, pos));
			objects.erase(0, pos + delimiter.length());

			pos = objects.find(delimiter);
			detObj.h = stof(objects.substr(0, pos));
			objects.erase(0, pos + delimiter.length());

			pos = objects.find(delimiter);
			detObj.x = stof(objects.substr(0, pos));
			objects.erase(0, pos + delimiter.length());

			pos = objects.find(delimiter);
			detObj.y = stof(objects.substr(0, pos));
			objects.erase(0, pos + delimiter.length());

			pos = objects.find(delimiter);
			detObj.conf = stof(objects.substr(0, pos));
			objects.erase(0, pos + delimiter.length());

			detObjs.push_back(detObj);
		}
	}

}
