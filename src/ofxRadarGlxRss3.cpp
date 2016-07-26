#include "ofxRadarGlxRss3.h"

ofxRadarGlxRss3::ofxRadarGlxRss3()
{
}

//------------------------------------------------
void ofxRadarGlxRss3::setup(string _ip)
{
	timerReader = ofGetElapsedTimeMillis();

	ofLogVerbose(OF_LOG_SILENT);

	setupRadar(_ip);

	//TODO Set a theme
	ofSetBackgroundColor(50, 50, 50);

}

//------------------------------------------------
void ofxRadarGlxRss3::setupRadar(string _ip) {

	ipRadar = _ip;
	url = "http://"+ipRadar+"/scan_radars";

	sensorMaxDistance = 150; //TODO program from from JSON Data Value
	angleMaxRangeRadar = 80;//Seems that now can be 80. But only 40 effectively-
	cout << "Radar Accurate Angle is = " << angleMaxRangeRadar*0.5 << " but the maximum angle detection is " << angleMaxRangeRadar << endl;
	
	//First Calc Max Values From Radar setup values
	float auxX;
	float auxY;
	float degreeRad = ofDegToRad(angleMaxRangeRadar*0.25);
	auxX = sensorMaxDistance * cos(degreeRad);
	auxY = sensorMaxDistance * sin(degreeRad);

	float auxX2;
	float auxY2;
	float degreeRad2 = ofDegToRad(angleMaxRangeRadar*0.5);
	auxX2 = sensorMaxDistance * cos(degreeRad2);
	auxY2 = sensorMaxDistance * sin(degreeRad2);
	
	ofVec2f posYMaxAccurateAngle = ofVec2f(auxX, auxY);
	ofVec2f posYMaximumAngle = ofVec2f(auxX2, auxY2);

	//ofVec2f posYMaxAccurateAngle = transformPolarToCartesian(sensorMaxDistance, angleMaxRangeRadar*0.25);
	//ofVec2f posYMaximumAngle = transformPolarToCartesian(sensorMaxDistance, angleMaxRangeRadar*0.5);

	sensorHeight = posYMaximumAngle.y * 2; // posYMaximumAngle is only half upper of the radar 
	sensorAccurateHeight = posYMaxAccurateAngle.y * 2; // posYMaxAccurateAngle is only half upper of the radar
	sensorWidth = sensorMaxDistance;

	//Setup Tracking Region
	vector<ofPoint> quadRadar;
	quadRadar.push_back(ofPoint(0, 0));
	quadRadar.push_back(ofPoint(sensorWidth, 0));
	quadRadar.push_back(ofPoint(sensorWidth, sensorHeight));
	quadRadar.push_back(ofPoint(0,  sensorHeight));

	resumedPostData.trackingRegion = ofPolyline(quadRadar);
	//Rect set(-sensorWidth*0.5, -sensorHeight*0.5, sensorWidth, sensorHeight);
	myHomography.setInPoints(quadRadar);

	defineIdealRadarArea();
	defineRadarArea();

	ofSetCircleResolution(40);

}

//------------------------------------------------
void ofxRadarGlxRss3::updateResumedData() {

	radarPostData.clear();
	//radarPostData.reserve(targetsData.size());

	for (int i = 0; i < targetsData.size(); i++) {
		processedData newProcessedData;
		newProcessedData.id = targetsData[i].id;
		newProcessedData.pos = cartesianRadar[i];

		//Calc a Basic and normalized postition inside the sensorDimension//TODO in tracking area defined
		if (resumedPostData.trackingRegion.inside(newProcessedData.pos)) {
			newProcessedData.bTracking = true;
			newProcessedData.normPos = ofVec2f(newProcessedData.pos.x / sensorWidth, (newProcessedData.pos.y / sensorHeight));
		}
		else {
			newProcessedData.bTracking = false;
			newProcessedData.normPos = ofVec2f(-1, -1);
		}

		newProcessedData.strength = targetsData[i].strength;

		newProcessedData.speedKm = targetsData[i].speedKm;

		radarPostData.push_back(newProcessedData);
	}

}

