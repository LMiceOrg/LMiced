/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#ifndef SSHSHELL_H
#define SSHSHELL_H

#include <QObject>
#include <QSharedPointer>

namespace QSsh {
class SshConnection;
class SshConnectionParameters;
class SshRemoteProcess;
}

QT_BEGIN_NAMESPACE
class QByteArray;
class QFile;
class QString;
QT_END_NAMESPACE

class SshShell : public QObject
{
    Q_OBJECT
public:
    enum {
        SH_DISCONNECT=0,
        SH_CONNECTED = 1
    };
    SshShell(const QSsh::SshConnectionParameters &parameters, QObject *parent = 0);
    ~SshShell();

    void run();
signals:
    void sshStandardOutput(const QString&);
    void sshStandardError(const QString&);
public slots:
    void writeToSsh(const QString&data);
private slots:
    void handleConnected();
    void handleConnectionError();
    void handleRemoteStdout();
    void handleRemoteStderr();
    void handleShellMessage(const QString &message);
    void handleChannelClosed(int exitStatus);
    void handleShellStarted();
    void handleStdin();

private:
    QSsh::SshConnection *m_connection;
    QSharedPointer<QSsh::SshRemoteProcess> m_shell;
    //QFile * const m_stdin;
    int m_status;
};

#endif // SSHSHELL_H
