/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_APPLICATION_H__
#define __GT_APPLICATION_H__

#include "gtobject.h"
#include <QtWidgets/QApplication>

GT_BEGIN_NAMESPACE

class GtDocManager;
class GtMainSettings;
class GtMainWindow;
class GtApplicationPrivate;

class GtApplication : public QApplication, public GtObject
{
    Q_OBJECT;

public:
    GtApplication(int &argc, char **argv);
    ~GtApplication();

public:
    bool isTheOnlyReader() const;
    GtMainWindow *mainWindow();
    QList<GtMainWindow*> mainWindows();
    GtMainSettings *settings();
    QThread *docThread();
    GtDocManager *docManager();

public:
    static GtApplication* instance();
    static QString dataFilePath(const QString &fileName);

protected:
#if defined(Q_WS_MAC)
    bool event(QEvent *event);
#endif

public Q_SLOTS:
    GtMainWindow* newMainWindow();

#if defined(Q_WS_MAC)
    void lastWindowClosed();
    void quitReader();
#endif

private Q_SLOTS:
    void postLaunch();
    void newLocalSocketConnection();

private:
    QScopedPointer<GtApplicationPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtApplication)
    Q_DECLARE_PRIVATE(GtApplication)
};

GT_END_NAMESPACE

#endif  /* __GT_APPLICATION_H__ */
