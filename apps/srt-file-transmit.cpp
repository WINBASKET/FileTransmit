/*
 * SRT - Secure, Reliable, Transport
 * Copyright (c) 2018 Haivision Systems Inc.
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 */

/*****************************************************************************
written by
   Haivision Systems Inc.
 *****************************************************************************/
#ifdef _WIN32
#include <direct.h>
#endif
#include <iostream>
#include <iterator>
#include <vector>
#include <map>
#include <stdexcept>
#include <string>
#include <csignal>
#include <thread>
#include <chrono>
#include <cassert>
#include <sys/stat.h>
#include <srt.h>
#include <udt.h>
#include <common.h>

#include "apputil.hpp"
#include "uriparser.hpp"
#include "logsupport.hpp"
#include "socketoptions.hpp"
#include "transmitmedia.hpp"
#include "verbose.hpp"

#include <glib.h>
#include "bus/ftDbus.h"
#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif


using namespace std;

void g_emit_path_signal(string stPath);

GMainLoop *loop = NULL;
static bool interrupt = false;
void OnINT_ForceExit(int)
{
    Verb() << "\n-------- REQUESTED INTERRUPT!\n";
    cout << "end loop" << endl;
    interrupt = true;
    g_main_loop_quit (loop);
}

typedef struct _InputConfig
{
    int argc;
    char** argv;

    string mode;
    string source;
    string target;
    string fileName;

    ftDbusFileTransmitAgent *interface;

}CONFIG;

CONFIG gstConfig;

struct FileTransmitConfig
{
    unsigned long chunk_size;
    bool skip_flushing;
    bool quiet = false;
    srt_logging::LogLevel::type loglevel = srt_logging::LogLevel::error;
    set<srt_logging::LogFA> logfas;
    string logfile;
    int bw_report = 0;
    int stats_report = 0;
    string stats_out;
    SrtStatsPrintFormat stats_pf = SRTSTATS_PROFMAT_2COLS;
    bool full_stats = false;

    string source;
    string target;
    
};

void PrintOptionHelp(const set<string> &opt_names, const string &value, const string &desc)
{
    cerr << "\t";
    int i = 0;
    for (auto opt : opt_names)
    {
        if (i++) cerr << ", ";
        cerr << "-" << opt;
    }

    if (!value.empty())
        cerr << ":" << value;
    cerr << "\t- " << desc << "\n";
}


