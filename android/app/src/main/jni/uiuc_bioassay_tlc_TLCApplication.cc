#include "uiuc_bioassay_tlc_TLCApplication.h"
#include "tlc.h"

JNIEXPORT jdoubleArray JNICALL Java_uiuc_bioassay_tlc_TLCApplication_processTLC
  (JNIEnv *env, jclass, jstring jstr) {
  const char *path = env->GetStringUTFChars(jstr, nullptr);

  imtoolbox::begin_log(std::string(path) + tlc::LOG_FILE);
  auto spots = tlc::process(path);

  if (spots.size() == 0) {
    LOGE("Couldn't detect any spot");
    return nullptr;
  }

  for (size_t i = 0; i < spots.size(); ++i) {
    imtoolbox::println_i("Spot " + std::to_string(i + 1) + ": ");
    imtoolbox::println_i("\tRf: " + std::to_string(spots[i].rf));
    imtoolbox::println_i("\tDarkness: " + std::to_string(spots[i].darkness));
  }

  jdoubleArray ret;
  // Copy result back
  std::vector<double> res(spots.size() * 2);
  for (size_t i = 0; i < spots.size(); ++i) {
    res[2 * i] = spots[i].rf;
    res[2 * i + 1] = spots[i].darkness;
  }

  ret = env->NewDoubleArray(res.size());
  if (ret == nullptr) {
    return nullptr;
  }
  env->SetDoubleArrayRegion(ret, 0, res.size(), &res[0]);
  env->ReleaseStringUTFChars(jstr, path);
  imtoolbox::end_log();
  return ret;
}

JNIEXPORT void JNICALL
Java_uiuc_bioassay_tlc_TLCApplication_cleanFolder(JNIEnv *env, jclass,
                                                    jstring jstr) {
  const char *path = env->GetStringUTFChars(jstr, nullptr);
  std::string cmd("exec rm -r ");
  cmd += path;
  cmd += "/*";
  LOGD("%s", cmd.c_str());
  system(cmd.c_str());
  env->ReleaseStringUTFChars(jstr, path);
}
