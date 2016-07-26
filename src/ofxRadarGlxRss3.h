#pragma once

#pragma once

#include "ofMain.h"
#include "ofxJSON.h"

#define RECONNECTING_TIME 5000
#define READABLE_FRAMERATE 200
#define CIRCLE_BLOB_SIZE 4
#define MIN_CIRCLE_STRENGHT_BLOB_SIZE 2
#define MAX_CIRCLE_STRENGHT_BLOB_SIZE 13
#define MIN_STRENGHT_DETECTED 2000000
#define MAX_STRENGHT_DETECTED 20000000
#define CIRCLE_CORNERS_TRACKINGAREA 4

struct processedData {
	int64_t id;
	ofVec2f pos;
	ofVec2f normPos;
	bool bTracking;
	int speedKm;
	int strength;
};

struct resumedTrackingData {
	ofPolyline trackingRegion;
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

class ofxQuadHomography {

public:

	vector<ofPoint> inPoints; //Input points to warp
	vector<ofPoint> outPoints; //by homography to output points

	void ofxQuadHomography::setInPoints(const vector<ofPoint>& _inPoints) {
		inPoints.clear();
		inPoints.insert(inPoints.end(), _inPoints.begin(), _inPoints.end());
	}

	void ofxQuadHomography::setOutPoints(const vector<ofPoint>& _outPoints) {
		outPoints.clear();
		outPoints.insert(outPoints.end(), _outPoints.begin(), _outPoints.end());
	}
	
	//--------------------------------------------------------------
	ofPoint ofxQuadHomography::getCorrespondenceHomografy(ofVec2f auxPoint) {
		ofPoint destPoint;
		//auto mat = findHomography(quad1, quad2);
		//destPoint = auxPoint*mat
		return destPoint;
	}

};

class ofxRadarGlxRss3 {
public:
	ofxRadarGlxRss3();
    void setup(string _ipRadar);
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
	
	void setupRadar(string _ip);
	void playSimFile(int _idPosFileInFolder);
	int getNumSimFiles();
	targetData getTargetData(int idTarget);
	ofPoint getCartesianTargetData(int idTarget);
	float sensorScale = 2.5;

	vector<processedData> radarPostData;
	resumedTrackingData resumedPostData;
	void setTrackingArea_leftUpCorner(ofPoint _point);
	void setTrackingArea_leftBottomCorner(ofPoint _point);
	void setTrackingArea_rightUpCorner(ofPoint _point);
	void setTrackingArea_rightBottomCorner(ofPoint _point);

	void playNextSimFile();
	void playPrevSimFile();

	float getRadarWidth();
	float getRadarHeight();
	float getRadarAccurateHeight();
	float getRadarMaxDistance();
	

private:

	//Warping methods
	ofxQuadHomography myHomography;

	///////////////////////////////////////////
	//Radar Functions
	bool parsingSuccessful = false;
	void updateRadar();
	void updateResumedData();
	void resetAllTargetVars();
	//Area Radar
	void defineIdealRadarArea();
	void defineRadarArea();
	ofPolyline radarAreaEffective, radarArea;
	void drawIdealAreaTracking(int x, int y);

	void drawResumedPostData(int _x, int _y);

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
	string ipRadar = "";
	float sensorHeight = 0;
	float sensorWidth = 0;
	float sensorAccurateHeight = 0;
	float angleMaxRangeRadar = 80; //Seems to be 80, but accuarate data it's inside the 40 degrees...



	//Record 
	void updateRecording();
	bool bRecording = false;
	bool bStartRecording = false;
	int framesRecorded = 0;
	//Player

	string fileName = "default.json";	//Save Load Simulation
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