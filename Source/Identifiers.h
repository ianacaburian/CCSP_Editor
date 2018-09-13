#pragma once
namespace ID
{
    #define DECLARE_ID(name) const Identifier name (#name);

    DECLARE_ID(STATE)

    // IMPORTANT: Grid type trees are called by index, meaning that there is a 
    // heavy dependence on their position as the first two child trees under STATE!! 
    DECLARE_ID(Grid)      
        DECLARE_ID(param_name)      // (String) file_name
        DECLARE_ID(param_no)      // (String) file_name

        DECLARE_ID(Param_Val)
            DECLARE_ID(param_val_name)
            DECLARE_ID(param_val_no)

    DECLARE_ID(Sample_Table)

        DECLARE_ID(Column_List)

            DECLARE_ID(Column)
                DECLARE_ID(column_name)
                DECLARE_ID(column_id)

        DECLARE_ID(Param_List)

            DECLARE_ID(Param)
                //DECLARE_ID(param_name)   
                //DECLARE_ID(param_no)

                //DECLARE_ID(param_val)
                    //DECLARE_ID(param_val_name)
                    //DECLARE_ID(param_val_no)

        DECLARE_ID(Sample_List)
            DECLARE_ID(samples_folder)
            DECLARE_ID(num_duplicates)

            DECLARE_ID(Sample)
                DECLARE_ID(file_name)
                DECLARE_ID(file_path)
                DECLARE_ID(root_note)
                DECLARE_ID(root_note_no)
                DECLARE_ID(key_no) 
                DECLARE_ID(duplicate_of) // only set if it is a duplicate
                DECLARE_ID(low_note_no) // only set if not a duplicate
                DECLARE_ID(note_range)// only set if not a duplicate

    DECLARE_ID(Fixed_Param_Vals) // has properties made up of <param_name>_val_no="<param_val_no>"

    DECLARE_ID(Key_Table)
        
        //DECLARE_ID(Column_List) // "key_no" "num_files" "notes" 

        //    DECLARE_ID(Column)
        //        DECLARE_ID(column_name)
        //        DECLARE_ID(column_id)

        DECLARE_ID(Key_List)

            DECLARE_ID(Key)
                //DECLARE_ID(key_no)
                DECLARE_ID(num_shared)
                DECLARE_ID(notes)

// Non-state tree ID's
    #undef DECLARE_ID
}