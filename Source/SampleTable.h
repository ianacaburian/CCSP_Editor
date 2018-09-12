#pragma once
class SampleTable : public Component, public TableListBoxModel, public cc::ValueTreePropertyAndChildChangeListener
{
public:
    SampleTable() 
    {
    }
    int getNumRows() override
    {
        return numRows;
    }

    void paintRowBackground(Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
    {
        auto alternateColour = getLookAndFeel().findColour(ListBox::backgroundColourId)
            .interpolatedWith(getLookAndFeel().findColour(ListBox::textColourId), 0.03f);
        if (rowIsSelected)
            g.fillAll(Colours::lightblue);
        else if (rowNumber % 2)
            g.fillAll(alternateColour);
    }

    void paintCell(Graphics& g, int rowNumber, int columnId,
        int width, int height, bool rowIsSelected) override
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
            TutorialDataSorter sorter(getAttributeNameForColumnId(newSortColumnId), isForwards);
            dataList->sortChildElements(sorter);

            table.updateContent();
        }
    }

    Component* refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/,
        Component* existingComponentToUpdate) override
    {
        if (columnId == 9)
        {
            auto* selectionBox = static_cast<SelectionColumnCustomComponent*> (existingComponentToUpdate);

            if (selectionBox == nullptr)
                selectionBox = new SelectionColumnCustomComponent(*this);

            selectionBox->setRowAndColumn(rowNumber, columnId);
            return selectionBox;
        }

        if (columnId == 8)
        {
            auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);

            if (textLabel == nullptr)
                textLabel = new EditableTextCustomComponent(*this);

            textLabel->setRowAndColumn(rowNumber, columnId);
            return textLabel;
        }

        jassert(existingComponentToUpdate == nullptr);
        return nullptr;
    }

    int getColumnAutoSizeWidth(int columnId) override
    {
        if (columnId == 9)
            return 50;

        int widest = 32;

        for (auto i = getNumRows(); --i >= 0;)
        {
            if (auto* rowElement = dataList->getChildElement(i))
            {
                auto text = rowElement->getStringAttribute(getAttributeNameForColumnId(columnId));

                widest = jmax(widest, font.getStringWidth(text));
            }
        }

        return widest + 8;
    }
    
    void valueTreePropertyChanged(ValueTree& t, const Identifier& p) override {}
   
    void valueTreeChildAdded(ValueTree&, ValueTree& c) override 
    {
        if (c.getType() == ID::Sample_Table) load_table(c);
        else if (c.getType() == ID::Key_Table) update_table_keys(c);
    }
    void valueTreeChildRemoved(ValueTree&, ValueTree& c, int) override 
    {
        if (c.getType() == ID::Sample_Table)
            DBG("Sample_Table removed");
    }
    void set_state(const ValueTree& state)
    {
        this->state = state;
        this->state.addListener(this);
    }
    void update_table_keys(ValueTree& table_tree)
    {
        auto& sample_table = state.getChildWithName(ID::Sample_Table);
        table.getHeader().removeAllColumns();
        tutorialData.reset(sample_table.createXml());
        dataList = tutorialData->getChildByName("Sample_List");
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
        table.getHeader().setColumnVisible(8, false);
        table.setMultipleSelectionEnabled(true);

    }
    void load_table(ValueTree& table_tree)
    {
        //auto t = table.getHeader().getNumColumns();
        table.getHeader().removeAllColumns();
        auto& column_list = table_tree.getChildWithName(ID::Column_List);
        auto& note_tree = ValueTree{ ID::Column };
        column_list.appendChild(note_tree, nullptr);
        note_tree.setProperty(ID::column_name, "note", nullptr);
        note_tree.setProperty(ID::column_id, column_list.getNumChildren(), nullptr);
        auto& note_no_tree = ValueTree{ ID::Column };
        column_list.appendChild(note_no_tree, nullptr);
        note_no_tree.setProperty(ID::column_name, "note_no", nullptr);
        const auto note_no_id = column_list.getNumChildren();
        note_no_tree.setProperty(ID::column_id, note_no_id, nullptr);
        auto& file_name_tree = ValueTree{ ID::Column };
        column_list.appendChild(file_name_tree, nullptr);
        file_name_tree.setProperty(ID::column_name, "file_name", nullptr);
        const auto file_name_id = column_list.getNumChildren();
        file_name_tree.setProperty(ID::column_id, file_name_id, nullptr);

        tutorialData.reset(table_tree.createXml());
        dataList = tutorialData->getChildByName("Sample_List");
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
        table.getHeader().setColumnWidth(note_no_id, 50);
        table.autoSizeColumn(file_name_id);
        table.getHeader().setSortColumnId(1, true);
        table.getHeader().setColumnVisible(8, false);
        table.setMultipleSelectionEnabled(true);
    }
    int getSelection(const int rowNumber) const
    {
        return dataList->getChildElement(rowNumber)->getIntAttribute("Select");
    }

    void setSelection(const int rowNumber, const int newSelection)
    {
        dataList->getChildElement(rowNumber)->setAttribute("Select", newSelection);
    }

    String getText(const int columnNumber, const int rowNumber) const
    {
        return dataList->getChildElement(rowNumber)->getStringAttribute(getAttributeNameForColumnId(columnNumber));
    }

    void setText(const int columnNumber, const int rowNumber, const String& newText)
    {
        const auto& columnName = table.getHeader().getColumnName(columnNumber);
        dataList->getChildElement(rowNumber)->setAttribute(columnName, newText);
    }

    //==============================================================================
    void resized() override
    {
        table.setBoundsInset(BorderSize<int>(8));
    }

