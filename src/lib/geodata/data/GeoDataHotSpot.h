//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Torsten Rahn <rahn@kde.org>
//


#ifndef GEODATAHOTSPOT_H
#define GEODATAHOTSPOT_H

#include <QtCore/QPoint>


class GeoDataHotSpot
{
  public:
    enum Units{Fraction, Pixels, InsetPixels};

    GeoDataHotSpot( const QPointF& hotSpot = QPointF( 0.5, 0.5 ), Units xunits = Fraction, Units yunits = Fraction );

    const QPointF& hotSpot( Units &xunits, Units &yunits ) const;

    void setHotSpot( const QPointF& hotSpot = QPointF( 0.5, 0.5 ), Units xunits = Fraction, Units yunits = Fraction );

  private:
    QPointF m_hotSpot;
    Units m_xunits;
    Units m_yunits;
};

#endif // GEODATAHOTSPOT_H
