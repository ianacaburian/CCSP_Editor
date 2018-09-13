#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "Utilities.h"
#include "Identifiers.h"
class Mapper : public Component, public cc::ValueTreePropertyAndChildChangeListener
{
public:
    Mapper();
    void resized() override;
    void set_state(const ValueTree& state);
private:
    struct CreateButton : public TextButton
    {
        CreateButton(Mapper& m);
        Mapper& m;
    };
    struct AffixDialog : public DialogWindow
    {
        AffixDialog(Mapper& m);
        void resized() override;
        Mapper& m;
        OwnedArray<Label> param_labels;
        OwnedArray<ComboBox> param_boxes;
    };
    struct GenKeysButton : public TextButton
    {
        GenKeysButton(Mapper& m);
        Mapper& m;
    };
    struct CellMapper : public ComboBox
    {
        CellMapper(OwnedArray<CellMapper>& cells);
        ValueTree param_val_tree;
    };
    struct GridMapper : public ComboBox
    {
        GridMapper(Mapper& m, OwnedArray<CellMapper>& cells, GridMapper& other_grid);
        void update_grid_boxes();
        Mapper& m;
        ValueTree grid_tree;
    };
    void valueTreePropertyChanged(ValueTree& t, const Identifier& p) override;
    void valueTreeChildAdded(ValueTree&, ValueTree& c) override;
    void valueTreeChildRemoved(ValueTree&, ValueTree& c, int) override;
    void update_sample_table(const bool added, const ValueTree& sample_table);
    void update_key_table(const bool added, const ValueTree& key_table);
    ValueTree state, fixed_param_vals, sample_table, param_list, sample_list, key_table, key_list;
    GridMapper grid0, grid1;
    OwnedArray<CellMapper> grid0_cells, grid1_cells;
    GenKeysButton gen_key_button;
    CreateButton create_button;
    TextButton affix_button;
    CriticalSection lock;
};
