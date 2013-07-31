/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_APPLICATION_H__
#define __GT_APPLICATION_H__

#include "gtobject.h"
#include <QtCore/QPointer>
#include <QtWidgets/QApplication>

class QLocalServer;

GT_BEGIN_NAMESPACE

class GtBookmarkManager;
class GtDocManager;
class GtUserClient;
class GtMainSettings;
class GtMainWindow;
class GtNoteManager;

class GtApplication : public QApplication, public GtObject
{
    Q_OBJECT;

public:
    GtApplication(int &argc, char **argv);
    ~GtApplication();

public:
    bool isTheOnlyReader() const;
    GtMainWindow* mainWindow();
    QList<GtMainWindow*> mainWindows();
    inline GtMainSettings* settings() { return m_settings; }
    inline QThread* docThread() { return m_docThread; }
    inline GtDocManager* docManager() { return m_docManager; }

public:
    static GtApplication* instance();

protected:
#if defined(Q_WS_MAC)
    bool event(QEvent *event);
#endif

private:
    void clearWindows();

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
    QList<QPointer<GtMainWindow> > m_mainWindows;
    QLocalServer *m_localServer;

    GtMainSettings *m_settings;
    QThread *m_docThread;

    GtNoteManager *m_noteManager;
    GtBookmarkManager *m_bookmarkManager;
    GtDocManager *m_docManager;

    // Network
    GtUserClient *m_userClient;
};

GT_END_NAMESPACE

#endif  /* __GT_APPLICATION_H__ */
