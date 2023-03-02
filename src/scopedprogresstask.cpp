#include "qgsapplication.h"
#include "qgstaskmanager.h"

#include "scopedprogresstask.h"

ScopedProgressTask::ScopedProgressTask( const QString &description, bool canCancel )
{
    mTask = std::make_unique<ProxyProgressTask>( description, canCancel );
    QgsApplication::taskManager()->addTask( mTask.get() );
}

ScopedProgressTask::~ScopedProgressTask() { mTask->finalize( true ); }

void ScopedProgressTask::setProgress( double progress ) { mTask->setProxyProgress( progress ); }

bool ScopedProgressTask::isCanceled() const { return mTask->isCanceled(); }
