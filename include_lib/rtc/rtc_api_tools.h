#ifndef __RTC_API_TOOLS_H__
#define __RTC_API_TOOLS_H__

#include "typedef.h"
#include "rtc_api.h"
void calendar_time_minus(RTC_TIME *curr_rtc_time, u8 coordinate);
void calendar_time_plus(RTC_TIME *curr_rtc_time, u8 coordinate);
void rtc_calculate_next_weekday(RTC_TIME *data_time);
void rtc_calculate_next_day(RTC_TIME *data_time);
u16   ymd_to_day(RTC_TIME *time);
void day_to_ymd(u16 day, RTC_TIME *rtc_time);
void RTC_TIME_to_rtc_data(struct rtc_data *time_out, RTC_TIME *time_in);
void rtc_data_to_RTC_TIME(RTC_TIME *time_out, struct rtc_data *time_in);
#endif // __RTC_API_TOOLS_H__
