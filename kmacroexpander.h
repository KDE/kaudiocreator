/*
    This file is part of the KDE libraries

    Copyright (c) 2002 Oswald Buddenhagen <ossi@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef _KMACROEXPANDER_H
#define _KMACROEXPANDER_H

#include <qstring.h>
#include <qmap.h>

// KMacroExpander is scheduled for inclusion in kdelibs
// We put it in a namespace here to prevent a name-collision when
// that happens.

namespace KAudioCreator {

struct KMacroExpanderPrivate;

class KMacroExpander {

public:
    KMacroExpander( QChar c = '%' );
    virtual ~KMacroExpander();
    void expandMacros( QString &str );
    /*
     * Explicitly supported constructs:
     *   \ '' "" $'' $"" {} () $(()) ${} $() ``
     * Implicitly supported constructs:
     *   (())
     * Unsupported constructs that will cause problems:
     *   Shortened "case $v in pat)" syntax. Use "case $v in (pat)" instead.
     * The rest of the shell (incl. bash) syntax is simply ignored,
     * as it is not expected to cause problems.
     */
    bool expandMacrosShellQuote( QString &str );
    bool expandMacrosShellQuote( QString &str, uint &pos );
    void setEscapeChar( QChar c );
    QChar escapeChar() const;

protected:
    virtual bool expandPlainMacro( const QString &str, uint pos, uint &len, QString &ret ) = 0;
    virtual bool expandEscapedMacro( const QString &str, uint pos, uint &len, QString &ret ) = 0;

private:
    enum Quoting { noquote, singlequote, doublequote, dollarquote, 
		parent, subst, group, math };
    typedef struct {
	Quoting current;
	bool dquote;
    } State;
    typedef struct {
	QString str;
	uint pos;
    } Save;
    KMacroExpanderPrivate *d;
};

struct KMacroMapExpanderBasePrivate;

class KMacroMapExpanderBase : public KMacroExpander {

public:
    KMacroMapExpanderBase( const QMap<QString,QString> &map, QChar c = '%' );
    virtual ~KMacroMapExpanderBase();
    void setMacroMap( const QMap<QString,QString> &map );
    QMap<QString,QString> &macroMap() const;

private:
    KMacroMapExpanderBasePrivate *d;
};

class KSelfDelimitingMacroMapExpander : public KMacroMapExpanderBase {

public:
    KSelfDelimitingMacroMapExpander( const QMap<QString,QString> &map, QChar c = '%' );
    void expandMacros( QString &str ) { KMacroExpander::expandMacros( str ); }
    bool expandMacrosShellQuote( QString &str ) { return KMacroExpander::expandMacrosShellQuote( str ); }
    bool expandMacrosShellQuote( QString &str, uint &pos ) { return KMacroExpander::expandMacrosShellQuote( str, pos ); }
    static void expandMacros( QString &str, const QMap<QString,QString> &map, QChar c = '%' );
    static bool expandMacrosShellQuote( QString &str, const QMap<QString,QString> &map, QChar c = '%' );

protected:
    virtual bool expandPlainMacro( const QString &str, uint pos, uint &len, QString &ret );
    virtual bool expandEscapedMacro( const QString &str, uint pos, uint &len, QString &ret );
};

class KHandDelimitedMacroMapExpander : public KMacroMapExpanderBase {

public:
    KHandDelimitedMacroMapExpander( const QMap<QString,QString> &map, QChar c = '%' );
    void expandMacros( QString &str ) { KMacroExpander::expandMacros( str ); }
    bool expandMacrosShellQuote( QString &str ) { return KMacroExpander::expandMacrosShellQuote( str ); }
    bool expandMacrosShellQuote( QString &str, uint &pos ) { return KMacroExpander::expandMacrosShellQuote( str, pos ); }
    static void expandMacros( QString &str, const QMap<QString,QString> &map, QChar c = '%' );
    static bool expandMacrosShellQuote( QString &str, const QMap<QString,QString> &map, QChar c = '%' );

protected:
    virtual bool expandPlainMacro( const QString &str, uint pos, uint &len, QString &ret );
    virtual bool expandEscapedMacro( const QString &str, uint pos, uint &len, QString &ret );

private:
    bool isIdentifier(uint c) { return c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'); }
};

};

#endif /* _KMACROEXPANDER_H */