int parse_args(FileTransmitConfig &cfg, int argc, char** argv)
{
    const OptionName
        o_chunk     = { "c", "chunk" },
        o_no_flush  = { "sf", "skipflush" },
        o_bwreport  = { "r", "bwreport", "report", "bandwidth-report", "bitrate-report" },
        o_statsrep  = { "s", "stats", "stats-report-frequency" },
        o_statsout  = { "statsout" },
        o_statspf   = { "pf", "statspf" },
        o_statsfull = { "f", "fullstats" },
        o_loglevel  = { "ll", "loglevel" },
        o_logfa     = { "logfa" },
        o_logfile   = { "logfile" },
        o_quiet     = { "q", "quiet" },
        o_verbose   = { "v", "verbose" },
        o_help      = { "h", "help" },
        o_version   = { "version" };

    const vector<OptionScheme> optargs = {
        { o_chunk,        OptionScheme::ARG_ONE },
        { o_no_flush,     OptionScheme::ARG_NONE },
        { o_bwreport,     OptionScheme::ARG_ONE },
        { o_statsrep,     OptionScheme::ARG_ONE },
        { o_statsout,     OptionScheme::ARG_ONE },
        { o_statspf,      OptionScheme::ARG_ONE },
        { o_statsfull,    OptionScheme::ARG_NONE },
        { o_loglevel,     OptionScheme::ARG_ONE },
        { o_logfa,        OptionScheme::ARG_ONE },
        { o_logfile,      OptionScheme::ARG_ONE },
        { o_quiet,        OptionScheme::ARG_NONE },
        { o_verbose,      OptionScheme::ARG_NONE },
        { o_help,         OptionScheme::ARG_NONE },
        { o_version,      OptionScheme::ARG_NONE }
    };

    options_t params = ProcessOptions(argv, argc, optargs);

          bool print_help    = Option<OutBool>(params, false, o_help);
    const bool print_version = Option<OutBool>(params, false, o_version);

    if (params[""].size() != 2 && !print_help && !print_version)
    {
        cerr << "ERROR. Invalid syntax. Specify source and target URIs.\n";
        if (params[""].size() > 0)
        {
            cerr << "The following options are passed without a key: ";
            copy(params[""].begin(), params[""].end(), ostream_iterator<string>(cerr, ", "));
            cerr << endl;
        }
        print_help = true; // Enable help to print it further
    }


    if (print_help)
    {
        cout << "SRT sample application to transmit files.\n";
        cerr << "Built with SRT Library version: " << SRT_VERSION << endl;
        const uint32_t srtver = srt_getversion();
        const int major = srtver / 0x10000;
        const int minor = (srtver / 0x100) % 0x100;
        const int patch = srtver % 0x100;
        cerr << "SRT Library version: " << major << "." << minor << "." << patch << endl;
        cerr << "Usage: srt-file-transmit [options] <input-uri> <output-uri>\n";
        cerr << "\n";

        PrintOptionHelp(o_chunk, "<chunk=1456>", "max size of data read in one step");
        PrintOptionHelp(o_no_flush, "", "skip output file flushing");
        PrintOptionHelp(o_bwreport, "<every_n_packets=0>", "bandwidth report frequency");
        PrintOptionHelp(o_statsrep, "<every_n_packets=0>", "frequency of status report");
        PrintOptionHelp(o_statsout, "<filename>", "output stats to file");
        PrintOptionHelp(o_statspf, "<format=default>", "stats printing format [json|csv|default]");
        PrintOptionHelp(o_statsfull, "", "full counters in stats-report (prints total statistics)");
        PrintOptionHelp(o_loglevel, "<level=error>", "log level [fatal,error,info,note,warning]");
        PrintOptionHelp(o_logfa, "<fas=general,...>", "log functional area [all,general,bstats,control,data,tsbpd,rexmit]");
        PrintOptionHelp(o_logfile, "<filename="">", "write logs to file");
        PrintOptionHelp(o_quiet, "", "quiet mode (default off)");
        PrintOptionHelp(o_verbose, "", "verbose mode (default off)");
        cerr << "\n";
        cerr << "\t-h,-help - show this help\n";
        cerr << "\t-version - print SRT library version\n";
        cerr << "\n";
        cerr << "\t<input-uri>  - URI specifying a medium to read from\n";
        cerr << "\t<output-uri> - URI specifying a medium to write to\n";
        cerr << "URI syntax: SCHEME://HOST:PORT/PATH?PARAM1=VALUE&PARAM2=VALUE...\n";
        cerr << "Supported schemes:\n";
        cerr << "\tsrt: use HOST, PORT, and PARAM for setting socket options\n";
        cerr << "\tudp: use HOST, PORT and PARAM for some UDP specific settings\n";
        cerr << "\tfile: file URI or file://con to use stdin or stdout\n";

        return 2;
    }

    if (Option<OutBool>(params, false, o_version))
    {
        cerr << "SRT Library version: " << SRT_VERSION << endl;
        return 2;
    }

    cfg.chunk_size    = stoul(Option<OutString>(params, "1456", o_chunk));
    cfg.skip_flushing = Option<OutBool>(params, false, o_no_flush);
    cfg.bw_report     = stoi(Option<OutString>(params, "0", o_bwreport));
    cfg.stats_report  = stoi(Option<OutString>(params, "0", o_statsrep));
    cfg.stats_out     = Option<OutString>(params, "", o_statsout);
    const string pf   = Option<OutString>(params, "default", o_statspf);
    if (pf == "default")
    {
        cfg.stats_pf = SRTSTATS_PROFMAT_2COLS;
    }
    else if (pf == "json")
    {
        cfg.stats_pf = SRTSTATS_PROFMAT_JSON;
    }
    else if (pf == "csv")
    {
        cfg.stats_pf = SRTSTATS_PROFMAT_CSV;
    }
    else
    {
        cfg.stats_pf = SRTSTATS_PROFMAT_2COLS;
        cerr << "ERROR: Unsupported print format: " << pf << endl;
        return 1;
    }

    cfg.full_stats = Option<OutBool>(params, false, o_statsfull);
    cfg.loglevel   = SrtParseLogLevel(Option<OutString>(params, "error", o_loglevel));
    cfg.logfas     = SrtParseLogFA(Option<OutString>(params, "", o_logfa));
    cfg.logfile    = Option<OutString>(params, "", o_logfile);
    cfg.quiet      = Option<OutBool>(params, false, o_quiet);

    if (Option<OutBool>(params, false, o_verbose))
        Verbose::on = !cfg.quiet;

    cfg.source = params[""].at(0);
    cfg.target = params[""].at(1);

    return 0;
}


