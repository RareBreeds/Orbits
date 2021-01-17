#pragma once

struct EugeneTheme
{
	std::string name;
	std::string panel;
	std::string knob_large;
	std::string knob_small;
	std::string switch_on;
	std::string switch_off;
	std::string screw;
	std::string port;

	void fromJson(json_t *root);
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

	void init();
	void fromJson(std::string path);
	void loadComponentPositions();
};

extern EugeneConfig eugene_config;