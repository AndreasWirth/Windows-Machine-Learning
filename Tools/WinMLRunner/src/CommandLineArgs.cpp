#include <Windows.h>
#include <string>
#include <iostream>
#include "CommandLineArgs.h"
#include <apiquery2.h>
#include <ctime>
#include <iomanip>

using namespace Windows::AI::MachineLearning;

void CommandLineArgs::PrintUsage() {
    std::cout << "WinML Runner" << std::endl;
    std::cout << " ---------------------------------------------------------------" << std::endl;
    std::cout << "WinmlRunner.exe <-model | -folder> <fully qualified path> [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "options: " << std::endl;
    std::cout << "  -version: prints the version information for this build of WinMLRunner.exe" << std::endl;
    std::cout << "  -CPU : run model on default CPU" << std::endl;
    std::cout << "  -GPU : run model on default GPU" << std::endl;
    std::cout << "  -GPUHighPerformance : run model on GPU with highest performance" << std::endl;
    std::cout << "  -GPUMinPower : run model on GPU with the least power" << std::endl;
    std::cout << "  -GPUAdapterIndex : run model on GPU specified by its index in DXGI enumeration. NOTE: Please only use this flag on DXCore supported machines." << std::endl;
    std::cout << "  -CreateDeviceOnClient : create the device on the client and pass it to WinML" << std::endl;
    std::cout << "  -CreateDeviceInWinML : create the device inside WinML" << std::endl;
    std::cout << "  -CPUBoundInput : bind the input to the CPU" << std::endl;
    std::cout << "  -GPUBoundInput : bind the input to the GPU" << std::endl;
    std::cout << "  -RGB : load the input as an RGB image" << std::endl;
    std::cout << "  -BGR : load the input as a BGR image" << std::endl;
    std::cout << "  -Tensor : load the input as a tensor" << std::endl;
    std::cout << "  -Perf optional:<all>: capture performance measurements such as timing and memory usage. Specifying \"all\" will output all measurements" << std::endl;
    std::cout << "  -Iterations : # times perf measurements will be run/averaged" << std::endl;
    std::cout << "  -Input <fully qualified path>: binds image or CSV to model" << std::endl;
    std::cout << "  -PerfOutput optional:<fully qualified path>: csv file to write the perf results to" << std::endl;
    std::cout << "  -SavePerIterationPerf : save per iteration performance results to csv file" << std::endl;
    std::cout << "  -SaveTensorData <saveMode folderPath>: saveMode: save first iteration or all iteration output tensor results to csv file [First, All]" << std::endl;
    std::cout << "                                         folderPath: Optional folder path can be specified to hold tensor data. It will be created if folder doesn't exist." << std::endl;
    std::cout << "  -Debug: print trace logs" << std::endl;
    std::cout << "  -Terse: Terse Mode (suppresses repetitive console output)" << std::endl;
    std::cout << "  -AutoScale <interpolationMode>: Enable image autoscaling and set the interpolation mode [Nearest, Linear, Cubic, Fant]" << std::endl;
    std::cout << std::endl;
    std::cout << "Concurrency Options:" << std::endl;
    std::cout << "  -ConcurrentLoad: load models concurrently" << std::endl;
    std::cout << "  -NumThreads <number>: number of threads to load a model. By default this will be the number of model files to be executed" << std::endl;
    std::cout << "  -ThreadInterval <milliseconds>: interval time between two thread creations in milliseconds" << std::endl;
}

