#pragma once
namespace ID
{
    #define DECLARE_ID(name) const Identifier name (#name);

    // Document these ID's with expected data type and valid values.
    DECLARE_ID(STATE)
    //DECLARE_ID(height)				// (int) //--------------tentative ids 
    //DECLARE_ID(width)				// (int)
    //DECLARE_ID(presets_folder)      // (String) file_name
    //DECLARE_ID(sampler_polyphony)   // (int) 
    //DECLARE_ID(sampler_preset)      // (String) file_name
    //DECLARE_ID(samples_folder)      // (String) file_name

    DECLARE_ID(Grid)      // (String) file_name
        DECLARE_ID(param_name)      // (String) file_name
        DECLARE_ID(param_no)      // (String) file_name

        DECLARE_ID(Param_Val)
            DECLARE_ID(param_val_name)
            DECLARE_ID(param_val_no)
    
    //DECLARE_ID(Grid)      // (String) file_name
        //DECLARE_ID(param_name)      // (String) file_name reused

        //DECLARE_ID(param_val)
        //    DECLARE_ID(param_val_name)
        //    DECLARE_ID(param_val_no)

    DECLARE_ID(Key_Table)
        
        DECLARE_ID(Column_List)

            DECLARE_ID(Column)
                DECLARE_ID(column_name)
                DECLARE_ID(column_id)

        DECLARE_ID(Key_List)

    DECLARE_ID(Sample_Table)

        //DECLARE_ID(Column_List)

            //DECLARE_ID(Column)
                //DECLARE_ID(column_name)
                //DECLARE_ID(column_id)

        DECLARE_ID(Param_List)

            DECLARE_ID(Param)
                //DECLARE_ID(param_name)    // reused
                //DECLARE_ID(param_no)

                //DECLARE_ID(param_val)
                    //DECLARE_ID(param_val_name)
                    //DECLARE_ID(param_val_no)

        DECLARE_ID(Sample_List)

            DECLARE_ID(Sample)
                DECLARE_ID(file_name)
                DECLARE_ID(file_path)
                DECLARE_ID(note)
                DECLARE_ID(note_no)


















            //DECLARE_ID(param_name)
            //DECLARE_ID(param_val)

            //    DECLARE_ID(param_file)

// Non-state tree ID's
    #undef DECLARE_ID
}