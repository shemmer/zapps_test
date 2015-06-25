#include "restore_cmd.h"

#include "shore_env.h"
#include "vol.h"

void RestoreCmd::setupOptions()
{
    KitsCommand::setupOptions();
    options.add_options()
        ("backup", po::value<string>(&opt_backup)->default_value(""),
            "Path on which to store backup file")
        ("segmentSize", po::value<unsigned>(&opt_segmentSize)
            ->default_value(1024),
            "Size of restore segment in number of pages")
        ("singlePass", po::value<bool>(&opt_singlePass)->default_value(false),
            "Whether to use single-pass restore scheduler from the start")
        ("instant", po::value<bool>(&opt_instant)->default_value(true),
            "Use instant restore (i.e., access data before restore is done)")
        ("evict", po::value<bool>(&opt_evict)->default_value(false),
            "Evict all pages from buffer pool when failure happens")
        ("crash", po::value<bool>(&opt_crash)->default_value(false),
            "Simulate media failure together with system failure")
        ("crashDelay", po::value<int>(&opt_crashDelay)->default_value(0),
            "Number of seconds passed between media and system failure. \
            If <= 0, system comes back up with device failed, i.e., \
            volume is marked failed immediately after log analysis.")
        ("postRestoreWorkFactor", po::value<float>(&opt_postRestoreWorkFactor)
            ->default_value(1.0),
            "Numer of transactions to execute after media failure is the \
            number executed before failure times this factor")
        ("concurrentArchiving", po::value<bool>(&opt_concurrentArchiving)
            ->default_value(false),
            "Run log archiving concurrently with benchmark execution and \
            restore, instead of generating log archive \"offline\" when \
            marking the volume as failed")
        // further options to add:
        // fail volume again while it is being restored
        // fail and restore multiple times in a loop
        //
    ;
}

void RestoreCmd::archiveLog()
{
    // archive whole log
    smlevel_0::logArchiver->activate(smlevel_0::log->curr_lsn(), true);
    while (smlevel_0::logArchiver->getNextConsumedLSN() < smlevel_0::log->curr_lsn()) {
        usleep(1000);
    }
    smlevel_0::logArchiver->start_shutdown();
    smlevel_0::logArchiver->join();
}

void RestoreCmd::run()
{
    if (archdir.empty()) {
        throw runtime_error("Log Archive is required to perform restore. \
                Specify path to archive directory with -a");
    }

    // STEP 1 - load database and take backup
    init();
    shoreEnv->load();

    vid_t vid(1);
    vol_t* vol = smlevel_0::vol->get(vid);

    archiveLog();
    if (!opt_backup.empty()) {
        vol->take_backup(opt_backup);
    }

    // STEP 2 - run benchmark and fail device
    runBenchmark();
    vol->mark_failed(opt_evict);

    // TODO if crash is on, move runBenchmark into a separate thread
    // and crash after specified delay. To crash, look at the restart
    // test classes and shutdown the SM like it's done there. Then,
    // bring it back up again and call mark_failed after system comes
    // back. If instant restart is on, then REDO will invoke restore.
    // Meanwhile, the thread running the benchmark will accumulate
    // errors, which should be ok (see trx_worker_t::_serve_action).

    // STEP 3 - continue benchmark on restored data
    runBenchmark();

    finish();
}