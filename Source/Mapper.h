#pragma once
class CellMapper : public ComboBox
{
    CellMapper(OwnedArray<CellMapper>& cells) : cells(cells)
    {
        setTextWhenNoChoicesAvailable("No values available...");
        setTextWhenNothingSelected("Unused value");
        onChange = [this]()
        {
            const auto i = getSelectedItemIndex();
            for (auto* const c : this->cells)
            {
                if (c != this && c->getSelectedItemIndex() == i)
                {
                    c->setSelectedItemIndex(-1);
                    break;
                }
            }
            param_val_tree.setProperty(ID::param_val_name, getItemText(i), nullptr);
            param_val_tree.setProperty(ID::param_val_no, getItemId(i), nullptr);
        };
    }
    ValueTree param_val_tree;
    OwnedArray<CellMapper>& cells;
    friend class Mapper;
};
class GridMapper : public ComboBox, public cc::ValueTreePropertyAndChildChangeListener
{
    GridMapper(OwnedArray<CellMapper>& cells, GridMapper& other_grid) : cells(cells)
    {
        setTextWhenNoChoicesAvailable("No params available...");
        setTextWhenNothingSelected(getTextWhenNoChoicesAvailable());
        onChange = [this, &other_grid]()
        {
            const auto i = getSelectedItemIndex();
            auto& param_tree = param_list.getChild(i);
            for (auto& pv : grid_tree)
            {
                pv.setProperty(ID::param_val_name, "", nullptr);
                pv.setProperty(ID::param_val_no, 0, nullptr);
            }
            grid_tree.setProperty(ID::param_name, param_tree[ID::param_name], nullptr);
            grid_tree.setProperty(ID::param_no, param_tree[ID::param_no], nullptr);
            for (int i = 0; i != cc::num_cells; ++i)
            {
                auto* const cell = this->cells.getUnchecked(i);
                cell->clear(sendNotificationAsync);
                for (auto& p : param_tree)
                {
                    const int param_val_no = p[ID::param_val_no];
                    cell->addItem(p[ID::param_val_name], param_val_no);
                    if (i == param_val_no - 1)
                        cell->setSelectedItemIndex(i);
                }
            }
            if (other_grid.getSelectedItemIndex() == i)
                other_grid.setSelectedItemIndex(-1);
        };
    }
    void valueTreePropertyChanged(ValueTree& t, const Identifier& p) override
    {
        // react to other grid and other cells
        //DBG("t: " << t.getType().toString() << " p: " << p.toString());
        
    }
    void valueTreeChildAdded(ValueTree&, ValueTree& c) override
    {
        auto load_grid_boxes = [this](const ValueTree& table_tree)
        {
            param_list = table_tree.getChildWithName(ID::Param_List);
            clear(sendNotificationAsync);
            for (auto& p : param_list)
                addItem(p[ID::param_name], p[ID::param_no]);
            setTextWhenNothingSelected("Please select param (" 
                                     + String{ param_list.getNumChildren() } + ")");
        };
        if (c.getType() == ID::Sample_Table) 
            load_grid_boxes(c);
    }
    void valueTreeChildRemoved(ValueTree&, ValueTree& c, int) override
    {
        //if (c.getType() == ID::Sample_Table) ;
    }
    OwnedArray<CellMapper>& cells;
    ValueTree state, param_list, grid_tree;
    SortedSet<int> used_vals;
    friend class Mapper;
};
class Mapper : public Component
{
public:
    Mapper() : grid0{ grid0_cells, grid1 }, grid1{ grid1_cells, grid0 }
    {
        cc::add_and_make_visible(*this, { &grid0, &grid1 });
        for (int i = 0; i != cc::num_cells; ++i)
            cc::add_and_make_visible(*this, { grid1_cells.add(new CellMapper{ grid1_cells }),
                                              grid0_cells.add(new CellMapper{ grid0_cells }) });
    }
    void resized() override
    {
        auto& b = getLocalBounds().toFloat();
        const auto box_h = b.getHeight() / cc::num_boxes;
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
        auto set_grid_state = [this, &state](GridMapper& grid, const int grid_no)
        {
            auto set_cell_states = [this](const ValueTree& grid_tree, OwnedArray<CellMapper>& cells)
            {
                for (int i = 0; i != cc::num_cells; ++i)
                    cells.getUnchecked(i)->param_val_tree = grid_tree.getChild(i);
            };
            grid.state = state;
            grid.state.addListener(&grid);
            grid.grid_tree = state.getChild(grid_no);
            set_cell_states(grid.grid_tree, grid_no ? grid1_cells : grid0_cells);
        };
        this->state = state;
        set_grid_state(grid0, 0);
        set_grid_state(grid1, 1);
    }
private:
    ValueTree state;
    GridMapper grid0, grid1;
    OwnedArray<CellMapper> grid0_cells, grid1_cells;
};