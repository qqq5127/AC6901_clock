#ifndef syn_ps_api_h__
#define syn_ps_api_h__


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


enum {
    SYNC_OUTPUT_NORMAL = 0,
    SYNC_OUTPUT_MORE = 1,
    SYNC_OUTPUT_LESS
};

typedef struct _SYNC_CONTEXT_PS_ {
    u16 autosync;                                                     //同步库里面根据obuf高低调整是否变数， 还是  根据外部的control_flag来变数
    u16 speed_rate;                                                   //主动变数的话，最大步长
    u16 samplerate;
    u16 nch;
    void *priv;
    u32(*getlen)(void *priv);                                         //��ȡbuf�ĳ���
    u32(*output)(void *priv, u8 *buf, int len);
    u32 Limit_up;
    u32 limit_down;
    u32 tws_setflag;              //tws为1的时候才会进的
    s32 *tws_pcm_phase;            //tws使用
    s32 *tws_pcm_cnt;
    s32 *tws_flag;
    u32(*tws_callback)(u32 len);          //output
    u32 more_len;                         //16-90
} SYNC_coef;


typedef struct _SYN_API_CONTEXT_ {
    u32(*need_size)(unsigned short nch, unsigned short samplerate);
    u32(*open)(u8 *ptr, SYNC_coef *coef_obj);
    u32(*run)(u8 *ptr, u8 *data, u32 len, u8 control_flag);

} SYN_API_CONTEXT;

extern SYN_API_CONTEXT   *get_sync_unit_api();

#endif // syn_ps_api_h__


#if 0

{
    SYN_API_CONTEXT *syn_ops = get_sync_unit_api();
    SYNC_coef testIO;

    testIO.autosync = 1;
    testIO.speed_rate = 6;
    testIO.samplerate = 8000;
    testIO.nch = 2 ;
    testIO.priv = NULL;
    testIO.getlen = getlen;
    testIO.output = output;
    testIO.Limit_up = 9 * 1024;
    testIO.limit_down = 6 * 1024;


    bufsize = syn_ops->need_size(testIO.nch, testIO.samplerate);
    ptr = malloc(bufsize);
    syn_ops->open(ptr, &testIO);

    while (1) {
        if (feof(fp1)) {
            break;
        }
        fread(test_buf, 1, TEST_LEN, fp1);
        rd_len = fread(test_buf, 1, TEST_LEN, fp1);
        while (rd_len) {
            int tmp;
            tmp = syn_ops->run(ptr, &test_buf[rd_using], rd_len, SYNC_OUTPUT_LESS);
            rd_len -= tmp;
            rd_using += tmp;
            if (rd_len < 0) {
                rd_len = rd_len;
            }
        }
    }

    free(ptr);

}


#endif
