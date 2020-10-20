#ifndef VIDEOPARSER_H
#define VIDEOPARSER_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <vector>

extern "C" {

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

}

#include "bs.h"

#define NALU_SLICE  1
#define NALU_IDR  5
#define NALU_SEI  6
#define NALU_SPS  7
#define NALU_PPS  8

#define MAX_DATA_SIZE 100

#define MAX_NALU_READ_SIZE 10

#define SAR_Extended      255        // Extended_SAR

//Table 7-1 NAL unit type codes
#define NAL_UNIT_TYPE_UNSPECIFIED                    0    // Unspecified
#define NAL_UNIT_TYPE_CODED_SLICE_NON_IDR            1    // Coded slice of a non-IDR picture
#define NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_A   2    // Coded slice data partition A
#define NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_B   3    // Coded slice data partition B
#define NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_C   4    // Coded slice data partition C
#define NAL_UNIT_TYPE_CODED_SLICE_IDR                5    // Coded slice of an IDR picture
#define NAL_UNIT_TYPE_SEI                            6    // Supplemental enhancement information (SEI)
#define NAL_UNIT_TYPE_SPS                            7    // Sequence parameter set
#define NAL_UNIT_TYPE_PPS                            8    // Picture parameter set
#define NAL_UNIT_TYPE_AUD                            9    // Access unit delimiter
#define NAL_UNIT_TYPE_END_OF_SEQUENCE               10    // End of sequence
#define NAL_UNIT_TYPE_END_OF_STREAM                 11    // End of stream
#define NAL_UNIT_TYPE_FILLER                        12    // Filler data
#define NAL_UNIT_TYPE_SPS_EXT                       13    // Sequence parameter set extension
                                             // 14..18    // Reserved
#define NAL_UNIT_TYPE_CODED_SLICE_AUX               19    // Coded slice of an auxiliary coded picture without partitioning
                                             // 20..23    // Reserved
                                             // 24..31    // Unspecified

typedef struct
{
    int profile_idc;
    int constraint_set0_flag;
    int constraint_set1_flag;
    int constraint_set2_flag;
    int constraint_set3_flag;
    int constraint_set4_flag;
    int constraint_set5_flag;
    int reserved_zero_2bits;
    int level_idc;
    int seq_parameter_set_id;
    int chroma_format_idc;
    int separate_colour_plane_flag;
    int ChromaArrayType;
    int bit_depth_luma_minus8;
    int bit_depth_chroma_minus8;
    int qpprime_y_zero_transform_bypass_flag;
    int seq_scaling_matrix_present_flag;
      int seq_scaling_list_present_flag[8];
      int ScalingList4x4[6];
      int UseDefaultScalingMatrix4x4Flag[6];
      int ScalingList8x8[2];
      int UseDefaultScalingMatrix8x8Flag[2];
    int log2_max_frame_num_minus4;
    int pic_order_cnt_type;
      int log2_max_pic_order_cnt_lsb_minus4;
      int delta_pic_order_always_zero_flag;
      int offset_for_non_ref_pic;
      int offset_for_top_to_bottom_field;
      int num_ref_frames_in_pic_order_cnt_cycle;
      int offset_for_ref_frame[256];
    int max_num_ref_frames;
    int gaps_in_frame_num_value_allowed_flag;
    int pic_width_in_mbs_minus1;
    int pic_height_in_map_units_minus1;
    int frame_mbs_only_flag;
    int mb_adaptive_frame_field_flag;
    int direct_8x8_inference_flag;
    int frame_cropping_flag;
      int frame_crop_left_offset;
      int frame_crop_right_offset;
      int frame_crop_top_offset;
      int frame_crop_bottom_offset;
    int vui_parameters_present_flag;

    struct
    {
        int aspect_ratio_info_present_flag;
          int aspect_ratio_idc;
            int sar_width;
            int sar_height;
        int overscan_info_present_flag;
          int overscan_appropriate_flag;
        int video_signal_type_present_flag;
          int video_format;
          int video_full_range_flag;
          int colour_description_present_flag;
            int colour_primaries;
            int transfer_characteristics;
            int matrix_coefficients;
        int chroma_loc_info_present_flag;
          int chroma_sample_loc_type_top_field;
          int chroma_sample_loc_type_bottom_field;
        int timing_info_present_flag;
          int num_units_in_tick;
          int time_scale;
          int fixed_frame_rate_flag;
        int nal_hrd_parameters_present_flag;
        int vcl_hrd_parameters_present_flag;
          int low_delay_hrd_flag;
        int pic_struct_present_flag;
        int bitstream_restriction_flag;
          int motion_vectors_over_pic_boundaries_flag;
          int max_bytes_per_pic_denom;
          int max_bits_per_mb_denom;
          int log2_max_mv_length_horizontal;
          int log2_max_mv_length_vertical;
          int num_reorder_frames;
          int max_dec_frame_buffering;
    } vui;

    struct
    {
        int cpb_cnt_minus1;
        int bit_rate_scale;
        int cpb_size_scale;
          int bit_rate_value_minus1[32]; // up to cpb_cnt_minus1, which is <= 31
          int cpb_size_value_minus1[32];
          int cbr_flag[32];
        int initial_cpb_removal_delay_length_minus1;
        int cpb_removal_delay_length_minus1;
        int dpb_output_delay_length_minus1;
        int time_offset_length;
    } hrd;

} sps_t;

class VideoParser
{
public:
    VideoParser();
    ~VideoParser();

    struct PacketItem {
        int length;
        int64_t pts;
        int64_t duration;
        char *data;

        PacketItem()
        {
            length = -1;
            pts = 0;
            duration = 0;
            data = (char *)malloc(50000);
            memset(data, 0, 50000);
//            data = (char *)malloc(100);
//            memset(data, 0, 100);
        }

        ~PacketItem() {
            if (data != NULL) {
                free(data);
                data = NULL;
            }
        }

        void copy(PacketItem *other)
        {
            length = other->length;
            pts = other->pts;
            duration = other->duration;
            memcpy(data, other->data, other->length);
        }
    };

    PacketItem * readNaluItems();

    int parseNaluItem(PacketItem *item);


    bool openFile(char *url);
    void closeFile();

    static void parseData(void *arg);
    void handleParseData();

//private:
public:

    typedef struct
    {
        int forbidden_zero_bit;
        int nal_ref_idc;
        int nal_unit_type;
        void* parsed; // FIXME
        int sizeof_parsed;

        //uint8_t* rbsp_buf;
        //int rbsp_size;
    } nal_t;

    int nal_to_rbsp(const int nal_header_size, const uint8_t* nal_buf, int* nal_size, uint8_t* rbsp_buf, int* rbsp_size);

    int read_nal_unit(uint8_t* buf, int size);

    bs_t* bs_new(uint8_t* buf, size_t size);

    bs_t* bs_init(bs_t* b, uint8_t* buf, size_t size);

private:
    std::vector<PacketItem *> mPackets;
    int mFrameCount;

    AVPacket pkt;
    AVFormatContext *m_fmt_ctx;
    int m_video_stream_idx;
    AVBitStreamFilterContext *m_video_bsf;

    void read_scaling_list(bs_t* b, int* scalingList, int sizeOfScalingList, int useDefaultScalingMatrixFlag );
    void read_hrd_parameters(bs_t* b, sps_t *sps);
    void read_vui_parameters(bs_t* b, sps_t *sps);
    void read_seq_parameter_set_rbsp(bs_t* b, sps_t *sps);

    void read_rbsp_trailing_bits(bs_t* b);

    void debug_sps(sps_t* sps);

};

#endif // VIDEOPARSER_H
