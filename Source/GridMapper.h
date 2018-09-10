#pragma once
class GridMapper : public Component, public cc::ValueTreePropertyAndChildChangeListener
{
public:
    GridMapper()
    {
        cc::add_and_make_visible(*this, { &grid0, &grid1 });
        for (int i = 0; i != num_cells; ++i)
            addAndMakeVisible(grid0_cells.add(new ComboBox{ String{ i } }));
        for (int i = 0; i != num_cells; ++i)
            addAndMakeVisible(grid1_cells.add(new ComboBox{ String{ i } }));
    }
    void resized() override
    {
        auto& b = getLocalBounds().toFloat();
        const auto box_h = b.getHeight() / num_boxes;
        auto set_bounds = [this, &b, &box_h](ComboBox& c) { c.setBounds(b.removeFromBottom(box_h).toNearestIntEdges()); };
        set_bounds(grid0);
        for (auto* const c : grid0_cells)
            set_bounds(*c);
        set_bounds(grid1);
        for (auto* const c : grid1_cells)
            set_bounds(*c);
    }
    void set_state(const ValueTree& state)
    {
        this->state = state;
        this->state.addListener(this);
    }
private:
    void valueTreePropertyChanged(ValueTree& t, const Identifier& p) override {}

    void valueTreeChildAdded(ValueTree&, ValueTree& c) override
    {
        auto load_grid_boxes = [this](const ValueTree& table_tree)
        {
            auto& param_list = table_tree.getChildWithName(ID::Param_List);
            for (auto& p : param_list)
            {
                grid0.addItem(p[ID::param_name], p[ID::param_no]);
                grid1.addItem(p[ID::param_name], p[ID::param_no]);
            }
        };
        if (c.getType() == ID::Sample_Table) load_grid_boxes(c);
    }
    void valueTreeChildRemoved(ValueTree&, ValueTree& c, int) override
    {
        //if (c.getType() == ID::Sample_Table) ;
    }
    
    ValueTree state;
    ComboBox grid0{ "grid0" }, grid1{ "grid1" };
    OwnedArray<ComboBox> grid0_cells, grid1_cells;
    static const int num_grids = 2, num_cells = 8, num_boxes = num_grids * num_cells + num_grids;
};