//------------------------------------------------
void ofxRadarGlxRss3::updateRadar() {
	readJsonDataRadar();
	radarPolarToCartesian();
	updateResumedData();
}

//------------------------------------------------
void ofxRadarGlxRss3::update() {

	if (bPlaying) {
		if (ofGetElapsedTimeMillis() - timerReader > READABLE_FRAMERATE) {
			//will playing Simulation data if its available
			updateRadar();
			timerReader = ofGetElapsedTimeMillis();
		}
	}
	else {
		//Different timings here if trying to connect not successful
		updateOnlineRadarData();
	}
}

void ofxRadarGlxRss3::updateOnlineRadarData() {
	if (parsingSuccessful) {
		//readJSon data
		if (ofGetElapsedTimeMillis() - timerReader > READABLE_FRAMERATE) {
			updateRadar();
			timerReader = ofGetElapsedTimeMillis();
			if (bRecording)updateRecording();
		}
	}
	else {
		//else not connected try it againg each 5 seconds
		//or playing another data
		if (ofGetElapsedTimeMillis() - timerReader > RECONNECTING_TIME) {
			readJsonDataRadar();
			timerReader = ofGetElapsedTimeMillis();
		}
	}
}

//------------------------------------------------
void ofxRadarGlxRss3::parseJsonDataRadar() {
	//Clear last frame data
	jsonRadar.clear(); 

	// Now parse the JSON
	parsingSuccessful = jsonRadar.open(url);

	if (parsingSuccessful)
	{
		ofLogNotice("ofxRadarGlxRss3::setup") << jsonRadar.getRawString(true);
	}
	else {
		ofLogNotice("ofxRadarGlxRss3::setup") << "Failed to parse JSON.";
	}
}

//------------------------------------------------
void ofxRadarGlxRss3::parseJsonDataRadar(ofxJSONElement _jsonRadar) {
	// Auto parsed success
	parsingSuccessful = true;

	jsonRadar.clear(); //Clear last frame datat
	jsonRadar = _jsonRadar; //Save the new one

}

//------------------------------------------------
void ofxRadarGlxRss3::readJsonDataRadar() {

	//Auto clean if no new data
	targetsData.clear();

	//Filll TargetsData with onlin
	if (bPlaying) {//TODO Set bSimulationMode
		parseJsonDataRadar(readingSimulationRadar());
	}
	else {
		parseJsonDataRadar();
	}

	if (jsonRadar["targets"].size() > 0) {

		//targetsData.reserve(jsonRadar["targets"].size());

		for (Json::ArrayIndex i = 0; i < jsonRadar["targets"].size(); ++i) {

			targetData auxTarget;
			auxTarget.id = jsonRadar["targets"][i]["id"].asInt64();
			auxTarget.distance = jsonRadar["targets"][i]["distance"].asInt();
			auxTarget.angleDegree = jsonRadar["targets"][i]["angle"].asInt();
			auxTarget.speedKm = jsonRadar["targets"][i]["speed"].asInt();
			auxTarget.strength = jsonRadar["targets"][i]["strength"].asInt();

			targetsData.push_back(auxTarget);
		}
	}

	//Any way this is always visible and should update timeStamp value properly ( try to find optimized frameRate reading )

	int timestamp = jsonRadar["timestamp"].asInt();
	std::string json_units = jsonRadar["json_units"].asString();
	std::string ui_units = jsonRadar["ui_units"].asString();
	textStatusRadar = "TimeStamp[" + ofToString(timestamp, 0) + "] - json_units[" + json_units + "] - ui_units[" + ui_units + "]";



}

//------------------------------------------------
void ofxRadarGlxRss3::drawSmallHorizontalLine(int _x, int _y) {

	int ls = 5;

	ofPushMatrix();
	ofTranslate(_x, _y);
	ofDrawLine(-ls, 0, ls, 0); //Horizontal
	ofPopMatrix();
}

//------------------------------------------------
void ofxRadarGlxRss3::drawSmallVerticalLine(int _x, int _y) {
	
	int ls = 5;
	
	ofPushMatrix();
	ofTranslate(_x, _y);
	ofDrawLine(0, -ls, 0, ls); //Vertical
	ofPopMatrix();
}

