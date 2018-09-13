#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "Utilities.h"
#include "Identifiers.h"
class Mapper : public Component
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
        AffixDialog(Mapper& m, const ValueTree& fixed_val_params);
        void resized() override;
        Mapper& m;
        OwnedArray<Label> param_labels;
        OwnedArray<ComboBox> param_boxes;
        ValueTree fixed_val_params;
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
        OwnedArray<CellMapper>& cells;
    };
    struct GridMapper : public ComboBox, public cc::ValueTreePropertyAndChildChangeListener
    {
        GridMapper(OwnedArray<CellMapper>& cells, GridMapper& other_grid);
        void valueTreePropertyChanged(ValueTree& t, const Identifier& p) override;
        void valueTreeChildAdded(ValueTree&, ValueTree& c) override;
        void valueTreeChildRemoved(ValueTree&, ValueTree& c, int) override;
        OwnedArray<CellMapper>& cells;
        ValueTree state, param_list, grid_tree;
        SortedSet<int> used_vals;
    };
    ValueTree state;
    GridMapper grid0, grid1;
    OwnedArray<CellMapper> grid0_cells, grid1_cells;
    GenKeysButton gen_key_button;
    CreateButton create_button;
    TextButton affix_button;
    CriticalSection lock;
};
