#include <stdio.h>

#include <string>
#include <vector>

#include <sstream>

#include "term_util.h"
#include "color_scheme.h"
#include "hh_scanner.h"
#include <pe_sieve_types.h>
#include "params_info/pe_sieve_params_info.h"

#define VERSION "0.2.2.6"

#define PARAM_SWITCH '/'
//scan options:
#define PARAM_HOOKS "/hooks"
#define PARAM_SHELLCODE "/shellc"
#define PARAM_DATA "/data"
#define PARAM_MODULES_FILTER "/mfilter"
#define PARAM_PNAME "/pname"
#define PARAM_LOOP "/loop"

//dump options:
#define PARAM_IMP_REC "/imp"
#define PARAM_DUMP_MODE "/dmode"

//output options:
#define PARAM_QUIET "/quiet"
#define PARAM_OUT_FILTER "/ofilter"
#define PARAM_SUSPEND "/suspend"
#define PARAM_KILL "/kill"
#define PARAM_UNIQUE_DIR "/uniqd"
#define PARAM_DIR "/dir"
#define PARAM_LOG "/log"

//info:
#define PARAM_HELP "/help"
#define PARAM_HELP2  "/?"
#define PARAM_VERSION  "/version"
#define PARAM_DEFAULTS "/default"


void print_logo()
{
    char logo2[] = ""
        "@@@  @@@  @@@@@@  @@@      @@@       @@@@@@  @@@  @@@  @@@  @@@@@@\n"
        "@@!  @@@ @@!  @@@ @@!      @@!      @@!  @@@ @@!  @@!  @@! !@@    \n"
        "@!@!@!@! @!@  !@! @!!      @!!      @!@  !@! @!!  !!@  @!@  !@@!! \n"
        "!!:  !!! !!:  !!! !!:      !!:      !!:  !!!  !:  !!:  !!      !:!\n"
        " :   : :  : :. :  : ::.: : : ::.: :  : :. :    ::.:  :::   ::.: : \n"
        "       @@@  @@@ @@@  @@@ @@@  @@@ @@@@@@@ @@@@@@@@ @@@@@@@        \n"
        "       @@!  @@@ @@!  @@@ @@!@!@@@   @!!   @@!      @@!  @@@       \n"
        "       @!@!@!@! @!@  !@! @!@@!!@!   @!!   @!!!:!   @!@!!@!        \n"
        "       !!:  !!! !!:  !!! !!:  !!!   !!:   !!:      !!: :!!        \n"
        "        :   : :  :.:: :  ::    :     :    : :: ::   :   : :       \n";
    char *logo = logo2;
    set_color(5);
    std::cout << "\n" << logo << std::endl;
}

