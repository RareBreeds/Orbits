#include <assert.h>
#include <string>

#include "plugin.hpp"

const auto max_rhythm_length = 32;
static std::bitset<max_rhythm_length> euclideanRhythm(unsigned int number_of_hits, unsigned int length_of_sequence)
{
	assert(number_of_hits <= length_of_sequence);

	if(0 == number_of_hits)
	{
		return std::bitset<max_rhythm_length>();
	}

	if(number_of_hits == length_of_sequence)
	{
		return ~std::bitset<max_rhythm_length>();
	}

	auto left_blocks = number_of_hits;
	std::bitset<max_rhythm_length> left_pattern;
	left_pattern[0] = true;
	unsigned int left_pattern_length = 1;

	auto right_blocks = length_of_sequence - left_blocks;
	std::bitset<max_rhythm_length> right_pattern;
	right_pattern[0] = false;
	unsigned int right_pattern_length = 1;

	while(right_blocks)
	{
		unsigned int remainder_blocks;
		std::bitset<max_rhythm_length> remainder_pattern;
		unsigned int remainder_pattern_length;

		if(left_blocks > right_blocks)
		{
			remainder_blocks = left_blocks - right_blocks;
			remainder_pattern = left_pattern;
			remainder_pattern_length = left_pattern_length;
			left_blocks = right_blocks;
		}
		else
		{
			remainder_blocks = right_blocks - left_blocks;
			remainder_pattern = right_pattern;
			remainder_pattern_length = right_pattern_length;
		}
		for(unsigned int i = 0; i < right_pattern_length; ++i)
		{
			left_pattern[left_pattern_length] = right_pattern[i];
			++left_pattern_length;
		}
		right_blocks = remainder_blocks;
		right_pattern = remainder_pattern;
		right_pattern_length = remainder_pattern_length;
	}

	for(unsigned int i = 1; i < left_blocks; ++i)
	{
		for(unsigned int j = 0; j < left_pattern_length; ++j)
		{
			left_pattern[left_pattern_length * i + j] = left_pattern[j];
		}
	}

	assert(left_pattern_length * left_blocks == length_of_sequence);
	assert(left_pattern.count() == number_of_hits);
	return left_pattern;
}

struct RareBreeds_Orbits_Eugene : Module {
	enum ParamIds {
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
	enum InputIds {
		CLOCK_INPUT,
		SYNC_INPUT,
		LENGTH_CV_INPUT,
		HITS_CV_INPUT,
		SHIFT_CV_INPUT,
		REVERSE_CV_INPUT,
		INVERT_CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		BEAT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
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

