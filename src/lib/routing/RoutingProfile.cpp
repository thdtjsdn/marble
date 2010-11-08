//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Niko Sams <niko.sams@gmail.com>
// Copyright 2010      Dennis Nienhüser <earthwings@gentoo.org>
//

#include "RoutingProfile.h"

namespace Marble
{

RoutingProfile::RoutingProfile( const QString &name ) :
        m_name( name )
{
    // nothing to do
}

QString RoutingProfile::name() const
{
    return m_name;
}

void RoutingProfile::setName( const QString &name )
{
    m_name = name;
}

const QHash<QString, QHash<QString, QVariant> >& RoutingProfile::pluginSettings() const
{
    return m_pluginSettings;
}

QHash<QString, QHash<QString, QVariant> >& RoutingProfile::pluginSettings()
{
    return m_pluginSettings;
}

}