void print_help()
{
    const int hdr_color = HEADER_COLOR;
    const int param_color = HILIGHTED_COLOR;
    const int separator_color = SEPARATOR_COLOR;

    print_in_color(hdr_color, "Optional: \n");
    print_in_color(separator_color, "\n---scan options---\n");

    print_in_color(param_color, PARAM_PNAME);
    std::cout << " <process_name>\n\t: Scan only processes with given name.\n";

    print_in_color(param_color, PARAM_HOOKS);
    std::cout << "  : Detect hooks and in-memory patches.\n";

    print_in_color(param_color, PARAM_SHELLCODE);
    std::cout << "\t: Detect shellcode implants. (By default it detects PE only).\n";

    print_in_color(param_color, PARAM_DATA);
    std::cout << "\t: If DEP is disabled scan also non-executable memory\n\t(which potentially can be executed).\n";

#ifdef _WIN64
    print_in_color(param_color, PARAM_MODULES_FILTER);
    std::cout << " <*mfilter_id>\n\t: Filter the scanned modules.\n";
    std::cout << "*mfilter_id:\n";
    for (size_t i = 0; i <= LIST_MODULES_ALL; i++) {
        std::cout << "\t" << i << " - " << translate_modules_filter(i) << "\n";
    }
#endif

    print_in_color(param_color, PARAM_LOOP);
    std::cout << "   : Enable continuous scanning.\n";

    print_in_color(separator_color, "\n---dump options---\n");

    print_in_color(param_color, PARAM_IMP_REC);
    std::cout << " <*imprec_mode>\n\t: Set in which mode the ImportTable should be recovered.\n";;
    std::cout << "*imprec_mode:\n";
    for (size_t i = 0; i < pesieve::PE_IMPREC_MODES_COUNT; i++) {
        pesieve::t_imprec_mode mode = (pesieve::t_imprec_mode)(i);
        std::cout << "\t" << mode << " - " << translate_imprec_mode(mode) << "\n";
    }

    print_in_color(param_color, PARAM_DUMP_MODE);
    std::cout << " <*dump_mode>\n\t: Set in which mode the detected PE files should be dumped.\n";
    std::cout << "*dump_mode:\n";
    for (size_t i = 0; i < 4; i++) {
        std::cout << "\t" << i << " - " << translate_dump_mode(i) << "\n";
    }

    print_in_color(separator_color, "\n---output options---\n");

    print_in_color(param_color, PARAM_OUT_FILTER);
    std::cout << " <*ofilter_id>\n\t: Filter the dumped output.\n";
    std::cout << "*ofilter_id:\n";
    for (size_t i = 0; i < pesieve::OUT_FILTERS_COUNT; i++) {
        pesieve::t_output_filter mode = (pesieve::t_output_filter)(i);
        std::cout << "\t" << mode << " - " << translate_out_filter(mode) << "\n";
    }

    print_in_color(param_color, PARAM_DIR);
    std::cout << " <output_dir>\n\t: Set a root directory for the output (default: current directory).\n";

    print_in_color(param_color, PARAM_UNIQUE_DIR);
    std::cout << "\t: Make a unique, timestamped directory for the output of each scan.\n"
        << "\t(Prevents overwriting results from previous scans)\n";

    print_in_color(param_color, PARAM_SUSPEND);
    std::cout << ": Suspend processes detected as suspicious\n";

    print_in_color(param_color, PARAM_KILL);
    std::cout << "   : Kill processes detected as suspicious\n";

    print_in_color(param_color, PARAM_QUIET);
    std::cout << "\t: Display only the summary and minimalistic info.\n";

    print_in_color(param_color, PARAM_LOG);
    std::cout << "\t: Append each scan summary to the log.\n";

    print_in_color(hdr_color, "\nInfo: \n");
    print_in_color(param_color, PARAM_HELP);
    std::cout << "    : Print this help.\n";
    print_in_color(param_color, PARAM_VERSION);
    std::cout << " : Print version number.\n";
    print_in_color(param_color, PARAM_DEFAULTS);
    std::cout << " : Print information about the default settings.\n";
    std::cout << "---" << std::endl;
}

std::string version_to_str(DWORD version)
{
    BYTE *chunks = (BYTE*) &version;
    std::stringstream stream;
    stream << std::hex <<
        (int)chunks[3] << "." <<
        (int)chunks[2] << "." <<
        (int)chunks[1] << "." <<
        (int)chunks[0];

    return stream.str();
}

std::string is_enabled(bool param)
{
    if (param) {
        return "Enabled";
    }
    return "Disabled";
}

void print_version()
{
    set_color(HILIGHTED_COLOR);
    std::cout << "HollowsHunter v." << VERSION;
#ifdef _WIN64
	std::cout << " (x64)" << "\n";
#else
	std::cout << " (x86)" << "\n";
#endif
	std::cout << "Built on: " << __DATE__ << "\n\n";

    DWORD pesieve_ver = PESieve_version();
    std::cout << "using: PE-sieve v." << version_to_str(pesieve_ver) << "\n";
    unset_color();
}

