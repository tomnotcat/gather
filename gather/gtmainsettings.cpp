/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtmainsettings.h"
#include <QtCore/QSettings>

GT_BEGIN_NAMESPACE

GtMainSettings::GtMainSettings(QObject *parent)
    : QObject(parent)
{
}

GtMainSettings::~GtMainSettings()
{
}

void GtMainSettings::load()
{
    QSettings settings;

    m_geometry = settings.value("geometry").toByteArray();
    m_docSplitter = settings.value("docSplitter").toByteArray();
    m_recentFiles = settings.value("recentFiles").toStringList();
    m_lastOpenPath = settings.value("lastOpenPath").toString();
}

void GtMainSettings::save()
{
    QSettings settings;
    settings.setValue("geometry", m_geometry);
    settings.setValue("docSplitter", m_docSplitter);
    settings.setValue("recentFiles", m_recentFiles);
    settings.setValue("lastOpenPath", m_lastOpenPath);
}

void GtMainSettings::setGeometry(const QByteArray &geometry)
{
    m_geometry = geometry;
}

void GtMainSettings::setDocSplitter(const QByteArray &docSplitter)
{
    m_docSplitter = docSplitter;
}

void GtMainSettings::setRecentFiles(const QStringList &files)
{
    m_recentFiles = files;
}

void GtMainSettings::setLastOpenPath(const QString &path)
{
    m_lastOpenPath = path;
}

GT_END_NAMESPACE
