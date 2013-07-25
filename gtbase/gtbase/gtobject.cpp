/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtobject.h"
#include <QtCore/QDebug>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <typeinfo>

GT_BEGIN_NAMESPACE

#ifdef GT_DEBUG

class GtObjectManager
{
public:
    static GtObjectManager& instance();

public:
    QMutex mutex;
    QList<GtObject*> objects;
};

GtObjectManager& GtObjectManager::instance()
{
    static GtObjectManager objectManager;
    return objectManager;
}

GtObject::GtObject()
{
    GtObjectManager &objectManager(GtObjectManager::instance());
    QMutexLocker locker(&objectManager.mutex);
    objectManager.objects.push_back(this);
}

GtObject::~GtObject()
{
    GtObjectManager &objectManager(GtObjectManager::instance());
    QMutexLocker locker(&objectManager.mutex);
    objectManager.objects.removeOne(this);
}

int GtObject::dumpObjects()
{
    GtObjectManager &objectManager(GtObjectManager::instance());
    QMutexLocker locker(&objectManager.mutex);

    foreach(GtObject *o, objectManager.objects) {
        QObject *qo = dynamic_cast<QObject*>(o);
        if (qo) {
            const char *className = qo->metaObject()->className();
            qWarning() << ">>>> GtObject:" << QString::fromUtf8(className) << o;
        }
        else {
            qWarning() << ">>>> GtObject:" << o;
        }
    }

    return objectManager.objects.size();
}

#endif  /* GT_DEBUG */

GT_END_NAMESPACE
