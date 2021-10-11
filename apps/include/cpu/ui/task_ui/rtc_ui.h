#ifndef _RTC_UI_H_
#define _RTC_UI_H_

#if UI_ENABLE
void ui_open_rtc(void *buf, u32 len);
void ui_close_rtc(void);
#else
#define ui_open_rtc(...)
#define ui_close_rtc(...)
#endif

#endif/*_RTC_UI_H_*/