void print_defaults()
{
    std::cout << "\nBy default it detects implanted and replaced PE files.\n"
        "All detected modules are dumped.\n"
        "The reports and dumps are saved into the current directory.\n"
        "\n";

    t_hh_params hh_args;
    hh_args_init(hh_args);

    std::cout << PARAM_PNAME << " : \"" << hh_args.pname << "\"" << "\n";
    if (hh_args.pname.length() == 0) {
        std::cout << "\tall running processes will be scanned\n";
    }
    else {
        std::cout << "\tonly the process with name: " << hh_args.pname << " will be scanned\n";
    }

    std::cout << PARAM_HOOKS << " : " << is_enabled(!hh_args.pesieve_args.no_hooks) << "\n";
    if (hh_args.pesieve_args.no_hooks) {
        std::cout << "\tdo not scan for hooks and patches";
    }
    else {
        std::cout << "\tinclude scan for hooks and patches";
    }
    std::cout << "\n";
    std::cout << PARAM_SHELLCODE << " : " << is_enabled(hh_args.pesieve_args.shellcode) << "\n";
    if (!hh_args.pesieve_args.shellcode) {
        std::cout << "\t do not scan for shellcodes";
    }
    std::cout << "\n";
    std::cout << PARAM_DATA << " : " << is_enabled(hh_args.pesieve_args.data) << "\n";
    if (!hh_args.pesieve_args.data) {
        std::cout << "\t scan only the memory areas that are set as executable";
    }
    std::cout << "\n";
    std::cout << PARAM_LOOP << " : " << is_enabled(hh_args.loop_scanning) << "\n";
    if (!hh_args.loop_scanning) {
        std::cout << "\tsingle scan";
    }
    std::cout << "\n";
    std::cout << PARAM_IMP_REC << " : " << std::dec << hh_args.pesieve_args.imprec_mode << "\n"
        << "\t" << translate_imprec_mode(hh_args.pesieve_args.imprec_mode) << "\n";

    std::cout << PARAM_DUMP_MODE << " : " << std::dec << hh_args.pesieve_args.dump_mode << "\n"
        << "\t" << translate_dump_mode(hh_args.pesieve_args.dump_mode) << "\n";

    std::cout << PARAM_OUT_FILTER << " : " << std::dec << hh_args.pesieve_args.out_filter << "\n"
        << "\t" << translate_out_filter(hh_args.pesieve_args.out_filter) << "\n";

    std::cout << PARAM_DIR << " : \"" << hh_args.out_dir << "\"\n";
    if (hh_args.out_dir.length() == 0) {
        std::cout << "\tcurrent directory";
    }
    std::cout << "\n";
    std::cout << PARAM_UNIQUE_DIR << " : " << is_enabled(hh_args.unique_dir) << "\n";
    if (!hh_args.unique_dir) {
        std::cout << " \tdo not create unique directory for the output\n";
    }
    std::cout << PARAM_SUSPEND << " : " << is_enabled(hh_args.suspend_suspicious) << "\n";
    if (!hh_args.suspend_suspicious) {
        std::cout << "\tdo not suspend suspicious processes";
    }
    std::cout << "\n";
    std::cout << PARAM_KILL << " : " << is_enabled(hh_args.kill_suspicious) << "\n";
    if (!hh_args.kill_suspicious) {
        std::cout << "\tdo not kill suspicious processes";
    }
    std::cout << "\n";
    std::cout << PARAM_QUIET << " : " << is_enabled(hh_args.quiet) << "\n";
    if (!hh_args.quiet) {
        std::cout << " \tprint all the information on the screen";
    }
    std::cout << "\n";
    std::cout << PARAM_LOG << " : " << is_enabled(hh_args.log) << "\n";
    if (!hh_args.log) {
        std::cout << " \tdo not add the results of the scan into the log file";
    }
    std::cout << "\n";
}

void print_unknown_param(const char *param)
{
    print_in_color(WARNING_COLOR, "Unknown parameter: ");
    std::cout << param << "\n";
}

