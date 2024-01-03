//
// Created by consti10 on 11.11.20.
//

#include <string>

#ifndef LIVEVIDEO10MS_NALUNITTYPE_H
#define LIVEVIDEO10MS_NALUNITTYPE_H

// h264 types come from h264_stream
// h265 types are declared here
// This is just a lot of boilerplate code so I put it into its own .hpp file
namespace NALUnitType{

    namespace H264{
        enum {
            //Table 7-1 NAL unit type codes
            NAL_UNIT_TYPE_UNSPECIFIED                    =0,    // Unspecified
            NAL_UNIT_TYPE_CODED_SLICE_NON_IDR            =1,    // Coded slice of a non-IDR picture
            NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_A   =2,    // Coded slice data partition A
            NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_B   =3,    // Coded slice data partition B
            NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_C   =4,    // Coded slice data partition C
            NAL_UNIT_TYPE_CODED_SLICE_IDR                =5,    // Coded slice of an IDR picture
            NAL_UNIT_TYPE_SEI                            =6,    // Supplemental enhancement information (SEI)
            NAL_UNIT_TYPE_SPS                            =7,    // Sequence parameter set
            NAL_UNIT_TYPE_PPS                            =8,    // Picture parameter set
            NAL_UNIT_TYPE_AUD                            =9,    // Access unit delimiter
            NAL_UNIT_TYPE_END_OF_SEQUENCE               =10,    // End of sequence
            NAL_UNIT_TYPE_END_OF_STREAM                 =11,    // End of stream
            NAL_UNIT_TYPE_FILLER                        =12,    // Filler data
            NAL_UNIT_TYPE_SPS_EXT                       =13,    // Sequence parameter set extension
            NAL_UNIT_TYPE_PREFIX_NAL                    =14,    // Prefix NAL unit
            NAL_UNIT_TYPE_SUBSET_SPS                    =15,    // Subset Sequence parameter set
            NAL_UNIT_TYPE_DPS                           =16,    // Depth Parameter Set
            // 17..18    // Reserved
            NAL_UNIT_TYPE_CODED_SLICE_AUX               =19,    // Coded slice of an auxiliary coded picture without partitioning
            NAL_UNIT_TYPE_CODED_SLICE_SVC_EXTENSION     =20,    // Coded slice of SVC extension
            // 20..23    // Reserved
            // 24..31    // Unspecified
        };
        static std::string unit_type_to_string(const int nal_unit_type){
            std::string nal_unit_type_name;
            switch (nal_unit_type){
                case  NAL_UNIT_TYPE_UNSPECIFIED :                   nal_unit_type_name = "NAL_UNIT_TYPE_UNSPECIFIED"; break;//Unspecified
                case  NAL_UNIT_TYPE_CODED_SLICE_NON_IDR :           nal_unit_type_name = "NAL_UNIT_TYPE_CODED_SLICE_NON_IDR"; break;//Coded slice of a non-IDR picture
                case  NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_A :  nal_unit_type_name = "NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_A"; break;//Coded slice data partition A
                case  NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_B :  nal_unit_type_name = "NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_B"; break;//Coded slice data partition B
                case  NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_C :  nal_unit_type_name = "NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_C"; break;//Coded slice data partition C
                case  NAL_UNIT_TYPE_CODED_SLICE_IDR :               nal_unit_type_name = "NAL_UNIT_TYPE_CODED_SLICE_IDR"; break;//Coded slice of an IDR picture
                case  NAL_UNIT_TYPE_SEI :                           nal_unit_type_name = "NAL_UNIT_TYPE_SEI"; break;//Supplemental enhancement information (SEI)
                case  NAL_UNIT_TYPE_SPS :                           nal_unit_type_name = "NAL_UNIT_TYPE_SPS"; break;//Sequence parameter set
                case  NAL_UNIT_TYPE_PPS :                           nal_unit_type_name = "NAL_UNIT_TYPE_PPS"; break;//Picture parameter set
                case  NAL_UNIT_TYPE_AUD :                           nal_unit_type_name = "NAL_UNIT_TYPE_AUD"; break;//Access unit delimiter
                case  NAL_UNIT_TYPE_END_OF_SEQUENCE :               nal_unit_type_name = "NAL_UNIT_TYPE_END_OF_SEQUENCE"; break;//End of sequence
                case  NAL_UNIT_TYPE_END_OF_STREAM :                 nal_unit_type_name = "NAL_UNIT_TYPE_END_OF_STREAM"; break;//End of stream
                case  NAL_UNIT_TYPE_FILLER :                        nal_unit_type_name = "NAL_UNIT_TYPE_FILLER"; break;//Filler data
                case  NAL_UNIT_TYPE_SPS_EXT :                       nal_unit_type_name = "SNAL_UNIT_TYPE_SPS_EXT"; break;//Sequence parameter set extension
                    // 14..18    // Reserved
                case  NAL_UNIT_TYPE_CODED_SLICE_AUX :               nal_unit_type_name = "NAL_UNIT_TYPE_CODED_SLICE_AUX"; break;
                //Coded slice of an auxiliary coded picture without partitioning
                    // 20..23    // Reserved
                    // 24..31    // Unspecified
                default :                                           nal_unit_type_name = std::string("Unknown")+std::to_string(nal_unit_type); break;
            }
            return nal_unit_type_name;
        };
    }

