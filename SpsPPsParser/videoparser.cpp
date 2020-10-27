#include "videoparser.h"
#include <QDebug>
#include <thread>

VideoParser::VideoParser() :
    mFrameCount(0),
    m_video_bsf(NULL),
    m_fmt_ctx(NULL),
    m_video_stream_idx(-1),
    mCurNalutype(0)
{
}

VideoParser::~VideoParser()
{
    for(int i = 0; i < mPackets.size(); i++) {
        delete mPackets.at(i);
    }

    mPackets.clear();
}

VideoParser::PacketItem * VideoParser::readNaluItems()
{
//    std::thread *t = new std::thread(std::bind(parseData, this));

    int ret = 0;
//    while(mFrameCount < MAX_NALU_READ_SIZE) {
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = NULL;
        pkt.size = 0;
        ret = av_read_frame(m_fmt_ctx, &pkt);
        if (ret >= 0) {
            if (pkt.stream_index == m_video_stream_idx) {
                uint8_t *out_data = NULL;
                int out_size = 0;

                int err = av_bitstream_filter_filter(m_video_bsf, m_fmt_ctx->streams[m_video_stream_idx]->codec, NULL, &out_data, &out_size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
                if (err <= 0 ) {
                    qDebug() << "run read, av_bitstream_filter_filter failed\n";
                }


                PacketItem *item = new PacketItem();

                item->pts = pkt.pts;
                item->duration = pkt.duration;
                item->length = out_size;
                memcpy(item->data, (char *)out_data, MAX_DATA_SIZE);
//                parseNaluItem(item);

                mPackets.push_back(item);
                return item;
            }

            av_packet_unref(&pkt);
        } else {
            printf("av read frame failed!\n");
            return NULL;
        }

        mFrameCount++;
//    }

    return NULL;
}

int VideoParser::parseNaluItem(PacketItem *item)
{
//    qDebug() << item->length << endl;
    if (item != NULL) {
        int type = item->data[4] & 0x1F;

        //        switch (type) {
        //        case NALU_SLICE:
        ////            qDebug() << "Slice Nalu" << endl;
        //            break;
        //        case NALU_IDR:
        //            qDebug() << "IDR Nalu" << endl;
        //            break;
        //        case NALU_SEI:
        //            qDebug() << "SEI Nalu" << endl;
        //            break;
        //        case NALU_SPS:
        //            qDebug() << "SPS Nalu" << endl;
        //            break;
        //        case NALU_PPS:
        //            qDebug() << "PPS Nalu" << endl;
        //            break;
        //        default:
        //            qDebug() << "Unknown Nalu" << endl;
        //            break;
        //        }

        return type;
    }
}

bool VideoParser::openFile(char *url)
{   
    /* open input file, and allocate format context */
    if (avformat_open_input(&m_fmt_ctx, (char *)url, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", url);
        return false;
    }

    mFileUrl = url;

    /* retrieve stream information */
    if (avformat_find_stream_info(m_fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        return false;
    }

    /* dump input information to stderr */
    av_dump_format(m_fmt_ctx, 0, url, 0);

    //find video stream index
    int ret = av_find_best_stream(m_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_AUDIO), url);
        return false;
    }

    m_video_stream_idx = ret;
    m_video_bsf = av_bitstream_filter_init("h264_mp4toannexb");

    return true;
}

void VideoParser::closeFile()
{
    av_bitstream_filter_close(m_video_bsf);
    avformat_close_input(&m_fmt_ctx);
}

void VideoParser::parseData(void *arg)
{
    VideoParser *pThis = (VideoParser *)arg;
    pThis->handleParseData();
}

void VideoParser::handleParseData()
{
    int ret = 0;
    while(mFrameCount < MAX_NALU_READ_SIZE) {
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = NULL;
        pkt.size = 0;
        ret = av_read_frame(m_fmt_ctx, &pkt);
        if (ret >= 0) {
            if (pkt.stream_index == m_video_stream_idx) {
                uint8_t *out_data = NULL;
                int out_size = 0;

//                int err = av_bitstream_filter_filter(m_video_bsf, m_fmt_ctx->streams[m_video_stream_idx]->codec, NULL, &out_data, &out_size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
//                if (err <= 0 ) {
//                    qDebug() << "run read, av_bitstream_filter_filter failed\n";
//                }


                PacketItem *item = new PacketItem();

                item->pts = pkt.pts;
                item->duration = pkt.duration;
                item->length = out_size;
//                memcpy(item->data, (char *)out_data, MAX_DATA_SIZE);
                memcpy(item->data, (char *)pkt.data, pkt.size);
                parseNaluItem(item);

                mPackets.push_back(item);
//                return item;
            }

            av_packet_unref(&pkt);
        } else {
            printf("av read frame failed!\n");
            return;
        }

        mFrameCount++;
    }

    return;
}

bs_t* VideoParser::bs_init(bs_t* b, uint8_t* buf, size_t size)
{
    b->start = buf;
    b->p = buf;
    b->end = buf + size;
    b->bits_left = 8;
    return b;
}

sps_t *VideoParser::getSps()
{
    return mSps;
}

VideoParser::PacketItem * VideoParser::getItem(int index)
{
   return mPackets.at(index);
}

bs_t* VideoParser::bs_new(uint8_t* buf, size_t size)
{
    bs_t* b = (bs_t*)malloc(sizeof(bs_t));
    bs_init(b, buf, size);
    return b;
}

int VideoParser::nal_to_rbsp(const int nal_header_size, const uint8_t* nal_buf, int* nal_size, uint8_t* rbsp_buf, int* rbsp_size)
{
    int i;
    int j     = 0;
    int count = 0;

    for( i = nal_header_size; i < *nal_size; i++ )
    {
        // in NAL unit, 0x000000, 0x000001 or 0x000002 shall not occur at any byte-aligned position
//        if( ( count == 2 ) && ( nal_buf[i] < 0x03) )
//        {
//            return -1;
//        }

        if( ( count == 2 ) && ( nal_buf[i] == 0x03) )
        {
            // check the 4th byte after 0x000003, except when cabac_zero_word is used, in which case the last three bytes of this NAL unit must be 0x000003
            if((i < *nal_size - 1) && (nal_buf[i+1] > 0x03))
            {
                return -1;
            }

            // if cabac_zero_word is used, the final byte of this NAL unit(0x03) is discarded, and the last two bytes of RBSP must be 0x0000
            if(i == *nal_size - 1)
            {
                break;
            }

            i++;
            count = 0;
        }

        if ( j >= *rbsp_size )
        {
            // error, not enough space
            return -1;
        }

        rbsp_buf[j] = nal_buf[i];
        if(nal_buf[i] == 0x00)
        {
            count++;
        }
        else
        {
            count = 0;
        }
        j++;
    }

    *nal_size = i;
    *rbsp_size = j;
    return j;
}

int VideoParser::read_nal_unit(uint8_t* buf, int size)
{
    nal_t* nal = (nal_t*)calloc(1, sizeof(nal_t));
    bs_t* b = bs_new(buf, size);

    nal->forbidden_zero_bit = bs_read_f(b,1);
    nal->nal_ref_idc = bs_read_u(b,2);
    nal->nal_unit_type = bs_read_u(b,5);
    qDebug() << "sps nalu type: " << nal->nal_unit_type << endl;
    nal->parsed = NULL;
    nal->sizeof_parsed = 0;

    bs_free(b);

    int nal_size = size;
    int rbsp_size = size;
    uint8_t* rbsp_buf = (uint8_t*)malloc(rbsp_size);

    int rc = nal_to_rbsp(1, buf, &nal_size, rbsp_buf, &rbsp_size);

    if (rc < 0) {
        qDebug() << "exit" << endl;
        free(rbsp_buf);
        return -1;
    } // handle conversion error

    b = bs_new(rbsp_buf, rbsp_size);

    switch ( nal->nal_unit_type )
        {
            case NAL_UNIT_TYPE_CODED_SLICE_IDR:
            case NAL_UNIT_TYPE_CODED_SLICE_NON_IDR:
            case NAL_UNIT_TYPE_CODED_SLICE_AUX:
//                read_slice_layer_rbsp(h, b);
//                nal->parsed = h->sh;
//                nal->sizeof_parsed = sizeof(slice_header_t);
                break;

            case NAL_UNIT_TYPE_SEI:
//                read_sei_rbsp(h, b);
//                nal->parsed = h->sei;
//                nal->sizeof_parsed = sizeof(sei_t);
//                break;

            case NAL_UNIT_TYPE_SPS:
                sps_t * sps = new sps_t();
//                read_seq_parameter_set_rbsp(h, b);
//                nal->parsed = h->sps;
                read_seq_parameter_set_rbsp(b, sps);
                debug_sps(sps);

                nal->parsed = sps;
                nal->sizeof_parsed = sizeof(sps_t);

                mSps = sps;
                break;

//            case NAL_UNIT_TYPE_PPS:
//                read_pic_parameter_set_rbsp(h, b);
//                nal->parsed = h->pps;
//                nal->sizeof_parsed = sizeof(pps_t);
//                break;

//            case NAL_UNIT_TYPE_AUD:
//                read_access_unit_delimiter_rbsp(h, b);
//                nal->parsed = h->aud;
//                nal->sizeof_parsed = sizeof(aud_t);
//                break;

//            case NAL_UNIT_TYPE_END_OF_SEQUENCE:
//                read_end_of_seq_rbsp(h, b);
//                break;

//            case NAL_UNIT_TYPE_END_OF_STREAM:
//                read_end_of_stream_rbsp(h, b);
//                break;
//            default:
//                // here comes the reserved/unspecified/ignored stuff
//                nal->parsed = NULL;
//                nal->sizeof_parsed = 0;
//                return 0;
        }

        mCurNalutype = nal->nal_unit_type;

        if (bs_overrun(b)) { bs_free(b); free(rbsp_buf); return -1; }

        bs_free(b);
        free(rbsp_buf);

        return nal_size;
}

void VideoParser::read_scaling_list(bs_t* b, int* scalingList, int sizeOfScalingList, int useDefaultScalingMatrixFlag)
{
    int j;
    if(scalingList == NULL)
    {
        return;
    }

    int lastScale = 8;
    int nextScale = 8;
    for( j = 0; j < sizeOfScalingList; j++ )
    {
        if( nextScale != 0 )
        {
            int delta_scale = bs_read_se(b);
            nextScale = ( lastScale + delta_scale + 256 ) % 256;
            useDefaultScalingMatrixFlag = ( j == 0 && nextScale == 0 );
        }
        scalingList[ j ] = ( nextScale == 0 ) ? lastScale : nextScale;
        lastScale = scalingList[ j ];
    }
}
void VideoParser::read_hrd_parameters(bs_t* b, sps_t *sps)
{
    int SchedSelIdx;

    sps->hrd.cpb_cnt_minus1 = bs_read_ue(b);
    sps->hrd.bit_rate_scale = bs_read_u(b,4);
    sps->hrd.cpb_size_scale = bs_read_u(b,4);
    for( SchedSelIdx = 0; SchedSelIdx <= sps->hrd.cpb_cnt_minus1; SchedSelIdx++ )
    {
        sps->hrd.bit_rate_value_minus1[ SchedSelIdx ] = bs_read_ue(b);
        sps->hrd.cpb_size_value_minus1[ SchedSelIdx ] = bs_read_ue(b);
        sps->hrd.cbr_flag[ SchedSelIdx ] = bs_read_u1(b);
    }
    sps->hrd.initial_cpb_removal_delay_length_minus1 = bs_read_u(b,5);
    sps->hrd.cpb_removal_delay_length_minus1 = bs_read_u(b,5);
    sps->hrd.dpb_output_delay_length_minus1 = bs_read_u(b,5);
    sps->hrd.time_offset_length = bs_read_u(b,5);
}

void VideoParser::read_vui_parameters(bs_t* b,  sps_t* sps)
{
    sps->vui.aspect_ratio_info_present_flag = bs_read_u1(b);
    if( sps->vui.aspect_ratio_info_present_flag )
    {
        sps->vui.aspect_ratio_idc = bs_read_u8(b);
        if( sps->vui.aspect_ratio_idc == SAR_Extended )
        {
            sps->vui.sar_width = bs_read_u(b,16);
            sps->vui.sar_height = bs_read_u(b,16);
        }
    }
    sps->vui.overscan_info_present_flag = bs_read_u1(b);
    if( sps->vui.overscan_info_present_flag )
    {
        sps->vui.overscan_appropriate_flag = bs_read_u1(b);
    }
    sps->vui.video_signal_type_present_flag = bs_read_u1(b);
    if( sps->vui.video_signal_type_present_flag )
    {
        sps->vui.video_format = bs_read_u(b,3);
        sps->vui.video_full_range_flag = bs_read_u1(b);
        sps->vui.colour_description_present_flag = bs_read_u1(b);
        if( sps->vui.colour_description_present_flag )
        {
            sps->vui.colour_primaries = bs_read_u8(b);
            sps->vui.transfer_characteristics = bs_read_u8(b);
            sps->vui.matrix_coefficients = bs_read_u8(b);
        }
    }
    sps->vui.chroma_loc_info_present_flag = bs_read_u1(b);
    if( sps->vui.chroma_loc_info_present_flag )
    {
        sps->vui.chroma_sample_loc_type_top_field = bs_read_ue(b);
        sps->vui.chroma_sample_loc_type_bottom_field = bs_read_ue(b);
    }
    sps->vui.timing_info_present_flag = bs_read_u1(b);
    if( sps->vui.timing_info_present_flag )
    {
        sps->vui.num_units_in_tick = bs_read_u(b,32);
        sps->vui.time_scale = bs_read_u(b,32);
        sps->vui.fixed_frame_rate_flag = bs_read_u1(b);
    }
    sps->vui.nal_hrd_parameters_present_flag = bs_read_u1(b);
    if( sps->vui.nal_hrd_parameters_present_flag )
    {
        read_hrd_parameters(b, sps);
    }
    sps->vui.vcl_hrd_parameters_present_flag = bs_read_u1(b);
    if( sps->vui.vcl_hrd_parameters_present_flag )
    {
        read_hrd_parameters(b, sps);
    }
    if( sps->vui.nal_hrd_parameters_present_flag || sps->vui.vcl_hrd_parameters_present_flag )
    {
        sps->vui.low_delay_hrd_flag = bs_read_u1(b);
    }
    sps->vui.pic_struct_present_flag = bs_read_u1(b);
    sps->vui.bitstream_restriction_flag = bs_read_u1(b);
    if( sps->vui.bitstream_restriction_flag )
    {
        sps->vui.motion_vectors_over_pic_boundaries_flag = bs_read_u1(b);
        sps->vui.max_bytes_per_pic_denom = bs_read_ue(b);
        sps->vui.max_bits_per_mb_denom = bs_read_ue(b);
        sps->vui.log2_max_mv_length_horizontal = bs_read_ue(b);
        sps->vui.log2_max_mv_length_vertical = bs_read_ue(b);
        sps->vui.num_reorder_frames = bs_read_ue(b);
        sps->vui.max_dec_frame_buffering = bs_read_ue(b);
    }
}

void VideoParser::read_rbsp_trailing_bits(bs_t* b)
{
    int rbsp_stop_one_bit = bs_read_u1( b ); // equal to 1

    while( !bs_byte_aligned(b) )
    {
        int rbsp_alignment_zero_bit = bs_read_u1( b ); // equal to 0
    }
}

void VideoParser::read_seq_parameter_set_rbsp(bs_t* b, sps_t *sps)
{
    int i;

    // NOTE can't read directly into sps because seq_parameter_set_id not yet known and so sps is not selected

    int profile_idc = bs_read_u8(b);
    int constraint_set0_flag = bs_read_u1(b);
    int constraint_set1_flag = bs_read_u1(b);
    int constraint_set2_flag = bs_read_u1(b);
    int constraint_set3_flag = bs_read_u1(b);
    int constraint_set4_flag = bs_read_u1(b);
    int constraint_set5_flag = bs_read_u1(b);
    int reserved_zero_2bits  = bs_read_u(b,2);  /* all 0's */
    int level_idc = bs_read_u8(b);
    int seq_parameter_set_id = bs_read_ue(b);

    // select the correct sps
//    h->sps = h->sps_table[seq_parameter_set_id];
//    sps_t* sps = h->sps;
    memset(sps, 0, sizeof(sps_t));

    sps->chroma_format_idc = 1;

    sps->profile_idc = profile_idc; // bs_read_u8(b);
    sps->constraint_set0_flag = constraint_set0_flag;//bs_read_u1(b);
    sps->constraint_set1_flag = constraint_set1_flag;//bs_read_u1(b);
    sps->constraint_set2_flag = constraint_set2_flag;//bs_read_u1(b);
    sps->constraint_set3_flag = constraint_set3_flag;//bs_read_u1(b);
    sps->constraint_set4_flag = constraint_set4_flag;//bs_read_u1(b);
    sps->constraint_set5_flag = constraint_set5_flag;//bs_read_u1(b);
    sps->reserved_zero_2bits = reserved_zero_2bits;//bs_read_u(b,2);
    sps->level_idc = level_idc; //bs_read_u8(b);
    sps->seq_parameter_set_id = seq_parameter_set_id; // bs_read_ue(b);
    if( sps->profile_idc == 100 || sps->profile_idc == 110 ||
        sps->profile_idc == 122 || sps->profile_idc == 144 )
    {
        sps->chroma_format_idc = bs_read_ue(b);
        sps->ChromaArrayType = sps->chroma_format_idc;
        if( sps->chroma_format_idc == 3 )
        {
            sps->separate_colour_plane_flag = bs_read_u1(b);
            if (sps->separate_colour_plane_flag) sps->ChromaArrayType = 0;
        }
        sps->bit_depth_luma_minus8 = bs_read_ue(b);
        sps->bit_depth_chroma_minus8 = bs_read_ue(b);
        sps->qpprime_y_zero_transform_bypass_flag = bs_read_u1(b);
        sps->seq_scaling_matrix_present_flag = bs_read_u1(b);
        if( sps->seq_scaling_matrix_present_flag )
        {
            for( i = 0; i < ((sps->chroma_format_idc!=3) ? 8 : 12); i++ )
            {
                sps->seq_scaling_list_present_flag[ i ] = bs_read_u1(b);
                if( sps->seq_scaling_list_present_flag[ i ] )
                {
                    if( i < 6 )
                    {
                        read_scaling_list( b, &sps->ScalingList4x4[ i ], 16,
                                      sps->UseDefaultScalingMatrix4x4Flag[ i ]);
                    }
                    else
                    {
                        read_scaling_list( b, &sps->ScalingList8x8[ i - 6 ], 64,
                                      sps->UseDefaultScalingMatrix8x8Flag[ i - 6 ] );
                    }
                }
            }
        }
    }
    sps->log2_max_frame_num_minus4 = bs_read_ue(b);
    sps->pic_order_cnt_type = bs_read_ue(b);
    if( sps->pic_order_cnt_type == 0 )
    {
        sps->log2_max_pic_order_cnt_lsb_minus4 = bs_read_ue(b);
    }
    else if( sps->pic_order_cnt_type == 1 )
    {
        sps->delta_pic_order_always_zero_flag = bs_read_u1(b);
        sps->offset_for_non_ref_pic = bs_read_se(b);
        sps->offset_for_top_to_bottom_field = bs_read_se(b);
        sps->num_ref_frames_in_pic_order_cnt_cycle = bs_read_ue(b);
        for( i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++ )
        {
            sps->offset_for_ref_frame[ i ] = bs_read_se(b);
        }
    }
    sps->max_num_ref_frames = bs_read_ue(b);
    sps->gaps_in_frame_num_value_allowed_flag = bs_read_u1(b);
    sps->pic_width_in_mbs_minus1 = bs_read_ue(b);
    sps->pic_height_in_map_units_minus1 = bs_read_ue(b);
    sps->frame_mbs_only_flag = bs_read_u1(b);
    if( !sps->frame_mbs_only_flag )
    {
        sps->mb_adaptive_frame_field_flag = bs_read_u1(b);
    }
    sps->direct_8x8_inference_flag = bs_read_u1(b);
    sps->frame_cropping_flag = bs_read_u1(b);
    if( sps->frame_cropping_flag )
    {
        sps->frame_crop_left_offset = bs_read_ue(b);
        sps->frame_crop_right_offset = bs_read_ue(b);
        sps->frame_crop_top_offset = bs_read_ue(b);
        sps->frame_crop_bottom_offset = bs_read_ue(b);
    }
    sps->vui_parameters_present_flag = bs_read_u1(b);
    if( sps->vui_parameters_present_flag )
    {
        read_vui_parameters(b, sps);
//        if (sps->vui.num_units_in_tick != 0)
//            h->info->max_framerate = (float)(sps->vui.time_scale) / (float)(sps->vui.num_units_in_tick);
    }
    read_rbsp_trailing_bits(b);

    // add by Late Lee
//    h->info->crop_left = sps->frame_crop_left_offset;
//    h->info->crop_right = sps->frame_crop_right_offset;
//    h->info->crop_top = sps->frame_crop_top_offset;
//    h->info->crop_bottom = sps->frame_crop_bottom_offset;

#if 01
//    h->info->width  = (sps->pic_width_in_mbs_minus1+1) * 16;
//    h->info->height = (2 - sps->frame_mbs_only_flag)* (sps->pic_height_in_map_units_minus1 +1) * 16;

    if(sps->frame_cropping_flag)
    {
        unsigned int crop_unit_x;
        unsigned int crop_unit_y;
        if (0 == sps->chroma_format_idc) // monochrome
        {
            crop_unit_x = 1;
            crop_unit_y = 2 - sps->frame_mbs_only_flag;
        }
        else if (1 == sps->chroma_format_idc) // 4:2:0
        {
            crop_unit_x = 2;
            crop_unit_y = 2 * (2 - sps->frame_mbs_only_flag);
        }
        else if (2 == sps->chroma_format_idc) // 4:2:2
        {
            crop_unit_x = 2;
            crop_unit_y = 2 - sps->frame_mbs_only_flag;
        }
        else // 3 == sps.chroma_format_idc   // 4:4:4
        {
            crop_unit_x = 1;
            crop_unit_y = 2 - sps->frame_mbs_only_flag;
        }

//        h->info->width  -= crop_unit_x * (sps->frame_crop_left_offset + sps->frame_crop_right_offset);
//        h->info->height -= crop_unit_y * (sps->frame_crop_top_offset  + sps->frame_crop_bottom_offset);
    }
#else
    int sub_width_c  = ((1 == sps->chroma_format_idc) || (2 == sps->chroma_format_idc)) && (0 == sps->separate_colour_plane_flag) ? 2 : 1;
    int sub_height_c =  (1 == sps->chroma_format_idc)                                   && (0 == sps->separate_colour_plane_flag) ? 2 : 1;

//    h->info->width = ((sps->pic_width_in_mbs_minus1 +1)*16) - sps->frame_crop_left_offset*sub_width_c - sps->frame_crop_right_offset*sub_width_c;
//    h->info->height= ((2 - sps->frame_mbs_only_flag)* (sps->pic_height_in_map_units_minus1 +1) * 16) - (sps->frame_crop_top_offset * sub_height_c) - (sps->frame_crop_bottom_offset * sub_height_c);
#endif
//    h->info->bit_depth_luma   = sps->bit_depth_luma_minus8 + 8;
//    h->info->bit_depth_chroma = sps->bit_depth_chroma_minus8 + 8;

//    h->info->profile_idc = sps->profile_idc;
//    h->info->level_idc = sps->level_idc;

//    h->info->chroma_format_idc = sps->chroma_format_idc;
}

void VideoParser::debug_sps(sps_t* sps)
{
    printf("======= SPS =======\n");
    printf(" profile_idc : %d \n", sps->profile_idc );
    printf(" constraint_set0_flag : %d \n", sps->constraint_set0_flag );
    printf(" constraint_set1_flag : %d \n", sps->constraint_set1_flag );
    printf(" constraint_set2_flag : %d \n", sps->constraint_set2_flag );
    printf(" constraint_set3_flag : %d \n", sps->constraint_set3_flag );
    printf(" constraint_set4_flag : %d \n", sps->constraint_set4_flag );
    printf(" constraint_set5_flag : %d \n", sps->constraint_set5_flag );
    printf(" reserved_zero_2bits : %d \n", sps->reserved_zero_2bits );
    printf(" level_idc : %d \n", sps->level_idc );
    printf(" seq_parameter_set_id : %d \n", sps->seq_parameter_set_id );
    printf(" chroma_format_idc : %d \n", sps->chroma_format_idc );
    printf(" separate_colour_plane_flag : %d \n", sps->separate_colour_plane_flag );
    printf(" bit_depth_luma_minus8 : %d \n", sps->bit_depth_luma_minus8 );
    printf(" bit_depth_chroma_minus8 : %d \n", sps->bit_depth_chroma_minus8 );
    printf(" qpprime_y_zero_transform_bypass_flag : %d \n", sps->qpprime_y_zero_transform_bypass_flag );
    printf(" seq_scaling_matrix_present_flag : %d \n", sps->seq_scaling_matrix_present_flag );
    //  int seq_scaling_list_present_flag[8];
    //  void* ScalingList4x4[6];
    //  int UseDefaultScalingMatrix4x4Flag[6];
    //  void* ScalingList8x8[2];
    //  int UseDefaultScalingMatrix8x8Flag[2];
    printf(" log2_max_frame_num_minus4 : %d \n", sps->log2_max_frame_num_minus4 );
    printf(" pic_order_cnt_type : %d \n", sps->pic_order_cnt_type );
      printf("   log2_max_pic_order_cnt_lsb_minus4 : %d \n", sps->log2_max_pic_order_cnt_lsb_minus4 );
      printf("   delta_pic_order_always_zero_flag : %d \n", sps->delta_pic_order_always_zero_flag );
      printf("   offset_for_non_ref_pic : %d \n", sps->offset_for_non_ref_pic );
      printf("   offset_for_top_to_bottom_field : %d \n", sps->offset_for_top_to_bottom_field );
      printf("   num_ref_frames_in_pic_order_cnt_cycle : %d \n", sps->num_ref_frames_in_pic_order_cnt_cycle );
    //  int offset_for_ref_frame[256];
    printf(" max_num_ref_frames : %d \n", sps->max_num_ref_frames );
    printf(" gaps_in_frame_num_value_allowed_flag : %d \n", sps->gaps_in_frame_num_value_allowed_flag );
    printf(" pic_width_in_mbs_minus1 : %d \n", sps->pic_width_in_mbs_minus1 );
    printf(" pic_height_in_map_units_minus1 : %d \n", sps->pic_height_in_map_units_minus1 );
    printf(" frame_mbs_only_flag : %d \n", sps->frame_mbs_only_flag );
    printf(" mb_adaptive_frame_field_flag : %d \n", sps->mb_adaptive_frame_field_flag );
    printf(" direct_8x8_inference_flag : %d \n", sps->direct_8x8_inference_flag );
    printf(" frame_cropping_flag : %d \n", sps->frame_cropping_flag );
      printf("   frame_crop_left_offset : %d \n", sps->frame_crop_left_offset );
      printf("   frame_crop_right_offset : %d \n", sps->frame_crop_right_offset );
      printf("   frame_crop_top_offset : %d \n", sps->frame_crop_top_offset );
      printf("   frame_crop_bottom_offset : %d \n", sps->frame_crop_bottom_offset );
    printf(" vui_parameters_present_flag : %d \n", sps->vui_parameters_present_flag );

    printf("=== VUI ===\n");
    printf(" aspect_ratio_info_present_flag : %d \n", sps->vui.aspect_ratio_info_present_flag );
      printf("   aspect_ratio_idc : %d \n", sps->vui.aspect_ratio_idc );
        printf("     sar_width : %d \n", sps->vui.sar_width );
        printf("     sar_height : %d \n", sps->vui.sar_height );
    printf(" overscan_info_present_flag : %d \n", sps->vui.overscan_info_present_flag );
      printf("   overscan_appropriate_flag : %d \n", sps->vui.overscan_appropriate_flag );
    printf(" video_signal_type_present_flag : %d \n", sps->vui.video_signal_type_present_flag );
      printf("   video_format : %d \n", sps->vui.video_format );
      printf("   video_full_range_flag : %d \n", sps->vui.video_full_range_flag );
      printf("   colour_description_present_flag : %d \n", sps->vui.colour_description_present_flag );
        printf("     colour_primaries : %d \n", sps->vui.colour_primaries );
        printf("   transfer_characteristics : %d \n", sps->vui.transfer_characteristics );
        printf("   matrix_coefficients : %d \n", sps->vui.matrix_coefficients );
    printf(" chroma_loc_info_present_flag : %d \n", sps->vui.chroma_loc_info_present_flag );
      printf("   chroma_sample_loc_type_top_field : %d \n", sps->vui.chroma_sample_loc_type_top_field );
      printf("   chroma_sample_loc_type_bottom_field : %d \n", sps->vui.chroma_sample_loc_type_bottom_field );
    printf(" timing_info_present_flag : %d \n", sps->vui.timing_info_present_flag );
      printf("   num_units_in_tick : %d \n", sps->vui.num_units_in_tick );
      printf("   time_scale : %d \n", sps->vui.time_scale );
      printf("   fixed_frame_rate_flag : %d \n", sps->vui.fixed_frame_rate_flag );
    printf(" nal_hrd_parameters_present_flag : %d \n", sps->vui.nal_hrd_parameters_present_flag );
    printf(" vcl_hrd_parameters_present_flag : %d \n", sps->vui.vcl_hrd_parameters_present_flag );
      printf("   low_delay_hrd_flag : %d \n", sps->vui.low_delay_hrd_flag );
    printf(" pic_struct_present_flag : %d \n", sps->vui.pic_struct_present_flag );
    printf(" bitstream_restriction_flag : %d \n", sps->vui.bitstream_restriction_flag );
      printf("   motion_vectors_over_pic_boundaries_flag : %d \n", sps->vui.motion_vectors_over_pic_boundaries_flag );
      printf("   max_bytes_per_pic_denom : %d \n", sps->vui.max_bytes_per_pic_denom );
      printf("   max_bits_per_mb_denom : %d \n", sps->vui.max_bits_per_mb_denom );
      printf("   log2_max_mv_length_horizontal : %d \n", sps->vui.log2_max_mv_length_horizontal );
      printf("   log2_max_mv_length_vertical : %d \n", sps->vui.log2_max_mv_length_vertical );
      printf("   num_reorder_frames : %d \n", sps->vui.num_reorder_frames );
      printf("   max_dec_frame_buffering : %d \n", sps->vui.max_dec_frame_buffering );

    printf("=== HRD ===\n");
    printf(" cpb_cnt_minus1 : %d \n", sps->hrd.cpb_cnt_minus1 );
    printf(" bit_rate_scale : %d \n", sps->hrd.bit_rate_scale );
    printf(" cpb_size_scale : %d \n", sps->hrd.cpb_size_scale );
    int SchedSelIdx;
    for( SchedSelIdx = 0; SchedSelIdx <= sps->hrd.cpb_cnt_minus1; SchedSelIdx++ )
    {
        printf("   bit_rate_value_minus1[%d] : %d \n", SchedSelIdx, sps->hrd.bit_rate_value_minus1[SchedSelIdx] ); // up to cpb_cnt_minus1, which is <= 31
        printf("   cpb_size_value_minus1[%d] : %d \n", SchedSelIdx, sps->hrd.cpb_size_value_minus1[SchedSelIdx] );
        printf("   cbr_flag[%d] : %d \n", SchedSelIdx, sps->hrd.cbr_flag[SchedSelIdx] );
    }
    printf(" initial_cpb_removal_delay_length_minus1 : %d \n", sps->hrd.initial_cpb_removal_delay_length_minus1 );
    printf(" cpb_removal_delay_length_minus1 : %d \n", sps->hrd.cpb_removal_delay_length_minus1 );
    printf(" dpb_output_delay_length_minus1 : %d \n", sps->hrd.dpb_output_delay_length_minus1 );
    printf(" time_offset_length : %d \n", sps->hrd.time_offset_length );
}
