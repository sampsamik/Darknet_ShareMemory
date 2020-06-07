#include "ShareMemory.h"

using namespace cv;
using namespace std;

int main()
{
	if (!initShareMemory())
	{
		cout << "Could not initiate shared memory" << endl;
		return -1;
	}

	//Open webcam
	VideoCapture cap(0);
	Mat frame;
	while (true)
	{
		try
		{
			//Get flag for shared memory
			char flag;
			readFlagFromShareMemory(FLAG_ADDRESS, flag);

			//Capture and share frame 
			if (0 == flag)
			{
				cap.read(frame);
				writeMat2ShareMemory(MAT_HEADER_ADDRESS, MAT_DATA_ADDRESS, frame);
				writeFlag2ShareMemory(FLAG_ADDRESS, 1);
			}

			//When the image has been processed in Darknet
			if (2 == flag)
			{

				//Must be initiated after the sender has initiated shared string(?)
				if (!initShareString())
				{
					cout << "Could not initiate shared string" << endl;
					return -1;
				}

				//Read string from Darknet
				string strval;
				readStringFromShareMemory(strval);

				///Do stuff with detected objects
				//If we receive detected objects, save them to a vector of detected objects
				if (strval.length() > 0)
				{
					vector<detectedObject> detectedObjects;
					getDetectedObjects(strval, detectedObjects);

					Mat frame_bbox = frame.clone();

					//Draw boundingbox for each detected object
					for (int i = 0; i < detectedObjects.size(); i++)
					{

						/*cout << detectedObjects[i].id << " "
							<< detectedObjects[i].w << " "
							<< detectedObjects[i].h << " "
							<< detectedObjects[i].x << " "
							<< detectedObjects[i].y << " "
							<< detectedObjects[i].conf << endl; */

						drawBbox(frame_bbox, detectedObjects[i]);
					}

					//Show capture frame with detected objects
					imshow("bbox", frame_bbox);

				}

				//Ready to send next frame
				writeFlag2ShareMemory(FLAG_ADDRESS, 0);
			}

			//Show capture frame
			imshow("frame", frame);

			//Exit when pressing escape key
			char key_press = waitKey(1);
			if (key_press == 27)
			{
				break;
			}

		}
		catch (...)
		{
			//We want to make sure Darknet closes if an error occurs in our program
			cout << "Error occurred. Setting flag 3." << endl;
			writeFlag2ShareMemory(FLAG_ADDRESS, 3);
			cap.release();
			destroyAllWindows();
			return 0;
		}
	}
	writeFlag2ShareMemory(FLAG_ADDRESS, 3); //Close darknet
	cap.release();
	destroyAllWindows();
	return 0;
}