#include "qgsproxyprogresstask.h"
#include "qgstaskmanager.h"

class ProxyProgressTask : public QgsProxyProgressTask
{
    public:
        ProxyProgressTask( const QString &description, bool canCancel = false )
            : QgsProxyProgressTask( description, canCancel ){};

        bool isCanceled() { return QgsProxyProgressTask::isCanceled(); };
};

class ScopedProgressTask
{
    public:
        ScopedProgressTask( const QString &description, bool canCancel = false );

        ~ScopedProgressTask();

        void setProgress( double progress );

        bool isCanceled() const;

    private:
        ProxyProgressTask *mTask = nullptr;
};