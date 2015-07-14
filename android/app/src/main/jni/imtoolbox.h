#ifndef IMTOOLBOX_H
#define IMTOOLBOX_H

#include "core/traits.h"
#include "core/utility.h"
#include "core/matrix.h"
#include "core/image.h"
#include "core/proc.h"

#ifdef ANDROID
#include <android/log.h>
#define LOGI(...)                                                              \
  __android_log_print(ANDROID_LOG_INFO, "IMTOOLBOX", __VA_ARGS__)
#define LOGD(...)                                                              \
  __android_log_print(ANDROID_LOG_DEBUG, "IMTOOLBOX", __VA_ARGS__)
#define LOGE(...)                                                              \
  __android_log_print(ANDROID_LOG_ERROR, "IMTOOLBOX", __VA_ARGS__)
#endif

#endif // IMTOOLBOX_H
