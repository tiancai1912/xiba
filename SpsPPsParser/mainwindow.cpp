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
           case NALU_UNKNOWN:
    //            qDebug() << "Unknown Nalu" << endl;
               setTableItem(count, "unknown");
               break;
           case NALU_SLICE:
    //            qDebug() << "Slice Nalu" << endl;
               setTableItem(count, "slice");
               break;
           case NALU_SLICE_DPA:
    //            qDebug() << "Slice dpa Nalu" << endl;
               setTableItem(count, "slice dpa");
               break;
           case NALU_SLICE_DPB:
    //            qDebug() << "Slice dpb Nalu" << endl;
               setTableItem(count, "slice dpb");
               break;
           case NALU_SLICE_DPC:
    //            qDebug() << "Slice dpc Nalu" << endl;
               setTableItem(count, "slice dpc");
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
           case NALU_AUD:
//               qDebug() << "AUD Nalu" << endl;
               setTableItem(count,  "aud");
               break;
           case NALU_END_SEQ:
//               qDebug() << "SEQ Nalu" << endl;
               setTableItem(count,  "end of seq");
               break;
           case NALU_END_STREAM:
//               qDebug() << "Stream Nalu" << endl;
               setTableItem(count, "end of stream");
               break;
           case NALU_FILTER_DATA:
//               qDebug() << "Filter data Nalu" << endl;
               setTableItem(count, "filter data");
               break;
           case NALU_SPS_EXT:
//               qDebug() << "Sps ext Nalu" << endl;
               setTableItem(count, "spp ext");
               break;
           default:
               qDebug() << "reserve Nalu" << endl;
               setTableItem(count, "reserve");
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


static QTreeWidgetItem *genItem(char *name, char *value)
{
    QString tmp;
    tmp = tmp.sprintf(name, value);
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, tmp);
    return  item;
}

static QTreeWidgetItem *genItem(char *name, int value)
{
    QString tmp;
    tmp = tmp.sprintf(name, value);
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, tmp);
    return  item;
}

static QTreeWidgetItem * addChildItem(QTreeWidgetItem *dst, char *name, char *value)
{
    QString tmp;
    tmp = tmp.sprintf(name, value);
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, tmp);

    dst->addChild(item);

    return item;
}

static QTreeWidgetItem * addChildItem(QTreeWidgetItem *dst, char *name, int value)
{
    QString tmp;
    tmp = tmp.sprintf(name, value);
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, tmp);

    dst->addChild(item);
    return item;
}

static QTreeWidgetItem * addChildItem(QTreeWidgetItem *dst, char *name, int value1, int value2)
{
    QString tmp;
    tmp = tmp.sprintf(name, value1, value2);
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, tmp);

    dst->addChild(item);
    return item;
}

