/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtuserserver.h"
#include <QtArg/Arg>
#include <QtArg/CmdLine>
#include <QtArg/Help>
#include <QtCore/QCoreApplication>

using namespace Gather;

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
                  QLatin1String("Server port (default 8701)"),
                  false, true);
    QtArg argThread(QLatin1Char('t'),
                    QLatin1String("thread"),
                    QLatin1String("Max thread count"),
                    false, true);
    cmd.addArg(argHost);
    cmd.addArg(argPort);
    cmd.addArg(argThread);

    QtArgHelp help(&cmd);
    help.printer()->setProgramDescription(QLatin1String("Gather user server."));
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

    GtUserServer server;
    QHostAddress host(QHostAddress::AnyIPv4);
    quint16 port = argPort.value().isNull() ?
                   8701 : argPort.value().toInt();

    if (!argHost.value().isNull())
        host.setAddress(argHost.value().toString());

    if (!argThread.value().isNull())
        server.setMaxThread(argThread.value().toInt());

    if (!server.listen(host, port)) {
        qWarning() << "listen failed:" << host << port;
        return -1;
    }

    return app.exec();
}