//------------------------------------------------
void ofxRadarGlxRss3::drawRadarCross(int _x, int _y) {
	
	float detailCross = 10;
	//float segments = sensorAccurateHeight sensorMaxDistance / detailCross;
	
	
	ofSetColor(ofColor::navajoWhite);
	ofPushMatrix();
	ofTranslate(_x, _y);
	ofScale(sensorScale, sensorScale, 0);

	ofSetColor(ofColor::darkRed);
	ofDrawLine(-sensorWidth*0.5, 0, sensorWidth*0.5, 0); // Horizontal Line Cross
	ofDrawLine(0, -sensorHeight*0.5, 0, sensorHeight*0.5); // Vertical Line Cross

	ofSetColor(ofColor::white);
	//Cross detail distances in horizontal Line
	for (int i = 0; i < detailCross; i++) {
		float xdetailH = 0;
		int ydetailH = ofMap(i, 0, detailCross, -sensorAccurateHeight*0.5, sensorAccurateHeight*0.5);
		drawSmallHorizontalLine(xdetailH, ydetailH);
	}
	//Cross detail distances in Vertical Line
	for (int i = 0; i < detailCross; i++) {
		
		float xdetailV = ofMap(i, 0, detailCross, -sensorWidth*0.5, sensorWidth*0.5);
		int ydetailV = 0;
		drawSmallVerticalLine(xdetailV, ydetailV);
	}

	ofPopMatrix();
}

//------------------------------------------------
void ofxRadarGlxRss3::draw()
{
	ofSetColor(ofColor::white);

	drawRadarCross(ofGetWidth()*0.5, ofGetHeight()*0.5);

	if (parsingSuccessful) {

		drawRawTextRadarInfo();
		drawRawTextRadarDetection();
		
		drawIdealAreaTracking(ofGetWidth()*0.5 - sensorWidth*0.5, ofGetHeight()*0.5 - sensorHeight*0.5);
		drawBlobsCartesian(ofGetWidth()*0.5 - sensorWidth*0.5, ofGetHeight()*0.5 - sensorHeight*0.5);
		drawResumedPostData(ofGetWidth()*0.5 - sensorWidth*0.5, ofGetHeight()*0.5 - sensorHeight*0.5);

	}
	else {
		ofDrawBitmapString("Parsing not succsefull. Reconnecting in " + ofToString(RECONNECTING_TIME - (ofGetElapsedTimeMillis() - timerReader), 0), 20, 40);
	}

}

//---------------------------------------------
void ofxRadarGlxRss3::defineRadarArea() {

	ofPoint cornerPoint = ofPoint(0, sensorHeight*0.5);

	ofVec2f UpArc = transformPolarToCartesian(sensorMaxDistance, angleMaxRangeRadar*0.5);
	ofVec2f DownArc = transformPolarToCartesian(sensorMaxDistance, -angleMaxRangeRadar*0.5);


	radarArea.addVertex(cornerPoint);
	radarArea.addVertex(UpArc);
	radarArea.addVertex(DownArc);
	radarArea.close();

	//polylines[polylinesIndex].addVertex(ofPoint(args.x - imageRecognitionPosition.x, args.y - imageRecognitionPosition.y));
}

//---------------------------------------------
void ofxRadarGlxRss3::defineIdealRadarArea() {

	ofPoint cornerPoint = ofPoint(0, sensorHeight*0.5);

	ofVec2f UpArc = transformPolarToCartesian(sensorMaxDistance, angleMaxRangeRadar*0.25);
	ofVec2f DownArc = transformPolarToCartesian(sensorMaxDistance, -angleMaxRangeRadar*0.25);


	radarAreaEffective.addVertex(cornerPoint);
	radarAreaEffective.addVertex(UpArc);
	radarAreaEffective.addVertex(DownArc);
	radarAreaEffective.close();

	//polylines[polylinesIndex].addVertex(ofPoint(args.x - imageRecognitionPosition.x, args.y - imageRecognitionPosition.y));
}

