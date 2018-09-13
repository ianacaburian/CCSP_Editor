#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "Identifiers.h"
#include "Utilities.h"
class SampleBrowser : public FileBrowserComponent
{
public:
    SampleBrowser(const File& init_dir);
    void set_state(const ValueTree& state);
private:
    void fileClicked(const File& f, const MouseEvent& e) override;
    ValueTree state;
    CriticalSection lock;
};