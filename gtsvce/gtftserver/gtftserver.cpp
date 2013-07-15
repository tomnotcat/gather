/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtftserver.h"
#include <QtArg/Arg>
#include <QtArg/CmdLine>
#include <QtArg/Help>
#include <QtCore/QCoreApplication>

using namespace Gather;

class FastDFS : public GtFTServer
{
public:
    void upload(const QString &fileId, QIODevice *device)
    {
    }

    QIODevice* download(const QString &fileId)
    {
        return 0;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QtArgCmdLine cmd(app.arguments());

    QtArg argHost(QLatin1Char('i'),
                  QLatin1String("host"),
                  QLatin1String("Server host"),
                  false, true);
    QtArg argPort(QLatin1Char('p'),
                  QLatin1String("port"),
                  QLatin1String("Server port"),
                  false, true);
    QtArg argTemp(QLatin1Char('m'),
                  QLatin1String("temp"),
                  QLatin1String("Temp path"),
                  false, true);
    QtArg argThread(QLatin1Char('t'),
                    QLatin1String("thread"),
                    QLatin1String("Max thread count"),
                    false, true);
    cmd.addArg(argHost);
    cmd.addArg(argPort);
    cmd.addArg(argTemp);
    cmd.addArg(argThread);

    QtArgHelp help(&cmd);
    help.printer()->setProgramDescription(QLatin1String("Gather file transfer server."));
    help.printer()->setExecutableName(QLatin1String(argv[0]));

    cmd.addArg(help);

    try {
        cmd.parse();
    }
    catch (const QtArgHelpHasPrintedEx &x)\
    {
        return 0;
    }
    catch (const QtArgBaseException &x)
    {
        qDebug() << x.what();
        return -1;
    }

    FastDFS server;
    QHostAddress host(QHostAddress::AnyIPv4);
    quint16 port = argPort.value().isNull() ?
                   9001 : argPort.value().toInt();

    if (!argHost.value().isNull())
        host.setAddress(argHost.value().toString());

    if (!argTemp.value().isNull())
        server.setTempPath(argTemp.value().toString());

    if (!argThread.value().isNull())
        server.setMaxThread(argThread.value().toInt());

    if (!server.listen(host, port)) {
        qWarning() << "listen failed:" << host << port;
        return -1;
    }

    return app.exec();
}
