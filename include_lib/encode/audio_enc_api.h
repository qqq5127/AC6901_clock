#ifndef audio_enc_api_h__
#define audio_enc_api_h__

#ifndef u8
#define u8  unsigned char
#endif

#ifndef u16
#define u16 unsigned short
#endif

#ifndef u32
#define u32 unsigned int
#endif

#ifndef s16
#define s16 signed short
#endif


typedef struct _EN_FILE_IO_ {
    void *priv;
    u16(*input_data)(void *priv, s16 *buf, u8 channel, u16 len);
    u32(*output_data)(void *priv, u8 *buf, u16 len);
} EN_FILE_IO;



typedef  struct   _ENC_DATA_INFO_ {
    u16 sr;            ///<sample rate
    u16 br;            ///<mp2的时候它是bitrate，但是adpcm的时候，它是blockSize,一般配成256/512/1024/2048，超过2048会被限制成2048
    u32 nch;
} ENC_DATA_INFO;


typedef struct _ENC_OPS {
    u32(*need_buf)();
    void (*open)(u8 *ptr, EN_FILE_IO *audioIO);
    void (*set_info)(u8 *ptr, ENC_DATA_INFO *data_info);
    u32(*init)(u8 *ptr);
    u32(*run)(u8 *ptr);
    u32(*write_head)(u8 *ptr);
} ENC_OPS;



#if 0

ENC_OPS *enc_test_ops = get_enadpcm_ops();
buf_size = enc_test_ops->need_buf();
buf_ptr = malloc(buf_size);
memset(buf_ptr, 0, buf_size);
enc_test_ops->open(buf_ptr, &mp2_testIO);
enc_test_ops->set_info(buf_ptr, &enc_info);

if (enc_test_ops->init(buf_ptr))
{
    printf("err;\n");
    goto mp2_err_out;
}
while (enc_test_ops->write_head(buf_ptr))
{

}//为wav头占位

while (1)
{
    enc_test_ops->run(buf_ptr);

    if (feof(file_io_t.fileIN)) {
        break;
    }
}

fseek(file_io_t.fileOUT, 0, SEEK_SET);                           //重新写头
while (enc_test_ops->write_head(buf_ptr))
{

} /


#endif

#endif // audio_enc_api_h__
