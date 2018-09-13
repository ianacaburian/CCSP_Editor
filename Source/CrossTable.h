#pragma once
class CrossTable : public Component, cc::ValueTreePropertyAndChildChangeListener
{
public:
    CrossTable()
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
        if (key_table.isValid())
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
    void valueTreePropertyChanged(ValueTree& t, const Identifier& p) override {}
    void valueTreeChildAdded(ValueTree&, ValueTree& c) override { if (c.getType() == ID::Key_Table) update_key_table(true, c); }
    void valueTreeChildRemoved(ValueTree&, ValueTree& c, int) override { if (c.getType() == ID::Key_Table) update_key_table(false, c); }
    void update_key_table(const bool added, ValueTree& key_table)
    {
        this->key_table = added ? key_table : ValueTree{};
        repaint();
    }
    ValueTree state, key_table, grid0_tree, grid1_tree;
    OwnedArray<OwnedArray<Rectangle<float>>> grid;    
};