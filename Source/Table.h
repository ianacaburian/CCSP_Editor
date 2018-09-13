#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "Identifiers.h"
#include "Utilities.h"

class Table : public Component, public TableListBoxModel, public cc::ValueTreePropertyAndChildChangeListener
{
public:
    Table(const Identifier& id_table, const String& data_list) : id_table(id_table), data_list(data_list) {}
   void set_state(const ValueTree& state)
    {
        this->state = state;
        this->state.addListener(this);
    }
protected:
    void valueTreeChildAdded(ValueTree&, ValueTree& c) override { if (c.getType() == id_table) load_table(c); }
    virtual void load_table(ValueTree& table_tree)
    {
        this->table_tree = table_tree;
        tutorialData.reset(table_tree.createXml());
        dataList = tutorialData->getChildByName(data_list);
        columnList = tutorialData->getChildByName("Column_List");
        numRows = dataList->getNumChildElements();
        if (getIndexOfChildComponent(&table) == -1)
            addAndMakeVisible(table);
        table.setColour(ListBox::outlineColourId, Colours::grey);
        table.setOutlineThickness(1);
        if (columnList != nullptr)
            forEachXmlChildElement(*columnList, columnXml)
            table.getHeader().addColumn(columnXml->getStringAttribute("column_name"),
                columnXml->getIntAttribute("column_id"), 30);
        table.autoSizeAllColumns();
        table.getHeader().setSortColumnId(1, true);
        table.setMultipleSelectionEnabled(true);
    }
    void remove_table(ValueTree& table_tree)
    {
        this->table_tree = ValueTree{};
        table.getHeader().removeAllColumns();
        resized();
        DBG(id_table.toString() << " removed");            
    }
    // ====================================================================================
    const Identifier& id_table;
    ValueTree state, table_tree;
    TableListBox table{ {}, this };
private:
    // ====================================================================================
    struct EditableTextCustomComponent : public Label
    {
        EditableTextCustomComponent(Table& td) : owner(td) { setEditable(false, true, false); }
        void mouseDown(const MouseEvent& event) override
        {
            owner.table.selectRowsBasedOnModifierKeys(row, event.mods, false);
            Label::mouseDown(event);
        }
        void textWasEdited() override { owner.setText(columnId, row, getText()); }
        void setRowAndColumn(const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            setText(owner.getText(columnId, row), dontSendNotification);
        }
        // EditableTextCustomComponent ====================================================
        Table& owner;
        int row, columnId;
        Colour textColour;
    };
    // ====================================================================================
    struct SelectionColumnCustomComponent : public Component
    {
        SelectionColumnCustomComponent(Table& td)
            : owner(td)
        {
            addAndMakeVisible(toggleButton);
            toggleButton.onClick = [this] { owner.setSelection(row, (int)toggleButton.getToggleState()); };
        }
        void resized() override { toggleButton.setBoundsInset(BorderSize<int>(2)); }
        void setRowAndColumn(int newRow, int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            toggleButton.setToggleState((bool)owner.getSelection(row), dontSendNotification);
        }
        // SelectionColumnCustomComponent =================================================
        Table& owner;
        ToggleButton toggleButton;
        int row, columnId;
    };
    // ====================================================================================
    struct DataSorter
    {
        DataSorter(const String& attributeToSortBy, bool forwards)
            : attributeToSort(attributeToSortBy), direction(forwards ? 1 : -1) {}
        int compareElements(XmlElement* first, XmlElement* second) const
        {
            auto result = first->getStringAttribute(attributeToSort)
                                .compareNatural(second->getStringAttribute(attributeToSort));
            if (result == 0)
                result = first->getStringAttribute("ID").compareNatural(second->getStringAttribute("ID"));
            return direction * result;
        }
        // DataSorter =====================================================================
        String attributeToSort;
        int direction;
    };
    // Table ==============================================================================
    void resized() override { table.setBounds(getLocalBounds()); }
    int getNumRows() override { return numRows; }
    void paintRowBackground(Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
    {
        auto alternateColour = getLookAndFeel().findColour(ListBox::backgroundColourId)
            .interpolatedWith(getLookAndFeel().findColour(ListBox::textColourId), 0.03f);
        if (rowIsSelected)
            g.fillAll(Colours::lightblue);
        else if (rowNumber % 2)
            g.fillAll(alternateColour);
    }
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override
    {
        g.setColour(rowIsSelected ? Colours::darkblue : getLookAndFeel().findColour(ListBox::textColourId));
        g.setFont(font);
        if (auto* rowElement = dataList->getChildElement(rowNumber))
        {
            auto text = rowElement->getStringAttribute(getAttributeNameForColumnId(columnId));
            g.drawText(text, 2, 0, width - 4, height, Justification::centredLeft, true);
        }
        g.setColour(getLookAndFeel().findColour(ListBox::backgroundColourId));
        g.fillRect(width - 1, 0, 1, height);
    }
    void sortOrderChanged(int newSortColumnId, bool isForwards) override
    {
        if (newSortColumnId != 0)
        {
            DataSorter sorter(getAttributeNameForColumnId(newSortColumnId), isForwards);
            dataList->sortChildElements(sorter);
            table.updateContent();
        }
    }
    Component* refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/,
        Component* existingComponentToUpdate) override
    {   // Selection and Editable rows are updated here -- see Juce TableList tutorial for details       
        jassert(existingComponentToUpdate == nullptr);
        return nullptr;
    }
    int getColumnAutoSizeWidth(int columnId) override
    {
        const auto col_name = getAttributeNameForColumnId(columnId);
        auto widest = font.getStringWidth(col_name);
        for (auto i = getNumRows(); --i >= 0;)
            if (auto* rowElement = dataList->getChildElement(i))
                widest = jmax(widest, font.getStringWidth(rowElement->getStringAttribute(col_name)));
        return widest + 24;
    }
    void valueTreePropertyChanged(ValueTree& t, const Identifier& p) override {}
    void valueTreeChildRemoved(ValueTree&, ValueTree& c, int) override { if (c.getType() == id_table) remove_table(c); }

