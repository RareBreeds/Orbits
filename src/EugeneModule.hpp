#pragma once

#include <bitset>
#include "plugin.hpp"

struct RareBreeds_Orbits_EugeneWidget;

static const auto max_rhythm_length = 32;

struct RareBreeds_Orbits_Eugene : Module
{
	enum ParamIds
    {
		LENGTH_KNOB_PARAM,
		HITS_KNOB_PARAM,
		SHIFT_KNOB_PARAM,
		LENGTH_CV_KNOB_PARAM,
		HITS_CV_KNOB_PARAM,
		SHIFT_CV_KNOB_PARAM,
		REVERSE_KNOB_PARAM,
		INVERT_KNOB_PARAM,
		NUM_PARAMS
	};
	enum InputIds
    {
		CLOCK_INPUT,
		SYNC_INPUT,
		LENGTH_CV_INPUT,
		HITS_CV_INPUT,
		SHIFT_CV_INPUT,
		REVERSE_CV_INPUT,
		INVERT_CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
    {
		BEAT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
    {
		NUM_LIGHTS
	};

	dsp::SchmittTrigger clockTrigger;
	dsp::SchmittTrigger syncTrigger;
	dsp::SchmittTrigger reverseTrigger;
	dsp::SchmittTrigger invertTrigger;
	dsp::PulseGenerator outputGenerator;
	unsigned int index = 0;
	std::bitset<max_rhythm_length> rhythm;
	unsigned int oldLength = max_rhythm_length + 1;
	unsigned int oldHits = max_rhythm_length + 1;
	unsigned int oldShift = max_rhythm_length + 1;
	bool oldReverse = false;
	bool oldInvert = false;
	RareBreeds_Orbits_EugeneWidget *widget = NULL;

	RareBreeds_Orbits_Eugene();

	unsigned int readLength();
	unsigned int readHits(unsigned int length);
	unsigned int readShift(unsigned int length);
	bool readReverse();
	bool readInvert();
	void advanceIndex();
	void updateOutput(const ProcessArgs &args);
	void updateEuclideanRhythm(unsigned int hits, unsigned int length, unsigned int shift, bool invert);
	void updateRhythm();
	void process(const ProcessArgs &args) override;
	json_t *dataToJson() override;
	void dataFromJson(json_t *root) override;
};