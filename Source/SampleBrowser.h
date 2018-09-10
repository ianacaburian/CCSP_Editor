#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "Identifiers.h"
#include "Utilities.h"
class SampleBrowser : public FileBrowserComponent
{
public:
    SampleBrowser(const File& init_dir);
    void fileClicked(const File& f, const MouseEvent& e) override;
    void set_state(const ValueTree& state);
    //void analyze_directory(const File& f);
private:

    ValueTree state;
    CriticalSection lock;
};