//---------------------------------------------
void ofxRadarGlxRss3::drawIdealAreaTracking(int _x, int _y) {
	ofPushMatrix();

	//ofTranslate(_x, _y, 0);
	ofTranslate(_x + sensorMaxDistance*0.5, _y + sensorHeight*0.5, 0);
	ofScale(sensorScale, sensorScale, 0);
	ofTranslate(-sensorMaxDistance*0.5, -sensorHeight*0.5, 0);

	ofNoFill();

	ofSetColor(ofColor::orangeRed);
	ofDrawRectangle(0, 0, sensorMaxDistance, sensorHeight);

	ofSetColor(ofColor::yellow);
	ofDrawRectangle(0, sensorHeight*0.5 - sensorAccurateHeight*0.5, sensorMaxDistance, sensorAccurateHeight);

	ofSetColor(ofColor::darkSeaGreen);
	radarArea.draw();

	ofSetColor(ofColor::darkOliveGreen);
	radarAreaEffective.draw();
	

	ofPopMatrix();
}

//-----------------------------------------------
void ofxRadarGlxRss3::drawResumedPostData(int _x, int _y) {
	
	vector<ofPoint> ptsTrackingArea = resumedPostData.trackingRegion.getVertices();

	//For now we play with a simple QUAD 
	if (ptsTrackingArea.size() == 4) {

		ofPoint topLeft = ptsTrackingArea[0];
		ofPoint topRight = ptsTrackingArea[1];//ptsTrackingArea.trackingRegion.getTopRight();
		ofPoint bottomRight = ptsTrackingArea[2];//resumedPostData.trackingRegion.getBottomRight();
		ofPoint bottomLeft = ptsTrackingArea[3];//resumedPostData.trackingRegion.getBottomLeft();


		ofPushStyle();
		ofPushMatrix();
		ofTranslate(_x + sensorWidth*0.5, _y + sensorHeight*0.5, 0);
		//ofTranslate(_x, _y, 0);
		ofScale(sensorScale, sensorScale, 0);
		ofTranslate(-sensorWidth*0.5, -sensorHeight*0.5, 0);

		//Draw Circle Nodes
		ofSetColor(ofColor::limeGreen);
		ofNoFill();
		ofDrawCircle(topLeft, CIRCLE_CORNERS_TRACKINGAREA);
		ofDrawCircle(topRight, CIRCLE_CORNERS_TRACKINGAREA);
		ofDrawCircle(bottomLeft, CIRCLE_CORNERS_TRACKINGAREA);	
		ofDrawCircle(bottomRight, CIRCLE_CORNERS_TRACKINGAREA);
		
		//Node Lines
		ofDrawLine(topLeft, topRight);
		ofDrawLine(topRight, bottomRight);
		ofDrawLine(bottomRight, bottomLeft);
		ofDrawLine(bottomLeft, topLeft);

		ofPopMatrix();
		ofPopStyle();

	}
	
}