void deploy_scan(t_hh_params &hh_args)
{
    do {
        HHScanner hhunter(hh_args);
        HHScanReport *report = hhunter.scan();
        if (report) {
            hhunter.summarizeScan(report);
            delete report;
        }
    } while (hh_args.loop_scanning);
}

int main(int argc, char *argv[])
{
    t_hh_params hh_args;
    hh_args_init(hh_args);

    //Parse parameters
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], PARAM_HELP) || !strcmp(argv[i], PARAM_HELP2)) {
            print_logo();
            print_version();
            std::cout << "\n";
            print_help();
            return 0;
        }
        if (!strcmp(argv[i], PARAM_VERSION)) {
            print_version();
            return 0;
        }
        if (!strcmp(argv[i], PARAM_DEFAULTS)) {
            print_version();
            print_defaults();
            return 0;
        }
        else if (!strcmp(argv[i], PARAM_IMP_REC)) {
            hh_args.pesieve_args.imprec_mode = pesieve::PE_IMPREC_AUTO;
            if ((i + 1) < argc) {
                char* mode_num = argv[i + 1];
                if (isdigit(mode_num[0])) {
                    hh_args.pesieve_args.imprec_mode = normalize_imprec_mode(atoi(mode_num));
                    ++i;
                }
            }
        }
        else if (!strcmp(argv[i], PARAM_MODULES_FILTER) && (i + 1) < argc) {
            hh_args.pesieve_args.modules_filter = atoi(argv[i + 1]);
            if (hh_args.pesieve_args.modules_filter > LIST_MODULES_ALL) {
                hh_args.pesieve_args.modules_filter = LIST_MODULES_ALL;
            }
            i++;
        }
        else if (!strcmp(argv[i], PARAM_HOOKS)) {
            hh_args.pesieve_args.no_hooks = false;
        }
        else if (!strcmp(argv[i], PARAM_SHELLCODE)) {
            hh_args.pesieve_args.shellcode = true;
        }
        else if (!strcmp(argv[i], PARAM_DATA)) {
            hh_args.pesieve_args.data = true;
        }
        else if (!strcmp(argv[i], PARAM_DUMP_MODE) && (i + 1) < argc) {
            hh_args.pesieve_args.dump_mode = normalize_dump_mode(atoi(argv[i + 1]));
            i++;
        }
        else if (!strcmp(argv[i], PARAM_OUT_FILTER) && (i + 1) < argc) {
            hh_args.pesieve_args.out_filter = static_cast<pesieve::t_output_filter>(atoi(argv[i + 1]));
            i++;
        }
        else if (!strcmp(argv[i], PARAM_LOG)) {
            hh_args.log = true;
        }
        else if (!strcmp(argv[i], PARAM_LOOP)) {
            hh_args.loop_scanning = true;
        }
        else if (!strcmp(argv[i], PARAM_SUSPEND)) {
            hh_args.suspend_suspicious = true;
        }
        else if (!strcmp(argv[i], PARAM_KILL)) {
            hh_args.kill_suspicious = true;
        }
        else if (!strcmp(argv[i], PARAM_PNAME) && (i + 1) < argc) {
            hh_args.pname = argv[i + 1];
            i++;
        }
        else if (!strcmp(argv[i], PARAM_QUIET)) {
            hh_args.quiet = true;
        }
        else if (!strcmp(argv[i], PARAM_UNIQUE_DIR)) {
            hh_args.unique_dir = true;
        }
        else if (!strcmp(argv[i], PARAM_DIR) && (i + 1) < argc) {
            hh_args.out_dir = argv[i + 1];
            ++i;
        }
        else if (strlen(argv[i]) > 0) {
            print_unknown_param(argv[i]);
            print_in_color(HILIGHTED_COLOR, "Available parameters:\n\n");
            print_help();
            return 0;
        }
    }

    print_version();
    deploy_scan(hh_args);
    return 0;
}
