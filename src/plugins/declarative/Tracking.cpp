//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Dennis Nienhüser <earthwings@gentoo.org>
//

#include "Tracking.h"

#include "MarbleWidget.h"
#include "MarbleModel.h"
#include "PositionTracking.h"
#include "RenderPlugin.h"

namespace Marble
{
namespace Declarative
{

Tracking::Tracking( QObject* parent) : QObject( parent ),
    m_showPosition( true ),
    m_showTrack( true ),
    m_positionSource( 0 ),
    m_positionMarker( 0 ),
    m_marbleWidget( 0 )
{
    // nothing to do
}

bool Tracking::showPosition() const
{
    return m_showPosition;
}

void Tracking::setShowPosition( bool show )
{
    if ( show != m_showPosition ) {
        if ( m_marbleWidget ) {
            setShowPositionMarkerPlugin( show );
        }

        m_showPosition = show;
        emit showPositionChanged();
    }
}

bool Tracking::showTrack() const
{
    return m_showTrack;
}

void Tracking::setShowTrack( bool show )
{
    if ( show != m_showTrack ) {
        if ( m_marbleWidget ) {
            m_marbleWidget->model()->positionTracking()->setTrackVisible( show );
            m_marbleWidget->update();
        }

        m_showTrack = show;
        emit showTrackChanged();
    }
}

PositionSource* Tracking::positionSource()
{
    return m_positionSource;
}

void Tracking::setPositionSource( PositionSource* source )
{
    if ( source != m_positionSource ) {
        m_positionSource = source;
        if ( source ) {
            source->setMarbleWidget( m_marbleWidget );
            connect( source, SIGNAL( positionChanged() ),
                    this, SLOT( updatePositionMarker() ) );
            connect( source, SIGNAL( hasPositionChanged() ),
                    this, SLOT( updatePositionMarker() ) );
            connect( m_marbleWidget, SIGNAL( visibleLatLonAltBoxChanged( GeoDataLatLonAltBox ) ),
                    this, SLOT( updatePositionMarker() ) );
        }
        emit positionSourceChanged();
    }
}

void Tracking::setMarbleWidget( MarbleWidget* widget )
{
    if ( widget != m_marbleWidget ) {
        if ( widget ) {
            widget->model()->positionTracking()->setTrackVisible( showTrack() );
            setShowPositionMarkerPlugin( showPosition() );
        }

        if ( m_positionSource ) {
            m_positionSource->setMarbleWidget( widget );
        }

        m_marbleWidget = widget;
    }
}

void Tracking::setPositionMarker( QObject* marker )
{
    if ( marker != m_positionMarker ) {
        m_positionMarker = marker;
        emit positionMarkerChanged();
    }
}

QObject* Tracking::positionMarker()
{
    return m_positionMarker;
}

void Tracking::updatePositionMarker()
{
    if ( m_marbleWidget && m_positionMarker && m_positionSource && m_positionSource->hasPosition() ) {
        Coordinate* position = m_positionSource->position();
        qreal x(0), y(0);
        bool const visible = m_marbleWidget->screenCoordinates( position->longitude(), position->latitude(), x, y );
        QDeclarativeItem* item = qobject_cast<QDeclarativeItem*>( m_positionMarker );
        if ( item ) {
            item->setVisible( visible );
            if ( visible ) {
                item->setPos( x - item->width() / 2.0, y - item->height() / 2.0 );
            }
        }
    } else if ( m_positionMarker && ( !m_positionSource || ( m_positionSource && !m_positionSource->hasPosition() ) ) ) {
        QDeclarativeItem* item = qobject_cast<QDeclarativeItem*>( m_positionMarker );
        if ( item ) {
            item->setVisible( false );
        }
    }
}

void Tracking::setShowPositionMarkerPlugin( bool visible )
{
    if ( m_marbleWidget ) {
        QList<RenderPlugin *> const renderPlugins = m_marbleWidget->renderPlugins();
        foreach( RenderPlugin* renderPlugin, renderPlugins ) {
            if ( renderPlugin->nameId() == "positionMarker" ) {
                renderPlugin->setEnabled( visible );
                renderPlugin->setVisible( visible );
            }
        }
    }
}

}

}

#include "Tracking.moc"