//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010-2012 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
// Copyright 2011      Jens-Michael Hoffmann <jmho@c-xx.com>
//

#ifndef MARBLE_ATMOSPHEREPLUGIN_H
#define MARBLE_ATMOSPHEREPLUGIN_H

#include "RenderPlugin.h"

namespace Marble
{

class AtmospherePlugin : public RenderPlugin
{
    Q_OBJECT
    Q_INTERFACES( Marble::RenderPluginInterface )
    MARBLE_PLUGIN( AtmospherePlugin )

public:
    AtmospherePlugin();

    explicit AtmospherePlugin( const MarbleModel *marbleModel );

    QStringList backendTypes() const;

    QString renderPolicy() const;

    QStringList renderPosition() const;

    QString name() const;

    QString guiString() const;

    QString nameId() const;

    QString version() const;

    QString description() const;

    QIcon icon() const;

    QString copyrightYears() const;

    QList<PluginAuthor> pluginAuthors() const;

    qreal zValue() const;

    void initialize();

    bool isInitialized() const;

    bool render( GeoPainter *painter, ViewportParams *viewport, const QString& renderPos, GeoSceneLayer * layer = 0 );
};

}

#endif // MARBLE_ATMOSPHEREPLUGIN_H