CommandLineArgs::CommandLineArgs(const std::vector<std::wstring> &args)
{
    for (UINT i = 0; i < args.size(); i++)
    {
        if ((_wcsicmp(args[i].c_str(), L"-CPU") == 0))
        {
            m_useCPU = true;
        }
        else if ((_wcsicmp(args[i].c_str(), L"-GPU") == 0))
        {
            m_useGPU = true;
        }
        else if ((_wcsicmp(args[i].c_str(), L"-GPUHighPerformance") == 0))
        {
            m_useGPUHighPerformance = true;
        }
        else if ((_wcsicmp(args[i].c_str(), L"-GPUMinPower") == 0))
        {
            m_useGPUMinPower = true;
        }
        else if ((_wcsicmp(args[i].c_str(), L"-GPUAdapterIndex") == 0) && i + 1 < args.size() && args[i + 1][0] != L'-')
        {
            CheckNextArgument(args, i);
            HMODULE library{ nullptr };
            library = LoadLibrary(L"ext-ms-win-dxcore-l1-1-0");
            if (!library)
            {
                throw hresult_invalid_argument(L"ERROR: DXCORE isn't supported on this machine. GpuAdapterIndex flag should only be used with DXCore supported machines.");
            }
            m_useGPU = true;
            m_adapterIndex = static_cast<UINT>(_wtoi(args[++i].c_str()));
        }
        else if ((_wcsicmp(args[i].c_str(), L"-CreateDeviceOnClient") == 0))
        {
            m_createDeviceOnClient = true;
        }
        else if ((_wcsicmp(args[i].c_str(), L"-CreateDeviceInWinML") == 0))
        {
            m_createDeviceInWinML = true;
        }
        else if ((_wcsicmp(args[i].c_str(), L"-Iterations") == 0) && (i + 1 < args.size()))
        {
            m_numIterations = static_cast<UINT>(_wtoi(args[++i].c_str()));
        }
        else if ((_wcsicmp(args[i].c_str(), L"-Model") == 0))
        {
            CheckNextArgument(args, i);
            m_modelPath = args[++i];
        }
        else if ((_wcsicmp(args[i].c_str(), L"-Folder") == 0))
        {
            CheckNextArgument(args, i);
            m_modelFolderPath = args[++i];
        }
        else if ((_wcsicmp(args[i].c_str(), L"-Input") == 0))
        {
            m_inputData = args[++i];
        }
        else if ((_wcsicmp(args[i].c_str(), L"-PerfOutput") == 0))
        {
            if (i + 1 < args.size() && args[i + 1][0] != L'-')
            {
                m_perfOutputPath = args[++i];
            }
            m_perfOutput = true;
        }
        else if ((_wcsicmp(args[i].c_str(), L"-RGB") == 0))
        {
            m_useRGB = true;
        }
        else if ((_wcsicmp(args[i].c_str(), L"-BGR") == 0))
        {
            m_useBGR = true;
        }
        else if ((_wcsicmp(args[i].c_str(), L"-Tensor") == 0))
        {
            m_useTensor = true;
        }
        else if ((_wcsicmp(args[i].c_str(), L"-CPUBoundInput") == 0))
        {
            m_useCPUBoundInput = true;
        }
        else if ((_wcsicmp(args[i].c_str(), L"-GPUBoundInput") == 0))
        {
            m_useGPUBoundInput = true;
        }
        else if ((_wcsicmp(args[i].c_str(), L"-Perf") == 0))
        {
            if (i + 1 < args.size() && args[i + 1][0] != L'-' && (_wcsicmp(args[i + 1].c_str(), L"all") == 0))
            {
                m_perfConsoleOutputAll = true;
                i++;
            }
            m_perfCapture = true;
        }
        else if ((_wcsicmp(args[i].c_str(), L"-Debug") == 0))
        {
            m_debug = true;
        }
        else if ((_wcsicmp(args[i].c_str(), L"-SavePerIterationPerf") == 0))
        {
            m_perIterCapture = true;
        }
        else if ((_wcsicmp(args[i].c_str(), L"-Terse") == 0))
        {
            m_terseOutput = true;
        }
        else if ((_wcsicmp(args[i].c_str(), L"-AutoScale") == 0))
        {
            CheckNextArgument(args, i);
            m_autoScale = true;
            if (_wcsicmp(args[++i].c_str(), L"Nearest") == 0)
            {
                m_autoScaleInterpMode = BitmapInterpolationMode::NearestNeighbor;
            }
            else if (_wcsicmp(args[i].c_str(), L"Linear") == 0)
            {
                m_autoScaleInterpMode = BitmapInterpolationMode::Linear;
            }
            else if (_wcsicmp(args[i].c_str(), L"Cubic") == 0)
            {
                m_autoScaleInterpMode = BitmapInterpolationMode::Cubic;
            }
            else if (_wcsicmp(args[i].c_str(), L"Fant") == 0)
            {
                m_autoScaleInterpMode = BitmapInterpolationMode::Fant;
            }
            else
            {
                PrintUsage();
                throw hresult_invalid_argument(L"Unknown AutoScale Interpolation Mode!");
            }
        }
        else if ((_wcsicmp(args[i].c_str(), L"-SaveTensorData") == 0) && (i + 1 < args.size()))
        {
            CheckNextArgument(args, i);
            m_saveTensor = true;
            if (_wcsicmp(args[++i].c_str(), L"First") == 0)
            {
                m_saveTensorMode = "First";
            }
            else if (_wcsicmp(args[i].c_str(), L"All") == 0)
            {
                m_saveTensorMode = "All";
            }
            else
            {
                PrintUsage();
                throw hresult_invalid_argument(L"Unknown Mode!");
            }
            try
            {
                // This is to check for second optional value for -SaveTensorData to specify path
                CheckNextArgument(args, i);
                SetTensorOutputPath(args[++i].c_str());
            }
            catch (...) {
                // set to default path
                auto time = std::time(nullptr);
                struct tm localTime;
                localtime_s(&localTime, &time);
                std::wostringstream oss;
                oss << std::put_time(&localTime, L"%Y-%m-%d_%H.%M.%S");
                SetTensorOutputPath(L"\\PerIterationRun[" + oss.str() + L"]");
            }
        }
        else if (_wcsicmp(args[i].c_str(), L"-version") == 0)
        {
            TCHAR szExeFileName[MAX_PATH];
            GetModuleFileName(NULL, szExeFileName, MAX_PATH);
            uint32_t versionInfoSize = GetFileVersionInfoSize(szExeFileName, 0);
            wchar_t *pVersionData = new wchar_t[versionInfoSize / sizeof(wchar_t)];
            GetFileVersionInfo(szExeFileName, 0, versionInfoSize, pVersionData);

            wchar_t *pOriginalFilename;
            uint32_t originalFilenameSize;
            VerQueryValue(pVersionData, L"\\StringFileInfo\\040904b0\\OriginalFilename", (void**)&pOriginalFilename, &originalFilenameSize);

            wchar_t *pProductVersion;
            uint32_t productVersionSize;
            VerQueryValue(pVersionData, L"\\StringFileInfo\\040904b0\\ProductVersion", (void**)&pProductVersion, &productVersionSize);

            wchar_t *pFileVersion;
            uint32_t fileVersionSize;
            VerQueryValue(pVersionData, L"\\StringFileInfo\\040904b0\\FileVersion", (void**)&pFileVersion, &fileVersionSize);

            std::wcout << pOriginalFilename << std::endl;
            std::wcout << L"Version: " << pFileVersion << "." << pProductVersion << std::endl;

            delete[] pVersionData;
            return;
        }
        else if ((_wcsicmp(args[i].c_str(), L"/?") == 0))
        {
            PrintUsage();
            return;
        }
        // concurrency options
        else if ((_wcsicmp(args[i].c_str(), L"-ConcurrentLoad") == 0))
        {
            ToggleConcurrentLoad(true);
        }
        else if ((_wcsicmp(args[i].c_str(), L"-NumThreads") == 0))
        {
            CheckNextArgument(args, i);
            unsigned num_threads = std::stoi(args[++i].c_str());
            SetNumThreads(num_threads);
        }
        else if ((_wcsicmp(args[i].c_str(), L"-ThreadInterval") == 0))
        {
            CheckNextArgument(args, i);
            unsigned thread_interval = std::stoi(args[++i].c_str());
            SetThreadInterval(thread_interval);
        }
        else
        {
            std::wstring msg = L"Unknown option ";
            msg += args[i].c_str();
            throw hresult_invalid_argument(msg.c_str());
        }
    }

    if (m_modelPath.empty() && m_modelFolderPath.empty())
    {
        std::cout << std::endl;
        PrintUsage();
        return;
    }

    if (!m_inputData.empty())
    {
        if (m_inputData.find(L".png") != std::string::npos || m_inputData.find(L".jpg") != std::string::npos)
        {
            m_imagePath = m_inputData;
        }
        if (m_inputData.find(L".csv") != std::string::npos)
        {
            m_csvData = m_inputData;
        }
    }
}

void CommandLineArgs::CheckNextArgument(const std::vector<std::wstring> &args, UINT i) {
    if (i + 1 >= args.size() || args[i + 1][0] == L'-') {
        std::wstring msg = L"Invalid parameter for ";
        msg += args[i].c_str();
        throw hresult_invalid_argument(msg.c_str());
    }
}