    int getSelection(const int rowNumber) const { return dataList->getChildElement(rowNumber)->getIntAttribute("Select"); }
    void setSelection(const int rowNumber, const int newSelection) { dataList->getChildElement(rowNumber)->setAttribute("Select", newSelection); }
    String getText(const int columnNumber, const int rowNumber) const { return dataList->getChildElement(rowNumber)->getStringAttribute(getAttributeNameForColumnId(columnNumber)); }
    void setText(const int columnNumber, const int rowNumber, const String& newText) { dataList->getChildElement(rowNumber)->setAttribute(table.getHeader().getColumnName(columnNumber), newText); }
    String getAttributeNameForColumnId(const int columnId) const
    {
        forEachXmlChildElement(*columnList, columnXml)
            if (columnXml->getIntAttribute("column_id") == columnId)
                return columnXml->getStringAttribute("column_name");
        return {};
    }
    // Table ==============================================================================
    Font font{ 14.0f };
    std::unique_ptr<XmlElement> tutorialData;
    XmlElement* columnList = nullptr;
    XmlElement* dataList = nullptr;
    const String data_list;
    int numRows = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Table)
};
class SampleTable : public Table
{
public:
    SampleTable() : Table{ ID::Sample_Table, "Sample_List" }
    {
    }
private:
    void valueTreeChildAdded(ValueTree& p, ValueTree& c) override
    {
        Table::valueTreeChildAdded(p, c);
        //if (c.getType() == ID::Key_Table) Table::load_table(get_table_tree());
    }
    void load_table(ValueTree& table_tree) override
    {
        auto& column_list = table_tree.getChildWithName(ID::Column_List);
        cc::create_column(column_list, "key_no");
        cc::create_column(column_list, "root_note");
        cc::create_column(column_list, "root_note_no");
        cc::create_column(column_list, "duplicate_of");
        cc::create_column(column_list, "low_note_no");
        cc::create_column(column_list, "note_range");
        cc::create_column(column_list, "file_name");
        Table::load_table(table_tree);
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleTable)
};
