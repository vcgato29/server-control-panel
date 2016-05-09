#include "jobscheduler.h"

JobScheduler::JobScheduler()
{

}

/**
 * Get list of active jobs (JobStatus::Active).
 */
QList<QString> JobScheduler::getActiveJobs()
{

}

/**
 * Get list of historic jobs (JobStatus::Done).
 */
QList<QString> JobScheduler::getHistoricJobs()
{

}

/**
 * Get details of a specific job.
 */
QList<QString> JobScheduler::getDetailsForJobId(int jobId)
{
    Q_UNUSED(jobId);
}
