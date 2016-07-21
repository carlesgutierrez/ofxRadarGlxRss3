#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	myRadar.setup();
	
}

//--------------------------------------------------------------
void ofApp::update(){
	myRadar.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	myRadar.draw();
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
		playNextSimFile();
	}
	else if (key == OF_KEY_LEFT) {
		playPrevSimFile();
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
