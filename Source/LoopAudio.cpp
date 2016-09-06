#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

class LoopAudio   : public AudioAppComponent,
                               private ButtonListener,
                               private Slider::Listener
{
public:
    LoopAudio()
       : currentLevel (0.0f),
         previousLevel (0.0f)
    {
        addAndMakeVisible (openButton);
        openButton.setButtonText ("Open...");
        openButton.addListener (this);

        addAndMakeVisible (clearButton);
        clearButton.setButtonText ("Clear");
        clearButton.addListener (this);

        addAndMakeVisible (levelSlider);
        levelSlider.setRange (0.0, 1.0);
        levelSlider.addListener (this);

        setSize (300, 200);

        formatManager.registerBasicFormats();
    }

    ~LoopAudio()
    {
        shutdownAudio();
    }

    void prepareToPlay (int /*samplesPerBlockExpected*/, double /*sampleRate*/) override
    {
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        const float level = currentLevel;
        const float startLevel = level == previousLevel ? level : previousLevel;

        const int numInputChannels = fileBuffer.getNumChannels();
        const int numOutputChannels = bufferToFill.buffer->getNumChannels();

        int outputSamplesRemaining = bufferToFill.numSamples;
        int outputSamplesOffset = bufferToFill.startSample;

        while (outputSamplesRemaining > 0)
        {
            int bufferSamplesRemaining = fileBuffer.getNumSamples() - position;
            int samplesThisTime = jmin (outputSamplesRemaining, bufferSamplesRemaining);

            for (int channel = 0; channel < numOutputChannels; ++channel)
            {
                bufferToFill.buffer->copyFrom (channel,
                                               outputSamplesOffset,
                                               fileBuffer,
                                               channel % numInputChannels,
                                               position,
                                               samplesThisTime);

                bufferToFill.buffer->applyGainRamp (channel, outputSamplesOffset, samplesThisTime, startLevel, level);
            }

            outputSamplesRemaining -= samplesThisTime;
            outputSamplesOffset += samplesThisTime;
            position += samplesThisTime;

            if (position == fileBuffer.getNumSamples())
                position = 0;
        }

        previousLevel = level;
    }

    void releaseResources() override
    {
        fileBuffer.setSize (0, 0);
    }

    void resized() override
    {
        openButton.setBounds (10, 10, getWidth() - 20, 20);
        clearButton.setBounds (10, 40, getWidth() - 20, 20);
        levelSlider.setBounds (10, 70, getWidth() - 20, 20);
    }

    void buttonClicked (Button* button) override
    {
        if (button == &openButton)
            openButtonClicked();
        else if (button == &clearButton)
            clearButtonClicked();
    }

    void sliderValueChanged (Slider* slider) override
    {
        if (slider == &levelSlider)
            currentLevel = (float) levelSlider.getValue();
    }


private:
    void openButtonClicked()
    {
        shutdownAudio();                                                                    // [1]

		FileChooser chooser("Select a Wave file shorter than 2 seconds to play...",
			File::nonexistent,
			"*.wav;*.flac");

        if (chooser.browseForFileToOpen())
        {
            const File file (chooser.getResult());
            ScopedPointer<AudioFormatReader> reader (formatManager.createReaderFor (file)); // [2]

            if (reader != nullptr)
            {
                const double duration = reader->lengthInSamples / reader->sampleRate;       // [3]

                if (duration < 2)
                {
                    fileBuffer.setSize (reader->numChannels, reader->lengthInSamples);      // [4]
                    reader->read (&fileBuffer,                                              // [5]
                                  0,                                                        //  [5.1]
                                  reader->lengthInSamples,                                  //  [5.2]
                                  0,                                                        //  [5.3]
                                  true,                                                     //  [5.4]
                                  true);                                                    //  [5.5]
                    position = 0;                                                           // [6]
                    setAudioChannels (0, reader->numChannels);                              // [7]
                }
                else
                {
                    // handle the error that the file is 2 seconds or longer..
					Logger::writeToLog("Error! file is 2 seconds or longer..");
                }
            }
        }
    }

    void clearButtonClicked()
    {
        shutdownAudio();
    }

    //==========================================================================
    TextButton openButton;
    TextButton clearButton;
    Slider levelSlider;

    AudioFormatManager formatManager;
    AudioSampleBuffer fileBuffer;
    int position;

    float currentLevel, previousLevel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoopAudio)
};


Component* createLoopAudio()     { return new LoopAudio(); }


#endif  // MAINCOMPONENT_H_INCLUDED
