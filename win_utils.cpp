#include <napi.h>
#include <windows.h>
#include "windows/AudioProcessMonitor.h"

// Gets a list of processes that are accessing input (microphone)
Napi::Value GetRunningInputAudioProcesses(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    std::vector<std::string> processes = GetAudioInputProcesses();
    
    Napi::Array result = Napi::Array::New(env);
    for (size_t i = 0; i < processes.size(); i++) {
      result.Set(i, Napi::String::New(env, processes[i]));
    }
    
    return result;
  } catch (const std::exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

// Initialize the module exports
Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "getRunningInputAudioProcesses"),
              Napi::Function::New(env, GetRunningInputAudioProcesses));
              
  return exports;
}

NODE_API_MODULE(win_utils, Init) 