    namespace H265{
        enum NalUnitType {
            NAL_UNIT_CODED_SLICE_TRAIL_N = 0,
            NAL_UNIT_CODED_SLICE_TRAIL_R,

            NAL_UNIT_CODED_SLICE_TSA_N,
            NAL_UNIT_CODED_SLICE_TSA_R,

            NAL_UNIT_CODED_SLICE_STSA_N,
            NAL_UNIT_CODED_SLICE_STSA_R,

            NAL_UNIT_CODED_SLICE_RADL_N,
            NAL_UNIT_CODED_SLICE_RADL_R,

            NAL_UNIT_CODED_SLICE_RASL_N,
            NAL_UNIT_CODED_SLICE_RASL_R,

            NAL_UNIT_RESERVED_VCL_N10,
            NAL_UNIT_RESERVED_VCL_R11,
            NAL_UNIT_RESERVED_VCL_N12,
            NAL_UNIT_RESERVED_VCL_R13,
            NAL_UNIT_RESERVED_VCL_N14,
            NAL_UNIT_RESERVED_VCL_R15,

            NAL_UNIT_CODED_SLICE_BLA_W_LP,
            NAL_UNIT_CODED_SLICE_BLA_W_RADL,
            NAL_UNIT_CODED_SLICE_BLA_N_LP,
            NAL_UNIT_CODED_SLICE_IDR_W_RADL,
            NAL_UNIT_CODED_SLICE_IDR_N_LP,
            NAL_UNIT_CODED_SLICE_CRA,
            NAL_UNIT_RESERVED_IRAP_VCL22,
            NAL_UNIT_RESERVED_IRAP_VCL23,

            NAL_UNIT_RESERVED_VCL24,
            NAL_UNIT_RESERVED_VCL25,
            NAL_UNIT_RESERVED_VCL26,
            NAL_UNIT_RESERVED_VCL27,
            NAL_UNIT_RESERVED_VCL28,
            NAL_UNIT_RESERVED_VCL29,
            NAL_UNIT_RESERVED_VCL30,
            NAL_UNIT_RESERVED_VCL31,