	RareBreeds_Orbits_Eugene() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(LENGTH_KNOB_PARAM, 1.f, max_rhythm_length, max_rhythm_length, "Length");
		configParam(HITS_KNOB_PARAM, 0.f, 1.f, 0.5f, "Hits", "%", 0.f, 100.f);
		configParam(SHIFT_KNOB_PARAM, 0.f, max_rhythm_length - 1, 0.f, "Shift");
		configParam(LENGTH_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Length CV");
		configParam(HITS_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Hits CV");
		configParam(SHIFT_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Shift CV");
		configParam(REVERSE_KNOB_PARAM, 0.f, 1.f, 0.f, "Reverse");
		configParam(INVERT_KNOB_PARAM, 0.f, 1.f, 0.f, "Invert");

		// Ensure the default rhythm exists before drawing
		updateRhythm();
	}

	// TODO: OPT: The number of calls to roundf shows up on the profiler (8.7% + 8.1% + 7.9% + 7.5% + 6.8%)
	unsigned int readLength(void) {
		float value = params[LENGTH_KNOB_PARAM].getValue();
		if(inputs[LENGTH_CV_INPUT].isConnected())
		{
			// bipolar +-5V input
			float input = inputs[LENGTH_CV_INPUT].getVoltage();
			float normalized_input = input / 5.f;
			float attenuation = params[LENGTH_CV_KNOB_PARAM].getValue();
			value += attenuation * normalized_input * (max_rhythm_length-1);
		}

		return clamp(int(std::round(value)), 1, max_rhythm_length);
	}

	unsigned int readHits(unsigned int length) {
		float value = params[HITS_KNOB_PARAM].getValue();
		if(inputs[HITS_CV_INPUT].isConnected())
		{
			// bipolar +-5V input
			float input = inputs[HITS_CV_INPUT].getVoltage();
			float normalized_input = input / 5.f;
			float attenuation = params[HITS_CV_KNOB_PARAM].getValue();
			value += attenuation * normalized_input;
		}

		float hits_float = value * length;
		return clamp(int(std::round(hits_float)), 0, length);
	}

	unsigned int readShift(unsigned int length) {
		float value = params[SHIFT_KNOB_PARAM].getValue();
		if(inputs[SHIFT_CV_INPUT].isConnected())
		{
			// bipolar +-5V input
			float input = inputs[SHIFT_CV_INPUT].getVoltage();
			float normalized_input = input / 5.f;
			float attenuation = params[SHIFT_CV_KNOB_PARAM].getValue();
			value += attenuation * normalized_input * (max_rhythm_length-1);
		}

		// TODO: OPT: This modulus shows up on the profiler (19.05%)
		return clamp(int(std::round(value)), 0, max_rhythm_length - 1) % length;
	}

	bool readReverse(void) {
		if(inputs[REVERSE_CV_INPUT].isConnected())
		{
			reverseTrigger.process(inputs[REVERSE_CV_INPUT].getVoltage());
			return reverseTrigger.isHigh();
		}
		else
		{
			return std::round(params[REVERSE_KNOB_PARAM].getValue());
		}
	}

	bool readInvert(void) {
		if(inputs[INVERT_CV_INPUT].isConnected())
		{
			invertTrigger.process(inputs[INVERT_CV_INPUT].getVoltage());
			return invertTrigger.isHigh();
		}
		else
		{
			return std::round(params[INVERT_KNOB_PARAM].getValue());
		}
	}

	void advanceIndex(void) {
		unsigned int length = readLength();
		if(readReverse())
		{
			if(index == 0)
			{
				index = length - 1;
			}
			else
			{
				--index;
			}
		}
		else
		{
			++index;
			if(index >= length)
			{
				index = 0;
			}
		}
	}

	void updateOutput(const ProcessArgs &args) {
		if(inputs[CLOCK_INPUT].isConnected() && clockTrigger.process(inputs[CLOCK_INPUT].getVoltage()))
		{
			advanceIndex();

			if(inputs[SYNC_INPUT].isConnected() && syncTrigger.process(inputs[SYNC_INPUT].getVoltage()))
			{
				index = 0;
			}

			if(rhythm[index])
			{
				outputGenerator.trigger(1e-3f);
			}
		}

		outputs[BEAT_OUTPUT].setVoltage(outputGenerator.process(args.sampleTime) ? 10.f : 0.f);
	}

	void updateEuclideanRhythm(unsigned int hits, unsigned int length, unsigned int shift, bool invert) {
		rhythm = euclideanRhythm(hits, length);

		auto tmp = rhythm;
		for(unsigned int i = 0; i < length; ++i)
		{
			rhythm[(i + shift) % length] = tmp[i];
		}

		if(invert)
		{
			rhythm.flip();
		}
	}

	void updateRhythm(void) {
		bool update = false;

		unsigned int length = readLength();
		if(length != oldLength)
		{
			oldLength = length;
			update = true;

			// wrap the index to the new length
			// to avoid accessing the rhythm out of bounds
			if(index >= length)
			{
				index = 0;
			}
		}

		unsigned int shift = readShift(length);
		if(shift != oldShift)
		{
			oldShift = shift;
			update = true;
		}

		unsigned int hits = readHits(length);
		if(hits != oldHits)
		{
			oldHits = hits;
			update = true;
		}

		bool reverse = readReverse();
		if(reverse != oldReverse)
		{
			oldReverse = reverse;
			update = true;
		}

		bool invert = readInvert();
		if(invert != oldInvert)
		{
			oldInvert = invert;
			update = true;
		}

		if(update)
		{
			// This function can take a bit of time to run
			// Calling it for every sample is way too expensive
			updateEuclideanRhythm(hits, length, shift, invert);
		}
	}

	void process(const ProcessArgs &args) override {
		updateRhythm();
		updateOutput(args);
	}
};

struct RhythmDisplay : TransparentWidget {
	RareBreeds_Orbits_Eugene *module = NULL;
	std::shared_ptr<Font> font;

	RhythmDisplay() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
	}

	void draw(const DrawArgs &args) override {
		if (!module)
			return;

		const auto length = module->readLength();
		const auto hits = module->readHits(length);
		const auto shift = module->readShift(length);

		nvgStrokeColor(args.vg, color::WHITE);
		nvgSave(args.vg);

		const Rect b = Rect(Vec(0, 0), box.size);
		nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

		// Translate so (0, 0) is the center of the screen
		nvgTranslate(args.vg, b.size.x / 2.f, b.size.y / 2.f);

		// Draw length text center bottom and hits text center top
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgFontSize(args.vg, 20);
		nvgFontFaceId(args.vg, font->handle);
		nvgFillColor(args.vg, color::WHITE);
		nvgText(args.vg, 0.f, -7.f, std::to_string(hits).c_str(), NULL);
		nvgText(args.vg, 0.f, 7.f, std::to_string(length).c_str(), NULL);

		// Scale to [-1, 1]
		nvgScale(args.vg, b.size.x / 2.f, b.size.y / 2.f);

		// Flip x and y so we start at the top and positive angle increments go clockwise
		nvgScale(args.vg, -1.f, -1.f);

		// set the on beat radius so 8 can fix on the screen
		const auto on_radius = 1.f / 8.f;

		// set outline radius so it's bigger than the on_radius
		const auto outline_radius = on_radius * 1.3f;

		// set the off beat radius so it's almost a dot
		const auto off_radius = on_radius / 10.f;

		// Placement of the inner ring of circles when there's too many of them for one ring
		const auto inner_ring_scale = 0.75f;

		// Width of the line when drawing circles
		const auto circle_stroke_width = 0.03f;
		nvgStrokeWidth(args.vg, circle_stroke_width);

		// Add a border of half a circle so we don't draw over the edge
		nvgScale(args.vg, 1.f - outline_radius, 1.f - outline_radius);

		const bool invert = module->readInvert();
		for(unsigned int k = 0; k < length; ++k)
		{
			float y_pos = 1.f;
			if(length > 16 and k % 2)
			{
				y_pos = inner_ring_scale;
			}

			float radius = off_radius;
			if(module->rhythm[k])
			{
				radius = on_radius;
			}

			nvgSave(args.vg);
			nvgRotate(args.vg, 2.f * k * M_PI / length);
			nvgBeginPath(args.vg);
			nvgCircle(args.vg, 0.f, y_pos, radius);
			if(invert)
			{
				nvgStroke(args.vg);
			}
			else
			{
				nvgFill(args.vg);
			}

			if(module->index == k)
			{
				nvgBeginPath(args.vg);
				nvgCircle(args.vg, 0.f, y_pos, outline_radius);
				nvgStroke(args.vg);
			}

			if(shift == k)
			{
				nvgBeginPath(args.vg);
				nvgMoveTo(args.vg, 0.f, 0.5f);
				nvgLineTo(args.vg, -0.05f, 0.4f);
				nvgLineTo(args.vg, 0.05f, 0.4f);
				nvgClosePath(args.vg);
				nvgFill(args.vg);
			}

			nvgRestore(args.vg);
		}

		nvgResetScissor(args.vg);
		nvgRestore(args.vg);
	}
};

struct RareBreeds_Orbits_EugeneWidget : ModuleWidget {
	RareBreeds_Orbits_EugeneWidget(RareBreeds_Orbits_Eugene *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RareBreeds_Orbits_Eugene.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(10.48, 67.0)), module, RareBreeds_Orbits_Eugene::LENGTH_KNOB_PARAM));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(30.48, 67.0)), module, RareBreeds_Orbits_Eugene::HITS_KNOB_PARAM));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(50.48, 67.0)), module, RareBreeds_Orbits_Eugene::SHIFT_KNOB_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.48, 86.0)), module, RareBreeds_Orbits_Eugene::LENGTH_CV_KNOB_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 86.0)), module, RareBreeds_Orbits_Eugene::HITS_CV_KNOB_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.48, 86.0)), module, RareBreeds_Orbits_Eugene::SHIFT_CV_KNOB_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(10.48, 112.0)), module, RareBreeds_Orbits_Eugene::REVERSE_KNOB_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(50.48, 112.0)), module, RareBreeds_Orbits_Eugene::INVERT_KNOB_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.24, 29.5)), module, RareBreeds_Orbits_Eugene::CLOCK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.24, 44.5)), module, RareBreeds_Orbits_Eugene::SYNC_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.48, 100.0)), module, RareBreeds_Orbits_Eugene::LENGTH_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 100.0)), module, RareBreeds_Orbits_Eugene::HITS_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(45.48, 100.0)), module, RareBreeds_Orbits_Eugene::SHIFT_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.813, 112.0)), module, RareBreeds_Orbits_Eugene::REVERSE_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(37.147, 112.0)), module, RareBreeds_Orbits_Eugene::INVERT_CV_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(53.72, 29.5)), module, RareBreeds_Orbits_Eugene::BEAT_OUTPUT));

		RhythmDisplay *r = createWidget<RhythmDisplay>(mm2px(Vec(14.48, 16.5)));
		r->module = module;
		r->box.size = mm2px(Vec(32.0, 32.0));
		addChild(r);
	}
};


Model *modelRareBreeds_Orbits_Eugene = createModel<RareBreeds_Orbits_Eugene, RareBreeds_Orbits_EugeneWidget>("RareBreeds_Orbits_Eugene");