void ExtractPath(string path, string& w_dir, string& w_fname)
{
    string directory = path;
    string filename = "";

    struct stat state;
    stat(path.c_str(), &state);

    if (!S_ISDIR(state.st_mode))
    {
        // Extract directory as a butlast part of path
        size_t pos = path.find_last_of("/");
        if ( pos == string::npos )
        {
            filename = path;
            directory = ".";
        }
        else
        {
            directory = path.substr(0, pos);
            filename = path.substr(pos+1);
        }
    }

    if (directory[0] != '/')
    {
        // Glue in the absolute prefix of the current directory
        // to make it absolute. This is needed to properly interpret
        // the fixed uri.
        static const size_t s_max_path = 4096; // don't care how proper this is
        char tmppath[s_max_path];
#ifdef _WIN32
        const char* gwd = _getcwd(tmppath, s_max_path);
#else
        const char* gwd = getcwd(tmppath, s_max_path);
#endif
        if ( !gwd )
        {
            // Don't bother with that now. We need something better for
            // that anyway.
            throw std::invalid_argument("Path too long");
        }
        const string wd = gwd;

        directory = wd + "/" + directory;
    }

    w_dir = directory;
    w_fname = filename;
}

bool DoUpload(UriParser& ut, string path, string filename,
              const FileTransmitConfig &cfg, std::ostream &out_stats)
{
    bool result = false;
    unique_ptr<Target> tar;
    SRTSOCKET s = SRT_INVALID_SOCK;
    bool connected = false;
    int pollid = -1;

    ifstream ifile(path, ios::binary);
    if ( !ifile )
    {
        cerr << "Error opening file: '" << path << "'";
        goto exit;
    }

    pollid = srt_epoll_create();
    if ( pollid < 0 )
    {
        cerr << "Can't initialize epoll";
        goto exit;
    }


    while (!interrupt)
    {
        if (!tar.get())
        {
            tar = Target::Create(ut.makeUri());
            if (!tar.get())
            {
                cerr << "Unsupported target type: " << ut.uri() << endl;
                goto exit;
            }

            int events = SRT_EPOLL_OUT | SRT_EPOLL_ERR;
            if (srt_epoll_add_usock(pollid,
                    tar->GetSRTSocket(), &events))
            {
                cerr << "Failed to add SRT destination to poll, "
                    << tar->GetSRTSocket() << endl;
                goto exit;
            }
            UDT::setstreamid(tar->GetSRTSocket(), filename);
        }

        s = tar->GetSRTSocket();
        assert(s != SRT_INVALID_SOCK);

        SRTSOCKET efd;
        int efdlen = 1;
        if (srt_epoll_wait(pollid,
            0, 0, &efd, &efdlen,
            100, nullptr, nullptr, 0, 0) < 0)
        {
            continue;
        }

        assert(efd == s);
        assert(efdlen == 1);

        SRT_SOCKSTATUS status = srt_getsockstate(s);

        switch (status)
        {
            case SRTS_LISTENING:
            {
                if (!tar->AcceptNewClient())
                {
                    cerr << "Failed to accept SRT connection" << endl;
                    goto exit;
                }

                srt_epoll_remove_usock(pollid, s);

                s = tar->GetSRTSocket();
                int events = SRT_EPOLL_OUT | SRT_EPOLL_ERR;
                if (srt_epoll_add_usock(pollid, s, &events))
                {
                    cerr << "Failed to add SRT client to poll" << endl;
                    goto exit;
                }
                cerr << "Target connected (listener)" << endl;
                connected = true;
            }
            break;
            case SRTS_CONNECTED:
            {
                if (!connected)
                {
                    cerr << "Target connected (caller)" << endl;
                    connected = true;
                }
            }
            break;
            case SRTS_BROKEN:
            case SRTS_NONEXIST:
            case SRTS_CLOSED:
            {
                cerr << "Target disconnected" << endl;
                goto exit;
            }
            default:
            {
                // No-Op
            }
            break;
        }

        if (connected)
        {
            vector<char> buf(cfg.chunk_size);
            size_t n = ifile.read(buf.data(), cfg.chunk_size).gcount();
             
            size_t shift = 0;
            while (n > 0)
            {
                int st = tar->Write(buf.data() + shift, n, out_stats);
                Verb() << "Upload: " << n << " --> " << st
                    << (!shift ? string() : "+" + Sprint(shift));
                if (st == SRT_ERROR)
                {
                    cerr << "Upload: SRT error: " << srt_getlasterror_str()
                        << endl;
                    goto exit;
                }

                n -= st;
                shift += st;
            }

            if (ifile.eof())
            {
                cerr << "File sent" << endl;
                result = true;
                break;
            }

            if ( !ifile.good() )
            {
                cerr << "ERROR while reading file\n";
                goto exit;
            }

        }
    }

    if (result && !cfg.skip_flushing)
    {
        assert(s != SRT_INVALID_SOCK);

        // send-flush-loop
        result = false;
        while (!interrupt)
        {
            size_t bytes;
            size_t blocks;
            int st = srt_getsndbuffer(s, &blocks, &bytes);
            
            if (st == SRT_ERROR)
            {
                cerr << "Error in srt_getsndbuffer: " << srt_getlasterror_str()
                    << endl;
                goto exit;
            }
            if (bytes == 0)
            {
                cerr << "Buffers flushed" << endl;
                result = true;
                break;
            }
            Verb() << "Sending buffer still: bytes=" << bytes << " blocks="
                << blocks;
            this_thread::sleep_for(chrono::milliseconds(250));
        }
    }

exit:
    if (pollid >= 0)
    {
        srt_epoll_release(pollid);
    }

    return result;
}