//---------------------------------------------
void ofxRadarGlxRss3::drawBlobsCartesian(int _x, int _y) {

	ofPushStyle();
	ofPushMatrix();
	ofTranslate(_x + sensorMaxDistance*0.5, _y + sensorHeight*0.5, 0);
	ofScale(sensorScale, sensorScale, 0);
	ofTranslate(-sensorMaxDistance*0.5, -sensorHeight*0.5, 0);

	ofFill();
	ofColor myBlobColor = ofColor::mediumVioletRed;
	ofEnableAlphaBlending();
	ofSetColor(myBlobColor.r, myBlobColor.g, myBlobColor.b, 200);
	for (int i = 0; i < radarPostData.size(); i++) {

		ofSetColor(ofColor::deepSkyBlue);
		ofDrawCircle(radarPostData[i].pos.x, radarPostData[i].pos.y, 5);

		//Map Strengt Into Out Circle Size no Fill
		ofNoFill();
		float strengCircleDim = ofMap(radarPostData[i].strength, MIN_STRENGHT_DETECTED, MAX_STRENGHT_DETECTED, MIN_CIRCLE_STRENGHT_BLOB_SIZE, MAX_CIRCLE_STRENGHT_BLOB_SIZE, true);
		ofSetColor(ofColor::paleVioletRed);
		ofDrawCircle(radarPostData[i].pos.x, radarPostData[i].pos.y, strengCircleDim);

		ofSetColor(ofColor::white);
		ofDrawBitmapString("x=" + ofToString(radarPostData[i].pos.x, 0), radarPostData[i].pos.x + 2, radarPostData[i].pos.y + 0);
		ofDrawBitmapString("y=" + ofToString(radarPostData[i].pos.y, 0), radarPostData[i].pos.x + 2, radarPostData[i].pos.y + 5);
		ofDrawBitmapString("id=" + ofToString(radarPostData[i].id, 0), radarPostData[i].pos.x + 2, radarPostData[i].pos.y + 10);
		ofDrawBitmapString("Speed=" + ofToString(radarPostData[i].speedKm, 0), radarPostData[i].pos.x + 2, radarPostData[i].pos.y + 15);
		ofDrawBitmapString("NormX=" + ofToString(radarPostData[i].normPos.x, 3), radarPostData[i].pos.x + 2, radarPostData[i].pos.y + 20);
		ofDrawBitmapString("NormY=" + ofToString(radarPostData[i].normPos.y, 3), radarPostData[i].pos.x + 2, radarPostData[i].pos.y + 25);
		
	}
	ofDisableAlphaBlending();
	ofPopMatrix();
	ofPopStyle();
}

//---------------------------------------------
void ofxRadarGlxRss3::drawRawTextRadarDetection() {

	ofSetColor(ofColor::indianRed);
	int gapInfoLines = 40;
	int gapInfoTargetInfo = 120;

	//Draw all Targets Info
	for (int i = 0; i < targetsData.size(); i++) {
		ofDrawBitmapString("id = " + ofToString(targetsData[i].id, 2), 20, i * gapInfoTargetInfo + gapInfoLines + 20);
		ofDrawBitmapString("distance = " + ofToString(targetsData[i].distance, 2), 20, i * gapInfoTargetInfo + gapInfoLines + 40);
		ofDrawBitmapString("angleDegree = " + ofToString(targetsData[i].angleDegree, 2), 20, i * gapInfoTargetInfo + gapInfoLines + 60);
		ofDrawBitmapString("speedKm = " + ofToString(targetsData[i].speedKm, 2), 20, i * gapInfoTargetInfo + gapInfoLines + 80);
		ofDrawBitmapString("strength = " + ofToString(targetsData[i].strength, 2), 20, i * gapInfoTargetInfo + gapInfoLines + 100);
		ofDrawBitmapString("---------------------", 20, i * gapInfoTargetInfo + gapInfoLines + 120);
	}
}

//-----------------------------------------------
void ofxRadarGlxRss3::drawRawTextRadarInfo() {

	int marginRight = ofGetWidth() - 300;
	int marginLeft = 20;
	////////////////////////////////////////////////
	ofSetColor(ofColor::green);
	//Draw Basic Info
	ofDrawBitmapString(textStatusRadar, marginLeft, 20);

	ofSetColor(ofColor::red);
	//Draw Resumed Blob Data
	ofDrawBitmapString(stargetResumedData = "Num targets= " + ofToString(jsonRadar["targets"].size(), 0), marginLeft, 40);

	////////////////////////////////////////////////
	//Some extra info
	ofSetColor(ofColor::whiteSmoke);
	ofDrawBitmapString("Scaling =" + ofToString(sensorScale, 2), marginRight, 10);
	ofDrawBitmapString("Framerate =" + ofToString(ofGetFrameRate(), 2), marginRight, 30);

	ofSetColor(ofColor::red);

	// Show the current mouse recording state
	if (bRecording) {
		ofDrawBitmapString("Recording Radar.. Frame #" + ofToString(framesRecorded, 0), marginRight, 100);
	}
	else if (bPlaying) {
		ofDrawBitmapString("Playing OffLine Radar " + fileName, marginRight, 80);
		ofDrawBitmapString("Num FramesSimulation = " + ofToString(counterFramesSimulation, 0), marginRight, 100);
		ofDrawBitmapString("Total Frames available = " + ofToString(jsonSimulationRadar.size(), 0), marginRight, 120);
		ofDrawBitmapString("Press RightArrow for Next", marginRight, 150);
		ofDrawBitmapString("Press LeftArrow for Previous", marginRight, 170);
	}



}

