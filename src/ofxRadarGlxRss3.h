#pragma once

#pragma once

#include "ofMain.h"
#include "ofxJSON.h"
#include "ofxCsv.h"

#define RECONNECTING_TIME 5000
#define READABLE_FRAMERATE 200

struct processedData {
	int64_t id;
	ofVec2f pos;
	ofVec2f vel;
	int strength;
};

struct resumedTrackingData {
	ofRectangle trackingRegion;
	float density; //Density = NumberOfTargets*(sizeByUser or streght target) / (AreaTrackWidth + AreaTrackHeight) 
	float average_vel;//average_vel = Sum(VelTargets[N])/N then ofMap(ResumedVelocity , -30, 30, 0, 1, )
};

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
	void updateOnlineRadarData();
    void draw();
	ofxJSONElement jsonRadar;
	ofxJSONElement jsonRecordingRadar;
	ofxJSONElement jsonSimulationRadar;

	void startPlaying();
	bool isPlaying();
	bool isRecording();
	void startRecorging();
	void stopRecorging();
	
	void setupRadar();
	void playSimFile(int _idPosFileInFolder);
	int getNumSimFiles();
	targetData getTargetData(int idTarget);
	ofPoint getCartesianTargetData(int idTarget);
	float sensorScale = 2;

	processedData radarPostData;
	resumedTrackingData resumedPostData;
	void playNextSimFile();
	void playPrevSimFile();

private:

	bool parsingSuccessful = false;
	void updateRadar();
	void updateResumedData();

	//Area Radar
	void defineIdealRadarArea();
	ofPolyline radarAreaEffective;
	void drawIdealAreaTracking(int x, int y);

	//Tracking Info
	void drawSmallHorizontalLine(int _x, int _y);
	void drawSmallVerticalLine(int _x, int _y);
	void drawRadarCross(int _x, int _y);
	void drawRawTextRadarInfo();
	void drawRawTextRadarDetection();
	void drawBlobsCartesian(int x, int y);

	void radarPolarToCartesian();
	vector<ofVec2f> cartesianRadar;
	ofVec2f transformPolarToCartesian(float _distance, float _angleDegree);

	void readJsonDataRadar();
	void parseJsonDataRadar();//Radar
	void parseJsonDataRadar(ofxJSONElement jsonRadar);//SimulationRadar


	int timerReader = 0;
	std::string url;
	std::string textStatusRadar;

	vector<targetData> targetsData;
	std::string stargetResumedData;

	//Radar Related Data
	float sensorMaxDistance = 0;

	float sensorHeight = 0;
	float sensorWidth = 0;
	float sensorAccurateHeight = 0;

	int angleMaxRadar = 40;

	//Save Load Simulation
	string fileName = "default.json";

	//write
	void updateRecording();
	bool bRecording = false;
	bool bStartRecording = false;
	int framesRecorded = 0;
	//read
	int idSimulaTorFile = 0;
	bool bPlaying = false;
	bool bStartPlaying= false;
	ofxJSONElement readingSimulationRadar();
	int counterFramesSimulation = 0;
	int totalFramesSimulation = 0;

	ofDirectory myDataFolder;
	int idActualSimulatorIter = 0;




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