void MainWindow::setSpsTreeItem(sps_t *sps)
{
    ui->mTreeWidget->clear();

    QTreeWidgetItem *tmp = NULL;
    QTreeWidgetItem *tmp2 = NULL;

    QTreeWidgetItem *topItem = new QTreeWidgetItem();
    topItem->setText(0, "Nalu");

    addChildItem(topItem, " ======= SPS =======", "");
    addChildItem(topItem, " profile_idc : %d", sps->profile_idc);
    addChildItem(topItem, " constraint_set0_flag : %d", sps->constraint_set0_flag);


    addChildItem(topItem, " constraint_set1_flag : %d", sps->constraint_set1_flag);
    addChildItem(topItem, " constraint_set2_flag : %d", sps->constraint_set2_flag);
    addChildItem(topItem, " constraint_set3_flag : %d", sps->constraint_set3_flag);
    addChildItem(topItem, " constraint_set4_flag : %d", sps->constraint_set4_flag);
    addChildItem(topItem, " constraint_set5_flag : %d", sps->constraint_set5_flag);


    addChildItem(topItem, " reserved_zero_2bits : %d", sps->reserved_zero_2bits);
    addChildItem(topItem, " level_idc : %d", sps->level_idc);
    addChildItem(topItem, " seq_parameter_set_id : %d", sps->seq_parameter_set_id);
    addChildItem(topItem, " chroma_format_idc : %d", sps->chroma_format_idc);
    addChildItem(topItem, " separate_colour_plane_flag : %d", sps->separate_colour_plane_flag);
    addChildItem(topItem, " bit_depth_luma_minus8 : %d", sps->bit_depth_luma_minus8);
    addChildItem(topItem, " bit_depth_chroma_minus8 : %d", sps->bit_depth_chroma_minus8);
    addChildItem(topItem, " qpprime_y_zero_transform_bypass_flag : %d", sps->qpprime_y_zero_transform_bypass_flag);
    addChildItem(topItem, " seq_scaling_matrix_present_flag : %d", sps->seq_scaling_matrix_present_flag);


    addChildItem(topItem, " log2_max_frame_num_minus4 : %d", sps->log2_max_frame_num_minus4);
    tmp = addChildItem(topItem, " pic_order_cnt_type : %d", sps->pic_order_cnt_type);
        addChildItem(tmp, " log2_max_pic_order_cnt_lsb_minus4 : %d", sps->log2_max_pic_order_cnt_lsb_minus4);
        addChildItem(tmp, " delta_pic_order_always_zero_flag : %d", sps->delta_pic_order_always_zero_flag);
        addChildItem(tmp, " offset_for_non_ref_pic : %d", sps->offset_for_non_ref_pic);
        addChildItem(tmp, " offset_for_top_to_bottom_field : %d", sps->offset_for_top_to_bottom_field);
        addChildItem(tmp, " num_ref_frames_in_pic_order_cnt_cycle : %d", sps->num_ref_frames_in_pic_order_cnt_cycle);
    topItem->addChild(tmp);

    addChildItem(topItem, " max_num_ref_frames : %d", sps->max_num_ref_frames);
    addChildItem(topItem, " gaps_in_frame_num_value_allowed_flag : %d", sps->gaps_in_frame_num_value_allowed_flag);
    addChildItem(topItem, " pic_width_in_mbs_minus1 : %d", sps->pic_width_in_mbs_minus1);
    addChildItem(topItem, " pic_height_in_map_units_minus1 : %d", sps->pic_height_in_map_units_minus1);
    addChildItem(topItem, " frame_mbs_only_flag : %d", sps->frame_mbs_only_flag);
    addChildItem(topItem, " mb_adaptive_frame_field_flag : %d", sps->mb_adaptive_frame_field_flag);
    addChildItem(topItem, " direct_8x8_inference_flag : %d", sps->direct_8x8_inference_flag);

    tmp = addChildItem(topItem, " frame_cropping_flag : %d", sps->frame_cropping_flag);
        addChildItem(tmp, " frame_crop_left_offset : %d", sps->frame_crop_left_offset);
        addChildItem(tmp, " frame_crop_right_offset : %d", sps->frame_crop_right_offset);
        addChildItem(tmp, " frame_crop_top_offset : %d", sps->frame_crop_top_offset);
        addChildItem(tmp, " frame_crop_bottom_offset : %d", sps->frame_crop_bottom_offset);
    topItem->addChild(tmp);
    addChildItem(topItem, " vui_parameters_present_flag : %d", sps->vui_parameters_present_flag);

    addChildItem(topItem, " === VUI ===", "");
    tmp = addChildItem(topItem, " aspect_ratio_info_present_flag : %d", sps->vui.aspect_ratio_info_present_flag);
        tmp2 = addChildItem(tmp, " aspect_ratio_idc : %d", sps->vui.aspect_ratio_idc);
            addChildItem(tmp2, " sar_width : %d", sps->vui.sar_width);
            addChildItem(tmp2, " sar_height : %d", sps->vui.sar_height);
        tmp->addChild(tmp2);
    topItem->addChild(tmp);

    tmp = addChildItem(topItem, " overscan_appropriate_flag : %d", sps->vui.overscan_appropriate_flag);
    topItem->addChild(tmp);

    tmp = addChildItem(topItem, " video_signal_type_present_flag : %d", sps->vui.video_signal_type_present_flag);
        addChildItem(tmp, " video_format : %d", sps->vui.video_format);
        addChildItem(tmp, " video_full_range_flag : %d", sps->vui.video_full_range_flag);
        tmp2 = addChildItem(tmp, " colour_description_present_flag : %d", sps->vui.colour_description_present_flag);
            addChildItem(tmp2, " colour_primaries : %d", sps->vui.colour_primaries);
            addChildItem(tmp2, " transfer_characteristics : %d", sps->vui.transfer_characteristics);
            addChildItem(tmp2, " matrix_coefficients : %d", sps->vui.matrix_coefficients);
        tmp->addChild(tmp2);
    topItem->addChild(tmp);

    tmp = addChildItem(topItem, " chroma_loc_info_present_flag : %d", sps->vui.chroma_loc_info_present_flag);
        addChildItem(tmp, " chroma_sample_loc_type_top_field : %d", sps->vui.chroma_sample_loc_type_top_field);
        addChildItem(tmp, " chroma_sample_loc_type_bottom_field : %d", sps->vui.chroma_sample_loc_type_bottom_field);
    topItem->addChild(tmp);

    tmp = addChildItem(topItem, " timing_info_present_flag : %d", sps->vui.timing_info_present_flag);
        addChildItem(tmp, " num_units_in_tick : %d", sps->vui.num_units_in_tick);
        addChildItem(tmp, " time_scale : %d", sps->vui.time_scale);
        addChildItem(tmp, " fixed_frame_rate_flag : %d", sps->vui.fixed_frame_rate_flag);
    topItem->addChild(tmp);

    addChildItem(topItem, " nal_hrd_parameters_present_flag : %d", sps->vui.nal_hrd_parameters_present_flag);
    tmp = addChildItem(topItem, " vcl_hrd_parameters_present_flag : %d", sps->vui.vcl_hrd_parameters_present_flag);
        addChildItem(tmp, " low_delay_hrd_flag : %d", sps->vui.low_delay_hrd_flag);
    topItem->addChild(tmp);

    addChildItem(topItem, " pic_struct_present_flag : %d", sps->vui.pic_struct_present_flag);
    tmp = addChildItem(topItem, " bitstream_restriction_flag : %d", sps->vui.bitstream_restriction_flag);
        addChildItem(tmp, " motion_vectors_over_pic_boundaries_flag : %d", sps->vui.motion_vectors_over_pic_boundaries_flag);
        addChildItem(tmp, " max_bytes_per_pic_denom : %d", sps->vui.max_bytes_per_pic_denom);
        addChildItem(tmp, " max_bits_per_mb_denom : %d", sps->vui.max_bits_per_mb_denom);
        addChildItem(tmp, " log2_max_mv_length_horizontal : %d", sps->vui.log2_max_mv_length_horizontal);
        addChildItem(tmp, " log2_max_mv_length_vertical : %d", sps->vui.log2_max_mv_length_vertical);
        addChildItem(tmp, " num_reorder_frames : %d", sps->vui.num_reorder_frames);
        addChildItem(tmp, " max_dec_frame_buffering : %d", sps->vui.max_dec_frame_buffering);
    topItem->addChild(tmp);


    addChildItem(topItem, " === HRD ===", "");
    addChildItem(topItem, " cpb_cnt_minus1 : %d", sps->hrd.cpb_cnt_minus1);
    addChildItem(topItem, " bit_rate_scale : %d", sps->hrd.bit_rate_scale);
    addChildItem(topItem, " cpb_size_scale : %d", sps->hrd.cpb_size_scale);

    int SchedSelIdx;
    for( SchedSelIdx = 0; SchedSelIdx <= sps->hrd.cpb_cnt_minus1; SchedSelIdx++ )
    {
        addChildItem(topItem, " bit_rate_value_minus1[%d] : %d", SchedSelIdx, sps->hrd.bit_rate_value_minus1[SchedSelIdx]);
        addChildItem(topItem, " cpb_size_value_minus1[%d] : %d", SchedSelIdx, sps->hrd.cpb_size_value_minus1[SchedSelIdx]);
        addChildItem(topItem, " cbr_flag[%d] : %d", SchedSelIdx, sps->hrd.cbr_flag[SchedSelIdx]);
    }

    addChildItem(topItem, " initial_cpb_removal_delay_length_minus1 : %d", sps->hrd.initial_cpb_removal_delay_length_minus1);
    addChildItem(topItem, " cpb_removal_delay_length_minus1 : %d", sps->hrd.cpb_removal_delay_length_minus1);
    addChildItem(topItem, " dpb_output_delay_length_minus1 : %d", sps->hrd.dpb_output_delay_length_minus1);
    addChildItem(topItem, " time_offset_length : %d", sps->hrd.time_offset_length);

    ui->mTreeWidget->addTopLevelItem(topItem);
}

void MainWindow::dropEvent(QDropEvent *event)
{
    qDebug() << "dropEvent\n";
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    qDebug() << "dragEnterEvent\n";
}
