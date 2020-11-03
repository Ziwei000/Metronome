#pragma once

#include <cstddef>
#include <wiringPi.h>

class metronome
{
public:
	enum { beat_samples = 3 };

public:
	metronome()
	: m_timing(false), m_beat_count(0) {}
	~metronome() {}

public:
	// Call when entering "learn" mode
	void start_timing() {
		m_timing = true;
		m_beat_count = 0;
		std::clog << "At least 4 taps are required.. \n ";
	}
	// Call when leaving "learn" mode
	void stop_timing() {
		m_timing = false;
	}


	bool is_timing() const { return m_timing; }

	// Should only record the current time when timing
	// Insert the time at the next free position of m_beats
	void tap(){
		size_t duration ;
		if(m_beat_count == 0) {
			prev_time = millis();
			m_beat_count++;
			//std::clog << "first_time.. ";
		}else {
			cur_time = millis();
			duration = cur_time - prev_time;
			if(m_beat_count <= 3){//directly insert samples
				m_beats[m_beat_count - 1] = duration;
			}else{//remove oldest samples and then insert
				m_beats[0] = m_beats[1];
				m_beats[1] = m_beats[2];
				m_beats[2] = duration;
			}
			prev_time = cur_time;
			m_beat_count++;
		}

	}

	// Calculate the BPM from the deltas between m_beats
	// Return 0 if there are not enough samples
	// size_t get_max() const{
	// 	size_t max;
	// 	max = m_beats[0] > m_beats[1] ? m_beats[0] : m_beats[1];
  //   max = max > m_beats[2] ? max : m_beats[2];
	// 	return max;
	// }
	// size_t get_min() const{
	// 	size_t min;
	// 	min = m_beats[0] < m_beats[1] ? m_beats[0] : m_beats[1];
  //   min = min < m_beats[2] ? min : m_beats[2];
	// 	return min;
	// }
	size_t get_bpm() const {
		if(m_beat_count <= 3){
			return 0;
		}else{
			return 60000/((m_beats[0] + m_beats[1] + m_beats[2])/3);
		}
	}

private:
	bool m_timing;
	size_t prev_time;
	size_t cur_time;
	// Insert new samples at the end of the array, removing the oldest
	size_t m_beats[beat_samples];
	size_t m_beat_count;
};
