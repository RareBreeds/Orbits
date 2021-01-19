#pragma once

struct EugeneTheme
{
	enum EugeneThemeComponents
	{
		panel,

		screw_top_left,
		screw_top_right,
		screw_bottom_left,
		screw_bottom_right,

		clock_port,
		sync_port,
		beat_port,

		length_knob,
		length_cv_knob,
		length_cv_port,

		hits_knob,
		hits_cv_knob,
		hits_cv_port,

		shift_knob,
		shift_cv_knob,
		shift_cv_port,

		reverse_switch_on,
		reverse_switch_off,
		reverse_cv_port,

		invert_switch_on,
		invert_switch_off,
		invert_cv_port,

		num_components
	};

	std::string m_name;
	std::string m_components[num_components];

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