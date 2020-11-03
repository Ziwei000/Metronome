#include <chrono>
#include <thread>
#include <string>
using namespace std::chrono_literals;
#include <cpprest/http_msg.h>
#include <wiringPi.h>
#include "metronome.hpp"
#include "rest.hpp"
#define LED_RED   17
#define LED_GREEN 27
#define BTN_MODE  23
#define BTN_TAP   24
volatile bool btn_mode_pressed = false;
volatile bool m_mode = false;//false = play; true = learn
bool putFlag = false ;//indicate if received put request from web
size_t maxb;
size_t minb;
size_t m_bpm;
metronome myMetro;

void learnMode() {
	while (true) {
		if(m_mode){
			digitalWrite(LED_GREEN, LOW);//turn green led off when switch to learn mode
			//std::clog << "learnmode... " ;
			//myMetro.start_timing();
			if(digitalRead(BTN_TAP) == HIGH){
				std::clog << "tap " ;
				myMetro.tap();
				std::this_thread::sleep_for(0.2s);
			}
		}
	}
}
void playMode(){
	while (true) {
		if(!m_mode){
			//std::clog << "playmode.. " ;
			if(!putFlag){
				m_bpm = myMetro.get_bpm();
			}
			if(m_bpm != 0){
				std::clog << "tempoIs:" << m_bpm << " " ;
				if(minb == 0){
					minb = m_bpm;
				}
				maxb = maxb > m_bpm ? maxb : m_bpm;
				minb = minb < m_bpm ? minb : m_bpm;
				std::clog << "(maxBPM:" << maxb << " ;minBPM:" << minb << ") " ;
				digitalWrite(LED_GREEN, LOW);
				std::this_thread::sleep_for(std::chrono::milliseconds(60000/m_bpm));
				//delay(m_bpm);
				digitalWrite(LED_GREEN, HIGH);
				std::this_thread::sleep_for(0.05s);//make the LED light apparent
			}
		}
	}
}
void Mode_LED_Blink(){
	digitalWrite(LED_RED, HIGH);
}
//set60() is just for test
void set60(web::http::http_request msg) {
	web::http::http_response response(200);
	response.headers().add("Access-Control-Allow-Origin", "*");
	response.headers().add("Access-Control-Allow-Methods", "GET, PUT, DELETE");
	response.headers().add("Access-Control-Allow-Headers", "Content-Type");
	response.set_body(web::json::value::string("done set to 60.."));
	msg.reply(response);
	m_bpm = 60;
	m_mode = false;
	putFlag = true;
}
//get current bpm
void bpmGet(web::http::http_request msg) {
	web::http::http_response response(200);
	response.headers().add("Access-Control-Allow-Origin", "*");
	response.headers().add("Access-Control-Allow-Methods", "GET, PUT, DELETE");
	response.headers().add("Access-Control-Allow-Headers", "Content-Type");
	response.set_body(web::json::value::number(m_bpm));
	msg.reply(response);
}
//refresh bpm
void bpmPut(web::http::http_request msg) {
	web::http::http_response response(200);
	response.headers().add("Access-Control-Allow-Origin", "*");
	response.headers().add("Access-Control-Allow-Methods", "GET, PUT, POST, DELETE");
	response.headers().add("Access-Control-Allow-Headers", "Content-Type");
	web::json::value json = msg.extract_json().get();
	response.set_body(json);
	msg.reply(response);
	m_bpm = json.as_integer();
	std::clog << "\n Get a BPM from dashboard : " << m_bpm << "..now play... \n";
	//once get bpm from http, device will go to play mode
	m_mode = false;//play mode
	putFlag = true;
}
//get min bpm
void minGet(web::http::http_request msg) {
	web::http::http_response response(200);
	//response.headers().add("Access-Control-Allow-Origin", "*");
	response.headers().add("Access-Control-Allow-Origin", "*");
	response.headers().add("Access-Control-Allow-Methods", "GET, PUT, DELETE");
	response.headers().add("Access-Control-Allow-Headers", "Content-Type");
	response.set_body(web::json::value::number(minb));
	msg.reply(response);
}
//delete min bpm
void minDelete(web::http::http_request msg) {
	web::http::http_response response(200);
	response.headers().add("Access-Control-Allow-Origin", "*");
	response.headers().add("Access-Control-Allow-Methods", "GET, PUT, DELETE");
	response.headers().add("Access-Control-Allow-Headers", "Content-Type");
	response.set_body(web::json::value::string("min deleted.."));
	msg.reply(response);
	minb = 0;
}
//get max bpm
void maxGet(web::http::http_request msg) {
	web::http::http_response response(200);
	response.headers().add("Access-Control-Allow-Origin", "*");
	response.headers().add("Access-Control-Allow-Methods", "GET, PUT, DELETE");
	response.headers().add("Access-Control-Allow-Headers", "Content-Type");
	response.set_body(web::json::value::number(maxb));
	msg.reply(response);
}
//delete max bpm
void maxDelete(web::http::http_request msg) {
	web::http::http_response response(200);
	response.headers().add("Access-Control-Allow-Origin", "*");
	response.headers().add("Access-Control-Allow-Methods", "GET, PUT, DELETE");
	response.headers().add("Access-Control-Allow-Headers", "Content-Type");
	response.set_body(web::json::value::string("max deleted.."));
	//response.set_body(web::json::value::number(maxb));
	msg.reply(response);
	maxb = 0;
}

int main() {
	wiringPiSetupGpio();
	pinMode(LED_RED, OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(BTN_MODE, INPUT);
	pinMode(BTN_TAP, INPUT);
	//6 endpoints
	auto bpm_rest = rest::make_endpoint("/bpm");
	bpm_rest.support(web::http::methods::GET, bpmGet);
	bpm_rest.support(web::http::methods::PUT, bpmPut);
	bpm_rest.open().wait();

	auto min_rest = rest::make_endpoint("/bpm/min");
	min_rest.support(web::http::methods::GET, minGet);
	min_rest.support(web::http::methods::DEL, minDelete);
	min_rest.open().wait();

	auto max_rest = rest::make_endpoint("/bpm/max");
	max_rest.support(web::http::methods::GET, maxGet);
	max_rest.support(web::http::methods::DEL, maxDelete);
	max_rest.open().wait();
	//for test
	auto set60_rest = rest::make_endpoint("/set60");
	set60_rest.support(web::http::methods::GET, set60);
	set60_rest.open().wait();

	// std::thread blink_thread(blink);
	// blink_thread.detach();
	std::thread learn_thread(learnMode);
	learn_thread.detach();
	std::thread play_thread(playMode);
	play_thread.detach();

	while (true) {
		// btn_mode_pressed = (digitalRead(BTN_MODE) == HIGH);
		// std::this_thread::sleep_for(10ms);
		//wiringPiISR (BTN_MODE, INT_EDGE_RISING, Mode_LED_Blink);
		// std::this_thread::sleep_for(10ms);
		//digitalWrite(LED_RED, digitalRead(BTN_MODE));
		//digitalWrite(LED_RED, LOW);
		if(digitalRead(BTN_MODE) == HIGH){
			digitalWrite(LED_RED, HIGH);
			std::clog << "\n switch_... " ;
			m_mode = !m_mode;
			if(m_mode){
				std::clog << "learnMode.. \n";
				myMetro.start_timing();
				putFlag = false;
			}else{
				std::clog << "playMode.. \n";
				myMetro.stop_timing();
			}
			std::this_thread::sleep_for(0.3s);
		}else{
			digitalWrite(LED_RED, LOW);
		}
	}
	return 0;
}
