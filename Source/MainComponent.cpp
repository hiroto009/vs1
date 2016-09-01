#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"


class MainContentComponent : public AudioAppComponent,
	public Slider::Listener, public Button::Listener
{
public:
	MainContentComponent()
	{
		Logger::writeToLog("constractor");
		targetLevel = 0.125;

		addNoiseLevel();
		addRateInfo();
		addController();
		setSize(800, 200);

		// start
//		setAudioChannels(0, 2);
	}

	~MainContentComponent()
	{
		//shutdownAudio();
		startButton.removeListener(this);
	}

	void addNoiseLevel() {
		levelSlider.setRange(0.0, 0.5);
		levelSlider.setValue(targetLevel, dontSendNotification);
		levelSlider.setTextBoxStyle(Slider::TextBoxRight, false, 100, 20);
		levelSlider.addListener(this);

		levelLabel.setText("Noise Level", dontSendNotification);

		addAndMakeVisible(&levelSlider);
		addAndMakeVisible(&levelLabel);
	}

	void addRateInfo() {
		rateLabel.setText("Sample Rate", dontSendNotification);
		rateValue.setText("--", dontSendNotification);
		addAndMakeVisible(&rateLabel);
		addAndMakeVisible(&rateValue);
	}

	void buttonClicked(Button* button) override {
		Logger::writeToLog("buttonClicked");

		if (button == &startButton) {
			clickStartButton();
		}
	}

	void clickStartButton() {
		String val = "start";
		String cur = startButton.getButtonText();
		if (val.compare(cur) == 0) {
			startButton.setColour(1, Colour());
			setAudioChannels(0, 2);
			val = "stop";
		}
		else {
			shutdownAudio();
			val = "start";
		}
		Logger::writeToLog(val);
		if (sampleRate > 0) {
			String val = String(sampleRate / 10.0);
			val.append("kHz", 3);
			rateValue.setText(val, dontSendNotification);
		} else {
			rateValue.setText("--", dontSendNotification);
		}
		startButton.setButtonText(val);
	}

	void addController() {
		startButton.setButtonText("start");
		startButton.setToggleState(false, sendNotification);
		startButton.addListener(this);
		addAndMakeVisible(&startButton);
	}

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
	{
		resetParameters();
	}

	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override
	{
		int numSamplesRemaining = bufferToFill.numSamples;
		
		sampleRate = numSamplesRemaining;

		int offset = 0;

		if (samplesToTarget > 0)
		{
			const float levelIncrement = (targetLevel - currentLevel) / samplesToTarget;
			const int numSamplesThisTime = jmin(numSamplesRemaining, samplesToTarget);

			for (int sample = 0; sample < numSamplesThisTime; ++sample)
			{
				for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
					bufferToFill.buffer->setSample(channel, sample, random.nextFloat() * currentLevel);

				currentLevel += levelIncrement;
				--samplesToTarget;
			}

			offset = numSamplesThisTime;
			numSamplesRemaining -= numSamplesThisTime;

			if (samplesToTarget == 0)
				currentLevel = targetLevel;
		}

		if (numSamplesRemaining > 0)
		{
			for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
			{
				float* buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample + offset);

				for (int sample = 0; sample < numSamplesRemaining; ++sample)
					*buffer++ = random.nextFloat() * currentLevel;
			}
		}
	}

	void releaseResources() override
	{
	}

	void resized() override
	{
		levelLabel.setBounds(10, 10, 90, 20);
		levelSlider.setBounds(100, 10, getWidth() - 110, 20);

		rateLabel.setBounds(10, 40, 90, 20);
		rateValue.setBounds(100, 40, 90, 20);
		
		startButton.setBounds(10, 60, 90, 20);
	}

	void sliderValueChanged(Slider* slider) override
	{
		if (&levelSlider == slider)
		{
			targetLevel = levelSlider.getValue();
			samplesToTarget = rampLengthSamples;
		}
	}

	void resetParameters()
	{
		currentLevel = targetLevel;
		samplesToTarget = 0;
	}


private:
	Random random;
	Slider levelSlider;
	Label levelLabel;

	Label rateLabel;
	Label rateValue;

	TextButton startButton;
	int sampleRate;

	float currentLevel;
	float targetLevel;
	int samplesToTarget;
	static const int rampLengthSamples;


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};

const int MainContentComponent::rampLengthSamples = 128;

Component* createMainContentComponent() { return new MainContentComponent(); }

#endif  // MAINCOMPONENT_H_INCLUDED
