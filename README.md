Example using shared memory for building applications with Darknet object detection running in the background. Made for windows.

Compile darknet (https://github.com/AlexeyAB/darknet) after replacing the files in Darknet_src.

### Usage:

Run darknet
.\darknet.exe detector demo .\cfg\coco.data .\cfg\yolov4.cfg .\backup\yolov4.weights -SM

Run example (main.cpp)
.\Darknet_ShareMemory.exe

### Requirements  
  
Visual Studio 2017  
OpenCV 3-4
