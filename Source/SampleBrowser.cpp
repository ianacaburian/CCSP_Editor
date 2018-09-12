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
    auto remove_table_tree = [this]()
    {
        const ScopedLock sl(lock);
        const auto table_i = state.indexOf(state.getChildWithName(ID::Sample_Table));
        if (table_i != -1)
            state.removeChild(table_i, nullptr);
    };
    auto analyze_directory = [this](const File& f)->ValueTree
    {
        auto add_new_val_tree = [this](ValueTree& param_tree, const String& param_val_str)
        {
            auto& param_val_tree = ValueTree{ ID::Param_Val };
            param_tree.appendChild(param_val_tree, nullptr);
            param_val_tree.setProperty(ID::param_val_name, param_val_str, nullptr);
            param_val_tree.setProperty(ID::param_val_no, param_tree.getNumChildren(), nullptr);
        };
        auto& table_tree = ValueTree{ ID::Sample_Table };
        auto& column_list = ValueTree{ ID::Column_List };
        table_tree.appendChild(column_list, nullptr);
        auto& param_list = ValueTree{ ID::Param_List };
        table_tree.appendChild(param_list, nullptr);
        auto& sample_list = ValueTree{ ID::Sample_List };
        table_tree.appendChild(sample_list, nullptr);
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
                auto& sample_tree = ValueTree{ ID::Sample };
                sample_list.appendChild(sample_tree, nullptr);
                sample_tree.setProperty(ID::file_name, file_name, nullptr);
                sample_tree.setProperty(ID::file_path, file.getFullPathName(), nullptr);
                sample_tree.setProperty(ID::note, note_str, nullptr);
                sample_tree.setProperty(ID::note_no, cc::get_note_no(note_str), nullptr);

                auto& params_str = file_name.replaceFirstOccurrenceOf("note=" + note_str, "", true).trim();
                while (params_str.isNotEmpty())
                {
                    const auto& param_str = params_str.upToFirstOccurrenceOf(" ", false, true);
                    params_str = params_str.fromFirstOccurrenceOf(" ", false, true);
                    const auto& param_name_str = param_str.upToFirstOccurrenceOf("=", false, true);
                    const auto& param_val_str = param_str.fromFirstOccurrenceOf("=", false, true);
                    if (param_name_str.isEmpty() || param_val_str.isEmpty())
                        continue;                    
                    auto& param_tree = param_list.getChildWithProperty(ID::param_name, param_name_str);
                    if (param_tree.isValid())
                    {
                        if (!param_tree.getChildWithProperty(ID::param_val_name, param_val_str).isValid())
                            add_new_val_tree(param_tree, param_val_str);
                    }
                    else
                    {
                        param_tree = ValueTree{ ID::Param };
                        param_list.appendChild(param_tree, nullptr);
                        param_tree.setProperty(ID::param_name, param_name_str, nullptr);
                        param_tree.setProperty(ID::param_no, param_list.getNumChildren(), nullptr);
                        add_new_val_tree(param_tree, param_val_str);

                        auto& column_tree = ValueTree{ ID::Column };
                        column_list.appendChild(column_tree, nullptr);
                        column_tree.setProperty(ID::column_name, param_name_str, nullptr);
                        column_tree.setProperty(ID::column_id, column_list.getNumChildren(), nullptr);
                    }
                    sample_tree.setProperty(Identifier{ param_name_str }, param_val_str, nullptr);
                    const auto param_val_no = param_tree.getChildWithProperty(ID::param_val_name, param_val_str)[ID::param_val_no];
                    sample_tree.setProperty(Identifier{ param_name_str + "_val_no" }, param_val_no, nullptr);
                }
            }
        }
        DBG(param_list.toXmlString());
        return table_tree;
    };
    auto add_new_table_tree = [this](const ValueTree& new_table_tree)
    {
        const ScopedLock sl(lock);
        state.appendChild(new_table_tree, nullptr);
    };
    if (f.isDirectory())
    {
        remove_table_tree();
        add_new_table_tree(analyze_directory(f));
        DBG(state.getChildWithName(ID::Sample_Table).getChildWithName(ID::Param_List).toXmlString());
    }
}

//void SampleBrowser::analyze_directory(const File& f)
//{
//    StringArray param_name_map;
//    OwnedArray<StringArray> param_val_map; // error check number of vals does not exceed 8
//    OwnedArray<OwnedArray<StringArray>> param_file_map;
//    auto& di = DirectoryIterator{ f, false };
//    while (di.next())
//    {
//        auto& file = di.getFile();
//        if (file.hasFileExtension("wav"))
//        {
//            const auto& file_name = file.getFileNameWithoutExtension();
//            const auto& note_str = file_name.fromFirstOccurrenceOf("note=", false, true)
//                                            .upToFirstOccurrenceOf(" ", false, true);
//            const auto note_no = cc::get_note_no(note_str); // error check
//            auto& params_str = file_name.replaceFirstOccurrenceOf("note=" + note_str, "", true).trim();
//            while (params_str.isNotEmpty())
//            {
//                const auto& param_str = params_str.upToFirstOccurrenceOf(" ", false, true);
//                params_str = params_str.fromFirstOccurrenceOf(" ", false, true);
//                const auto& param_name_str = param_str.upToFirstOccurrenceOf("=", false, true);
//                const auto& param_val_str = param_str.fromFirstOccurrenceOf("=", false, true);
//                if (param_name_map.contains(param_name_str))
//                {
//                    const auto param_name_i = param_name_map.indexOf(param_name_str);
//                    auto* param_vals = param_val_map[param_name_i];
//                    const auto param_val_i = param_vals->indexOf(param_val_str);
//                    if (param_val_i == -1)
//                    {
//                        param_vals->add(param_val_str);
//                        param_file_map[param_name_i]->add(new StringArray{ file_name });
//                    }                        
//                    else
//                        param_file_map[param_name_i]->operator[](param_val_i)->add(file_name);
//                }
//                else
//                {
//                    param_name_map.add(param_name_str);
//                    param_val_map.add(new StringArray{});
//                    param_file_map.add(new OwnedArray<StringArray>{});
//                }
//            }
//        }
//    }
//    const ScopedLock sl(lock);
//    state.removeAllChildren(nullptr);
//    for (int i = 0; i != param_name_map.size(); ++i)
//    {
//        const auto& param_name_str = param_name_map.getReference(i);
//        auto param_tree = ValueTree{ ID::param_name };
//        state.appendChild(param_tree, nullptr);
//        param_tree.setProperty(ID::param_name, param_name_str, nullptr);
//        auto* param_vals = param_val_map.getUnchecked(i);
//        for (int j = 0; j != param_vals->size(); ++j)
//        {
//            auto param_val_tree = ValueTree{ ID::param_val };
//            param_tree.appendChild(param_val_tree, nullptr);
//            param_val_tree.setProperty(ID::param_val, param_vals->getReference(j), nullptr);
//            auto* param_files = param_file_map.getUnchecked(i)->getUnchecked(j);
//            for (int k = 0; k != param_files->size(); ++k)
//            {
//                auto param_file_tree = ValueTree{ ID::param_file };
//                param_val_tree.appendChild(param_file_tree, nullptr);
//                param_file_tree.setProperty(ID::param_file, param_files->getReference(k), nullptr);
//            }
//        }
//    }
//}
