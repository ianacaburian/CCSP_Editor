#include "MainComponent.h"
MainComponent::MainComponent() : state_str{ "state" }, xml { "xml" }
{
    auto set_state = [this]()
    {
        auto create_default_state = [this]()
        {
            state = ValueTree{ ID::STATE };
            for (int i = 0; i != cc::num_grids; ++i)
            {
                auto& grid_tree = ValueTree{ ID::Grid };                
                state.appendChild(grid_tree, nullptr);                
                for (int j = 0; j != cc::num_cells; ++j)
                {
                    auto& param_val_tree = ValueTree{ ID::Param_Val };
                    grid_tree.appendChild(param_val_tree, nullptr);
                }
            }
            state.appendChild(ValueTree{ ID::Fixed_Param_Vals }, nullptr);
        };
        if (!state.isValid())
            create_default_state();
        sample_browser->set_state(state);
        sample_table->set_state(state);
        key_table->set_state(state);
        cross_table->set_state(state);
        mapper->set_state(state);
    };
    state_str.onClick = [this]() { DBG(state.toXmlString()); };
    xml.onClick = [this]() { DBG(state.getChildWithName(ID::Fixed_Param_Vals).toXmlString()); };
    sample_browser = std::make_unique<SampleBrowser>(File{ "C:\\GitRepos\\CC_quencer\\Samples" });
    sample_table = std::make_unique<SampleTable>();
    key_table = std::make_unique<KeyTable>();
    cross_table = std::make_unique<CrossTable>();
    mapper = std::make_unique<Mapper>();
    cc::add_and_make_visible(*this, { sample_browser.get(), sample_table.get(), mapper.get(), 
                                      key_table.get(), cross_table.get(), &state_str, &xml });
    set_state();
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
    const auto h = b.getHeight();

    auto& left = b.removeFromLeft(w / 4);
    state_str.setBounds(left.removeFromBottom(20).toNearestIntEdges());
    xml.setBounds(left.removeFromBottom(20).toNearestIntEdges());
    mapper->setBounds(left.removeFromBottom(h / 2).toNearestIntEdges());
    sample_browser->setBounds(left.toNearestIntEdges());
    sample_table->setBounds(b.removeFromTop(h / 2).toNearestIntEdges());
    cross_table->setBounds(b.removeFromRight(b.getHeight()).toNearestIntEdges());
    key_table->setBounds(b.toNearestIntEdges());
}