bool DoDownload(UriParser& us, string directory, string filename,
                const FileTransmitConfig &cfg, std::ostream &out_stats)
{
    bool result = false;
    unique_ptr<Source> src;
    SRTSOCKET s = SRT_INVALID_SOCK;
    bool connected = false;
    int pollid = -1;
    string id;
    ofstream ofile;
    SRT_SOCKSTATUS status;
    SRTSOCKET efd;
    int efdlen = 1;
    string strFileEof = ".complete";
    string strFileNameComplete; 

    pollid = srt_epoll_create();
    if ( pollid < 0 )
    {
        cerr << "Can't initialize epoll";
        goto exit;
    }

    while (!interrupt)
    {
        if (!src.get())
        {
            src = Source::Create(us.makeUri());
            if (!src.get())
            {
                cerr << "Unsupported source type: " << us.uri() << endl;
                goto exit;
            }

            int events = SRT_EPOLL_IN | SRT_EPOLL_ERR;
            if (srt_epoll_add_usock(pollid,
                    src->GetSRTSocket(), &events))
            {
                cerr << "Failed to add SRT source to poll, "
                    << src->GetSRTSocket() << endl;
                goto exit;
            }
        }

        s = src->GetSRTSocket();
        assert(s != SRT_INVALID_SOCK);

        if (srt_epoll_wait(pollid,
            &efd, &efdlen, 0, 0,
            100, nullptr, nullptr, 0, 0) < 0)
        {
            continue;
        }

        assert(efd == s);
        assert(efdlen == 1);

        status = srt_getsockstate(s);
       // Verb() << "Event with status " << status << "\n";

        switch (status)
        {
            case SRTS_LISTENING:
            {
                if (!src->AcceptNewClient())
                {
                    cerr << "Failed to accept SRT connection" << endl;
                    goto exit;
                }

                srt_epoll_remove_usock(pollid, s);

                s = src->GetSRTSocket();
                int events = SRT_EPOLL_IN | SRT_EPOLL_ERR;
                if (srt_epoll_add_usock(pollid, s, &events))
                {
                    cerr << "Failed to add SRT client to poll" << endl;
                    goto exit;
                }
                id = UDT::getstreamid(s);
                cerr << "Source connected (listener), id ["
                    << id << "]" << endl;

                gstConfig.fileName = id; 
               
                connected = true;
                continue;
            }
            break;
            case SRTS_CONNECTED:
            {
                if (!connected)
                {
                    id = UDT::getstreamid(s);
                    
                    cerr << "Source connected (caller), id ["
                        << id << "]" << endl;
                    connected = true;
                }
            }
            break;
            case SRTS_BROKEN:
            case SRTS_NONEXIST:
            case SRTS_CLOSED:
            {
                g_emit_path_signal(gstConfig.fileName);
                cerr << "Source disconnected" << endl;
                
                goto exit;
            }
            break;
            default:
            {
                // No-Op
            }
            break;
        }

        if (connected)
        {
            vector<char> buf(cfg.chunk_size);

            if (!ofile.is_open())
            {
                const char * fn = id.empty() ? filename.c_str() : id.c_str();
                directory.append("/");
                directory.append(fn);
                ofile.open(directory.c_str(), ios::out | ios::trunc | ios::binary);

                if (!ofile.is_open())
                {
                    cerr << "Error opening file [" << directory << "]" << endl;
                    goto exit;
                }
                cerr << "Writing output to [" << directory << "]" << endl;
            }

            int n = src->Read(cfg.chunk_size, buf, out_stats);
            if (n == SRT_ERROR)
            {
                cerr << "Download: SRT error: " << srt_getlasterror_str() << endl;
                goto exit;
            }

	    //---last packet will be less then chunk size
	    //   if file's size is n*chunk_size, then result is not to be true 
            if (n < cfg.chunk_size)
            {
                if(n > 0)
                     ofile.write(buf.data(), n);
                result = true;
                strFileNameComplete =  directory + strFileEof;
                
                rename(directory.c_str(), strFileNameComplete.c_str());


                cerr << "Download COMPLETE."<< directory.c_str() <<endl;
                cerr <<"Rename : " << strFileNameComplete.c_str() << endl;
                
                break;
            }

            // Write to file any amount of data received
            Verb() << "Download: --> " << n;
            ofile.write(buf.data(), n);
            if (!ofile.good())
            {
                cerr << "Error writing file" << endl;
                goto exit;
            }

        }
    }

exit:
    if (pollid >= 0)
    {
        srt_epoll_release(pollid);
    }

    return result;
}

