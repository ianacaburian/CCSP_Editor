#include "MainComponent.h"
MainComponent::MainComponent() 
{
    auto init_state = [this]()
    {
        state = ValueTree{ ID::STATE };
        sample_browser->set_state(state);
        sample_table->set_state(state);
        grid_mapper->set_state(state);
    };

    sample_browser = std::make_unique<SampleBrowser>(File{ "C:\\GitRepos\\CC_quencer\\Samples" });
    sample_table = std::make_unique<SampleTable>();
    grid_mapper = std::make_unique<GridMapper>();
    cc::add_and_make_visible(*this, { sample_browser.get(), sample_table.get(), grid_mapper.get() });
    init_state();    
    setSize(1280, 720); setPaintingIsUnclipped(true); setOpaque(true);
}
MainComponent::~MainComponent() {}
void MainComponent::paint (Graphics& g) 
{
    g.setColour(Colours::black);
    g.fillRect(getLocalBounds());
}
void MainComponent::resized()
{
    auto& b = getLocalBounds().toFloat();
    const auto w = b.getWidth();
    const auto quarter_w = w / 4;

    sample_browser->setBounds(b.removeFromLeft(quarter_w).toNearestIntEdges());
    sample_table->setBounds(b.removeFromLeft(quarter_w * 2).toNearestIntEdges());
    grid_mapper->setBounds(b.toNearestIntEdges());
}
