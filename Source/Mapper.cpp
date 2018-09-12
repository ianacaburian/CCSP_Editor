#include "Mapper.h"
GenKeyDialog::GenKeyDialog()
    : DocumentWindow("Affix parameter value...", Colours::black, TitleBarButtons::closeButton)
{

}
GenKeyButton::GenKeyButton(Mapper& m) : m(m), TextButton("Generate Key")
{
    onClick = [this]
    {
        this->m.state.removeChild(this->m.state.getChildWithName(ID::Key_Table), nullptr);
        auto& sample_table = this->m.state.getChildWithName(ID::Sample_Table);
        auto& fixed_param_vals = this->m.state.getOrCreateChildWithName(ID::Fixed_Param_Vals, nullptr);
        for (int i = 0; i != fixed_param_vals.getNumProperties(); ++i)
        {
            if (!static_cast<int>(fixed_param_vals[fixed_param_vals.getPropertyName(i)]))
            {
                DBG("Please affix all unused params before generating key.");
                return;
            }
        }
        auto& sample_column_list = sample_table.getChildWithName(ID::Column_List);
        auto& key_col = ValueTree{ ID::Column };
        sample_column_list.appendChild(key_col, nullptr);
        key_col.setProperty(ID::column_name, "key_no", nullptr);
        key_col.setProperty(ID::column_id, sample_column_list.getNumChildren(), nullptr);

        auto& sample_list = sample_table.getChildWithName(ID::Sample_List);
        for (auto& s : sample_list)
            s.removeProperty(ID::key_no, nullptr);        
        auto& key_table = ValueTree{ ID::Key_Table };        
        auto& column_list = ValueTree{ ID::Column_List };
        key_table.appendChild(column_list, nullptr);
        auto& key_no_col = ValueTree{ ID::Column };
        column_list.appendChild(key_no_col, nullptr);
        key_no_col.setProperty(ID::column_name, "key_no", nullptr);
        key_no_col.setProperty(ID::column_id, column_list.getNumChildren(), nullptr);
        auto& num_files_col = ValueTree{ ID::Column };
        column_list.appendChild(num_files_col, nullptr);
        num_files_col.setProperty(ID::column_name, "num_files", nullptr);
        num_files_col.setProperty(ID::column_id, column_list.getNumChildren(), nullptr);
        auto& notes_col = ValueTree{ ID::Column };
        column_list.appendChild(notes_col, nullptr);
        notes_col.setProperty(ID::column_name, "notes", nullptr);
        notes_col.setProperty(ID::column_id, column_list.getNumChildren(), nullptr);
        auto& duplicates_exist_col = ValueTree{ ID::Column };
        column_list.appendChild(duplicates_exist_col, nullptr);
        duplicates_exist_col.setProperty(ID::column_name, "duplicates_exist", nullptr);
        duplicates_exist_col.setProperty(ID::column_id, column_list.getNumChildren(), nullptr);


        auto& key_list = ValueTree{ ID::Key_List };
        key_table.appendChild(key_list, nullptr);
        auto& key_tree = ValueTree{ ID::Key_Tree };
        key_table.appendChild(key_tree, nullptr);

        auto& grid_tree0 = this->m.state.getChild(0);
        auto& grid_tree1 = this->m.state.getChild(1);
        const auto param_no0 = static_cast<int>(grid_tree0[ID::param_no]);
        const auto param_no1 = static_cast<int>(grid_tree1[ID::param_no]);
        auto& param_val_no_id0 = Identifier{ grid_tree0[ID::param_name].toString() + "_val_no" };
        auto& param_val_no_id1 = Identifier{ grid_tree1[ID::param_name].toString() + "_val_no" };
        for (int i = 0; i != cc::num_cells; ++i)
        {
            auto& param_val_tree0 = grid_tree0.getChild(i);
            const auto param_val_no0 = static_cast<int>(param_val_tree0[ID::param_val_no]);
            if (!param_val_no0)
                continue;
            for (int j = 0; j != cc::num_cells; ++j)
            {
                auto& param_val_tree1 = grid_tree1.getChild(j);
                const auto param_val_no1 = static_cast<int>(param_val_tree1[ID::param_val_no]);
                if (!param_val_no1) 
                    continue;
                auto& list_key = ValueTree{ ID::Key };
                key_list.appendChild(list_key, nullptr);
                auto& tree_key = ValueTree{ ID::Key };
                key_tree.appendChild(tree_key, nullptr);

                const auto key_no = (j << cc::octal_size) | i;
                list_key.setProperty(ID::key_no, key_no, nullptr);
                tree_key.setProperty(ID::key_no, key_no, nullptr);

                auto notes_str{ String{} };
                auto duplicates_exist = 0;
                Array<var> note_nos;
                for (auto& s : sample_list)
                {
                    auto is_unmatched = false;
                    for (int i = 0; i != fixed_param_vals.getNumProperties(); ++i)
                    {
                        auto& fixed_param = fixed_param_vals.getPropertyName(i);
                        if (static_cast<int>(s[fixed_param]) != static_cast<int>(fixed_param_vals[fixed_param]))
                        {
                            is_unmatched = true;
                            break;
                        }
                    }
                    if (is_unmatched)
                        continue;
                    if (param_val_no0 == static_cast<int>(s[param_val_no_id0])
                        && param_val_no1 == static_cast<int>(s[param_val_no_id1]))
                    {
                        s.setProperty(ID::key_no, key_no, nullptr);
                        tree_key.appendChild(s.createCopy(), nullptr);
                        if (note_nos.contains(s[ID::note_no]))
                            duplicates_exist = 1;
                        else
                            note_nos.add(s[ID::note_no]);
                        notes_str += s[ID::note_no].toString() + ", ";
                    }
                }
                list_key.setProperty(ID::num_files, tree_key.getNumChildren(), nullptr);
                list_key.setProperty(ID::notes, notes_str.trimCharactersAtEnd(", "), nullptr);
                list_key.setProperty(ID::duplicates_exist, duplicates_exist, nullptr);

                DBG("Param0_val" << i << " and Param1_val" << j << " done...");
            }
        }
        this->m.state.appendChild(key_table, nullptr);
    };
}
CellMapper::CellMapper(OwnedArray<CellMapper>& cells) : cells(cells)
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
GridMapper::GridMapper(OwnedArray<CellMapper>& cells, GridMapper& other_grid) : cells(cells)
{
    setTextWhenNoChoicesAvailable("No params available...");
    setTextWhenNothingSelected(getTextWhenNoChoicesAvailable());
    onChange = [this, &other_grid]()
    {
        const auto selection = getSelectedItemIndex();
        auto& param_tree = param_list.getChild(selection);
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
        const auto other_grid_selection = other_grid.getSelectedItemIndex();
        if (other_grid_selection == selection)
            other_grid.setSelectedItemIndex(-1);
        else if (other_grid_selection != -1)
        {
            auto& fixed_param_vals = state.getOrCreateChildWithName(ID::Fixed_Param_Vals, nullptr);
            fixed_param_vals.removeAllProperties(nullptr);
            for (int i = 0; i != param_list.getNumChildren(); ++i)
                if (i != selection && i != other_grid_selection)
                    fixed_param_vals.setProperty(
                        Identifier{ param_list.getChild(i)[ID::param_name].toString() + "_val_no" }, 0, nullptr);
        }
    };
}
void GridMapper::valueTreePropertyChanged(ValueTree& t, const Identifier& p) 
{
    // react to other grid and other cells
    //DBG("t: " << t.getType().toString() << " p: " << p.toString());

}
void GridMapper::valueTreeChildAdded(ValueTree&, ValueTree& c) 
{
    auto load_grid_boxes = [this](const ValueTree& table_tree)
    {
        param_list = table_tree.getChildWithName(ID::Param_List);
        clear(sendNotificationAsync);
        for (auto& p : param_list)
            addItem(p[ID::param_name], p[ID::param_no]);
        setTextWhenNothingSelected("Please select param ("
            + String{ param_list.getNumChildren() } +")");
    };
    if (c.getType() == ID::Sample_Table)
        load_grid_boxes(c);
}
void GridMapper::valueTreeChildRemoved(ValueTree&, ValueTree& c, int)
{
    //if (c.getType() == ID::Sample_Table) ;
}
Mapper::Mapper() : grid0{ grid0_cells, grid1 }, grid1{ grid1_cells, grid0 }, gen_key_button(*this)
{
    cc::add_and_make_visible(*this, { &grid0, &grid1, &gen_key_button });
    for (int i = 0; i != cc::num_cells; ++i)
        cc::add_and_make_visible(*this, { grid1_cells.add(new CellMapper{ grid1_cells }),
                                          grid0_cells.add(new CellMapper{ grid0_cells }) });

}
void Mapper::resized()
{
    auto& b = getLocalBounds().toFloat();
    const auto box_h = b.getHeight() / (cc::num_boxes + 1);
    gen_key_button.setBounds(b.removeFromBottom(box_h).toNearestIntEdges());
    auto set_bounds = [this, &b, &box_h](ComboBox& c) { c.setBounds(b.removeFromBottom(box_h).toNearestIntEdges()); };
    set_bounds(grid0);
    for (auto* const c : grid0_cells)
        set_bounds(*c);
    set_bounds(grid1);
    for (auto* const c : grid1_cells)
        set_bounds(*c);
}
void Mapper::set_state(const ValueTree& state)
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