bool Upload(UriParser& srt_target_uri, UriParser& fileuri,
            const FileTransmitConfig &cfg, std::ostream &out_stats)
{
    if ( fileuri.scheme() != "file" )
    {
        cerr << "Upload: source accepted only as a file\n";
        return false;
    }
    // fileuri is source-reading file
    // srt_target_uri is SRT target

    string path = fileuri.path();
    string directory, filename;
    ExtractPath(path, (directory), (filename));
    Verb() << "Extract path '" << path << "': directory=" << directory << " filename=" << filename;
    // Set ID to the filename.
    // Directory will be preserved.

    // Add some extra parameters.
    srt_target_uri["transtype"] = "file";

    return DoUpload(srt_target_uri, path, filename, cfg, out_stats);
}

bool Download(UriParser& srt_source_uri, UriParser& fileuri,
              const FileTransmitConfig &cfg, std::ostream &out_stats)
{
    if (fileuri.scheme() != "file" )
    {
        cerr << "Download: target accepted only as a file\n";
        return false;
    }

    string path = fileuri.path(), directory, filename;
    ExtractPath(path, (directory), (filename));
    Verb() << "Extract path '" << path << "': directory=" << directory << " filename=" << filename;

    // Add some extra parameters.
    srt_source_uri["transtype"] = "file";

    return DoDownload(srt_source_uri, directory, filename, cfg, out_stats);
}


