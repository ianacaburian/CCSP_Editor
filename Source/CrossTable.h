#pragma once
class CrossTable : public Component, cc::ValueTreePropertyChangeListener
{
public:
    CrossTable()
        : grids_mapped(0)
    {
    }
    void resized() override
    {
        auto& b = getLocalBounds().toFloat();
        auto& gsq = b.getHeight() < b.getWidth() ? b.removeFromLeft(b.getHeight())
                                                 : b.removeFromBottom(b.getWidth());
        
        const auto c_length = gsq.getWidth() / 9;
        for (int x = 0; x != cc::num_cells + 1; ++x)
        {
            auto& col_rect = gsq.removeFromLeft(c_length);
            auto* col = new OwnedArray<Rectangle<float>>{};
            grid.add(col);
            for (int y = 0; y != cc::num_cells + 1; ++y)
                col->add(new Rectangle<float>{ col_rect.removeFromBottom(c_length) });
        }
    }
    void paint(Graphics& g) override
    {
        if (grids_mapped == 2)
        {
            g.setColour(Colours::white);
            for (int x = 0; x != cc::num_cells + 1; ++x)
            {
                auto* col = grid.getUnchecked(x);            
                for (int y = 0; y != cc::num_cells + 1; ++y)
                {
                    auto* cell = col->getUnchecked(y);
                    g.drawRect(*cell);
                    if (x == 0 && y != 0)
                        g.drawText(String{ y }, *cell, Justification::centred);
                    else if (y == 0 && x != 0)
                        g.drawText(String{ x }, *cell, Justification::centred);
                }
            }
        }
    }
    void set_state(const ValueTree& state)
    {
        this->state = state;
        this->state.addListener(this);
        grid0_tree = state.getChild(0);
        grid1_tree = state.getChild(1);
    }
private:
    void valueTreePropertyChanged(ValueTree& t, const Identifier& p) override
    {
        auto update_grids_mapped = [this](const int mapped)
        {
            grids_mapped += mapped ? 1 : -1;
            grids_mapped = jlimit(0, 2, grids_mapped);
            repaint();
        };
        if (t.getType() == ID::Grid && p == ID::param_no) update_grids_mapped(t[p]);
    }
    ValueTree state, grid0_tree, grid1_tree;
    OwnedArray<OwnedArray<Rectangle<float>>> grid;
    int grids_mapped; //---- remove this and have cross table showing depend on add/remove key_list child
};