//-----------------------------------------------
void ofxRadarGlxRss3::radarPolarToCartesian() {


	//Auto Clean cartesian if there is no new targetsData
	cartesianRadar.clear();
	//cartesianRadar.reserve(targetsData.size());

	for (int i = 0; i < targetsData.size(); i++) {

		ofVec2f auxpoint = transformPolarToCartesian(targetsData[i].distance, targetsData[i].angleDegree);
		cartesianRadar.push_back(auxpoint);
	}


}

//--------------------------------------------------------------
ofVec2f ofxRadarGlxRss3::transformPolarToCartesian(float _distance, float _angleDegree) {

	float auxX;
	float auxY;

		float degreeRad = ofDegToRad(_angleDegree);

		auxX = _distance * cos(degreeRad);
		auxY = _distance * sin(degreeRad);

		//Then apply point Radar adaptation ( positive angle )
		auxY = auxY + sensorHeight*0.5;

	return ofVec2f(auxX, auxY);
}

//-------------------------------------------------------
//RECORDER

void ofxRadarGlxRss3::updateRecording() {

	if (bRecording) {

		/*
		//When has no targets:
		{"targets":[],"timestamp":1412,"json_units":"kmh","ui_units":"kmh"}
		//When has targets:
		{"targets":[{"id":137693,"distance":35.7982,"angle":15.8591,"speed":-0.55,"strength":1.06788e+07}],"timestamp":1595,"json_units":"kmh","ui_units":"kmh"}
		*/

		ofxJSONElement saverBlobJS;
		saverBlobJS = jsonRadar;

		jsonRecordingRadar.append(saverBlobJS);
	}

}

//-------------------------------------------------------
void ofxRadarGlxRss3::startRecorging() {

	if (!bStartRecording) {
		bStartRecording = true;

		jsonRecordingRadar.clear();
		framesRecorded = 0;
		bRecording = true;
	}
	else {
		cout << "Error action. Was already Reconding. Stop Recording fisrt, press t" << endl;
	}

}

//-------------------------------------------------------
void ofxRadarGlxRss3::stopRecorging() {

	if (bRecording) {
		jsonRecordingRadar.save("radar_" + ofGetTimestampString() + ".json", false); //true for readable or False for faster
		bRecording = false;
		bStartRecording = false;
	}
	else if (!bStartRecording) {
		bStartRecording = true;
	}
	else {
		cout << "Error action. Was already Reconding. Stop Recording fisrt, press t" << endl;
	}


}

//-------------------------------------------------------
//PLAYER

void ofxRadarGlxRss3::startPlaying() {
	//jsonRecordingRadar.

	if (bPlaying) {
		//Cameback To Radar
		bPlaying = false;
		bStartPlaying = false;
	}
	else if (bStartRecording || bRecording) {
		bStartPlaying = false;
		bPlaying = false;
	}
	else {
		//!bPlaying case
		if (!bStartPlaying) {
			counterFramesSimulation = 0;
			jsonSimulationRadar.clear();
			bool bOpen = jsonSimulationRadar.openLocal(fileName);
			if (bOpen) {
				bStartPlaying = true;
				bPlaying = true;
				//Save Total Frames to do loop over this ones
				totalFramesSimulation = jsonSimulationRadar.size();
			}
			else {
				cout << "startPlaying:: Error opening Simulation radar.json" << endl;
			}

		}

	}
}

//-------------------------------------------------------
ofxJSONElement ofxRadarGlxRss3::readingSimulationRadar() {

	ofxJSONElement auxReadFrame;

	if (counterFramesSimulation > totalFramesSimulation) {
		counterFramesSimulation = 0;
	}
	else {
		counterFramesSimulation++;
	}

	auxReadFrame = jsonSimulationRadar[counterFramesSimulation];

	cout << "My new Simulation Frame is " << auxReadFrame << endl;

	return auxReadFrame;
}