//-------------------------------------------------------------------------------
//    main
//------------------------------------------------------------------------------
bool FileTransmitManagement()
{
    
 
    FileTransmitConfig cfg;
    
    cfg.chunk_size    = 1456;      //stoul(Option<OutString>(params, "1456", o_chunk));
    cfg.skip_flushing = false;     //Option<OutBool>(params, false, o_no_flush);
    cfg.bw_report     = 0;         //stoi(Option<OutString>(params, "0", o_bwreport));
    cfg.stats_report  = 0;         //stoi(Option<OutString>(params, "0", o_statsrep));
    cfg.stats_out     = "";        //Option<OutString>(params, "", o_statsout);
    cfg.full_stats = false;        //Option<OutBool>(params, false, o_statsfull);    
    cfg.logfile    = "";           //Option<OutString>(params, "", o_logfile);
    cfg.quiet      = false;        //Option<OutBool>(params, false, o_quiet);
    //cfg.loglevel   = "error";     //SrtParseLogLevel(Option<OutString>(params, "error", o_loglevel));
    //cfg.logfas     = "";          //SrtParseLogFA(Option<OutString>(params, "", o_logfa));

    const string pf   = "default"; //Option<OutString>(params, "default", o_statspf);
   
    if (pf == "default")
    {
        cfg.stats_pf = SRTSTATS_PROFMAT_2COLS;
    }
    else if (pf == "json")
    {
        cfg.stats_pf = SRTSTATS_PROFMAT_JSON;
    }
    else if (pf == "csv")
    {
        cfg.stats_pf = SRTSTATS_PROFMAT_CSV;
    }
    else
    {
        cfg.stats_pf = SRTSTATS_PROFMAT_2COLS;
        cerr << "ERROR: Unsupported print format: " << pf << endl;
        return false;
    }   

    //if (Option<OutBool>(params, false, o_verbose))
    //    Verbose::on = !cfg.quiet;
    
    if(gstConfig.mode == "caller")
        cfg.source = gstConfig.source +  gstConfig.fileName;
    else 
        cfg.source = gstConfig.source;

    cfg.target = gstConfig.target;

    //
    // Set global config variables
    //

    if (cfg.chunk_size != SRT_LIVE_MAX_PLSIZE)
        transmit_chunk_size = cfg.chunk_size;
    transmit_stats_writer = SrtStatsWriterFactory(cfg.stats_pf);
    transmit_bw_report = cfg.bw_report;
    transmit_stats_report = cfg.stats_report;
    transmit_total_stats = cfg.full_stats;

    //
    // Set SRT log levels and functional areas
    //
    srt_setloglevel(cfg.loglevel);
    for (set<srt_logging::LogFA>::iterator i = cfg.logfas.begin(); i != cfg.logfas.end(); ++i)
        srt_addlogfa(*i);

    //
    // SRT log handler
    //
    std::ofstream logfile_stream; // leave unused if not set
    if (!cfg.logfile.empty())
    {
        logfile_stream.open(cfg.logfile.c_str());
        if (!logfile_stream)
        {
            cerr << "ERROR: Can't open '" << cfg.logfile.c_str() << "' for writing - fallback to cerr\n";
        }
        else
        {
            UDT::setlogstream(logfile_stream);
        }
    }

    //
    // SRT stats output
    //
    std::ofstream logfile_stats; // leave unused if not set
    if (cfg.stats_out != "" && cfg.stats_out != "stdout")
    {
        logfile_stats.open(cfg.stats_out.c_str());
        if (!logfile_stats)
        {
            cerr << "ERROR: Can't open '" << cfg.stats_out << "' for writing stats. Fallback to stdout.\n";
            return false;
        }
    }
    else if (cfg.bw_report != 0 || cfg.stats_report != 0)
    {
        g_stats_are_printed_to_stdout = true;
    }

    ostream &out_stats = logfile_stats.is_open() ? logfile_stats : cout;

    //
    // File transmission code
    //

    UriParser us(cfg.source), ut(cfg.target);

    Verb() << "SOURCE type=" << us.scheme() << ", TARGET type=" <<  ut.scheme();

    //signal(SIGINT, OnINT_ForceExit);
    //signal(SIGTERM, OnINT_ForceExit);


    cout <<"source:" << cfg.source << endl;
    cout << "target:" << cfg.target << endl; 

    try
    {
        if ( us.scheme() == "srt")
        {
            if ( ut.scheme() != "file")
            {
                cerr << "SRT to FILE should be specified\n";
                return false;
            }
                   
            g_print("Download start\n");
         
            if(Download(us, ut, cfg, out_stats) != true)
		    return false;
        }
        else if (ut.scheme() == "srt")
        {
            if (us.scheme() != "file")
            {
                cerr << "FILE to SRT should be specified\n";
                return false;
            }
            g_print("upload start\n");
            
	    if(Upload( ut, us, cfg, out_stats) != true)
                return false;
            
        }
        else
        {
            cerr << "SRT URI must be one of given media.\n";
           return false;
        }
    }
    catch (std::exception& x)
    {
        cerr << "ERROR: " << x.what() << endl;
        return false;
    }  
    
    return true;

}