            NAL_UNIT_VPS,
            NAL_UNIT_SPS,
            NAL_UNIT_PPS,
            NAL_UNIT_ACCESS_UNIT_DELIMITER,
            NAL_UNIT_EOS,
            NAL_UNIT_EOB,
            NAL_UNIT_FILLER_DATA,
            NAL_UNIT_PREFIX_SEI,
            NAL_UNIT_SUFFIX_SEI,
            NAL_UNIT_RESERVED_NVCL41,
            NAL_UNIT_RESERVED_NVCL42,
            NAL_UNIT_RESERVED_NVCL43,
            NAL_UNIT_RESERVED_NVCL44,
            NAL_UNIT_RESERVED_NVCL45,
            NAL_UNIT_RESERVED_NVCL46,
            NAL_UNIT_RESERVED_NVCL47,
            NAL_UNIT_UNSPECIFIED_48,
            NAL_UNIT_UNSPECIFIED_49,
            NAL_UNIT_UNSPECIFIED_50,
            NAL_UNIT_UNSPECIFIED_51,
            NAL_UNIT_UNSPECIFIED_52,
            NAL_UNIT_UNSPECIFIED_53,
            NAL_UNIT_UNSPECIFIED_54,
            NAL_UNIT_UNSPECIFIED_55,
            NAL_UNIT_UNSPECIFIED_56,
            NAL_UNIT_UNSPECIFIED_57,
            NAL_UNIT_UNSPECIFIED_58,
            NAL_UNIT_UNSPECIFIED_59,
            NAL_UNIT_UNSPECIFIED_60,
            NAL_UNIT_UNSPECIFIED_61,
            NAL_UNIT_UNSPECIFIED_62,
            NAL_UNIT_UNSPECIFIED_63,
            NAL_UNIT_INVALID,
        };
        static std::string unit_type_to_string(const int nal_unit_type){
            std::string nal_unit_type_name="Unimpl";
            switch (nal_unit_type){
                case NalUnitType::NAL_UNIT_CODED_SLICE_TRAIL_N:
                    nal_unit_type_name="NAL_UNIT_CODED_SLICE_TRAIL_N";break;
                case NalUnitType::NAL_UNIT_CODED_SLICE_TRAIL_R:
                    nal_unit_type_name="NAL_UNIT_CODED_SLICE_TRAIL_R";break;

                case NalUnitType::NAL_UNIT_CODED_SLICE_TSA_N:
                    nal_unit_type_name="NAL_UNIT_CODED_SLICE_TSA_N";break;
                case NalUnitType::NAL_UNIT_CODED_SLICE_TSA_R:
                    nal_unit_type_name="NAL_UNIT_CODED_SLICE_TSA_R";break;

                case NalUnitType::NAL_UNIT_CODED_SLICE_STSA_N:
                    nal_unit_type_name="NAL_UNIT_CODED_SLICE_STSA_N";break;
                case NalUnitType::NAL_UNIT_CODED_SLICE_STSA_R:
                    nal_unit_type_name="NAL_UNIT_CODED_SLICE_STSA_R";break;

                case NalUnitType::NAL_UNIT_CODED_SLICE_RADL_N:
                    nal_unit_type_name="NAL_UNIT_CODED_SLICE_RADL_N";break;
                case NalUnitType::NAL_UNIT_CODED_SLICE_RADL_R:
                    nal_unit_type_name="NAL_UNIT_CODED_SLICE_RADL_R";break;

                case NalUnitType::NAL_UNIT_CODED_SLICE_RASL_N:
                    nal_unit_type_name="NAL_UNIT_CODED_SLICE_RASL_N";break;
                case NalUnitType::NAL_UNIT_CODED_SLICE_RASL_R:
                    nal_unit_type_name="NAL_UNIT_CODED_SLICE_RASL_R";break;

                case NalUnitType::NAL_UNIT_RESERVED_VCL_N10:
                    nal_unit_type_name="NAL_UNIT_RESERVED_VCL_N10";break;
                case NalUnitType::NAL_UNIT_RESERVED_VCL_R11:
                    nal_unit_type_name="NAL_UNIT_RESERVED_VCL_R11";break;
                case NalUnitType::NAL_UNIT_RESERVED_VCL_N12:
                    nal_unit_type_name="NAL_UNIT_RESERVED_VCL_N12";break;
                case NalUnitType::NAL_UNIT_RESERVED_VCL_R13:
                    nal_unit_type_name="NAL_UNIT_RESERVED_VCL_R13";break;
                case NalUnitType::NAL_UNIT_RESERVED_VCL_N14:
                    nal_unit_type_name="NAL_UNIT_RESERVED_VCL_N14";break;
                case NalUnitType::NAL_UNIT_RESERVED_VCL_R15:
                    nal_unit_type_name="NAL_UNIT_RESERVED_VCL_R15";break;

                case NalUnitType::NAL_UNIT_CODED_SLICE_BLA_W_LP:
                    nal_unit_type_name="NAL_UNIT_CODED_SLICE_BLA_W_LP";break;
                case NalUnitType::NAL_UNIT_CODED_SLICE_BLA_W_RADL:
                    nal_unit_type_name="NAL_UNIT_CODED_SLICE_BLA_W_RADL";break;
                case NalUnitType::NAL_UNIT_CODED_SLICE_BLA_N_LP:
                    nal_unit_type_name="NAL_UNIT_CODED_SLICE_BLA_N_LP";break;
                case NalUnitType::NAL_UNIT_CODED_SLICE_IDR_W_RADL:
                    nal_unit_type_name="NAL_UNIT_CODED_SLICE_IDR_W_RADL";break;
                case NalUnitType::NAL_UNIT_CODED_SLICE_IDR_N_LP:
                    nal_unit_type_name="NAL_UNIT_CODED_SLICE_IDR_N_LP";break;
                case NalUnitType::NAL_UNIT_CODED_SLICE_CRA:
                    nal_unit_type_name="NAL_UNIT_CODED_SLICE_CRA";break;
                case NalUnitType::NAL_UNIT_RESERVED_IRAP_VCL22:
                    nal_unit_type_name="NAL_UNIT_RESERVED_IRAP_VCL22";break;
                case NalUnitType:: NAL_UNIT_RESERVED_IRAP_VCL23:
                    nal_unit_type_name="NAL_UNIT_RESERVED_IRAP_VCL23";break;

                case NalUnitType::NAL_UNIT_RESERVED_VCL24:
                    nal_unit_type_name="NAL_UNIT_RESERVED_VCL24";break;
                case NalUnitType::NAL_UNIT_RESERVED_VCL25:
                    nal_unit_type_name="NAL_UNIT_RESERVED_VCL25";break;
                case NalUnitType::NAL_UNIT_RESERVED_VCL26:
                    nal_unit_type_name="NAL_UNIT_RESERVED_VCL26";break;
                case NalUnitType::NAL_UNIT_RESERVED_VCL27:
                    nal_unit_type_name="NAL_UNIT_RESERVED_VCL27";break;
                case NalUnitType::NAL_UNIT_RESERVED_VCL28:
                    nal_unit_type_name="NAL_UNIT_RESERVED_VCL28";break;
                case NalUnitType::NAL_UNIT_RESERVED_VCL29:
                    nal_unit_type_name="NAL_UNIT_RESERVED_VCL29";break;
                case NalUnitType::NAL_UNIT_RESERVED_VCL30:
                    nal_unit_type_name="NAL_UNIT_RESERVED_VCL30";break;
                case NalUnitType::NAL_UNIT_RESERVED_VCL31:
                    nal_unit_type_name="NAL_UNIT_RESERVED_VCL31";break;

                case NalUnitType::NAL_UNIT_VPS:
                    nal_unit_type_name="NAL_UNIT_VPS";break;
                case NalUnitType::NAL_UNIT_SPS:
                    nal_unit_type_name="NAL_UNIT_SPS";break;
                case NalUnitType::NAL_UNIT_PPS:
                    nal_unit_type_name="NAL_UNIT_PPS";break;
                case NalUnitType::NAL_UNIT_ACCESS_UNIT_DELIMITER:
                    nal_unit_type_name="NAL_UNIT_ACCESS_UNIT_DELIMITER";break;
                case NalUnitType::NAL_UNIT_EOS:
                    nal_unit_type_name="NAL_UNIT_EOS";break;
                case NalUnitType::NAL_UNIT_EOB:
                    nal_unit_type_name="NAL_UNIT_EOB";break;
                case NalUnitType:: NAL_UNIT_FILLER_DATA:
                    nal_unit_type_name="NAL_UNIT_FILLER_DATA";break;
                case NalUnitType::NAL_UNIT_PREFIX_SEI:
                    nal_unit_type_name="NAL_UNIT_PREFIX_SEI";break;
                case NalUnitType::NAL_UNIT_SUFFIX_SEI:
                    nal_unit_type_name="NAL_UNIT_SUFFIX_SEI";break;
                case NalUnitType::NAL_UNIT_RESERVED_NVCL41:
                    nal_unit_type_name="NAL_UNIT_RESERVED_NVCL41";break;
                case NalUnitType::NAL_UNIT_RESERVED_NVCL42:
                    nal_unit_type_name="NAL_UNIT_RESERVED_NVCL42";break;
                case NalUnitType::NAL_UNIT_RESERVED_NVCL43:
                    nal_unit_type_name="NAL_UNIT_RESERVED_NVCL43";break;
                case NalUnitType::NAL_UNIT_RESERVED_NVCL44:
                    nal_unit_type_name="NAL_UNIT_RESERVED_NVCL44";break;
                case NalUnitType:: NAL_UNIT_RESERVED_NVCL45:
                    nal_unit_type_name="NAL_UNIT_RESERVED_NVCL45";break;
                case NalUnitType:: NAL_UNIT_RESERVED_NVCL46:
                    nal_unit_type_name="NAL_UNIT_RESERVED_NVCL46";break;
                case NalUnitType::NAL_UNIT_RESERVED_NVCL47:
                    nal_unit_type_name="NAL_UNIT_RESERVED_NVCL47";break;
                case NalUnitType::NAL_UNIT_UNSPECIFIED_48:
                    nal_unit_type_name="NAL_UNIT_UNSPECIFIED_48";break;
                case NalUnitType::NAL_UNIT_UNSPECIFIED_49:
                    nal_unit_type_name="NAL_UNIT_UNSPECIFIED_49";break;
                case NalUnitType::NAL_UNIT_UNSPECIFIED_50:
                    nal_unit_type_name="NAL_UNIT_UNSPECIFIED_50";break;
                case NalUnitType::NAL_UNIT_UNSPECIFIED_51:
                    nal_unit_type_name="NAL_UNIT_UNSPECIFIED_51";break;
                case NalUnitType:: NAL_UNIT_UNSPECIFIED_52:
                    nal_unit_type_name="NAL_UNIT_UNSPECIFIED_52";break;
                case NalUnitType:: NAL_UNIT_UNSPECIFIED_53:
                    nal_unit_type_name="NAL_UNIT_UNSPECIFIED_53";break;
                case NalUnitType::NAL_UNIT_UNSPECIFIED_54:
                    nal_unit_type_name="NAL_UNIT_UNSPECIFIED_54";break;
                case NalUnitType::NAL_UNIT_UNSPECIFIED_55:
                    nal_unit_type_name="NAL_UNIT_UNSPECIFIED_55";break;
                case NalUnitType:: NAL_UNIT_UNSPECIFIED_56:
                    nal_unit_type_name="NAL_UNIT_UNSPECIFIED_56";break;
                case NalUnitType::NAL_UNIT_UNSPECIFIED_57:
                    nal_unit_type_name="NAL_UNIT_UNSPECIFIED_57";break;
                case NalUnitType:: NAL_UNIT_UNSPECIFIED_58:
                    nal_unit_type_name="NAL_UNIT_UNSPECIFIED_58";break;
                case NalUnitType:: NAL_UNIT_UNSPECIFIED_59:
                    nal_unit_type_name="NAL_UNIT_UNSPECIFIED_59";break;
                case NalUnitType::NAL_UNIT_UNSPECIFIED_60:
                    nal_unit_type_name="NAL_UNIT_UNSPECIFIED_60";break;
                case NalUnitType:: NAL_UNIT_UNSPECIFIED_61:
                    nal_unit_type_name="NAL_UNIT_UNSPECIFIED_61";break;
                case NalUnitType::NAL_UNIT_UNSPECIFIED_62:
                    nal_unit_type_name="NAL_UNIT_UNSPECIFIED_62";break;
                case NalUnitType:: NAL_UNIT_UNSPECIFIED_63:
                    nal_unit_type_name="NAL_UNIT_UNSPECIFIED_63";break;
                case NalUnitType:: NAL_UNIT_INVALID:
                    nal_unit_type_name="NAL_UNIT_INVALID";break;
                default :
                    nal_unit_type_name = std::string("Unknown")+std::to_string(nal_unit_type); break;
            }
            return nal_unit_type_name;
        }
    }
}
#endif //LIVEVIDEO10MS_NALUNITTYPE_H
