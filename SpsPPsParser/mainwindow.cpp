#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <string>

#define COLUMENUM 2

#define COLUME_INDEX 0
#define COLUME_TYPE 1

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mParser = new VideoParser();
    initTable();

    setTreeItem();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    mParser->openFile("test.mp4");

    int count = 0;
    while(count < MAX_NALU_READ_SIZE) {
        VideoParser::PacketItem *item = mParser->readNaluItems();
        if (item != NULL) {
           int type = mParser->parseNaluItem(item);
           switch (type) {
           case NALU_SLICE:
    //            qDebug() << "Slice Nalu" << endl;
               setTableItem(count, "slice");
               break;
           case NALU_IDR:
               qDebug() << "IDR Nalu" << endl;
               setTableItem(count, "idr");
               break;
           case NALU_SEI:
               qDebug() << "SEI Nalu" << endl;
               setTableItem(count, "sei");
               break;
           case NALU_SPS:
               qDebug() << "SPS Nalu" << endl;
               setTableItem(count, "sps");

               mParser->read_nal_unit((unsigned char *)(item->data + 4), item->length - 4);
//               sps_t *sps1 = mParser->getSps();
               setSpsTreeItem(mParser->getSps());
               break;
           case NALU_PPS:
//               qDebug() << "PPS Nalu" << endl;
               setTableItem(count, "pps");
               break;
           default:
               qDebug() << "Unknown Nalu" << endl;
               setTableItem(count, "unknown");
               break;
           }

           count ++;
        }
    }

    mParser->closeFile();
}


void MainWindow::initTable()
{
    QStringList headers;
    headers << "Index" << "Nalu Type";

    ui->mTable->setColumnCount(headers.count());
    ui->mTable->setRowCount(MAX_NALU_READ_SIZE);
    ui->mTable->setHorizontalHeaderLabels(headers);
    ui->mTable->verticalHeader()->setHidden(true);
}

void MainWindow::setTableItem(int index, char *type)
{
//    qDebug() << index << type << endl;
    QTableWidgetItem *item;

    for (int j = 0; j < COLUMENUM; j++) {
        if (j == COLUME_INDEX) {
            item = new QTableWidgetItem(std::to_string(index).c_str());
            item->setBackgroundColor(Qt::red);
            ui->mTable->setItem(index, j, item);
        } else if (j == COLUME_TYPE) {
            item = new QTableWidgetItem(type);
            item->setBackgroundColor(Qt::green);
            ui->mTable->setItem(index, j, item);
        }
    }
}

void MainWindow::setTreeItem()
{
    ui->mTreeWidget->setColumnCount(1);
    ui->mTreeWidget->setHeaderLabel("data");
    ui->mTreeWidget->setHeaderHidden(true);

    QTreeWidgetItem *item = new QTreeWidgetItem();
    QString text;
    text.sprintf("width: %d", 10);
    text.sprintf("%s %s", text.toLatin1().data(), "gop");
    item->setText(0, text);

    QTreeWidgetItem *itemChild1 = new QTreeWidgetItem();
    itemChild1->setText(0, "height");

    QTreeWidgetItem *itemChild2 = new QTreeWidgetItem();
    itemChild2->setText(0, "height");

    QTreeWidgetItem *itemChild3 = new QTreeWidgetItem();
    itemChild3->setText(0, "height");

    item->addChild(itemChild1);
    item->addChild(itemChild2);
    item->addChild(itemChild3);

    ui->mTreeWidget->addTopLevelItem(item);
}

QTreeWidgetItem * MainWindow::addItem(QTreeWidgetItem *src, QString dst)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, dst);
    src->addChild(item);
    return src;
}