//---download thread
void *thread_download(void *data)
{
    //CONFIG *p = (CONFIG *)data;
    static int cnt = 3;

    bool rv =  FileTransmitManagement();
    
    if(rv == true) {
        g_print("Download success! \n");
	cnt = 3;
    }
    else {
        cnt--;
	g_print("Download failed!(#:%d) \n",cnt);

	if(cnt == 0)
		interrupt = true;
    }
	g_main_loop_quit(loop);

     return (void **)rv;

}

//---upload thread
void *thread_upload(void *data)
{
    bool rv =  FileTransmitManagement();
    
    if(rv == true)
        g_print("Upload success! \n");
    else
        g_print("Upload failed! \n");

    return (void **)rv;
}

//---emit signal
void g_emit_path_signal(string stPath)
{
    gchar *path = g_strdup_printf("%s",stPath.c_str());

    g_print("Emit a path : %s\n", path);

    ft_dbus_file_transmit_agent_emit_receive(gstConfig.interface, path);
   
    return;
}
//---server handler register & receive from client bus
static gboolean
on_handle_dbus_send (ftDbusFileTransmitAgent *interface, GDBusMethodInvocation *invocation,
					const gchar* path)
{   
	pthread_t pth;
    int res, pid;   
    guint gid;
    
    gchar *response;

    //---time check start
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

    response = g_strdup_printf("%s%s", gstConfig.source.c_str(), path);

    gstConfig.fileName = path;
   
    if(pthread_create(&pth, NULL, thread_upload, NULL) < 0)
    {
        perror("thread create error");
        return FALSE;
    }
    
    pthread_join(pth, (void **)&res);

    if(res) {
        pid = getpid();
    }
    else
    {
        pid = 0;
    }
    
    //---send condition 
    ft_dbus_file_transmit_agent_complete_send(interface, invocation, pid);    
    
    g_main_loop_quit(loop);

    std::chrono::duration<double> sec = std::chrono::system_clock::now() -start;
    std::cout << "Send Time :" << sec.count() << "sec"<< std::endl;

    g_free (response);

    return TRUE;
}

