#pragma once
#include "Identifiers.h"
namespace cc
{
    // Global constants ===================================================================
    static const char* const chromatic_scale[]{ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    static const auto octave_for_middle_c = 3, chromatic_scale_size = 12, num_grids = 2, 
                      num_cells = 8, num_boxes = num_grids * num_cells + num_grids,
                      octal_size = 3, highest_note_no = 127;

    // Data structure utilities ===========================================================
    template<typename T>
    inline String print_array(const T& arr, const String& label)
    {
        auto str{ label + " [ " };
        for (const auto& e : arr)
            str << String{ e } +" ";
        return str + " ]";
    }
    template<typename T>
    inline String print_owned_array(const OwnedArray<T>& arr, const String& label)
    {
        auto str{ label + " | " };
        for (const auto& e : arr)
            str << " | " + e->debug_str();
        return str;
    }
    inline std::pair<int, int> get_int_pair(const String& pair_str)
    {
        auto int_str{ String{} };
        auto int_pair{ std::pair<int,int>{} };
        auto i = 0;
        auto& char_ptr = pair_str.getCharPointer();
        for (;;)
        {
            if (auto ch = char_ptr.getAndAdvance())
            {
                int_str += ch;
                if (i == 1)
                {
                    int_pair.first = int_str.getIntValue();
                    int_str.clear();
                }
                ++i;
            }
            else
            {
                if (i != 4) return { -1, -1 };
                int_pair.second = int_str.getIntValue();
                break;
            }
        }
        return int_pair;
    }
    template<class T>
    inline String pad(const T& t, const char& char_, const int& num_spaces)
    {
        return String{ t }.paddedLeft(char_, num_spaces);
    }
    template<class A, class B>
    inline bool contains(const A& lower, const B& value, const A& upper)
    {
        return lower <= value && value <= upper;
    }
    inline void create_column(ValueTree& column_list, const String& name)
    {
        auto& column_tree = ValueTree{ ID::Column };
        column_list.appendChild(column_tree, nullptr);
        column_tree.setProperty(ID::column_name, name, nullptr);
        column_tree.setProperty(ID::column_id, column_list.getNumChildren(), nullptr);
    }
    inline ValueTree create_and_append(const Identifier& id, ValueTree& parent)
    {
        auto& new_child = ValueTree{ id };
        parent.appendChild(new_child, nullptr);
        return new_child;
    }
    // Tree wrappers ======================================================================
    struct ValueTreePropertyChangeListener : public ValueTree::Listener
    {
        void valueTreeChildAdded(ValueTree&, ValueTree&) override {}
        void valueTreeChildRemoved(ValueTree&, ValueTree&, int) override {}
        void valueTreeChildOrderChanged(ValueTree&, int, int) override {}
        void valueTreeParentChanged(ValueTree&) override {}
        void valueTreeRedirected(ValueTree&) override {}
    };
    struct ValueTreePropertyAndChildChangeListener : public ValueTree::Listener
    {
        void valueTreeChildOrderChanged(ValueTree&, int, int) override {}
        void valueTreeParentChanged(ValueTree&) override {}
        void valueTreeRedirected(ValueTree&) override {}
    };
    struct ValueTreePropertyAndChildChangeWithOrderListener : public ValueTree::Listener
    {
        void valueTreeParentChanged(ValueTree&) override {}
        void valueTreeRedirected(ValueTree&) override {}
    };

    // Tree utilities =====================================================================
    inline void set_int_pair_state(ValueTree& tree, const Identifier& prop, std::pair<int, int> int_pair, UndoManager* undo_man)
    {
        const auto& str{ pad(int_pair.first, '0', 2) + pad(int_pair.second, '0', 2) };
        if (tree[prop] == str)
            tree.sendPropertyChangeMessage(prop);
        else
            tree.setProperty(prop, str, undo_man);
    }
    inline void print_prop_val(const ValueTree& tree, const Identifier& prop, const String& caller)
    {
        String s; s << tree.getType().toString()  // tree type
            << " " << String{ tree.getParent().indexOf(tree) }.paddedLeft('0', 2) << " | " // index
            << prop.toString() << "= " << tree[prop].toString().paddedLeft('0', 2)
            << " (" << caller << ")"; // prop=val
        DBG(s);
    }
    // Component utilities ================================================================
    template<typename FunctionType>
    inline void visit_components(std::initializer_list<Component*> comps, FunctionType&& fn)
    {
        std::for_each(std::begin(comps), std::end(comps), fn);
    }
    inline void add_and_make_visible(Component& parent, std::initializer_list<Component*> comps)
    {
        std::for_each(std::begin(comps), std::end(comps),
            [&](Component* c) { parent.addAndMakeVisible(c); });
    }
    inline void add_children(Component& parent, std::initializer_list<Component*> comps)
    {
        std::for_each(std::begin(comps), std::end(comps),
            [&](Component* c) { parent.addChildComponent(c); });
    }
    inline bool double_clicked(const MouseEvent& e, const uint32& last_click_time)
    {   // uint32 holds a max 2^32 seconds, then wraps back to zero, may cause rare bugs.
        return  e.eventTime.getMillisecondCounter() - last_click_time < e.getDoubleClickTimeout();
    }
    template <class T>
    inline int get_step_no(const Point<T>& pos, const float step_width)
    {
        const auto pos_y_step = static_cast<int>(pos.x / step_width);
        return pos_y_step < 0 ? 0 : cc::steps_per_page <= pos_y_step ? cc::steps_per_page - 1 : pos_y_step;
    }
    template <class T>
    inline int get_cell_no(const Point<T>& pos, const int num_cells, const float grid_height, const float cell_height)
    {
        const auto pos_y_cell = static_cast<int>((grid_height - pos.y) / cell_height);
        return pos_y_cell < 0 ? 0 : num_cells <= pos_y_cell ? num_cells - 1 : pos_y_cell;
    }
    // Midi utilities =====================================================================
    inline String get_note_name(const int& note_no)
    {
        return MidiMessage::getMidiNoteName(note_no, true, true, octave_for_middle_c);
    }
    inline String get_note_key(const int& note_no)
    {
        return MidiMessage::getMidiNoteName(note_no, true, false, octave_for_middle_c);
    }
    inline std::pair<String, int> split_note_key_octave(const String& note_name)
    {
        auto note_key{ String{} }, octave{ String{} };
        auto& char_ptr = note_name.getCharPointer();
        for (;;)
        {
            if (auto ch = char_ptr.getAndAdvance())
            {
                if ((char_ptr - 1).isLetter() || ch == '#') note_key += ch;
                else octave += ch;
            }
            else return { note_key, octave.getIntValue() };
        }
    }
    inline String drop_octave(const String& note_name)
    {
        auto note_key{ String{} };
        auto& char_ptr = note_name.getCharPointer();
        for (;;)
        {
            if (auto ch = char_ptr.getAndAdvance())
            {
                if ((char_ptr - 1).isLetter() || ch == '#') note_key += ch;
                else return note_key;
            }
        }
    }
    inline int get_note_no(const String& note_name)
    {
        jassert(note_name != "");
        // Oddities re: note numbering:
        // International "middle C" is C4 (note number = 48.)
        // JUCE Plug-in Host C4 = 72 (C6)
        // JUCE reports C3 as noteNum = 60.
        // However, ignoring all of this makes things work
        // Future: provide visual feedback for sample selected and configure octNoteNum notes.
        const auto& note_pair = split_note_key_octave(note_name);
        const auto& octave_note_no = note_pair.second - octave_for_middle_c + 5;
        for (int i = 0; i != cc::chromatic_scale_size; ++i)
            if (!note_pair.first.compare(chromatic_scale[i]))
                return i + octave_note_no * cc::chromatic_scale_size;
    }
    inline int get_octave(const int& note_no)
    {
        return note_no / 12 + (octave_for_middle_c - 5);
    }
}