#include "selfupdater.h"

namespace Updater
{
    /**
     * @brief self_update::self_update
     *
     * Self_Update implements a self-update strategy for this executable.
     *
     * 1. download new version as "wpnxm-scp.new-version.exe"
     * 2. rename running "wpnxm-scp.exe" to "wpnxm-scp.exe.old"
     * 3. rename "wpnxm-scp.new-version.exe" to "wpnxm-scp.exe" (new version replaces old one)
     * 4. on (re)start of new exe: remove "wpnxm-scp.exe.old"
     *
     */
    SelfUpdater::SelfUpdater()
    {

    }

}