//---dbus name acquired
static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    g_print("[dbus name]%s\n", name);

	gstConfig.interface = ft_dbus_file_transmit_agent_skeleton_new();  

    //---listener : emit signal of file path .....test
    //---caller : receive a file path from client  
    if(gstConfig.mode == "listener") {
        g_print("listener start\n");         
    }
    else {
        g_print("caller start\n");
        g_signal_connect(gstConfig.interface, "handle-send", G_CALLBACK (on_handle_dbus_send), NULL);
    }
    
    g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (gstConfig.interface), connection, "/org/hwangsaeul/Gaeul2/FileTransmitAgent", NULL);
   
}

//---parsing config file's string
vector<string> split(string str, char delim) 
{
    vector<string> internal;
    stringstream ss(str);
    string temp;

    while(getline(ss, temp, delim)) {
        internal.push_back(temp);
    }

    return internal;
}

//---main function
int main(int argc, char** argv)
{
    
    pthread_t pth;
  
    gstConfig.argc = argc;
    gstConfig.argv = argv;   

    char delim = ' ';
    vector<string> strMode;
    vector<string> strSrcPath;
    vector<string> strTargetPath;

    g_print("Srt File Transmit App Start!\n");  

    signal(SIGINT, OnINT_ForceExit);
    signal(SIGTERM, OnINT_ForceExit);

    //---read a configuration file 
    std::ifstream readFile;
    readFile.open("configure.txt");

    if(readFile.is_open())
    {
        while(!readFile.eof())
        {
            string str;
            getline(readFile, str);  //read 1 line
          
	    cout << str << endl;

           // stringstream ss(str);
           // cout << "1 "<< str << endl;
            if(str.find("mode :") != string::npos) {
                strMode = split(str, delim);
                gstConfig.mode = strMode[2];
                // for(int i = 0 ; i < strMode.size(); i++)
                //    cout << strMode[i] << endl;
            }

            if(str.find("source :") != string::npos) {
                strSrcPath = split(str, delim);
                gstConfig.source = strSrcPath[2];
                //for(int i = 0 ; i < strSrcPath.size(); i++)
                    cout <<"parsing source: " << gstConfig.source << endl;
            }

            if(str.find("target :") != string::npos) {
                strTargetPath = split(str, delim);
                gstConfig.target = strTargetPath[2];
                //for(int i = 0 ; i < strTargetPath.size(); i++)
                cout << "parsing target: " << gstConfig.target << endl;
            }

           if(str == "") {
                cout << "file read end "<< endl; 
                readFile.close();
                break;
            }   
        }
    }
    //---gdbus registry 
    g_bus_own_name(G_BUS_TYPE_SESSION, "org.hwangsaeul.Gaeul2", G_BUS_NAME_OWNER_FLAGS_NONE, NULL, on_name_acquired, NULL, NULL, NULL);
   
    //---mode classify
     while(!interrupt) {
        loop = g_main_loop_new (NULL, FALSE);
        
	g_print("\n======================\n");
	g_print("up/download loop start\n");
    
         if(gstConfig.mode == "listener") {
            if(pthread_create(&pth, NULL, thread_download, NULL) < 0)
            {
                perror("thread create error");
                exit(0);
            }
            
        }
       
        g_main_loop_run(loop);
        g_main_loop_unref(loop);
        
     }
   

    return 0;


}
