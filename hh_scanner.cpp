#include "hh_scanner.h"

#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <time.h>

#include "util\suspend.h"

using namespace pesieve;

std::string join_path(std::string baseDir, std::string subpath)
{
    std::stringstream stream;
    if (baseDir.length() > 0) {
        stream << baseDir;
        stream << "\\";
    }
    stream << subpath;
    return stream.str();
}

std::string make_dir_name(std::string baseDir, time_t timestamp)
{
    std::stringstream stream;
    if (baseDir.length() > 0) {
        stream << baseDir;
        stream << "\\";
    }
    stream << "scan_";
    stream << timestamp;
    return stream.str();
}

bool set_output_dir(t_params &args, const char *new_dir)
{
    if (!new_dir) return false;

    size_t new_len = strlen(new_dir);
    size_t buffer_len = sizeof(args.output_dir);
    if (new_len > buffer_len) return false;

    memset(args.output_dir, 0, buffer_len);
    memcpy(args.output_dir, new_dir, new_len);
    return true;
}

bool get_process_name(DWORD processID, CHAR szProcessName[MAX_PATH])
{
    memset(szProcessName, 0, MAX_PATH);

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (!hProcess) return false;

    HMODULE hMod = nullptr;
    DWORD cbNeeded = 0;
    bool is_ok = false;
    if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
        GetModuleBaseNameA(hProcess, hMod, szProcessName, MAX_PATH);
        is_ok = true;
    }
    CloseHandle(hProcess);
    return is_ok;
}

size_t suspend_suspicious(std::vector<DWORD> &suspicious_pids)
{
    size_t done = 0;
    std::vector<DWORD>::iterator itr;
    for (itr = suspicious_pids.begin(); itr != suspicious_pids.end(); itr++) {
        DWORD pid = *itr;
        if (!suspend_process(pid)) {
            std::cerr << "Could not suspend the process. PID = " << pid << std::endl;
        }
    }
    return done;
}

size_t kill_suspicious(std::vector<DWORD> &suspicious_pids)
{
    size_t killed = 0;
    std::vector<DWORD>::iterator itr;
    for (itr = suspicious_pids.begin(); itr != suspicious_pids.end(); itr++) {
        DWORD pid = *itr;
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (!hProcess) {
            continue;
        }
        if (TerminateProcess(hProcess, 0)) {
            killed++;
        }
        else {
            std::cerr << "Could not terminate the process. PID = " << pid << std::endl;
        }
        CloseHandle(hProcess);
    }
    return killed;
}

bool is_searched_process(const char* processName, const char* searchedName)
{
    if (_stricmp(processName, searchedName) == 0) {
        return true;
    }
    return false;
}

void HHScanner::initOutDir(time_t start_time)
{
    //set unique path
    if (hh_args.unique_dir) {
        this->outDir = make_dir_name(hh_args.out_dir, start_time);
        set_output_dir(hh_args.pesieve_args, outDir.c_str());
    }
    else {
        this->outDir = hh_args.out_dir;
        set_output_dir(hh_args.pesieve_args, hh_args.out_dir.c_str());
    }
}

HHScanReport* HHScanner::scan()
{
    const size_t max_processes = 1024;
    DWORD aProcesses[max_processes], cbNeeded;
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        return NULL;
    }
    //calculate how many process identifiers were returned.
    size_t cProcesses = cbNeeded / sizeof(DWORD);
    if (cProcesses == 0) {
        return NULL;
    }

    time_t start_time = time(NULL);
    initOutDir(start_time);
    HHScanReport *my_report = new HHScanReport(GetTickCount(), start_time);

    bool found = false;
    for (size_t i = 0; i < cProcesses; i++) {
        if (aProcesses[i] == 0) continue;
        
        DWORD pid = aProcesses[i];
        char image_buf[MAX_PATH] = { 0 };
        get_process_name(pid, image_buf);
        
        if (hh_args.pname.length() > 0) {
            if (!is_searched_process(image_buf, hh_args.pname.c_str())) {
                //it is not the searched process, so skip it
                continue;
            }
            found = true;
            if (!hh_args.quiet) {
                std::cout << image_buf << " (PID: " << std::dec << pid << ")\n";
            }
        }
        if (!hh_args.quiet) {
            std::cout << ">> Scanning PID: " << std::dec << pid << std::endl;
        }
        hh_args.pesieve_args.pid = pid;
        pesieve::t_report report = PESieve_scan(hh_args.pesieve_args);
        my_report->appendReport(report, image_buf);
    }
    if (!found && hh_args.pname.length() > 0) {
        if (!hh_args.quiet) {
            std::cout << "[WARNING] Process with the name: " << hh_args.pname << " not found!" << std::endl;
        }
    }
    my_report->setEndTick(GetTickCount(), time(NULL));
    return my_report;
}

bool write_to_file(std::string report_path, std::string summary_str, bool append)
{
    std::ofstream final_report;
    if (append) {
        final_report.open(report_path, std::ios_base::app);
    }
    else {
        final_report.open(report_path);
    }
    if (final_report.is_open()) {
        final_report << summary_str;
        final_report.close();
        return true;
    }
    return false;
}

void HHScanner::summarizeScan(HHScanReport *hh_report)
{
    if (!hh_report) return;
    
    std::string summary_str = hh_report->toString();
    std::cout << summary_str;

    if (hh_args.pesieve_args.out_filter != OUT_NO_DIR) {
        //file the same report into the directory with dumps:
        if (hh_report->suspicious.size()) {
            std::string report_path = join_path(this->outDir, "summary.json");
            write_to_file(report_path, hh_report->toJSON(this->hh_args), false);
        }
    }
    if (hh_args.log) {
        write_to_file("hollows_hunter.log", summary_str, true);
    }
    if (hh_args.suspend_suspicious) {
        suspend_suspicious(hh_report->suspicious);
    }
    if (hh_args.kill_suspicious) {
        kill_suspicious(hh_report->suspicious);
    }
}
