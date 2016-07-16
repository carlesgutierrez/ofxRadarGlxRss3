#pragma once

#pragma once

#include "ofMain.h"
#include "ofxJSON.h"
#include "ofxCsv.h"

#define RECONNECTING_TIME 5000

struct targetData {
	int64_t id;
	int distance;
	int angleDegree; //Neg values (left), Positive values (right)
	int speedKm;
	int strength;
};

class ofxRadarGlxRss3 {
public:
	ofxRadarGlxRss3();
    void setup();
	void update();
    void draw();
	ofxJSONElement json;
	bool parsingSuccessful = false;

	
	void setupRadar();

	//Area Radar
	void defineIdealRadarArea();
	ofPolyline radarAreaEffective;
	void drawIdealAreaTracking(int x, int y);

	//Tracking Info
	void drawRawTextRadarInfo();
	void drawRawTextRadarDetection();
	void drawBlobsCartesian(int x, int y);

	void radarPolarToCartesian();
	vector<ofVec2f> cartesianRadar;
	ofVec2f transformPolarToCartesian(float _distance, float _angleDegree);

	void readJsonDataRadar();
	void parseJsonDataRadar();


	int timerReader = 0;
	std::string url;
	std::string textStatusRadar;

	vector<targetData> targetsData;
	std::string stargetResumedData;

	//Radar Related Data
	float sensorScale = 2;
	float sensorMaxDistance = 0;

	float sensorHeight = 0;
	float sensorWidth = 0;
	float sensorAccurateHeight = 0;

	int angleMaxRadar = 40;

	//Save Load Simulation
	void updateRecording();
	void startRecorging();
	void stopRecorging();
	bool bRecording = false;
	bool bStartRecording = false;
	int framesRecorded = 0;
	ofxCsv myCsvData;

};


/* 
Improve reading CVS
https://forum.openframeworks.cc/t/large-csv-file/9006/4


Thanks for your help. FYI I guess I have found the fastest possible way for my situation. I just wrote a little program to convert CSV files to binary files and used below code to read it:

fstream myFile ((ofToString(currentFrame) + ".bin").c_str(), ios::in | ios::binary);
myFile.seekg(0);

int num_points;
myFile.read ((char*)&num_points, sizeof (int));

allPoints = new pointData[num_points];

myFile.read ((char*)allPoints, sizeof (pointData) * num_points);

myFile.close();

Before it was about 7 frames (files) per second, now it is around 300 fps.

*/