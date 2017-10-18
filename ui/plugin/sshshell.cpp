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

#include "sshshell.h"

#include <ssh/sshconnection.h>
#include <ssh/sshremoteprocess.h>

#include <QCoreApplication>
#include <QFile>
#include <QSocketNotifier>

#include <cstdlib>
#include <iostream>

using namespace QSsh;
struct c_map
{
    int id;
    QString color;
};

namespace {
inline QString bg_color(const QList<int>& cl)
{
    QString color="white";
    c_map bg[] = {
        {49, "black"},//default
        {40, "black"},
        {41, "red"},
        {42, "green"},
        {43, "yellow"},
        {44, "blue"},
        {45, "magenta"},
        {46, "cyan"},
        {47, "LightGray"},
        {100, "DarkGray"},
        {101, "LightRed"},
        {102, "LightGreen"},
        {103, "LightYellow"},
        {104, "LightBlue"},
        {105, "LightMagenta"},
        {106, "LightCyan"},
        {107, "white"}

    };

    foreach(int c, cl)
    {
        for(size_t i=0; i< sizeof(bg)/sizeof(c_map); ++i)
        {
            c_map&m = bg[i];
            if(m.id == c)
                color = m.color;
        }
    }

    return color;
}

inline QString fore_color(const QList<int>& cl)
{
    QString color = "black";


    c_map fg[]={
        {39, "black"},
        {30, "black"},
        {31, "red"},
        {32, "green"},
        {33, "yellow"},
        {34, "blue"},
        {35, "magenta"},
        {36, "cyan"},
        {37, "LightGray"},
        {90, "DarkGray"},
        {91, "LightRed"},
        {92, "LightGreen"},
        {93, "LightYellow"},
        {94, "LightBlue"},
        {95, "LightMagenta"},
        {96, "LightCyan"},
        {97, "white"}
    };
    foreach(int c, cl)
    {
        for(size_t i=0; i< sizeof(fg)/sizeof(c_map); ++i)
        {
            c_map&m = fg[i];
            if(m.id == c)
                color = m.color;
        }
    }

    return color;

}

inline bool read_color(QString::iterator& ite, QString::iterator end, int& c)
{
    QString s;
    QChar ch;
    bool ret = false;

    while(ite != end)
    {
        ch = *ite;
        if(!ch.isDigit())
            break;

        s.append(ch);
        ite++;
        ret = true;
    }

    c = s.toInt();
    return ret;

}
void read_colors(QString::iterator& ite, QString::iterator end, QList<int>& cl)
{
    bool ret;
    int c;
    while(true)
    {
        if(*ite == "m")
            break;

        ret = read_color(ite, end, c);
        if(!ret)
            break;
        cl.append(c);
        ite++;
    }

}

template<bool>
inline void refine(QString& data)
{
}

template<>
inline void refine<true>(QString& data) {
    QString::iterator ite = data.begin();
    QString::iterator end = data.end();

    QString result;
    int level = 0;

    // Ignore Beep
    if(data.at(0) == 0x7)
        return;

    while(ite != end)
    {
        if(*ite == "\033")
        {
            ite++;
            if(*ite == "[")
            {
                QList<int> cl;
                QString s;
                QChar ch;
                ite++;

                while(*ite != "m")
                {

                    while(true)
                    {
                        ch = *ite;
                        if(!ch.isDigit())
                            break;
                        s.append(ch);
                        ite++;
                    }

                    if(!s.isEmpty())
                    {
                        cl<<s.toInt();
                        qDebug()<<s;
                        s.clear();
                    }

                    if(*ite == ";")
                    {
                        ite ++;
                    }
                    else if(*ite == "m")
                    {
                        ite ++;
                        break;
                    }
                    else
                    {
                        break;
                    }
                }


                QString style="color:%1;background-color:%2;font-weight:%3;text-decoration:%4;";
                QString fc = fore_color(cl);
                QString bc = bg_color(cl);
                QString fw="normal";
                QString td = "normal";
                style = style.arg(fc).arg(bc).arg(fw).arg(td);

                QString div = QString("<span style=\"%1\">").arg(style);
                if(cl.contains(0) )
                {

                    if(level > 0)
                    {
                        level --;
                        div = "</span>";
                    }
                    else
                    {
                        level ++;
                        div = "<span>";
                    }
                }
                else
                {
                    level ++;
                }

                result += div;
                qDebug()<<cl;

            }
        }
        else if(*ite == "\r")
        {
            ite ++;
        }
        else if(*ite =="\n")
        {
            result += "<br>";
            ite ++;
        }
        else
        {
            result += *ite;
            ite++;
        }


    } //while-end


    for(int i=0; i<level; ++i)
    {
        result.append("</span>");
    }

    result = "<span>" + result + "</span>";

    qDebug()<<data<<"\n"<<result;

    data = result;
}

template <>
inline void refine<false>(QString& data) {
    QString ba[][2] ={
        {"[[][^m]+m",""},
        {"\033[0m", "<font color=\"black\" >"},
        {"\033[01;36m", "<font color=\"cyan\">"},
        {"\033[01;33m", "<font color=\"yellow\">"},
        {"\033[01;34m", "<font color=\"blue\">"},
        {"\033[01;31m", "<font color=\"light_red\">"},
        {NULL, NULL}

    };
//    int i=0;
    QRegExp rx(ba[0][0]);
    qDebug()<<data;
    data.replace(rx, ba[0][1]);
    data.replace("\033","");

//    while(1)
//    {
//        if(ba[i][0] == NULL)
//            break;
//        data.replace(ba[i][0], ba[i][1]);
//        ++i;
//    }
}
}