//printf("======= SPS =======\n");
//printf(" profile_idc : %d \n", sps->profile_idc );
//printf(" constraint_set0_flag : %d \n", sps->constraint_set0_flag );
//printf(" constraint_set1_flag : %d \n", sps->constraint_set1_flag );
//printf(" constraint_set2_flag : %d \n", sps->constraint_set2_flag );
//printf(" constraint_set3_flag : %d \n", sps->constraint_set3_flag );
//printf(" constraint_set4_flag : %d \n", sps->constraint_set4_flag );
//printf(" constraint_set5_flag : %d \n", sps->constraint_set5_flag );
//printf(" reserved_zero_2bits : %d \n", sps->reserved_zero_2bits );
//printf(" level_idc : %d \n", sps->level_idc );
//printf(" seq_parameter_set_id : %d \n", sps->seq_parameter_set_id );
//printf(" chroma_format_idc : %d \n", sps->chroma_format_idc );
//printf(" separate_colour_plane_flag : %d \n", sps->separate_colour_plane_flag );
//printf(" bit_depth_luma_minus8 : %d \n", sps->bit_depth_luma_minus8 );
//printf(" bit_depth_chroma_minus8 : %d \n", sps->bit_depth_chroma_minus8 );
//printf(" qpprime_y_zero_transform_bypass_flag : %d \n", sps->qpprime_y_zero_transform_bypass_flag );
//printf(" seq_scaling_matrix_present_flag : %d \n", sps->seq_scaling_matrix_present_flag );
////  int seq_scaling_list_present_flag[8];
////  void* ScalingList4x4[6];
////  int UseDefaultScalingMatrix4x4Flag[6];
////  void* ScalingList8x8[2];
////  int UseDefaultScalingMatrix8x8Flag[2];
//printf(" log2_max_frame_num_minus4 : %d \n", sps->log2_max_frame_num_minus4 );
//printf(" pic_order_cnt_type : %d \n", sps->pic_order_cnt_type );
//  printf("   log2_max_pic_order_cnt_lsb_minus4 : %d \n", sps->log2_max_pic_order_cnt_lsb_minus4 );
//  printf("   delta_pic_order_always_zero_flag : %d \n", sps->delta_pic_order_always_zero_flag );
//  printf("   offset_for_non_ref_pic : %d \n", sps->offset_for_non_ref_pic );
//  printf("   offset_for_top_to_bottom_field : %d \n", sps->offset_for_top_to_bottom_field );
//  printf("   num_ref_frames_in_pic_order_cnt_cycle : %d \n", sps->num_ref_frames_in_pic_order_cnt_cycle );
////  int offset_for_ref_frame[256];
//printf(" max_num_ref_frames : %d \n", sps->max_num_ref_frames );
//printf(" gaps_in_frame_num_value_allowed_flag : %d \n", sps->gaps_in_frame_num_value_allowed_flag );
//printf(" pic_width_in_mbs_minus1 : %d \n", sps->pic_width_in_mbs_minus1 );
//printf(" pic_height_in_map_units_minus1 : %d \n", sps->pic_height_in_map_units_minus1 );
//printf(" frame_mbs_only_flag : %d \n", sps->frame_mbs_only_flag );
//printf(" mb_adaptive_frame_field_flag : %d \n", sps->mb_adaptive_frame_field_flag );
//printf(" direct_8x8_inference_flag : %d \n", sps->direct_8x8_inference_flag );
//printf(" frame_cropping_flag : %d \n", sps->frame_cropping_flag );
//  printf("   frame_crop_left_offset : %d \n", sps->frame_crop_left_offset );
//  printf("   frame_crop_right_offset : %d \n", sps->frame_crop_right_offset );
//  printf("   frame_crop_top_offset : %d \n", sps->frame_crop_top_offset );
//  printf("   frame_crop_bottom_offset : %d \n", sps->frame_crop_bottom_offset );
//printf(" vui_parameters_present_flag : %d \n", sps->vui_parameters_present_flag );

