#pragma once

struct EugeneTheme
{
	std::string name;

	std::string panel;

	std::string screw_top_left;
	std::string screw_top_right;
	std::string screw_bottom_left;
	std::string screw_bottom_right;

	std::string clock_port;
	std::string sync_port;
	std::string beat_port;

	std::string length_knob;
	std::string length_cv_knob;
	std::string length_cv_port;

	std::string hits_knob;
	std::string hits_cv_knob;
	std::string hits_cv_port;

	std::string shift_knob;
	std::string shift_cv_knob;
	std::string shift_cv_port;

	std::string reverse_switch_on;
	std::string reverse_switch_off;
	std::string reverse_cv_port;

	std::string invert_switch_on;
	std::string invert_switch_off;
	std::string invert_cv_port;

	bool fromJson(json_t *root);
};

struct EugeneConfig
{
	std::vector<EugeneTheme> m_themes;
	size_t m_default;

	Vec length_knob;
	Vec hits_knob;
	Vec reverse_knob;
	Vec invert_knob;
	Vec length_cv;
	Vec hits_cv;
	Vec reverse_cv;
	Vec invert_cv;
	Vec beat;
	Vec clock;
	Vec sync;
	Vec shift_knob;
	Vec shift_cv;
	Vec length_cv_knob;
	Vec hits_cv_knob;
	Vec shift_cv_knob;
	Vec display;

	bool init();
	bool fromJson(std::string path);
	bool loadComponentPositions();
};

extern EugeneConfig eugene_config;