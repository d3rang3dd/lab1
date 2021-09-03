#include <iostream>
#include <windows.h>

const int MAX_LENGTH = 256;

void variadic_print(std::ostream& os) {}

template <typename T, typename ...Args>
void variadic_print(std::ostream& os, T&& x, Args&&... args)
{
    os << x;
    variadic_print(std::cout, args...);
}

// функция для удобной печати сообщений
template <typename Status, typename T, typename ...Args>
void test_status_print(Status status, T action, Args&&... args)
{
    bool typetest = std::is_same <Status, LSTATUS>::value;
    if ((typetest && status != ERROR_SUCCESS) || (!typetest && !status))
        std::cerr << "Error " << GetLastError() << action << '\n';
    else
        variadic_print(std::cout, args...);
}

int main()
{
    // версия ОС
    OSVERSIONINFOA ver_info {};
    ver_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    test_status_print(GetVersionEx(&ver_info),
                      " occured during determining OS version!",
                      "OS Version: ",
                      ver_info.dwMajorVersion, '.', ver_info.dwMinorVersion, '\n');
    std::cout << '\n';

    // системная директория
    char sys_dir[MAX_LENGTH];
    test_status_print(GetSystemDirectory(sys_dir, sizeof(sys_dir)),
                      " occured during determining system directory!",
                      "System directory: ", sys_dir, '\n');
    std::cout << '\n';

    // имя компьютера и пользователя
    char user_name[MAX_LENGTH];
    char pc_name[MAX_LENGTH];
    DWORD sz_user = sizeof(user_name);
    DWORD sz_pc = sizeof(pc_name);
    test_status_print(GetUserName(user_name, &sz_user),
                      " occured during determining username!",
                      "Username: ", user_name, '\n');
    std::cout << '\n';
    test_status_print(GetComputerName(pc_name, &sz_pc),
                      " occured during determining PC name!",
                      "PC name: ", pc_name, '\n');
    std::cout << '\n';

    // работа с томами
    char vol_name [MAX_LENGTH];
    char vol_letter [MAX_LENGTH];
    ULARGE_INTEGER user_bytes {};
    ULARGE_INTEGER total_bytes {};
    HANDLE first_vol = FindFirstVolume(vol_name, sizeof(vol_name));
    std::cout << "Volume information:\n";
    do
    {
        std::cout << "GUID: " << vol_name << '\n';
        test_status_print(GetVolumePathNamesForVolumeName
                             (vol_name, vol_letter, sizeof(vol_letter), nullptr),
                          " occured during determining volume letter!",
                          "Volume letter: ", vol_letter, '\n');
        test_status_print(GetDiskFreeSpaceEx
                             (vol_letter, &user_bytes, &total_bytes, nullptr),
                          " occured during determining free space!",
                          "Free/total (bytes): ",
                          user_bytes.QuadPart, '/', total_bytes.QuadPart, '\n');
        // очищение переменных
        memset(vol_name, 0, sizeof(vol_name));
        memset(vol_letter, 0, sizeof(vol_letter));
        total_bytes = ULARGE_INTEGER {};
        user_bytes = ULARGE_INTEGER {};
    }
    while (FindNextVolume(first_vol, vol_name, sizeof(vol_name)));

    BOOL bstatus = FindVolumeClose(first_vol);
    if (!bstatus)
        std::cerr << "Error " << GetLastError()
                  << " occured during closing volume search!\n";

    if (GetLastError() != ERROR_NO_MORE_FILES)
        std::cerr << "Error " << GetLastError()
                  << " occured during volume search!\n";
    std::cout << '\n';

    // работа с реестром
    HKEY hkey {};
    test_status_print(RegOpenKeyEx(HKEY_CURRENT_USER,
                                   "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                                   0, KEY_ALL_ACCESS, &hkey),
                      " occured during opening registry key!");
    char name [MAX_LENGTH];
    DWORD sz_name = sizeof(name);
    std::cout << "Startup programs:\n";

    LSTATUS status;
    for (int i = 0; ; ++ i)
    {
        status = RegEnumValue
                    (hkey, i, name, &sz_name, nullptr, nullptr, nullptr, nullptr);
        if (status == ERROR_SUCCESS)
        {
            test_status_print(RegQueryValueEx
                                  (hkey, name, nullptr, nullptr, nullptr, nullptr),
                              " occured during reading registry value!",
                              name, '\n');
            memset(name, 0, sz_name);
            sz_name = sizeof(name);
        }
        else break;
    }
    std::cout << '\n';

    // замер производительности ЦП
    LARGE_INTEGER perf_counter_start;
    test_status_print(QueryPerformanceCounter(&perf_counter_start),
                      " occured during determining performance count!");
    LARGE_INTEGER perf_freq;
    test_status_print(QueryPerformanceFrequency(&perf_freq),
                      " occured during determining performance frequency!",
                      "Performance frequency: ", perf_freq.QuadPart, " Hz\n");
    LARGE_INTEGER perf_counter_end;
    test_status_print(QueryPerformanceCounter(&perf_counter_end),
                      " occured during determining performance count!");
    std::cout << "Performance counter: "
              << (1000000.0 * (perf_counter_end.QuadPart - perf_counter_start.QuadPart))
                    / perf_freq.QuadPart
              << " us" << '\n';
}
