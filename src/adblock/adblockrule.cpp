/*
 * Copyright (c) 2009, Zsombor Gegesy <gzsombor@gmail.com>
 * Copyright (c) 2009, Benjamin C. Meyer <ben@meyerhome.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Benjamin Meyer nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * ============================================================
 *
 * This file is a part of the rekonq project
 *
 * Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy 
 * defined in Section 14 of version 3 of the license.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================ */


#include "adblockrule.h"

#include <QDebug>
#include <QRegExp>
#include <QUrl>


AdBlockRule::AdBlockRule(const QString &filter)
    : m_cssRule(false)
    , m_exceptionRule(false)
    , m_enabledRule(true)
{
    bool isRegExpRule = false;

    if (filter.startsWith(QLatin1String("!")) || filter.trimmed().isEmpty())
        m_enabledRule = false;

    if (filter.contains(QLatin1String("##")))
        m_cssRule = true;

    QString parsedLine = filter;
    if (parsedLine.startsWith(QLatin1String("@@"))) 
    {
        m_exceptionRule = true;
        parsedLine = parsedLine.mid(2);
    }
    
    if (parsedLine.startsWith(QLatin1Char('/'))) 
    {
        if (parsedLine.endsWith(QLatin1Char('/'))) 
        {
            parsedLine = parsedLine.mid(1);
            parsedLine = parsedLine.left(parsedLine.size() - 1);
            isRegExpRule = true;
        }
    }
    
    int options = parsedLine.indexOf(QLatin1String("$"), 0);
    if (options >= 0) 
    {
        m_options = parsedLine.mid(options + 1).split(QLatin1Char(','));
        parsedLine = parsedLine.left(options);
    }

    if(!isRegExpRule)
        parsedLine = convertPatternToRegExp(parsedLine);
    m_regExp = QRegExp(parsedLine, Qt::CaseInsensitive, QRegExp::RegExp2);

    if (m_options.contains(QLatin1String("match-case"))) 
    {
        m_regExp.setCaseSensitivity(Qt::CaseSensitive);
        m_options.removeOne(QLatin1String("match-case"));
    }
}


// here return false means that rule doesn't match,
// so that url is allowed
// return true means "matched rule", so stop url!
bool AdBlockRule::match(const QString &encodedUrl) const
{
    if (m_cssRule)
        return false;

    if (!m_enabledRule)
        return false;

    bool matched = m_regExp.indexIn(encodedUrl) != -1;

    if (matched && !m_options.isEmpty()) 
    {
        // we only support domain right now
        if (m_options.count() == 1)
        {
            foreach (const QString &option, m_options) 
            {
                if (option.startsWith(QLatin1String("domain="))) 
                {
                    QUrl url = QUrl::fromEncoded(encodedUrl.toUtf8());
                    QString host = url.host();
                    QStringList domainOptions = option.mid(7).split(QLatin1Char('|'));
                    foreach (QString domainOption, domainOptions) 
                    {
                        bool negate = domainOption.at(0) == QLatin1Char('~');
                        if (negate)
                            domainOption = domainOption.mid(1);
                        bool hostMatched = domainOption == host;
                        if (hostMatched && !negate)
                            return true;
                        if (!hostMatched && negate)
                            return true;
                    }
                }
            }
        }
        return false;
    }

    return matched;
}


QString AdBlockRule::convertPatternToRegExp(const QString &wildcardPattern)
{
    QString pattern = wildcardPattern;
    return pattern.replace(QRegExp(QLatin1String("\\*+")), QLatin1String("*"))   // remove multiple wildcards
        .replace(QRegExp(QLatin1String("\\^\\|$")), QLatin1String("^"))        // remove anchors following separator placeholder
        .replace(QRegExp(QLatin1String("^(\\*)")), QLatin1String(""))          // remove leading wildcards
        .replace(QRegExp(QLatin1String("(\\*)$")), QLatin1String(""))          // remove trailing wildcards
        .replace(QRegExp(QLatin1String("(\\W)")), QLatin1String("\\\\1"))      // escape special symbols
        .replace(QRegExp(QLatin1String("^\\\\\\|\\\\\\|")),
                 QLatin1String("^[\\w\\-]+:\\/+(?!\\/)(?:[^\\/]+\\.)?"))       // process extended anchor at expression start
        .replace(QRegExp(QLatin1String("\\\\\\^")),
                 QLatin1String("(?:[^\\w\\d\\-.%]|$)"))                        // process separator placeholders
        .replace(QRegExp(QLatin1String("^\\\\\\|")), QLatin1String("^"))       // process anchor at expression start
        .replace(QRegExp(QLatin1String("\\\\\\|$")), QLatin1String("$"))       // process anchor at expression end
        .replace(QRegExp(QLatin1String("\\\\\\*")), QLatin1String(".*"))       // replace wildcards by .*
        ;
}