SshShell::SshShell(const SshConnectionParameters &parameters, QObject *parent)
    : QObject(parent),
      m_connection(new SshConnection(parameters)),
      //m_stdin(new QFile(this)),
      m_status(SH_DISCONNECT)
{
    connect(m_connection, SIGNAL(connected()), SLOT(handleConnected()));
    connect(m_connection, SIGNAL(dataAvailable(QString)), SLOT(handleShellMessage(QString)));
    connect(m_connection, SIGNAL(error(QSsh::SshError)), SLOT(handleConnectionError()));
}

SshShell::~SshShell()
{
    delete m_connection;
}

void SshShell::run()
{
//    if (!m_stdin->open(stdin, QIODevice::ReadOnly | QIODevice::Unbuffered)) {
//        std::cerr << "Error: Cannot read from standard input." << std::endl;
//        qApp->exit(EXIT_FAILURE);
//        return;
//    }

    m_connection->connectToHost();
}

void SshShell::handleConnectionError()
{
    QString data = QString("SSH connection error: %1\n").arg( qPrintable(m_connection->errorString()) );
    std::cerr << qPrintable(data);
    emit sshStandardError(data);
    //qApp->exit(EXIT_FAILURE);
}

void SshShell::handleShellMessage(const QString &message)
{
    std::cout << qPrintable(message);
}

void SshShell::handleConnected()
{
    m_shell = m_connection->createRemoteShell();
    connect(m_shell.data(), SIGNAL(started()), SLOT(handleShellStarted()));
    connect(m_shell.data(), SIGNAL(readyReadStandardOutput()), SLOT(handleRemoteStdout()));
    connect(m_shell.data(), SIGNAL(readyReadStandardError()), SLOT(handleRemoteStderr()));
    connect(m_shell.data(), SIGNAL(closed(int)), SLOT(handleChannelClosed(int)));
    m_shell->start();
}

void SshShell::handleShellStarted()
{
//    QSocketNotifier * const notifier = new QSocketNotifier(0, QSocketNotifier::Read, this);
//    connect(notifier, SIGNAL(activated(int)), SLOT(handleStdin()));
}

void SshShell::handleRemoteStdout()
{
    QString data = QString::fromUtf8( m_shell->readAllStandardOutput().data() );
    //std::cout << data.toUtf8().data() << std::flush;
    refine<true>(data);
    emit sshStandardOutput(data);
}

void SshShell::handleRemoteStderr()
{
    QString data = m_shell->readAllStandardError().data();
    //std::cerr << data.toUtf8().data() << std::flush;
    refine<true>(data);
    emit sshStandardError(data);

}

void SshShell::handleChannelClosed(int exitStatus)
{
    std::cerr << "Shell closed. Exit status was " << exitStatus << ", exit code was "
        << m_shell->exitCode() << "." << std::endl;
//    qApp->exit(exitStatus == SshRemoteProcess::NormalExit && m_shell->exitCode() == 0
//        ? EXIT_SUCCESS : EXIT_FAILURE);
    disconnect(m_shell.data(), SIGNAL(started()));
    disconnect(m_shell.data(), SIGNAL(readyReadStandardOutput()));
    disconnect(m_shell.data(), SIGNAL(readyReadStandardError()));
    disconnect(m_shell.data(), SIGNAL(closed(int)));

    m_status = SH_DISCONNECT;
}

void SshShell::handleStdin()
{
//    QByteArray cmd = m_stdin->readLine();
//    qDebug()<<"stdin="<<cmd;
//    m_shell->write(cmd);
}

void SshShell::writeToSsh(const QString& data)
{
    m_shell->write(data.toUtf8());

}
