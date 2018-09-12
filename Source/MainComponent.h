/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Identifiers.h"
#include "Utilities.h"

#include "SampleBrowser.h"
#include "SampleTable.h"
#include "KeyTable.h"
#include "CrossTable.h"
#include "Mapper.h"
//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    ValueTree state;
    std::unique_ptr<SampleBrowser> sample_browser;
    std::unique_ptr<SampleTable> sample_table;
    std::unique_ptr<Mapper> mapper;
    std::unique_ptr<KeyTable> key_table;
    std::unique_ptr<CrossTable> cross_table;
    TextButton state_str, xml;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
