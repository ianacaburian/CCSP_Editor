#include "Mapper.h"
Mapper::CreateButton::CreateButton(Mapper& m) : m{ m }, TextButton{ "Create Preset" }
{
    onClick = [this]
    {
        if (!this->m.key_table.isValid())
        {
            DBG("Key has not been generated.");
            return;
        }
        const auto num_duplicates = static_cast<int>(this->m.state[ID::num_duplicates]);
        if (num_duplicates)
        {
            DBG(num_duplicates << " duplicates exist, cannot create preset.");
            return;
        }
        auto& save_tree = ValueTree{ ID::Sample_Table };
        save_tree.appendChild(this->m.grid0.grid_tree.createCopy(), nullptr); 
        save_tree.appendChild(this->m.grid1.grid_tree.createCopy(), nullptr);         
        auto& save_sample_list = cc::create_and_append(ID::Sample_List, save_tree);
        for (auto& s : this->m.sample_list)
            if (s[ID::note_range].isInt()) // if a note range has been set, it's a valid sample.
                save_sample_list.appendChild(s.createCopy(), nullptr);
        auto& save_folder = File{ "C:\\GitRepos\\CC_quencer\\Samples" }; // user chooses along with new_file name
        auto& new_file = File{ save_folder.addTrailingSeparator(save_folder.getFullPathName()) + "test" + ".ccsp" };
        if (new_file.existsAsFile())
        {
            DBG("Overwriting file...");
            new_file.deleteFile();
        }        
        new_file.create();
        std::unique_ptr<XmlElement> xml_state(save_tree.createXml());
        xml_state->writeToFile(new_file, "");        
        DBG("New file created: " << new_file.getFullPathName() << 
            " samples_folder: " << this->m.sample_list[ID::samples_folder].toString());
    };
}
Mapper::AffixDialog::AffixDialog(Mapper& m)
    : DialogWindow("Affix unused params...", Colours::black, TitleBarButtons::closeButton), m(m)
{
    for (int i = 0; i != this->m.fixed_param_vals.getNumProperties(); ++i)
    {
        const auto& prop = this->m.fixed_param_vals.getPropertyName(i);
        const auto& name = prop.toString();
        
        auto* box = new ComboBox{};
        for (auto& p : this->m.param_list)
        {
            if (prop == Identifier{ p[ID::param_name].toString() + "_val_no" })
            {
                for (auto& v : p)
                    box->addItem(v[ID::param_val_name], v[ID::param_val_no]);
                break;
            }
        }
        box->onChange = [this, prop, box]
        {
            this->m.fixed_param_vals.setProperty(prop, box->getSelectedId(), nullptr);
        };
        cc::add_and_make_visible(*this, { param_labels.add(new Label{ name, name }),
                                          param_boxes.add(box) });
    }
}
void Mapper::AffixDialog::resized()
{
    auto& b = getLocalBounds().toFloat();
    const auto h = b.getHeight() / cc::num_boxes;
    for (int i = 0; i != param_labels.size(); ++i)
    {
        auto& p = b.removeFromBottom(h);
        param_labels.getUnchecked(i)->setBounds(p.removeFromLeft(proportionOfWidth(0.5f)).toNearestIntEdges());
        param_boxes.getUnchecked(i)->setBounds(p.toNearestIntEdges());
    }
}
Mapper::GenKeysButton::GenKeysButton(Mapper& m) : m(m), TextButton("Generate Keys")
{
    onClick = [this]
    {
        auto params_unfixed = [this]()->const bool
        {
            for (int i = 0; i != this->m.fixed_param_vals.getNumProperties(); ++i)
            {
                if (!static_cast<int>(this->m.fixed_param_vals[this->m.fixed_param_vals.getPropertyName(i)]))
                {
                    DBG("Please affix all unused params before generating key.");
                    return true;
                }
            }
            return false;
        };
        auto reset_sample_list = [this]()
        {
            const ScopedLock sl(this->m.lock);            
            this->m.sample_list.setProperty(ID::num_duplicates, 0, nullptr);
            for (auto& s : this->m.sample_list)
            {
                s.removeProperty(ID::key_no, nullptr);
                s.removeProperty(ID::duplicate_of, nullptr);
                s.removeProperty(ID::low_note_no, nullptr);
                s.removeProperty(ID::note_range, nullptr);
            }
        };
        auto reset_key_table = [this]()->ValueTree
        {
            const ScopedLock sl(this->m.lock);
            this->m.state.removeChild(this->m.key_table, nullptr);
            this->m.key_table = ValueTree{};
            auto& new_key_table = ValueTree{ ID::Key_Table };
            auto& key_column_list = ValueTree{ ID::Column_List };
            new_key_table.appendChild(key_column_list, nullptr);
            cc::create_column(key_column_list, "key_no");
            cc::create_column(key_column_list, "num_shared");
            cc::create_column(key_column_list, "notes");
            cc::create_column(key_column_list, "duplicates_exist");
            return new_key_table;
        };
        auto fixed_params_match = [this](const ValueTree& fixed_param_vals, const ValueTree& sample)->bool
        {
            for (int i = 0; i != fixed_param_vals.getNumProperties(); ++i)
            {
                auto& fixed_param = fixed_param_vals.getPropertyName(i);
                if (static_cast<int>(sample[fixed_param]) != static_cast<int>(fixed_param_vals[fixed_param]))
                    return false;
            }
            return true;
        };
        auto analyze_notes = [this](ValueTree& sample_list, ValueTree& sample, Array<std::pair<int, ValueTree>>& note_nos, String& notes_str)
        {
            notes_str += sample[ID::root_note_no].toString() + ", ";
            const auto sample_note_no = static_cast<int>(sample[ID::root_note_no]);
            for (int k = 0, last_k = note_nos.size() - 1; k != note_nos.size(); ++k)
            {
                auto& other_note_no = note_nos.getUnchecked(k);
                if (sample == other_note_no.second)
                    continue;
                else if (sample_note_no < other_note_no.first)
                {
                    note_nos.insert(k, { sample_note_no, sample });
                    break;
                }
                else if (sample_note_no == other_note_no.first)
                {
                    sample.setProperty(ID::duplicate_of, other_note_no.second[ID::file_name], nullptr);
                    const auto increment = static_cast<int>(sample_list[ID::num_duplicates]) + 1;
                    sample_list.setProperty(ID::num_duplicates, increment, nullptr);
                    break;
                }
                else if (k == last_k)
                    note_nos.add({ sample_note_no, sample });
            }
            if (note_nos.isEmpty())
                note_nos.add({ sample_note_no, sample });
        };
        auto set_note_ranges = [this](Array<std::pair<int, ValueTree>>& note_nos)
        {
            for (int k = 0, low_note_no = 0, last_k = note_nos.size() - 1; k != note_nos.size(); ++k)
            {   // calculate note ranges
                auto& note_no = note_nos.getUnchecked(k);
                note_no.second.setProperty(ID::low_note_no, low_note_no, nullptr);
                const auto high_note_no = k == last_k ? cc::highest_note_no : note_no.first + 1;
                note_no.second.setProperty(ID::note_range, high_note_no - low_note_no, nullptr);
                low_note_no = high_note_no;
            }
        };
        auto add_key_list_entry = [this](ValueTree& key_list, const int key_no, const int num_shared, const String& notes_str)
        {
            auto& list_key = cc::create_and_append(ID::Key, key_list);
            list_key.setProperty(ID::key_no, key_no, nullptr);
            list_key.setProperty(ID::num_shared, num_shared, nullptr);
            list_key.setProperty(ID::notes, notes_str.trimCharactersAtEnd(", "), nullptr);
            DBG("Key " << key_no << " done...");
        };
        if (!this->m.grid0.grid_tree[ID::param_no].isInt() 
         || !this->m.grid1.grid_tree[ID::param_no].isInt())
        {
            DBG("Please map params to both grids");
            return;
        }
        if (params_unfixed())
            return;
        reset_sample_list();
        auto& new_key_table = reset_key_table();
        auto& key_list = cc::create_and_append(ID::Key_List, new_key_table);
        auto& param_val_no_id0 = Identifier{ this->m.grid0.grid_tree[ID::param_name].toString() + "_val_no" };
        auto& param_val_no_id1 = Identifier{ this->m.grid1.grid_tree[ID::param_name].toString() + "_val_no" };
        for (int p0 = 0; p0 != cc::num_cells; ++p0)
        {
            auto& param_val_tree0 = this->m.grid0.grid_tree.getChild(p0);
            const auto param_val_no0 = static_cast<int>(param_val_tree0[ID::param_val_no]);
            if (!param_val_no0)
                continue;
            for (int p1 = 0; p1 != cc::num_cells; ++p1)
            {
                auto& param_val_tree1 = this->m.grid1.grid_tree.getChild(p1);
                const auto param_val_no1 = static_cast<int>(param_val_tree1[ID::param_val_no]);
                if (!param_val_no1) 
                    continue;
                const auto key_no = (p1 << cc::octal_size) | p0;
                auto notes_str{ String{} };
                auto num_shared = 0;
                Array<std::pair<int, ValueTree>> note_nos;
                for (auto& s : this->m.sample_list)
                {
                    if (!fixed_params_match(this->m.fixed_param_vals, s))
                        continue;
                    if (param_val_no0 == static_cast<int>(s[param_val_no_id0])
                        && param_val_no1 == static_cast<int>(s[param_val_no_id1]))
                    {   // if the sample corresponds to the mapped params
                        s.setProperty(ID::key_no, key_no, nullptr);
                        ++num_shared;
                        analyze_notes(this->m.sample_list, s, note_nos, notes_str);
                    }
                }
                set_note_ranges(note_nos);
                add_key_list_entry(key_list, key_no, num_shared, notes_str);
            }
        }
        this->m.state.appendChild(new_key_table, nullptr);
    };
}
Mapper::CellMapper::CellMapper(OwnedArray<CellMapper>& cells) 
{
    setTextWhenNoChoicesAvailable("No values available...");
    setTextWhenNothingSelected("Unused value");
    onChange = [this, &cells]()
    {
        const auto i = getSelectedItemIndex();
        for (auto* const c : cells)
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
Mapper::GridMapper::GridMapper(Mapper& m, OwnedArray<CellMapper>& cells, GridMapper& other_grid) : m(m)
{
    const auto& no_params = "No params available...";
    setTextWhenNoChoicesAvailable(no_params);
    setTextWhenNothingSelected(no_params);

    onChange = [this, &cells, &other_grid]()
    {
        const auto selection = getSelectedItemIndex();
        auto& param_tree = this->m.param_list.getChild(selection);
        for (auto& pv : grid_tree)
        {
            pv.setProperty(ID::param_val_name, "", nullptr);
            pv.setProperty(ID::param_val_no, 0, nullptr);
        }
        grid_tree.setProperty(ID::param_name, param_tree[ID::param_name], nullptr);
        grid_tree.setProperty(ID::param_no, param_tree[ID::param_no], nullptr);
        for (int i = 0; i != cc::num_cells; ++i)
        {
            auto* const cell = cells.getUnchecked(i);
            cell->clear(sendNotificationAsync);
            for (auto& p : param_tree)
            {
                const int param_val_no = p[ID::param_val_no];
                cell->addItem(p[ID::param_val_name], param_val_no);
                if (i == param_val_no - 1)
                    cell->setSelectedItemIndex(i);
            }
        }
        this->m.fixed_param_vals.removeAllProperties(nullptr);
        const auto other_grid_selection = other_grid.getSelectedItemIndex();
        if (other_grid_selection == selection)
        {
            other_grid.setSelectedItemIndex(-1);
        }
        else if (other_grid_selection != -1)
        {
            for (int i = 0; i != this->m.param_list.getNumChildren(); ++i)
                if (i != selection && i != other_grid_selection)
                    this->m.fixed_param_vals.setProperty(
                        Identifier{ this->m.param_list.getChild(i)[ID::param_name].toString() + "_val_no" }, 0, nullptr);
        }
    };
}

void Mapper::GridMapper::update_grid_boxes()
{
    clear(sendNotificationAsync);
    for (auto& p : m.param_list)
        addItem(p[ID::param_name], p[ID::param_no]);
    setTextWhenNothingSelected("Please select param (" + String{ m.param_list.getNumChildren() } +")");
}
Mapper::Mapper()
    : grid0{ *this, grid0_cells, grid1 }, grid1{ *this, grid1_cells, grid0 }, 
      affix_button("Affix"), gen_key_button(*this), create_button(*this)
{
    cc::add_and_make_visible(*this, { &grid0, &grid1, &affix_button, &gen_key_button, &create_button });
    for (int i = 0; i != cc::num_cells; ++i)
        cc::add_and_make_visible(*this, { grid1_cells.add(new CellMapper{ grid1_cells }),
                                          grid0_cells.add(new CellMapper{ grid0_cells }) });
    affix_button.onClick = [this] 
    { 
        if (fixed_param_vals.getNumProperties())
        {
            DialogWindow::LaunchOptions dialog;
            dialog.content = OptionalScopedPointer<Component>{ new AffixDialog{ *this }, true };
            dialog.componentToCentreAround = this;
            dialog.content->setBounds(getLocalBounds());
            dialog.launchAsync();
        }
    };
}
void Mapper::resized()
{
    auto& b = getLocalBounds().toFloat();
    const auto box_h = b.getHeight() / (cc::num_boxes + 1);
    auto& buttons = b.removeFromBottom(box_h);
    const auto button_w = proportionOfWidth(1.f / 3);
    affix_button.setBounds(buttons.removeFromLeft(button_w).toNearestIntEdges());
    gen_key_button.setBounds(buttons.removeFromLeft(button_w).toNearestIntEdges());
    create_button.setBounds(buttons.toNearestIntEdges());
    auto set_box_bounds = [this, &b, &box_h](ComboBox& c) { c.setBounds(b.removeFromBottom(box_h).toNearestIntEdges()); };
    set_box_bounds(grid0);
    for (auto* const c : grid0_cells)
        set_box_bounds(*c);
    set_box_bounds(grid1);
    for (auto* const c : grid1_cells)
        set_box_bounds(*c);
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
        grid.grid_tree = state.getChild(grid_no);
        set_cell_states(grid.grid_tree, grid_no ? grid1_cells : grid0_cells);
    };
    this->state = state;
    fixed_param_vals = state.getChildWithName(ID::Fixed_Param_Vals);
    this->state.addListener(this);
    set_grid_state(grid0, 0);
    set_grid_state(grid1, 1);
}
void Mapper::valueTreePropertyChanged(ValueTree& t, const Identifier& p)
{
    // react to other grid and other cells
    //DBG("t: " << t.getType().toString() << " p: " << p.toString());
}
void Mapper::valueTreeChildAdded(ValueTree&, ValueTree& c)
{
    if (c.getType() == ID::Sample_Table) update_sample_table(true, c);
    if (c.getType() == ID::Key_Table) update_key_table(true, c);
}
void Mapper::valueTreeChildRemoved(ValueTree&, ValueTree& c, int)
{
    if (c.getType() == ID::Sample_Table) update_sample_table(false, c);
    if (c.getType() == ID::Key_Table) update_key_table(false, c);
}
void Mapper::update_sample_table(const bool added, const ValueTree& sample_table)
{
    this->sample_table = added ? sample_table : ValueTree{};
    sample_list = added ? sample_table.getChildWithName(ID::Sample_List) : ValueTree{};
    param_list = added ? sample_table.getChildWithName(ID::Param_List) : ValueTree{};
    grid0.update_grid_boxes();
    grid1.update_grid_boxes();
}
void Mapper::update_key_table (const bool added, const ValueTree& key_table)
{
    this->key_table = added ? key_table : ValueTree{};
    key_list = added ? key_table.getChildWithName(ID::Key_List) : ValueTree{};
};