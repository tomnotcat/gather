/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_MAIN_SETTINGS_H__
#define __GT_MAIN_SETTINGS_H__

#include "gtobject.h"
#include <QtCore/QObject>
#include <QtCore/QStringList>

GT_BEGIN_NAMESPACE

class GtMainSettings : public QObject, public GtObject
{
    Q_OBJECT

public:
    explicit GtMainSettings(QObject *parent = 0);
    ~GtMainSettings();

public:
    void load();
    void save();

    inline QByteArray geometry() const { return m_geometry; }
    void setGeometry(const QByteArray &geometry);

    inline QStringList recentFiles() const { return m_recentFiles; }
    void setRecentFiles(const QStringList &files);

    inline QString lastOpenPath() const { return m_lastOpenPath; }
    void setLastOpenPath(const QString &path);

private:
    Q_DISABLE_COPY(GtMainSettings)

private:
    QByteArray m_geometry;
    QStringList m_recentFiles;
    QString m_lastOpenPath;
};

GT_END_NAMESPACE

#endif  /* __GT_MAIN_SETTINGS_H__ */
