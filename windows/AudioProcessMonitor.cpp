// AudioProcessMonitor.cpp
//

#include <windows.h>

#include <audiopolicy.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include <iostream>
#include <psapi.h>
#include <string>
#include <vector>
#include "AudioProcessMonitor.h"
#include <Audioclient.h>
#include <unordered_set>

#pragma comment(lib, "Ole32.lib")

// Function to get process executable path from PID
static std::string GetProcessExecutablePath(DWORD processID) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processID);
    if (!hProcess) return "Unknown";

    WCHAR path[MAX_PATH];
    DWORD size = MAX_PATH;
    
    if (QueryFullProcessImageNameW(hProcess, 0, path, &size)) {
        CloseHandle(hProcess);
        // Convert wide string to regular string
        int strSize = WideCharToMultiByte(CP_UTF8, 0, path, -1, nullptr, 0, nullptr, nullptr);
        std::string result(strSize, 0);
        WideCharToMultiByte(CP_UTF8, 0, path, -1, &result[0], strSize, nullptr, nullptr);
        if (!result.empty() && result.back() == 0) {
            result.pop_back();
        }
        return result;
    }

    CloseHandle(hProcess);
    return "Unknown";
}

std::vector<std::string> GetAudioInputProcesses() {
    std::vector<std::string> results;
    std::unordered_set<std::string> seen;  // Track unique strings
    HRESULT hr = CoInitialize(nullptr);

    if (FAILED(hr)) {
        return results;
    }

    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioSessionManager2* pSessionManager = nullptr;
    IAudioSessionEnumerator* pSessionEnum = nullptr;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) {
        return results;
    }

    // Get default capture (microphone) device
    hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eMultimedia, &pDevice);
    if (FAILED(hr)) {
        pEnumerator->Release();
        return results;
    }

    bool isActive = false;
    IAudioMeterInformation* pMeter = nullptr;

    // Get the Audio Meter Interface
    hr = pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, nullptr, (void**)&pMeter);
    if (SUCCEEDED(hr)) {
        float peakValue = 0.0f;
        pMeter->GetPeakValue(&peakValue);
        isActive = (peakValue > 0.0f);
        pMeter->Release();
    }

    if (!isActive) {
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return results;
    }

    // Get session manager
    hr = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void**)&pSessionManager);
    if (FAILED(hr)) {
        std::cerr << "Failed to activate IAudioSessionManager2. HRESULT: " << std::hex << hr << std::endl;
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return results;
    }

    // Get audio session enumerator
    hr = pSessionManager->GetSessionEnumerator(&pSessionEnum);
    if (FAILED(hr)) {
        std::cerr << "Failed to get IAudioSessionEnumerator. HRESULT: " << std::hex << hr << std::endl;
        pSessionManager->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return results;
    }

    int sessionCount = 0;
    pSessionEnum->GetCount(&sessionCount);

    for (int i = 0; i < sessionCount; i++) {
        IAudioSessionControl* pSessionControl = nullptr;
        hr = pSessionEnum->GetSession(i, &pSessionControl);
        if (FAILED(hr)) continue;

        IAudioSessionControl2* pSessionControl2 = nullptr;
        hr = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pSessionControl2);
        if (SUCCEEDED(hr)) {
            DWORD processID = 0;
            pSessionControl2->GetProcessId(&processID);

            if (processID != 0) {
                std::string processPath = GetProcessExecutablePath(processID);
                
                // Only insert if not already seen
                if (seen.insert(processPath).second) {
                    results.push_back(processPath);
                }
            }
            pSessionControl2->Release();
        }
        pSessionControl->Release();
    }

    // Cleanup
    pSessionEnum->Release();
    pSessionManager->Release();
    pDevice->Release();
    pEnumerator->Release();
    CoUninitialize();

    return results;
}