//-------------------------------------------------------
bool ofxRadarGlxRss3::isPlaying() {
	return bPlaying;
}

//-------------------------------------------------------
bool ofxRadarGlxRss3::isRecording() {
	return bRecording;
}

//------------------------------------------------------
void ofxRadarGlxRss3::resetAllTargetVars() {
	targetsData.clear();
	radarPostData.clear();
	cartesianRadar.clear();
}
//------------------------------------------------------
void ofxRadarGlxRss3::playSimFile(int _idPosFileInFolder) {
	myDataFolder.listDir("");
	myDataFolder.sort();

	if (_idPosFileInFolder < myDataFolder.size() && _idPosFileInFolder > -1) {
		idActualSimulatorIter = _idPosFileInFolder;
		fileName = myDataFolder.getPath(idActualSimulatorIter);

		//resetPlayer vars
		bPlaying = false;
		bStartPlaying = false;

		startPlaying();

		resetAllTargetVars();
	}

}

//------------------------------------------------------
void ofxRadarGlxRss3::playNextSimFile() {

	idSimulaTorFile++;
	if (idSimulaTorFile < getNumSimFiles()) {
		playSimFile(idSimulaTorFile);
	}
	else {
		idSimulaTorFile = 0;
		playSimFile(idSimulaTorFile);
	}
}

//------------------------------------------------------
void ofxRadarGlxRss3::playPrevSimFile() {

	idSimulaTorFile--;
	if (idSimulaTorFile > 0) {
		playSimFile(idSimulaTorFile);
	}
	else {
		idSimulaTorFile = getNumSimFiles();
		playSimFile(idSimulaTorFile);
	}
}


//GETTERS
//------------------------------------------------------
int ofxRadarGlxRss3::getNumSimFiles() {
	myDataFolder.listDir("");
	myDataFolder.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order

	//allocate the vector to have as many ofImages as files
	return myDataFolder.size();
}

//------------------------------------------------------
targetData ofxRadarGlxRss3::getTargetData(int _idTarget) {
	return targetsData[_idTarget];
}

//------------------------------------------------------
ofPoint ofxRadarGlxRss3::getCartesianTargetData(int _idTarget) {
	return cartesianRadar[_idTarget];
}

//------------------------------------------------------
float ofxRadarGlxRss3::getRadarWidth() {
	return sensorWidth;
}
//------------------------------------------------------
float ofxRadarGlxRss3::getRadarHeight() {
	return sensorHeight;
}
//------------------------------------------------------
float ofxRadarGlxRss3::getRadarAccurateHeight() {
	return sensorAccurateHeight;
}
//------------------------------------------------------
float ofxRadarGlxRss3::getRadarMaxDistance() {
	return sensorMaxDistance;
}

//SETTERS
//------------------------------------------------------
void ofxRadarGlxRss3::setTrackingArea_leftUpCorner(ofPoint _point){
	if (resumedPostData.trackingRegion.size() == 4) {
		resumedPostData.trackingRegion[0] = ofPoint(_point.x*sensorWidth, _point.y*sensorHeight);
	}
}
//------------------------------------------------------
void ofxRadarGlxRss3::setTrackingArea_rightUpCorner(ofPoint _point) {
	if (resumedPostData.trackingRegion.size() == 4) {
		resumedPostData.trackingRegion[1] = ofPoint(_point.x*sensorWidth, _point.y*sensorHeight);
	}
}
//------------------------------------------------------
void ofxRadarGlxRss3::setTrackingArea_rightBottomCorner(ofPoint _point) {
	if (resumedPostData.trackingRegion.size() == 4) {
		resumedPostData.trackingRegion[2] = ofPoint(_point.x*sensorWidth, _point.y*sensorHeight);
	}
}
//------------------------------------------------------
void ofxRadarGlxRss3::setTrackingArea_leftBottomCorner(ofPoint _point) {
	if (resumedPostData.trackingRegion.size() == 4) {
		resumedPostData.trackingRegion[3] = ofPoint(_point.x*sensorWidth, _point.y*sensorHeight);
	}
}
