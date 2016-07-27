#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	myRadar.setup("192.168.10.101");
	myGui.setup();
	
}

//--------------------------------------------------------------
void ofApp::update(){
	myRadar.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	myRadar.draw();
	
	ofFill();

	myGui.begin();

	static float upLeft_Track[2] = { 0.f, 0.f};
	static float upRight_Track[2] = { 1.0f, 0.f };
	static float downLeft_Track[2] = { 0.f, 1.0f };
	static float downRight_Track[2] = { 1.0f, 1.0f };
	static int sensorMaxDistance = 22;
	

	ImGui::SliderFloat("Scale Visualization", &myRadar.sensorScale, 1, 100);

	if (ImGui::SliderInt("Max Distance Radar", &sensorMaxDistance, 4, 150)) {
		myRadar.SetSensorMaxDistance(sensorMaxDistance);
	}

	if (ImGui::Button("Radar/Simulation")) {
		myRadar.startPlaying();
	}

	ImGui::SameLine();

	if (ImGui::Button("next")) {
		myRadar.playNextSimFile();
	}

	ImGui::SameLine();

	if (ImGui::Button("prev")) {
		myRadar.playPrevSimFile();
	}
	
	ImGui::Separator();

	ImGui::PushItemWidth(100);

	if (ImGui::SliderFloat2("upLeft Track", upLeft_Track, 0, 1)) {
		myRadar.setTrackingArea_leftUpCorner(ofPoint(upLeft_Track[0], upLeft_Track[1]));
	}
	if (ImGui::SliderFloat2("upRight Track", upRight_Track, 0, 1)){
		myRadar.setTrackingArea_rightUpCorner(ofPoint(upRight_Track[0], upRight_Track[1]));
	}
	if (ImGui::SliderFloat2("downLeft Track", downLeft_Track, 0, 1)) {
		myRadar.setTrackingArea_leftBottomCorner(ofPoint(downLeft_Track[0], downLeft_Track[1]));
	}
	if (ImGui::SliderFloat2("downRight Track", downRight_Track, 0, 1)) {
		myRadar.setTrackingArea_rightBottomCorner(ofPoint(downRight_Track[0], downRight_Track[1]));
	}

	ImGui::PopItemWidth();

	myGui.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	
	if (key == '+') {
		myRadar.sensorScale += 0.1;
	}
	else if (key == '-') {
		myRadar.sensorScale -= 0.1;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	if (key == 'r')myRadar.startRecorging();
	else if(key == 't')myRadar.stopRecorging();
	else if (key == 'p') {
		myRadar.startPlaying();
	}
	else if (key == OF_KEY_RIGHT) {
		myRadar.playNextSimFile();
	}
	else if (key == OF_KEY_LEFT) {
		myRadar.playPrevSimFile();
	}
	
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