private:
    ValueTree state, table_tree;
    TableListBox table{ {}, this };
    Font font{ 14.0f };

    std::unique_ptr<XmlElement> tutorialData;
    XmlElement* columnList = nullptr;
    XmlElement* dataList = nullptr;
    int numRows = 0;

    //==============================================================================
    class EditableTextCustomComponent : public Label
    {
    public:
        EditableTextCustomComponent(SampleTable& td)
            : owner(td)
        {
            setEditable(false, true, false);
        }

        void mouseDown(const MouseEvent& event) override
        {
            owner.table.selectRowsBasedOnModifierKeys(row, event.mods, false);

            Label::mouseDown(event);
        }

        void textWasEdited() override
        {
            owner.setText(columnId, row, getText());
        }

        void setRowAndColumn(const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            setText(owner.getText(columnId, row), dontSendNotification);
        }

    private:
        SampleTable& owner;
        int row, columnId;
        Colour textColour;
    };

    //==============================================================================
    class SelectionColumnCustomComponent : public Component
    {
    public:
        SelectionColumnCustomComponent(SampleTable& td)
            : owner(td)
        {
            addAndMakeVisible(toggleButton);

            toggleButton.onClick = [this] { owner.setSelection(row, (int)toggleButton.getToggleState()); };
        }

        void resized() override
        {
            toggleButton.setBoundsInset(BorderSize<int>(2));
        }

        void setRowAndColumn(int newRow, int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            toggleButton.setToggleState((bool)owner.getSelection(row), dontSendNotification);
        }

    private:
        SampleTable& owner;
        ToggleButton toggleButton;
        int row, columnId;
    };

    //==============================================================================
    class TutorialDataSorter
    {
    public:
        TutorialDataSorter(const String& attributeToSortBy, bool forwards)
            : attributeToSort(attributeToSortBy),
            direction(forwards ? 1 : -1)
        {}

        int compareElements(XmlElement* first, XmlElement* second) const
        {
            auto result = first->getStringAttribute(attributeToSort)
                .compareNatural(second->getStringAttribute(attributeToSort));

            if (result == 0)
                result = first->getStringAttribute("ID")
                .compareNatural(second->getStringAttribute("ID"));

            return direction * result;
        }

    private:
        String attributeToSort;
        int direction;
    };

    //==============================================================================
    String getAttributeNameForColumnId(const int columnId) const
    {
        forEachXmlChildElement(*columnList, columnXml)
        {
            if (columnXml->getIntAttribute("column_id") == columnId)
                return columnXml->getStringAttribute("column_name");
        }

        return {};
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleTable)
};
