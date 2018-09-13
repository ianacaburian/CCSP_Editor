#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "Utilities.h"
#include "Identifiers.h"
class Mapper;
class AffixDialog : public DialogWindow
{
    AffixDialog(Mapper& m, const ValueTree& fixed_val_params);
    void resized() override;
    Mapper& m;
    OwnedArray<Label> param_labels;
    OwnedArray<ComboBox> param_boxes;
    ValueTree fixed_val_params;
    friend class Mapper;
};
class GenKeyButton : public TextButton
{
    GenKeyButton(Mapper& m);
    Mapper& m;
    friend class Mapper;
};
class CellMapper : public ComboBox
{
    CellMapper(OwnedArray<CellMapper>& cells);
    ValueTree param_val_tree;
    OwnedArray<CellMapper>& cells;
    friend class Mapper;
};

class GridMapper : public ComboBox, public cc::ValueTreePropertyAndChildChangeListener
{
    GridMapper(OwnedArray<CellMapper>& cells, GridMapper& other_grid);
    void valueTreePropertyChanged(ValueTree& t, const Identifier& p) override;
    void valueTreeChildAdded(ValueTree&, ValueTree& c) override;
    void valueTreeChildRemoved(ValueTree&, ValueTree& c, int) override;
    OwnedArray<CellMapper>& cells;
    ValueTree state, param_list, grid_tree;
    SortedSet<int> used_vals;
    friend class Mapper;
};

class Mapper : public Component
{
public:
    Mapper();
    void resized() override;
    void set_state(const ValueTree& state);
private:
    ValueTree state;
    GridMapper grid0, grid1;
    OwnedArray<CellMapper> grid0_cells, grid1_cells;
    GenKeyButton gen_key_button;
    TextButton affix_button;
    friend class GenKeyButton;
    friend class AffixDialog;
};