//printf("=== VUI ===\n");
//printf(" aspect_ratio_info_present_flag : %d \n", sps->vui.aspect_ratio_info_present_flag );
//  printf("   aspect_ratio_idc : %d \n", sps->vui.aspect_ratio_idc );
//    printf("     sar_width : %d \n", sps->vui.sar_width );
//    printf("     sar_height : %d \n", sps->vui.sar_height );
//printf(" overscan_info_present_flag : %d \n", sps->vui.overscan_info_present_flag );
//  printf("   overscan_appropriate_flag : %d \n", sps->vui.overscan_appropriate_flag );
//printf(" video_signal_type_present_flag : %d \n", sps->vui.video_signal_type_present_flag );
//  printf("   video_format : %d \n", sps->vui.video_format );
//  printf("   video_full_range_flag : %d \n", sps->vui.video_full_range_flag );
//  printf("   colour_description_present_flag : %d \n", sps->vui.colour_description_present_flag );
//    printf("     colour_primaries : %d \n", sps->vui.colour_primaries );
//    printf("   transfer_characteristics : %d \n", sps->vui.transfer_characteristics );
//    printf("   matrix_coefficients : %d \n", sps->vui.matrix_coefficients );
//printf(" chroma_loc_info_present_flag : %d \n", sps->vui.chroma_loc_info_present_flag );
//  printf("   chroma_sample_loc_type_top_field : %d \n", sps->vui.chroma_sample_loc_type_top_field );
//  printf("   chroma_sample_loc_type_bottom_field : %d \n", sps->vui.chroma_sample_loc_type_bottom_field );
//printf(" timing_info_present_flag : %d \n", sps->vui.timing_info_present_flag );
//  printf("   num_units_in_tick : %d \n", sps->vui.num_units_in_tick );
//  printf("   time_scale : %d \n", sps->vui.time_scale );
//  printf("   fixed_frame_rate_flag : %d \n", sps->vui.fixed_frame_rate_flag );
//printf(" nal_hrd_parameters_present_flag : %d \n", sps->vui.nal_hrd_parameters_present_flag );
//printf(" vcl_hrd_parameters_present_flag : %d \n", sps->vui.vcl_hrd_parameters_present_flag );
//  printf("   low_delay_hrd_flag : %d \n", sps->vui.low_delay_hrd_flag );
//printf(" pic_struct_present_flag : %d \n", sps->vui.pic_struct_present_flag );
//printf(" bitstream_restriction_flag : %d \n", sps->vui.bitstream_restriction_flag );
//  printf("   motion_vectors_over_pic_boundaries_flag : %d \n", sps->vui.motion_vectors_over_pic_boundaries_flag );
//  printf("   max_bytes_per_pic_denom : %d \n", sps->vui.max_bytes_per_pic_denom );
//  printf("   max_bits_per_mb_denom : %d \n", sps->vui.max_bits_per_mb_denom );
//  printf("   log2_max_mv_length_horizontal : %d \n", sps->vui.log2_max_mv_length_horizontal );
//  printf("   log2_max_mv_length_vertical : %d \n", sps->vui.log2_max_mv_length_vertical );
//  printf("   num_reorder_frames : %d \n", sps->vui.num_reorder_frames );
//  printf("   max_dec_frame_buffering : %d \n", sps->vui.max_dec_frame_buffering );

//printf("=== HRD ===\n");
//printf(" cpb_cnt_minus1 : %d \n", sps->hrd.cpb_cnt_minus1 );
//printf(" bit_rate_scale : %d \n", sps->hrd.bit_rate_scale );
//printf(" cpb_size_scale : %d \n", sps->hrd.cpb_size_scale );
//int SchedSelIdx;
//for( SchedSelIdx = 0; SchedSelIdx <= sps->hrd.cpb_cnt_minus1; SchedSelIdx++ )
//{
//    printf("   bit_rate_value_minus1[%d] : %d \n", SchedSelIdx, sps->hrd.bit_rate_value_minus1[SchedSelIdx] ); // up to cpb_cnt_minus1, which is <= 31
//    printf("   cpb_size_value_minus1[%d] : %d \n", SchedSelIdx, sps->hrd.cpb_size_value_minus1[SchedSelIdx] );
//    printf("   cbr_flag[%d] : %d \n", SchedSelIdx, sps->hrd.cbr_flag[SchedSelIdx] );
//}
//printf(" initial_cpb_removal_delay_length_minus1 : %d \n", sps->hrd.initial_cpb_removal_delay_length_minus1 );
//printf(" cpb_removal_delay_length_minus1 : %d \n", sps->hrd.cpb_removal_delay_length_minus1 );
//printf(" dpb_output_delay_length_minus1 : %d \n", sps->hrd.dpb_output_delay_length_minus1 );
//printf(" time_offset_length : %d \n", sps->hrd.time_offset_length );

void MainWindow::setSpsTreeItem(sps_t *sps)
{
    ui->mTreeWidget->clear();

    QTreeWidgetItem *topItem = new QTreeWidgetItem();
    topItem->setText(0, "Nalu");

    QString tmp;
    tmp = tmp.sprintf(" profile_idc : %d \n", sps->profile_idc);
    addItem(topItem, tmp);

    ui->mTreeWidget->addTopLevelItem(topItem);
}
