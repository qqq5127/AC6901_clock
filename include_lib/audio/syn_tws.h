#ifndef syn_tws_h__
#define syn_tws_h__


#ifndef u8
#define u8  unsigned char
#endif

#ifndef u16
#define u16 unsigned short
#endif

#ifndef s16
#define s16 short
#endif


#ifndef u32
#define u32 unsigned int
#endif

#ifndef s16
#define s16 signed short
#endif
#define TWS_CUSTOM_VARIABLE_MIN_1   0
#define TWS_CUSTOM_VARIABLE_MIN_2   1
#define TWS_CUSTOM_VARIABLE_MAX_1   2
#define TWS_CUSTOM_VARIABLE_MAX_2   3
#define TWS_CUSTOM_ASB_LEN_FLAG     4
#define TWS_CUSTOM_VARIABLE_TIME    5
#define TWS_CUSTOM_RESET_HW_SYNC    6
#define TWS_CUSTOM_RESTART_SBC      7
#define  SY_OBUF_LEN             64

typedef struct _tws_sync_parm_t {
    u32(*tws_audio_read)(u32 prt, u32 len, u8 cmd);
    u16(*get_audio_stream_dac_len)();
    u16(*dac_get_samplerate)(void);
    void (*clear_audio_stream_clear)();
    void (*dac_int_disable)(void);
    void (*dac_int_enable)(void);
    void (*clear_dac_dma_buf)();

    void (*tws_sync_inc_dac_cnt)();
    int (*tws_sync_inc_dac_read)();
    void (*tws_sync_init)();
    void (*tws_master_reset_sync_parm)();
    void (*tws_music_decoder_loop_resume)(void);
    void (*tws_sync_set_timer2_us)(u32 time_us, void (*fun)());
    void (*debug_tws_date_state)(u16 debug_lbuf_alloc_total, u16 send_packet_num, u16 get_packet_num, u32 cbuf_date_size, u16 send_sbc_list_cnt);
    u8 set_user_ch;
    u8 set_ch_mode;
    u16 sbc_bitpool_value;

    volatile u32 PCM_unsing;                    //当前使用到的解码位置
    volatile u32 PCM_dest;                      //下一次slot_N 要消耗到的位置
    volatile u32 lasts32_slot_finecnt;
    volatile u32 last32_slot;
    u32 first_connect;
    u32 pcm_pos;
    int low_tmpt;
    u32 dac_cnt;
    u32 pre_slot_cntv;
    u32 pre_slot_delta;
    short obuf[SY_OBUF_LEN];
    int inpcm;
    int outpcm;
    int flag;
    int pcm_cnt;
    int pcm_phase;
    short last_pcm[2];
    u32 cmdi;
    s32 delta_out;
    s32 delta_points_cnt;
    u32 first_dac;
    u32  threshold_hold;
    u32  dac_total_d;
    u32  dac_slotv;
    u32  dac_delta;

    int low_flag;                   //low data flag
    u32 last_daccnt;
    u32 last_slotcnt;

    u32 slot_delta;
    u32 err_cnt;
    s16 syn_flag_cnt;
    /* SYNC_audio_IO tws_io; */
    /* SYN_DX_API_CONTEXT *tws_ops;     */
    /* u8 *tws_ops_buf; */
    volatile u32 tws_conn_working;
    volatile u8 tws_ps_flag;
    volatile u32 rd_using;
    u32 stime_cnt;
    // u8 dac_read_en;
    u8 dac_ide;
    volatile u8 dec_remain_frame;
    volatile u8 dec_in_total_frame;

    u32 frame_out_cnt;
    u32 frame_pcm_len;
    volatile u32 packet_pcm_cnt;
    volatile u32 pre_packet_pcm_cnt;
    volatile u32 dec_packet_pcm_cnt;
    volatile u8 reset_decode_en;
    u8 custom[20];


} tws_sync_parm_t;

extern void *get_tws_sync_parm();

#endif // syn_ps_api_h__



