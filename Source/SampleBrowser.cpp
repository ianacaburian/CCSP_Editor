#include "SampleBrowser.h"
SampleBrowser::SampleBrowser(const File& init_dir)
    : FileBrowserComponent(FileChooserFlags::canSelectMultipleItems
                         | FileChooserFlags::canSelectDirectories
                         | FileChooserFlags::canSelectFiles
                         | FileChooserFlags::openMode, init_dir, nullptr, nullptr)
{
}
void SampleBrowser::set_state(const ValueTree& state)
{
    this->state = state;
}
void SampleBrowser::fileClicked(const File& f, const MouseEvent& e) 
{
    auto analyze_directory = [this](const File& f)->ValueTree
    {
        auto add_new_val_tree = [this](ValueTree& param_tree, const String& param_val_str)
        {
            auto& param_val_tree = cc::create_and_append(ID::Param_Val, param_tree);
            param_val_tree.setProperty(ID::param_val_name, param_val_str, nullptr);
            param_val_tree.setProperty(ID::param_val_no, param_tree.getNumChildren(), nullptr);
        };
        auto set_sample_info = [this](ValueTree& sample_tree, const String& file_name, const String& file_path, const String& note_str)
        {
            sample_tree.setProperty(ID::file_name, file_name, nullptr);
            sample_tree.setProperty(ID::file_path, file_path, nullptr);
            sample_tree.setProperty(ID::root_note, note_str, nullptr);
            sample_tree.setProperty(ID::root_note_no, cc::get_note_no(note_str), nullptr);
        };

        auto analyze_params_str = [this, &add_new_val_tree]
        (String& params_str, ValueTree& param_list, ValueTree& column_list, ValueTree& sample_tree)
        {
            auto add_new_param_tree = [this, &add_new_val_tree]
            (ValueTree& param_tree, ValueTree& param_list, ValueTree& column_list, const String& param_name_str, const String& param_val_str)
            {
                param_tree = cc::create_and_append(ID::Param, param_list);
                param_tree.setProperty(ID::param_name, param_name_str, nullptr);
                param_tree.setProperty(ID::param_no, param_list.getNumChildren(), nullptr);
                add_new_val_tree(param_tree, param_val_str);
                cc::create_column(column_list, param_name_str);
            };
            while (params_str.isNotEmpty())
            {
                const auto& param_str = params_str.upToFirstOccurrenceOf(" ", false, true);
                params_str = params_str.fromFirstOccurrenceOf(" ", false, true);
                const auto& param_name_str = param_str.upToFirstOccurrenceOf("=", false, true);
                const auto& param_val_str = param_str.fromFirstOccurrenceOf("=", false, true);
                if (param_name_str.isEmpty() || param_val_str.isEmpty())
                    continue;
                auto& param_tree = param_list.getChildWithProperty(ID::param_name, param_name_str);
                auto& param_val_tree = param_tree.getChildWithProperty(ID::param_val_name, param_val_str);
                if (!param_tree.isValid())
                    add_new_param_tree(param_tree, param_list, column_list, param_name_str, param_val_str);
                else if (!param_val_tree.isValid()) // && param_tree.isValid()
                    add_new_val_tree(param_tree, param_val_str);
                sample_tree.setProperty(Identifier{ param_name_str }, param_val_str, nullptr);
                sample_tree.setProperty(Identifier{ param_name_str + "_val_no" }, param_val_tree[ID::param_val_no], nullptr);
            }
        };
        auto& table_tree = ValueTree{ ID::Sample_Table };
        auto& column_list = cc::create_and_append(ID::Column_List, table_tree);
        auto& param_list = cc::create_and_append(ID::Param_List, table_tree);
        auto& sample_list = cc::create_and_append(ID::Sample_List, table_tree);
        sample_list.setProperty(ID::samples_folder, f.getFullPathName(), nullptr);
        auto& di = DirectoryIterator{ f, false };
        while (di.next())
        {
            auto& file = di.getFile();
            if (file.hasFileExtension("wav"))
            {
                const auto& file_name = file.getFileNameWithoutExtension();
                const auto& note_str = file_name.fromFirstOccurrenceOf("note=", false, true)
                                                .upToFirstOccurrenceOf(" ", false, true);
                if (note_str.isEmpty())
                    continue;
                auto& sample_tree = cc::create_and_append(ID::Sample, sample_list);
                set_sample_info(sample_tree, file_name, file.getFullPathName(), note_str);
                analyze_params_str(file_name.replaceFirstOccurrenceOf("note=" + note_str, "", true).trim(), 
                                   param_list, column_list, sample_tree);
            }
        }
        return table_tree;
    };
    if (f.isDirectory())
    {
        const ScopedLock sl(lock);
        state.removeChild(state.getChildWithName(ID::Sample_Table), nullptr);
        state.removeChild(state.getChildWithName(ID::Key_Table), nullptr);
        state.appendChild(analyze_directory(f), nullptr);
    }
}